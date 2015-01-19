fs = require 'fs'

for file in fs.readdirSync(__dirname) when /\.json$/.test(file)
  name    = file.replace '.json', ''
  json    = require "./#{name}"
  # combine defaults with env-specific settings
  config  = _.extend {}, json.defaults, json[app.get('env')]
  app.set "#{name}_config", config
