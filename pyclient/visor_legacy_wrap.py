import os
import socket
import json

import pyclient

DEFAULT_CONFIG_FILE = '/Data/src/cpuvisor-srv/config.prototxt'

DEFAULT_SERVE_IP = '127.0.0.1'
DEFAULT_SERVE_PORT = 5005
BUFFER_SIZE = 1024

TCP_TERMINATOR = '$$$'

# Legacy wrapper for old VISOR backend API documented on the following page:
# https://bitbucket.org/kencoken/visor/wiki/backend-api

class VisorLegacyWrap(object):

    def __init__(self, protoconfig_path,
                 serve_ip=DEFAULT_SERVE_IP,
                 serve_port=DEFAULT_SERVE_PORT):

        self.client = pyclient.VisorClientLegacyExt(protoconfig_path)
        self.next_num_query_id = 1
        self.query_id_dict = {}

        self.serve_ip = serve_ip
        self.serve_port = serve_port

    def serve(self):

        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.bind((self.serve_ip, self.serve_port))
        s.listen(1)

        conn, addr = s.accept()
        print 'Connection address:', addr

        while True:

            req_data = conn.recv(BUFFER_SIZE)
            if not req_data: break
            term_idx = req_data.find(TCP_TERMINATOR)
            if term_idx < 0: break
            req_data = req_data[0:term_idx]
            print "received data: " + req_data

            req_obj = json.loads(req_data)
            rep_obj = self.dispatch(req_obj)

            rep_data = json.dumps(rep_obj)
            print "sending response: " + rep_data

            conn.send(rep_data + TCP_TERMINATOR)

        conn.close()

    def dispatch(self, req_dict):

        if req_dict['func'] == 'selfTest':
            return self.self_test()
        elif req_dict['func'] == 'getQueryId':
            return self.get_query_id()
        elif req_dict['func'] == 'releaseQueryId':
            return self.release_query_id(req_dict['query_id'])
        elif req_dict['func'] == 'addPosTrs':
            return self.add_pos_trs(req_dict['query_id'], req_dict['impath'])
        elif req_dict['func'] == 'addNegTrs':
            return self.add_neg_trs()
        elif req_dict['func'] == 'saveClassifier':
            return self.save_classifier()
        elif req_dict['func'] == 'saveAnnotations':
            return self.save_annotations()
        elif req_dict['func'] == 'train':
            return self.train(req_dict['query_id'])
        elif req_dict['func'] == 'rank':
            return self.rank(req_dict['query_id'])
        elif req_dict['func'] == 'getRanking':
            return self.get_ranking(req_dict['query_id'])
        else:
            raise RuntimeError('Unknown command: %s' % req_dict['func'])


    def self_test(self):
        return {'success': True}

    def get_query_id(self):

        query_id = self.client.start_query()

        num_query_id = self.next_num_query_id
        self.next_num_query_id = self.next_num_query_id + 1
        self.query_id_dict[num_query_id] = query_id

        return {'success': True,
                'query_id': num_query_id}

    def release_query_id(self, num_query_id):
        query_id = self._get_query_id(num_query_id)

        self.client.free_query(query_id)

        return {'success': True}

    def add_pos_trs(self, num_query_id, path):
        query_id = self._get_query_id(num_query_id)

        self.client.add_trs_from_file(query_id, path)

        return {'success': True}

    def add_neg_trs(self):
        print 'Not supported!'
        return {'success': True}

    def save_classifier(self):
        print 'Not supported!'
        return {'success': True}

    def save_annotations(self):
        print 'Not supported!'
        return {'success': True}

    def train(self, num_query_id):
        query_id = self._get_query_id(num_query_id)

        self.client.train(query_id)
        return {'success': True}

    def rank(self, num_query_id):
        query_id = self._get_query_id(num_query_id)

        self.client.rank(query_id)
        return {'success': True}

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
            uri = os.path.join(dset_name, os.path.splitext(ritem.path)[0])

            ranking_obj.append({'image': image,
                                'score': score,
                                'uri': uri})

        return {'success': True,
                'ranklist': ranking_obj}


    def _get_query_id(self, num_query_id):
        return self.query_id_dict[num_query_id]

if __name__ == "__main__":

    legacy_client = VisorLegacyWrap(DEFAULT_CONFIG_FILE)
    legacy_client.serve()
