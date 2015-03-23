fs = require "fs"
path = require "path"
zmq = require "zmq"
protobufjs = require "protobufjs"

GoogleSearcher = require "./google-searcher"
VisorNotifier = require "./visor-notifier"

class VisorClient

  constructor: (protoconfig_path) ->
    builder = protobufjs.newBuilder({ convertFieldsToCamelCase: false });
    protobufjs.loadProtoFile(path.resolve(__dirname, "../../../src/proto/visor_common.proto"), builder)
    protobufjs.loadProtoFile(path.resolve(__dirname, "../../../src/proto/visor_srv.proto"), builder)
    protobufjs.loadProtoFile(path.resolve(__dirname, "../../../src/proto/visor_config.proto"), builder)
    protobufjs.loadProtoFile(path.resolve(__dirname, "../../../src/proto/cpuvisor_srv.proto"), builder)
    protobufjs.loadProtoFile(path.resolve(__dirname, "../../../src/proto/cpuvisor_config.proto"), builder)
    @proto_classes = builder.build()

    data = fs.readFileSync protoconfig_path, {encoding: 'utf8'}

    @server_endpoint = data.match(/server_endpoint: "([^"]+)"/)[1]

    @google_searcher = new GoogleSearcher()

    @req_socket = zmq.socket('req')
    console.log('Connecting to server_endpoint...')
    @req_socket.connect(@server_endpoint)
    console.log('Connected!')

  start_query: (callback) =>
    console.log('REQ: start_query')

    @req_socket.send(@generate_encoded_req_('start_query'))

    @req_socket.once 'message', (data) =>
      rep = @parse_message_(data)
      if !rep.success
        callback(new Error(rep.err_msg))
      else
        callback(null, rep.id)

  download_trs: (query_id, query, callback) =>
    console.log('REQ: download_trs')

    req = @generate_encoded_req_ 'set_tag', {
      id: query_id
      tag: query
    }
    @req_socket.send(req)

    @req_socket.once 'message', (data) =>
      rep = @parse_message_(data)
      if !rep.success
        callback(rep.err_msg)
      else
        # add_trs

        @google_searcher.query query, (err, results) =>
          if err then callback(err)

          image_urls_obj = new @proto_classes.cpuvisor.TrainImageUrls()
          for result in results
            image_urls_obj.add('urls', result.url)

          req = @generate_encoded_req_ 'add_trs', {
            id: query_id
            train_image_urls: image_urls_obj
          }

          @req_socket.send(req)

          @req_socket.once 'message', (data) =>
            rep = @parse_message_(data)
            if !rep.success
              callback(rep.err_msg)
            else
              callback()

  train_rank_get_ranking: (query_id, callback) =>
    console.log('REQ: train_rank_get_ranking')

    req = @generate_encoded_req_ 'train_rank_get_ranking', {
      id: query_id
    }
    @req_socket.send(req)

    @req_socket.once 'message', (data) =>
      rep = @parse_message_(data)
      if !rep.success
        callback(new Error(rep.err_msg))
      else
        callback(null, rep.ranking)

  get_ranking: (query_id, page=1, callback) =>
    console.log('REQ: get_ranking')

    req = @generate_encoded_req_ 'get_ranking', {
      id: query_id
      retrieve_page: page
    }
    @req_socket.send(req)

    @req_socket.once 'message', (data) =>
      rep = @parse_message_(data)
      if !rep.success
        callback(rep.err_msg)
      else
        callback(null, rep.ranking)

  free_query: (query_id, callback) =>
    console.log('REQ: free_query')

    req = @generate_encoded_req_ 'free_query', {
      id: query_id
    }
    @req_socket.send(req)

    @req_socket.once 'message', (data) =>
      rep = @parse_message_(data)
      if !rep.success
        callback(rep.err_msg)
      else
        callback()

  train: (query_id, blocking=false, callback) =>
    console.log('REQ: train')

    if blocking
      req = @generate_encoded_req_ 'train_and_wait', {
        id: query_id
      }
    else
      req = @generate_encoded_req_ 'train', {
        id: query_id
      }
    @req_socket.send(req)

    @req_socket.once 'message', (data) =>
      rep = @parse_message_(data)
      if !rep.success
        callback(rep.err_msg)
      else
        callback()

  rank: (query_id, blocking=false, callback) =>
    console.log('REQ: rank')

    if blocking
      req = @generate_encoded_req_ 'rank_and_wait', {
        id: query_id
      }
    else
      req = @generate_encoded_req_ 'rank', {
        id: query_id
      }
    @req_socket.send(req)

  generate_encoded_req_: (req_str, fields = {}) =>
    pbjs_obj = @generate_req_(req_str, fields)
    return @encode_req_(pbjs_obj)

  generate_req_: (req_str, fields = {}) =>
    req_dict = fields
    req_dict.request_string = req_str

    return new @proto_classes.visor.RPCReq(req_dict)

  encode_req_: (req_obj) =>
    return req_obj.encode().toBuffer()


  parse_message_: (message) =>
    return @proto_classes.visor.RPCRep.decode(message)


if require.main == module

  IMDOWNLOAD_TIMEOUT = 20000

  client = new VisorClient("../../../config.prototxt")
  notifier = new VisorNotifier("../../../config.prototxt")
  notifier.subscribe()

  client.start_query (err, query_id) =>
    if err then throw err
    console.log('Started query with ID: ' + query_id)

    timed_out = false
    done_datacoll = false

    notifier.on 'statechange', (id, newstate) =>
      if id == query_id
        switch newstate
          when 'QS_DATACOLL_COMPLETE'
            done_datacoll = true
            if !timed_out
              client.train query_id, false, (err) =>
                if err then throw err
          when 'QS_TRAINED'
            client.rank query_id, false, (err) =>
              if err then throw err
          when  'QS_RANKED'
            client.get_ranking query_id, 1, (err, ranking) =>
              if err then throw err

              console.log(ranking)

              client.free_query query_id, (err) =>
                if err then throw err

                process.exit(code=0)

    client.download_trs query_id, 'car', (err) =>
      if err then throw err
      console.log('Done starting download_trs')

    setTimeout () =>
      timed_out = !done_datacoll
      if timed_out
        client.train query_id, false, (err) =>
          if err then throw err
    , IMDOWNLOAD_TIMEOUT

module.exports = VisorClient