import zmq
from google import protobuf
from proto import cpuvisor_config_pb2 as protoconfig
from proto import cpuvisor_srv_pb2 as protosrv

from time import sleep

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

image_urls = ['http://i.telegraph.co.uk/multimedia/archive/02565/Ford-Fiesta_jpg_2565695b.jpg',
              'http://www.extremetech.com/wp-content/uploads/2012/12/Audi-A1.jpg',
              'http://i.telegraph.co.uk/multimedia/archive/02556/Ford-Fiesta-2_2556130k.jpg',
              'http://i.telegraph.co.uk/multimedia/archive/01249/car_ultimate_aero__1249846c.jpg',
              'http://cdn.carbuyer.co.uk/sites/carbuyer_d7/files/jato_uploaded/Hyundai-i10-micro-car-2012-front-quarter-main.jpg',
              'http://www.popularmechanics.com/cm/popularmechanics/images/Rl/future_cars_09_0211-lgn.jpg',
              'http://i.telegraph.co.uk/multimedia/archive/02417/Sandero-1_2417618k.jpg',
              'http://s3.amazonaws.com/rapgenius/filepicker%2FsVUlzueDRFOaweMrMWzl_car.jpg',
              'http://upload.wikimedia.org/wikipedia/commons/2/26/Metropolitan_car_club_meeting.JPG',
              'http://i.huffpost.com/gen/1211885/thumbs/o-CAR-570.jpg',
              'http://static1.businessinsider.com/image/52caecaa6da811c50e9ac55a/pandora-and-iheartradio-step-up-their-war-to-control-the-inside-of-your-car.jpg',
              'http://www.weddingcarsomerset.co.uk/wp-content/uploads/2014/05/wedding-car-hire-bristol.jpg',
              'http://upload.wikimedia.org/wikipedia/commons/5/5a/1970_AMC_The_Machine_2-door_muscle_car_in_RWB_trim_by_lake.JPG',
              'http://www.hdwallpaperscool.com/wp-content/uploads/2013/11/muscle-cars-top-images-new-hd-wallpapers-classic.jpg']
for image_url in image_urls:
    req.train_image_urls.urls.append(image_url)

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
req.request_string = 'train'
req.id = query_id

socket.send(req.SerializeToString())

message = socket.recv()
rpc_rep = protosrv.RPCRep()
rpc_rep.ParseFromString(message)
assert(rpc_rep.success == True)

print 'Train command returned!'

# for test_func in ['start_query', 'add_trs', 'train', 'rank', 'free_query', 'nonsense']:

#     req = protosrv.RPCReq()
#     req.request_string = test_func
#     req.id = "10"
#     socket.send(req.SerializeToString())

#     message = socket.recv()
#     rpc_rep = protosrv.RPCRep()
#     rpc_rep.ParseFromString(message)
#     print "Received reply %s [ %s ]" % (req, rpc_rep)
