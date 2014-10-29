import threading

import zmq
from google import protobuf
from proto import cpuvisor_config_pb2 as protoconfig
from proto import cpuvisor_srv_pb2 as protosrv

import logging
log = logging.getLogger(__name__)

class VisorNotificationMonitor(threading.Thread):

    def __init__(self, protoconfig_path, callback=None, context=None):

        threading.Thread.__init__(self)
        self.daemon = True
        self.callback = callback

        # setup ZMQ context for work
        self.context = context or zmq.Context.instance()

        # read in configuration
        self.config = protoconfig.Config()
        with open(protoconfig_path, 'rb') as f:
            protobuf.text_format.Merge(f.read(), self.config)

        # connect to PUB server
        log.info('Connecting to ZMQ SUB socket...')
        self.sub_socket = self.context.socket(zmq.SUB)
        self.sub_socket.connect(self.config.server_config.notify_endpoint)

        self.sub_socket.setsockopt(zmq.SUBSCRIBE, '')

    def run(self):

        while True:

            message = self.sub_socket.recv()

            notification = protosrv.VisorNotification()
            notification.ParseFromString(message)

            log.info('Received notification of type: ' +
                     protosrv.NotificationType.Name(notification.type))

            if self.callback:
                self.callback(notification)


class VisorNotifier(object):

    def __init__(self, protoconfig_path, callback=None, context=None):

        self.monitor_thread_ = VisorNotificationMonitor(protoconfig_path, callback, context)
        self.monitor_thread_.start()
