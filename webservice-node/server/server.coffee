process.execPath = 'coffee'

cluster = require('cluster')
numCPUs = require('os').cpus().length

# Spawn only 1 fork for development.
#if process.env.NODE_ENV in ["local", "development", "serverdev"]
#  numCPUs = 0
#
#if cluster.isMaster
#  # Fork workers.
#  for i in [0..numCPUs]
#    cluster.fork()
#
#  cluster.on 'exit', (worker, code, signal) ->
#    console.log('worker ' + worker.process.pid + ' died')
#    cluster.fork()
#else
#  app = require("./boot")
#  server.listen app.get("port"), ->
#    console.log(
#      "Express server listening on port #{app.settings.port}" +
#      " in #{app.settings.env} mode"
#    )

app = require("./boot")
server.listen app.get("port"), ->
  console.log(
    "Express server listening on port #{app.settings.port}" +
    " in #{app.settings.env} mode"
  )
