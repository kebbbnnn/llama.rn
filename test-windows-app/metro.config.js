const { getDefaultConfig, mergeConfig } = require('@react-native/metro-config')
const path = require('path')

const defaultConfig = getDefaultConfig(__dirname)
const root = path.resolve(__dirname, '..')

module.exports = mergeConfig(defaultConfig, {
  projectRoot: __dirname,
  watchFolders: [root],
  resolver: {
    nodeModulesPaths: [
      path.resolve(__dirname, 'node_modules'),
    ],
    extraNodeModules: {
      'llama.rn': root,
    },
    blockList: [
      /.*[/\\]\.build-windows-.*[/\\].*/,
      /.*[/\\]third_party[/\\].*/,
      /.*[/\\]windows[/\\].*/,
      /.*[/\\]example[/\\].*/,
      /.*[/\\]android[/\\].*/,
      /.*[/\\]ios[/\\].*/,
      /.*[/\\]macos[/\\].*/,
    ],
  },
})
