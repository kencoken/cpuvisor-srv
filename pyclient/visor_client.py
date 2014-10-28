import os
import sys

import zmq
from google import protobuf
from proto import cpuvisor_config_pb2 as protoconfig
from proto import cpuvisor_srv_pb2 as protosrv

import logging
log = logging.getLogger(__name__)

file_dir = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(file_dir, '..', 'imsearch-tools'))
from imsearchtools import engines as imsearch_handlers

class InvalidRequestError(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)

class VisorClient(object):

    def __init__(self, protoconfig_path):

        # setup ZMQ context for work
        self.context = zmq.Context()

        # read in configuration
        self.config = protoconfig.Config()
        with open(protoconfig_path, 'rb') as f:
            protobuf.text_format.Merge(f.read(), self.config)

        # connect to server
        log.info('Connecting to ZMQ REQ socket...')
        self.req_socket = self.context.socket(zmq.REQ)
        self.req_socket.connect(self.config.server_config.server_endpoint)

    def start_query(self):
        """ Start a new query
        retval --> QUERY_ID
        """
        log.info('REQ: start_query')

        req = self.generate_req_('start_query')
        self.req_socket.send(req.SerializeToString())

        rep = self.parse_message_(self.req_socket.recv())

        return rep.id

    def download_trs(self, query_id, query):
        """ Download and compute features for a given text query
        (Operates asynchronously)
        """
        log.info('REQ: download_trs')

        req = self.generate_req_('set_tag')
        req.id = query_id
        req.tag = query
        self.req_socket.send(req.SerializeToString())

        self.parse_message_(self.req_socket.recv())

        # add_trs ---

        req = self.generate_req_('add_trs')
        req.id = query_id
        google_searcher = imsearch_handlers.GoogleWebSearch()
        results = google_searcher.query(query)

        for result in results:
            req.train_image_urls.urls.append(result['url'])

        self.req_socket.send(req.SerializeToString())

        self.parse_message_(self.req_socket.recv())

    def train_rank_get_ranking(self, query_id):
        """ Train classifier and rank
        retval --> ranking (first page)
        """
        log.info('REQ: train_rank_get_ranking')

        req = self.generate_req_('train_rank_get_ranking');
        req.id = query_id
        self.req_socket.send(req.SerializeToString())

        rep = self.parse_message_(self.req_socket.recv())

        return rep.ranking

    def get_ranking(self, query_id, page=1):
        """ Retrieve ranking of completed query
        retval --> ranking (page given by parameter)
        """
        log.info('REQ: get_ranking')

        req = self.generate_req_('get_ranking')
        req.id = query_id
        req.retrieve_page = page
        self.req_socket.send(req.SerializeToString())

        rep = self.parse_message_(self.req_socket.recv())

        return rep.ranking

    def free_query(self, query_id):
        """ Free a query that is no longer required in the backend to save memory
        """
        log.info('REQ: free_query')

        req = self.generate_req_('free_query')
        req.id = query_id
        self.req_socket.send(req.SerializeToString())

        self.parse_message_(self.req_socket.recv())

    # ---------------

    def generate_req_(self, req_str):
        req = protosrv.RPCReq()
        req.request_string = req_str
        return req

    def parse_message_(self, message):
        rep = protosrv.RPCRep()
        rep.ParseFromString(message)
        if not rep.success:
            raise InvalidRequestError(rep.err_msg)

        return rep
