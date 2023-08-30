const { createProxyMiddleware } = require('http-proxy-middleware')

module.exports = function(app) {
  // default, for linux development where we bind to a specific local IP
  const SERVER = 'http://192.168.12.1:12345'
  // for mac, use this one
  // const SERVER = 'http://localhost:12345'
  app.use(
    '/',
    createProxyMiddleware({
      target: 'http://192.168.12.1:12345',
      changeOrigin: true,
    }),
  )
  app.listen(3001)
}

