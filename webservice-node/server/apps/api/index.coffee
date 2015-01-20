path = require "path"

VisorClient = require "../../nodeclient/visor-client"
client = new VisorClient(path.resolve(__dirname, "../../../../config.prototxt"))

# notifications

notifyio = io.of('/api/query/notifications')

notifyio.on 'connection', (socket) ->
  console.log('socket connected')
  socket.on 'event', (data) ->
    console.log('socket event occurred')
  socket.on 'disconnect', () ->
    console.log('socket disconnected')

VisorNotifier = require "../../nodeclient/visor-notifier"
notifier = new VisorNotifier(path.resolve(__dirname, "../../../../config.prototxt"))
notifier.subscribe()

notifier.on 'notification', (type, id, data) ->
  console.log({type: type, id: id, data: data})
  notifyio.emit('notification',
    {type: type, data: data, id: id})

# regular routes

router = express.Router()

router.use (err, req, res, next) ->
  res.status(500).json({success: false, err_msg: err.message})

router.get "/", (req, res) ->
  res.send "CPU-VISOR Demo API"

router.post "/query/start_query", (req, res, next) ->
  client.start_query (err, query_id) =>
    if err? then return next(err)
    res.json({success: true, query_id: query_id})

router.put "/query/:query_id/add_trs/:query_string", (req, res, next) ->
  client.download_trs req.params.query_id, req.params.query_string, (err) =>
    if err? then return next(err)
    res.json({success: true})

router.put "/query/:query_id/train", (req, res, next) ->
  client.train req.params.query_id, false, (err) =>
    if err? then return next(err)
    res.json({success: true})

router.put "/query/:query_id/rank", (req, res, next) ->
  client.rank req.params.query_id, false, (err) =>
    if err? then return next(err)
    res.json({success: true})

router.get "/query/:query_id/ranking/:page", (req, res, next) ->
  client.get_ranking req.params.query_id, Number(req.params.page), (err, ranking) =>
    if err? then return next(err)
    res.json({success: true, ranking: ranking})

router.put "/query/:query_id/free", (req, res, next) ->
  client.rank req.params.query_id, (err) =>
    if err? then return next(err)
    res.json({success: true})

# apply routes

app.use '/api', router