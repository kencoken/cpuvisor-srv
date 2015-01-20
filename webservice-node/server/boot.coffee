express       = require "express"
#mongoose      = require "mongoose"
path          = require "path"

errorhandler  = require "errorhandler"

# Global objects
global.async  = require "async"
global._      = require "underscore"
_.str         = require "underscore.string"
_.mixin(_.str.exports())

global.express = express
global.app    = express()
#global.mongoose = mongoose
global.utils  = require "./lib/utils"
global.PROJECT_ROOT = __dirname
global.CLIENT_ROOT = path.resolve(__dirname, '../client/app')

global.server  = require('http').createServer(app)
global.io      = require('socket.io')(server)

require('./config')

###########################
# Database Configurations #
###########################

# MongoDB setup
#mongoUri = app.get('mongo_config').uri
#mongoOpts = app.get('mongo_config').opts
#mongoose.connect(mongoUri, mongoOpts)

#################################
# Express Configuration(Common) #
#################################

app.set "port", process.env.PORT or 3000
#app.set "views", __dirname + "/views"
#app.set "view engine", "ejs"
#app.use express.static(__dirname + "/public")
#app.use '/static', express.static(CLIENT_ROOT)

########################################
# Express Configuration(Envinromental) #
########################################

env = process.env.NODE_ENV || 'development';
if env == 'test' || env == 'development'
  app.use errorhandler
    dumpExceptions: true
    showStack: true

#######################
# Route Configuration #
#######################

require('./apps')

module.exports = app