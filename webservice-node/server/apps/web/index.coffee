app.use '/', express.static(CLIENT_ROOT)

#app.get "/", (req, res) ->
#  res.render('index')
#
#app.get "/partials/:name", (req, res) ->
#  name = req.params.name
#  res.render('partials/' + name)
