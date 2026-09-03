const http = require('http')
const fs = require('fs')
const path = require('path')

const logFile = path.resolve(__dirname, 'ci-app.log')
const markerFile = path.resolve(__dirname, 'rnllama-install-ok.txt')

const server = http.createServer((req, res) => {
  let body = ''
  req.on('data', (chunk) => {
    body += chunk
  })
  req.on('end', () => {
    const line = `[CI-APP] ${new Date().toISOString()} ${req.method} ${req.url}: ${body}\n`
    process.stdout.write(line)
    try {
      fs.appendFileSync(logFile, line)
    } catch (_) {}

    if (req.url === '/success') {
      try {
        fs.writeFileSync(markerFile, 'ok\n')
        if (process.env.RNLLAMA_INSTALL_MARKER) {
          fs.writeFileSync(process.env.RNLLAMA_INSTALL_MARKER, 'ok\n')
        }
      } catch (_) {}
    }

    res.writeHead(200, {
      'Content-Type': 'text/plain',
      'Access-Control-Allow-Origin': '*',
      'Access-Control-Allow-Methods': 'GET, POST, OPTIONS',
      'Access-Control-Allow-Headers': '*',
    })
    res.end('ok')
  })
})

server.listen(8082, '0.0.0.0', () => {
  console.log('[CI-LOGGER] Listening on http://0.0.0.0:8082')
})
