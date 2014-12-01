from multiprocessing import Process, Queue
import socket
import json
from time import sleep
import os

import matplotlib.pyplot as plt
import matplotlib.image as mpimg

import pyclient

from google import protobuf
from proto import cpuvisor_config_pb2 as protoconfig

VISUALISE_RESULTS = True    # if TRUE visualise the output ranking

SERVE_IP = '127.0.0.1'
SERVE_PORT = 5005
BUFFER_SIZE = 1024

TEST_QUERY = 'car'

TCP_TERMINATOR = '$$$'

CONFIG_FILE = 'config.prototxt'

def legacy_serve():

    # connect to server

    client = pyclient.VisorLegacyWrap(CONFIG_FILE,
                                      SERVE_IP, SERVE_PORT,
                                      use_greenlets=True)

    print 'Serving...'
    client.serve()

def send_req_obj(socket, req_obj):

    socket.send(json.dumps(req_obj) + TCP_TERMINATOR)

def recv_rep_obj(socket):

    rep_data = socket.recv(BUFFER_SIZE)
    term_idx = rep_data.find(TCP_TERMINATOR)
    while term_idx < 0:
        append_data = socket.recv(BUFFER_SIZE)
        if not append_data:
            raise RuntimeError("No data received!")
        else:
            rep_data = rep_data + append_data
            term_idx = rep_data.find(TCP_TERMINATOR)

    rep_data = rep_data[0:term_idx]

    return json.loads(rep_data)

if __name__ == "__main__":

    # read in configuration
    config = protoconfig.Config()
    with open(CONFIG_FILE, 'rb') as f:
        protobuf.text_format.Merge(f.read(), config)

    # launch legacy wrapper

    p = Process(target=legacy_serve)
    p.start()


    sleep(1)

    # connect to legacy wrapper

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((SERVE_IP, SERVE_PORT))

    # send requests

    print 'Sending selfTest'
    req_obj = {'func': 'selfTest'}
    send_req_obj(s, req_obj)
    print 'Received response:'
    rep_obj = recv_rep_obj(s)
    print rep_obj

    print 'Sending getQueryId'
    req_obj = {'func': 'getQueryId'}
    send_req_obj(s, req_obj)
    print 'Received response:'
    rep_obj = recv_rep_obj(s)
    print rep_obj

    query_id = rep_obj['query_id']

    pos_trs_dir = os.path.join(config.server_config.image_cache_path, TEST_QUERY)
    pos_trs_paths = [os.path.join(pos_trs_dir, pos_trs_fname) for
                     pos_trs_fname in os.listdir(pos_trs_dir)]

    for pos_trs_path in pos_trs_paths:
        print 'Sending addPosTrs'
        req_obj = {'func': 'addPosTrsAndWait',
                   'query_id': query_id,
                   'impath': pos_trs_path}
        send_req_obj(s, req_obj)
        print 'Received response:'
        rep_obj = recv_rep_obj(s)
        print rep_obj

    print 'Sending train'
    req_obj = {'func': 'train',
               'query_id': query_id}
    send_req_obj(s, req_obj)
    print 'Received response:'
    rep_obj = recv_rep_obj(s)
    print rep_obj

    print 'Sending rank'
    req_obj = {'func': 'rank',
               'query_id': query_id}
    send_req_obj(s, req_obj)
    print 'Received response:'
    rep_obj = recv_rep_obj(s)
    print rep_obj

    print 'Sending getRanking'
    req_obj = {'func': 'getRanking',
               'query_id': query_id}
    send_req_obj(s, req_obj)
    print 'Received response:'
    rep_obj = recv_rep_obj(s)
    print rep_obj

    rank_result = rep_obj

    print 'Sending releaseQueryId'
    req_obj = {'func': 'releaseQueryId',
               'query_id': query_id}
    send_req_obj(s, req_obj)
    print 'Received response:'
    rep_obj = recv_rep_obj(s)
    print rep_obj

    # cleanup

    s.close()
    p.join()

    # visualise ranked results

    if VISUALISE_RESULTS:

        rlist_plt = rank_result['ranklist'][:3*6]

        fig, axes = plt.subplots(3, 6, figsize=(12, 6),
                                 subplot_kw={'xticks': [], 'yticks': []})
        fig.subplots_adjust(hspace=0.3, wspace=0.05)

        for ax, ritem in zip(axes.flat, rlist_plt):
            print ritem
            im = mpimg.imread(ritem['image'])
            ax.imshow(im)
            ax.set_title(ritem['score'])

        plt.show()
