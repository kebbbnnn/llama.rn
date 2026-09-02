/**
 * Minimal React Native Windows test app that consumes the local llama.rn
 * package (root of this repo) via react-native.config.js. Used by
 * .github/workflows/test-windows-app.yml to build llama.rn's Windows native
 * module (windows/*) end-to-end and prove the JSI install path on Windows.
 */

const path = require('path')
const pak = require('../package.json')

module.exports = {
  dependencies: {
    [pak.name]: {
      root: path.join(__dirname, '..'),
      platforms: {
        windows: {
          sourceDir: 'windows',
          solutionFile: 'RNLlama.sln',
          projects: [
            {
              projectFile: 'RNLlama.vcxproj',
              directDependency: true,
            },
          ],
        },
      },
    },
  },
}
