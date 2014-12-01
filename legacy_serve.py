import os

import logging
logging.basicConfig(format='%(asctime)s %(levelname)s:%(message)s', level=logging.DEBUG)

import pyclient

SERVE_IP = '127.0.0.1'
SERVE_PORT = 35217

CONFIG_FILE = 'config.prototxt'

if __name__ == "__main__":

    client = pyclient.VisorLegacyWrap(CONFIG_FILE,
                                      SERVE_IP, SERVE_PORT,
                                      multiple_connections=True,
                                      use_greenlets=True)

    client.serve()
