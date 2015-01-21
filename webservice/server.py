import os
import sys
import logging
import json

from flask import Flask
from flask.ext.socketio import SocketIO
from protobuf_to_dict import protobuf_to_dict
log = logging.getLogger(__name__)
logging.basicConfig()

file_dir = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(file_dir, '..'))
import pyclient

from google import protobuf
from proto import cpuvisor_config_pb2 as protoconfig
from proto import cpuvisor_srv_pb2 as protosrv

# setup flask app

app = Flask(__name__, static_url_path='/static')
app.config.from_object(__name__)

app.config.update(dict(
    DEBUG=True,
    SECRET_KEY='secret',
    SERVER_CONFIG=os.path.normpath(os.path.join(file_dir, '../config.prototxt'))
))

# pass through socketio middleware

socketio = SocketIO(app)

server_config = protoconfig.Config()
with open(app.config['SERVER_CONFIG'], 'rb') as f:
    protobuf.text_format.Merge(f.read(), server_config)

# link dsetimages and downloaded into static dir
cwd_store = os.getcwd()
os.chdir('static/app/')

if os.path.islink('dsetimages'):
    os.unlink('dsetimages')
if not os.path.exists('dsetimages'):
    print 'Linking dataset base dir: %s' % server_config.preproc_config.dataset_im_base_path
    os.symlink(server_config.preproc_config.dataset_im_base_path, 'dsetimages')

if os.path.islink('downloaded'):
    os.unlink('downloaded')
if not os.path.exists('downloaded'):
    print 'Linking downloaded base dir: %s' % server_config.server_config.image_cache_path
    os.symlink(server_config.server_config.image_cache_path, 'downloaded')

os.chdir(cwd_store)

# setup client

def recv_notification(notification):

    print 'From location: /api/query/%s/notifications' % notification.id
    print '++++++++++++++++++'
    print notification
    print '++++++++++++++++++'

    if notification.type == protosrv.NTFY_IMAGE_PROCESSED:
        notification.data = os.path.join('downloaded', os.path.relpath(notification.data, server_config.server_config.image_cache_path))

    emit_notification(notification.type, notification.data, notification.id)


def emit_notification(type, data, id):

    # TODO: switch to qid specific notifications: '/api/query/%s/notifications' % notification.id
    socketio.emit('notification', {'type': protosrv.NotificationType.Name(type),
                                   'data': data,
                                   'id': id},
                  namespace='/api/query/notifications')


client = pyclient.VisorClientLegacyExt(app.config['SERVER_CONFIG'])
notifier = pyclient.VisorNotifier(app.config['SERVER_CONFIG'], recv_notification)

# funcs

##

@app.route('/api/')
def api_base():
    return "CPU-VISOR Demo API"

@app.route('/api/query/start_query', methods=['POST'])
@pyclient.decorators.api_err_handler(True)
def start_query():
    query_id = client.start_query()
    return json.dumps({'success': True, 'query_id': query_id})

@app.route('/api/query/<string:query_id>/add_trs/<string:query_string>', methods=['PUT'])
@pyclient.decorators.api_err_handler(True)
def add_trs(query_id, query_string):
    client.download_trs(query_id, query_string)
    return json.dumps({'success': True})

@app.route('/api/query/<string:query_id>/train', methods=['PUT'])
@pyclient.decorators.api_err_handler(True)
def train(query_id):
    client.train(query_id, blocking=False)
    return json.dumps({'success': True})

@app.route('/api/query/<string:query_id>/rank', methods=['PUT'])
@pyclient.decorators.api_err_handler(True)
def rank(query_id):
    client.rank(query_id, blocking=False)
    return json.dumps({'success': True})

@app.route('/api/query/<string:query_id>/ranking/<int:page>', methods=['GET'])
@pyclient.decorators.api_err_handler(True)
def ranking(query_id, page):
    ranking_result = client.get_ranking(query_id, page=page)
    return json.dumps({'success': True, 'ranking': protobuf_to_dict(ranking_result)})

@app.route('/api/query/<string:query_id>/free', methods=['PUT'])
@pyclient.decorators.api_err_handler(True)
def free_query(query_id):
    client.free_query(query_id)
    return json.dumps({'success': True})

@socketio.on('connect', namespace='/api/query/notifications')
def test_connect():
    print('Client connected')

@socketio.on('disconnect', namespace='/api/query/notifications')
def test_disconnect():
    print('Client disconnected')

##

@app.route('/')
def static_proxy_index():
    return app.send_static_file(os.path.join('app', 'index.html'))

@app.route('/<path:path>')
def static_proxy(path):
    # send_static_file will guess the correct MIME type

    return app.send_static_file(os.path.join('app', path))

if __name__ == "__main__":
    #app.run(processes=10, host='0.0.0.0', port=8915)
    socketio.run(app, host='0.0.0.0', port=8915)
