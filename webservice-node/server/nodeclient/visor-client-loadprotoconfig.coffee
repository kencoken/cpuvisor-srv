fs = require "fs"
zmq = require "zmq"
ProtoBuf = require "protobufjs"
pb = require "protocol-buffers"

class VisorClient

  constructor: (protoconfig_path) ->
    messages_srv = pb(fs.readFileSync("../../../src/proto/cpuvisor_srv.proto"))
    messages_config = pb(fs.readFileSync("../../../src/proto/cpuvisor_config.proto"))

    fs.readFile protoconfig_path, {encoding: 'utf8'}, (err, data) =>
      if err then throw err

      @config_alt = messages_config.Config.decode(data)
      console.log(@config_alt)


#    builder = ProtoBuf.newBuilder({ convertFieldsToCamelCase: true });
#    ProtoBuf.loadProtoFile("../../../src/proto/cpuvisor_srv.proto", builder)
#    ProtoBuf.loadProtoFile("../../../src/proto/cpuvisor_config.proto", builder)
#    @proto_classes = builder.build("cpuvisor")
#
#    #console.log(@proto_classes)
#
#    buff = ''
#    fs.createReadStream protoconfig_path
#    .on 'error', (error) ->
#      throw error
#    .on 'data', (chunk) ->
#      buff += chunk
#    .on 'end', =>
#      console.log(buff)
#
#      for m in @proto_classes.Config
#        console.log(m)
#        if typeof m == 'function'
#          console.log(m)
#
#      @config = @proto_classes.Config.decode(buff)
#      console.log(@config)

#    fs.readFile protoconfig_path, {encoding: 'utf8'}, (err, data) =>
#      if err then throw err
#
#      @config = @proto_classes.Config.decode(data)
#      console.log(@config)

#    @req_socket = zmq.socket('rep')
#    @req_socket.connect(@config.server_config.server_endpoint)

  start_query: =>
    console.log('REQ: start_query')

    @req_socket.send('hello world')


if require.main == module
  client = new VisorClient("../../../config.prototxt")