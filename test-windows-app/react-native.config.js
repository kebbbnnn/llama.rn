/**
 * RN Windows test app's local-dependency config.
 *
 * Tells RNW's autolinker where the llama.rn package lives (the repo root, since
 * this standalone test app consumes it from source, not from node_modules). RNW
 * then reads llama.rn's OWN react-native.config.js (repo root) for the
 * `platforms.windows` shape that auto-links windows/RNLlama.vcxproj.
 */

const path = require('path')
const pak = require('../package.json')

module.exports = {
  dependencies: {
    [pak.name]: {
      root: path.join(__dirname, '..'),
    },
  },
}
