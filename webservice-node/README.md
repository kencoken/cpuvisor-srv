kng-node-seed
=============

This package contains a simple node server in coffeescript.
It is configured to be used with an angular client.

Installation
------------

First install the server dependencies:

    $ cd server
	$ npm install

Then launch the server:

    $ coffee server.coffee

Client-side code
----------------

A simple static page is provided in the `client/` folder.

For a more fully-fleged example using angular, use `kng-seed`:

    $ rm -r client
	$ git clone git@github.com:kencoken/kng-seed.git
	$ mv kng-seed client

