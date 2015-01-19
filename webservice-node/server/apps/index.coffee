require('./web')
require('./api')

app.get "/status", (req, res) ->
  res.send
    status: "OK"
app.get "/404", (req, res) ->
  res.send("404 - page not found")
app.all "/*", (req, res) ->
  res.statusCode = 404
  res.redirect "/404/"