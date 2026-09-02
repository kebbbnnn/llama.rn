const { getDefaultConfig, mergeConfig } = require('@react-native/metro-config')
const path = require('path')

const defaultConfig = getDefaultConfig(__dirname)
const root = path.resolve(__dirname, '..')

module.exports = mergeConfig(defaultConfig, {
  projectRoot: __dirname,
  watchFolders: [root],
})
