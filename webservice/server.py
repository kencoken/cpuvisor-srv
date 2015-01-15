from flask import Flask, g, request, abort
from flask.ext.socketio import SocketIO, emit

import os

# setup flask app

app = Flask(__name__)
app = Flask(__name__, static_url_path='/static')
app.config.from_object(__name__)

app.config.update(dict(
    DEBUG=True
))

# pass through socketio middleware

socketio = SocketIO(app)

# funcs

##

@app.route('/api/')
def api_base():
    return "CPU-VISOR Demo API"

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
