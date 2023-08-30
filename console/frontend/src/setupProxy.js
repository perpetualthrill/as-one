const { createProxyMiddleware } = require('http-proxy-middleware')

module.exports = function(app) {
  // default, for linux development where we bind to a specific local IP
  const SERVER = 'http://192.168.12.1:12345'
  // for mac, use this one
  // const SERVER = 'http://localhost:12345'

  // I have no idea why matching a path that does not exist makes this
  // work, but it totally does. If set to "/" it serves static files
  // from the actual server, which is not helpful
  app.use(
    '/nomatcheshere',
    createProxyMiddleware({
      target: SERVER,
      changeOrigin: true,
    }),
  )
}

