SUPPORT_GREENLETS = True

import os
import socket
import json
import errno

if SUPPORT_GREENLETS:
    import gevent

from threading import Thread

import pyclient

import logging
log = logging.getLogger(__name__)

DEFAULT_CONFIG_FILE = 'config.prototxt'

DEFAULT_SERVE_IP = '127.0.0.1'
DEFAULT_SERVE_PORT = 5005
BUFFER_SIZE = 1024

TCP_TERMINATOR = '$$$'

# Legacy wrapper for old VISOR backend API documented on the following page:
# https://bitbucket.org/kencoken/visor/wiki/backend-api

class VisorLegacyWrap(object):

    def __init__(self, protoconfig_path,
                 serve_ip=DEFAULT_SERVE_IP,
                 serve_port=DEFAULT_SERVE_PORT,
                 multiple_connections=False,
                 use_greenlets=False):

        self.client = pyclient.VisorClientLegacyExt(protoconfig_path)
        self.next_num_query_id = 1
        self.query_id_dict = {}

        self.serve_ip = serve_ip
        self.serve_port = serve_port

        self.multiple_connections = multiple_connections
        self.use_greenlets = use_greenlets

        if self.use_greenlets and not SUPPORT_GREENLETS:
            raise RuntimeError('Must set SUPPORT_GREENLETS to True')

    def serve(self):

        if self.use_greenlets:
            s = gevent.socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        else:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.bind((self.serve_ip, self.serve_port))
        s.listen(1)

        while True:

            conn, addr = s.accept()

            if self.use_greenlets:
                ses_handler = gevent.Greenlet(self.process_session, conn)
                ses_handler.start()
            else:
                ses_handler = Thread(target=self.process_session, args=(conn,))
                ses_handler.daemon = self.multiple_connections
                ses_handler.start()

            if not self.multiple_connections:
                ses_handler.join()
                break


    def process_session(self, conn):

        leftovers = ''

        while True:
            log.info('Starting receive cycle...')

            term_idx = -1
            req_data = leftovers

            log.debug('Starting with req_data: "%s"', req_data)

            conn_closed = False
            try:
                while term_idx < 0:
                    log.debug('Waiting for data...')
                    req_chunk = conn.recv(BUFFER_SIZE)
                    if not req_chunk:
                        log.info('Connection closed! Ending session...')
                        conn_closed = True
                        break

                    log.debug('Received chunk: "%s"', req_chunk)
                    req_data = req_data + req_chunk
                    term_idx = req_data.find(TCP_TERMINATOR)
            except socket.error as e:
                if e.errno != errno.ECONNRESET: raise
                log.info('Connection reset! Ending session...')
                conn_closed = True

            if conn_closed: break

            excess_sz = term_idx + len(TCP_TERMINATOR)
            leftovers = req_data[excess_sz:]

            req_data = req_data[0:term_idx]

            log.debug('Received data: "%s", leftovers: "%s"', req_data, leftovers)

            req_obj = json.loads(req_data)

            log.debug('Dispatching request...')
            rep_obj = self.dispatch(req_obj)
            rep_data = json.dumps(rep_obj)
            log.debug('Dispatch returned response: %s', rep_data)

            rep_data = rep_data + TCP_TERMINATOR
            total_sent = 0
            while total_sent < len(rep_data):
                log.info('Sending response chunk...')
                sent = conn.send(rep_data[total_sent:])
                if sent == 0:
                    raise RuntimeError('Socket connection broken')
                total_sent = total_sent + sent

            log.info('Response sent!')

        conn.close()

    @pyclient.decorators.api_err_handler
    def dispatch(self, req_dict):

        if req_dict['func'] == 'selfTest':
            return self.self_test()
        elif req_dict['func'] == 'getQueryId':
            return self.get_query_id()
        elif req_dict['func'] == 'releaseQueryId':
            return self.release_query_id(req_dict['query_id'])
        elif req_dict['func'] == 'addPosTrs':
            return self.add_pos_trs(req_dict['query_id'], req_dict['impath'],
                                    blocking=True)
        elif req_dict['func'] == 'addPosTrsAsync':
            return self. add_pos_trs(req_dict['query_id'], req_dict['impath'],
                                     blocking=False)
        elif req_dict['func'] == 'addNegTrs':
            return self.add_neg_trs()
        elif req_dict['func'] == 'saveAnnotations':
            return self.save_annotations(req_dict['query_id'], req_dict['filepath'])
        elif req_dict['func'] == 'getAnnotations':
            return self.get_annotations(req_dict['filepath'])
        elif req_dict['func'] == 'loadAnnotationsAndTrs':
            return self.load_annotations_and_trs()
        elif req_dict['func'] == 'saveClassifier':
            return self.save_classifier(req_dict['query_id'], req_dict['filepath'])
        elif req_dict['func'] == 'loadClassifier':
            return self.load_classifier(req_dict['query_id'], req_dict['filepath'])
        elif req_dict['func'] == 'train':
            return self.train(req_dict['query_id'])
        elif req_dict['func'] == 'rank':
            return self.rank(req_dict['query_id'])
        elif req_dict['func'] == 'getRanking' or req_dict['func'] == 'getRankingSubset':
            rep_dict = self.get_ranking(req_dict['query_id'])
            if rep_dict['success']:
                rep_dict['total_len'] = len(rep_dict['ranklist'])
            return rep_dict
        else:
            raise RuntimeError('Unknown command: %s' % req_dict['func'])


    def self_test(self):
        return {'success': True}

    @pyclient.decorators.api_err_handler
    def get_query_id(self):

        query_id = self.client.start_query()

        num_query_id = self.next_num_query_id
        self.next_num_query_id = self.next_num_query_id + 1
        self.query_id_dict[num_query_id] = query_id

        return {'success': True,
                'query_id': num_query_id}

    @pyclient.decorators.api_err_handler
    def release_query_id(self, num_query_id):
        query_id = self._get_query_id(num_query_id)

        self.client.free_query(query_id)

        return {'success': True}

    @pyclient.decorators.api_err_handler
    def add_pos_trs(self, num_query_id, path, blocking=False):
        query_id = self._get_query_id(num_query_id)

        self.client.add_trs_from_file(query_id, path, blocking)

        return {'success': True}

    @pyclient.decorators.api_err_handler
    def add_neg_trs(self):
        print 'Not supported!'
        return {'success': True}

    @pyclient.decorators.api_err_handler
    def save_annotations(self, num_query_id, path):
        query_id = self._get_query_id(num_query_id)

        self.client.save_annotations(query_id, path)

        return {'success': True}

    @pyclient.decorators.api_err_handler
    def get_annotations(self, path):
        annos_proto = self.client.get_annotations(path)

        anno_list = []
        for anno_proto in annos_proto:
            anno_list.append({'image': anno_proto.path,
                              'anno': anno_proto.anno})

        return {'success': True,
                'annos': anno_list}

    @pyclient.decorators.api_err_handler
    def load_annotations_and_trs(self):
        print 'Not supported!'
        return {'success': False}

    @pyclient.decorators.api_err_handler
    def save_classifier(self, num_query_id, path):
        query_id = self._get_query_id(num_query_id)

        self.client.save_classifier(query_id, path)

        return {'success': True}

    @pyclient.decorators.api_err_handler
    def load_classifier(self, num_query_id, path):
        query_id = self._get_query_id(num_query_id)

        self.client.load_classifier(query_id, path)

        return {'success': True}

    @pyclient.decorators.api_err_handler
    def train(self, num_query_id):
        query_id = self._get_query_id(num_query_id)

        self.client.train(query_id)
        return {'success': True}

    @pyclient.decorators.api_err_handler
    def rank(self, num_query_id):
        query_id = self._get_query_id(num_query_id)

        self.client.rank(query_id)
        return {'success': True}

    @pyclient.decorators.api_err_handler
    def get_ranking(self, num_query_id):
        query_id = self._get_query_id(num_query_id)

        ranking = self.client.get_ranking(query_id, 1)

        ranking_obj = []

        for ritem in ranking.rlist:
            dset_dir = self.client.config.preproc_config.dataset_im_base_path
            dset_name = os.path.split(dset_dir)[1]
            if not dset_name:
                dset_name = os.path.split(dset_dir[0:-1])[1]

            image = os.path.join(dset_dir, ritem.path)
            score = ritem.score
            uri = os.path.splitext(ritem.path)[0]#os.path.join(dset_name, os.path.splitext(ritem.path)[0])
            path = ritem.path

            ranking_obj.append({'image': image,
                                'score': score,
                                'uri': uri,
                                'path': path})

        return {'success': True,
                'ranklist': ranking_obj}


    def _get_query_id(self, num_query_id):
        return self.query_id_dict[num_query_id]

if __name__ == "__main__":

    legacy_client = VisorLegacyWrap(DEFAULT_CONFIG_FILE)
    legacy_client.serve()
