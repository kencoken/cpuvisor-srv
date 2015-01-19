express       = require "express"
#mongoose      = require "mongoose"
path          = require "path"

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

app.configure ->
  app.set "port", process.env.PORT or 3000
  app.set "views", __dirname + "/views"
  app.set "view engine", "ejs"
  #app.use express.static(__dirname + "/public")
  app.use '/static', express.static(CLIENT_ROOT)
  app.use app.router

########################################
# Express Configuration(Envinromental) #
########################################

app.configure "test", ->
  app.use express.errorHandler
    dumpExceptions: true
    showStack: true

app.configure "development", ->
  app.use express.errorHandler
    dumpExceptions: true
    showStack: true

#######################
# Route Configuration #
#######################

require('./apps')

module.exports = app