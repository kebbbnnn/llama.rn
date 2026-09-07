const { getDefaultConfig, mergeConfig } = require('@react-native/metro-config')
const path = require('path')

const defaultConfig = getDefaultConfig(__dirname)

// llama.rn is symlinked into node_modules, so Metro resolves its `react-native`
// import from llama.rn's own node_modules (a stale local copy) instead of the
// app's. Force `react-native` to always resolve to the app's installed copy so
// the bundle shares one RN instance.
const rnPath = path.resolve(__dirname, 'node_modules', 'react-native', 'index.js')
const rnmPath = path.resolve(__dirname, 'node_modules', 'react-native-macos', 'index.js')
const origResolveRequest = defaultConfig.resolver.resolveRequest

defaultConfig.resolver.resolveRequest = (
  context,
  moduleName,
  platform,
) => {
  if (moduleName === 'react-native') {
    return { type: 'sourceFile', filePath: rnPath }
  }
  if (moduleName === 'react-native-macos') {
    return { type: 'sourceFile', filePath: rnmPath }
  }
  if (origResolveRequest) {
    return origResolveRequest(context, moduleName, platform)
  }
  return context.resolveRequest(context, moduleName, platform)
}

module.exports = mergeConfig(defaultConfig, {
  projectRoot: __dirname,
  watchFolders: [__dirname + '/..'],
})