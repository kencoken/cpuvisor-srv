import sys
import os
import zmq
from google import protobuf
from proto import cpuvisor_config_pb2 as protoconfig
from proto import cpuvisor_srv_pb2 as protosrv

from time import sleep

file_dir = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(file_dir, 'imsearch-tools'))
from imsearchtools import engines as imsearch_handlers

import matplotlib.pyplot as plt
import matplotlib.image as mpimg

VISUALISE_RESULTS = True

context = zmq.Context()

# read in configuration

config = protoconfig.Config()

with open('/Data/src/cpuvisor-srv/config.prototxt', 'rb') as f:
    protobuf.text_format.Merge(f.read(), config)

# connect to server

print 'Conntecting to %s...' % config.server_config.server_endpoint
socket = context.socket(zmq.REQ)
socket.connect(config.server_config.server_endpoint)

# send a request

print 'Getting Query ID...'

req = protosrv.RPCReq()
req.request_string = 'start_query'
req.tag = 'car'
socket.send(req.SerializeToString())

message = socket.recv()
# print '***********'
# print message
# print '***********'

rpc_rep = protosrv.RPCRep()
rpc_rep.ParseFromString(message)
assert(rpc_rep.success == True)
query_id = rpc_rep.id

print 'Query ID is: %s'  % query_id

# ----

print 'Adding Training Samples...'

req = protosrv.RPCReq()
req.request_string = 'add_trs'
req.id = query_id

google_searcher = imsearch_handlers.GoogleWebSearch()
results = google_searcher.query('car')

for result in results:
    req.train_image_urls.urls.append(result['url'])

socket.send(req.SerializeToString())

message = socket.recv()
rpc_rep = protosrv.RPCRep()
rpc_rep.ParseFromString(message)
assert(rpc_rep.success == True)

print 'Started training sample addition process!'

sleep(20)

# ----

print 'Sending train command...'

req = protosrv.RPCReq()
req.request_string = 'train_rank_get_ranking';
req.id = query_id

socket.send(req.SerializeToString())

message = socket.recv()
rpc_rep = protosrv.RPCRep()
rpc_rep.ParseFromString(message)
assert(rpc_rep.success == True)

print 'Train, rank and get ranking command returned!'

ctr = 1;
for ritem in rpc_rep.ranking.rlist:
    print '%d: %s (%f)' % (ctr, ritem.path, ritem.score)
    ctr = ctr + 1
    if ctr > 10:
        break

# visualise ranked results

if VISUALISE_RESULTS:

    rlist_plt = rpc_rep.ranking.rlist[:3*6]

    fig, axes = plt.subplots(3, 6, figsize=(12, 6),
                             subplot_kw={'xticks': [], 'yticks': []})
    fig.subplots_adjust(hspace=0.3, wspace=0.05)

    for ax, ritem in zip(axes.flat, rlist_plt):
        im = mpimg.imread(os.path.join(config.preproc_config.dataset_im_base_path, ritem.path))
        ax.imshow(im)
        ax.set_title(ritem.score)

    plt.show()
