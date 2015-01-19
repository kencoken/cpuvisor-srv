http = require "http"
querystring = require "querystring"
crypto = require "crypto"

_ = require "underscore"
async = require "async"

class GoogleSearcher

  constructor: ->
    @num_results = 100
    @query_timeout = 500
    @image_type = 'photo' # could be: photo, clipart, lineart, face

    @_results_per_req = 100

  query: (query, callback) =>

    params =
      tbm: 'isch' # image search mode
      ijn: 0 # causes AJAX request contents only to be returned
      isz: 'm' # image size: s, m, l
      itp: @image_type # image type
      q: query

    headers =
      'User-Agent': 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10.8; rv:25.0) Gecko/20100101 Firefox/25.0'

    @_fetch_results(query, @num_results, params, headers, callback)

  _fetch_results: (query, num_results, params, headers, callback) =>

    fetch_funcs = (((cb) => @_fetch_results_from_offset(query, result_offset, params, headers, cb)) for result_offset in [0..num_results] by @_results_per_req)

    async.parallel fetch_funcs, (err, results) =>
      if err then callback(err)

      results = _.flatten(results)

      if results.length < 1
        callback(new Error "No image URLs could be retrieved")
      else
        callback(null, results)


  _fetch_results_from_offset: (query, result_offset,
                               params, headers, callback) =>

    # 1. issue request

    page_idx = Math.floor(result_offset/@_results_per_req)
    params.ijn = page_idx # ijn is the AJAX page index (0 = first page)


    opts =
      hostname: 'www.google.com'
      path: '/search?' + querystring.stringify(params)
      method: 'GET'
      headers: headers

    req = http.request opts, (rep) =>

      body = ''

      rep.on 'data', (chunk) =>
        body += chunk

      rep.on 'end', =>
        # 2. parse request
        urls = @_parse_rep_body(body)
#        console.log(urls)
        callback(null, urls)


    req.on 'error', (err) =>
      callback(err)

    req.setTimeout(@query_timeout)
    req.end()

  _parse_rep_body: (body) =>
    image_div_pattern = /<div class="rg_di(.*?)<\/div>/ig
    image_url_pattern = /imgurl=(.*?)&/i
    image_id_pattern = /name="(.*?):/i

    url_data = []

    while div = image_div_pattern.exec(body)
      div = div[1]

      image_url_match = image_url_pattern.exec(div)[1]
      image_id_match = image_id_pattern.exec(div)[1]

#      console.log('-----------')
#      console.log(image_url_match)

      if image_url_match? && image_id_match?
        md5sum = crypto.createHash('md5')
        md5sum.update(image_url_match)

        url_data.push {
          url: image_url_match
          image_id: md5sum.digest('hex')
          rank: url_data.length+1
        }

    return url_data

if require.main == module
  searcher = new GoogleSearcher()
  searcher.query 'car', (err, results) =>
    if err then throw err

    console.log(results)

module.exports = GoogleSearcher



