fs = require "fs"
events = require "events"
zmq = require "zmq"
protobufjs = require "protobufjs"

class VisorNotifier extends events.EventEmitter

  constructor: (protoconfig_path) ->
    @builder = protobufjs.newBuilder({ convertFieldsToCamelCase: false });
    protobufjs.loadProtoFile("../../../src/proto/cpuvisor_srv.proto", @builder)
    protobufjs.loadProtoFile("../../../src/proto/cpuvisor_config.proto", @builder)
    @proto_classes = @builder.build("cpuvisor")

    data = fs.readFileSync protoconfig_path, {encoding: 'utf8'}

    @notify_endpoint = data.match(/notify_endpoint: "([^"]+)"/)[1]
    @listener = null

  subscribe: =>

    @req_socket = zmq.socket('sub')
    console.log('Subscribing to notify_endpoint...')
    @req_socket.connect(@notify_endpoint)
    @req_socket.subscribe('')
    console.log('Connected!')

    @listener = @req_socket.on 'message', (message) =>
      notification_types = @get_enum_keys_('cpuvisor.NotificationType')

      notification_obj = @proto_classes.VisorNotification.decode(message)
      notification_type = notification_types[notification_obj.type]
      console.log(notification_type)

      switch notification_type
        when 'NTFY_STATE_CHANGE'
          event_str = 'statechange'
        when 'NTFY_IMAGE_PROCESSED'
          event_str = 'imageprocessed'
        when 'NTFY_ALL_IMAGES_PROCESSED'
          event_str = 'allimagesprocessed'
        else
          throw new Error("Unrecognized notification type - shouldn't get here")

      @emit event_str, notification_obj.id, notification_obj.data

  unsubscribe: =>

    if @listener?
      @req_socket.removeListener('message', @listener)
      @listener = null

  get_enum_keys_: (path) =>
    enum_rev_map = @builder.lookup(path).object
    enum_map = {}
    for key, val of enum_rev_map
      enum_map[val] = key
    return enum_map

module.exports = VisorNotifier