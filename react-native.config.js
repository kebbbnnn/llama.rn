/**
 * react-native-windows autolinking config for llama.rn itself.
 *
 * This dependency-side declaration (per the RNW "React Native Config Schema")
 * lets RNW's `autolink-windows` / `run-windows` discover llama.rn's Windows
 * native module (windows/RNLlama.vcxproj) and link it into a consuming app.
 *
 * Unlike the example app's config (which is for the mobile targets), this is
 * the library's OWN declaration — RNW reads each dependency's
 * react-native.config.js from node_modules and fuses its `platforms.windows`.
 */
module.exports = {
  dependency: {
    platforms: {
      windows: {
        sourceDir: 'windows',
        solutionFile: 'RNLlama.sln',
        projects: [
          {
            projectFile: 'RNLlama.vcxproj',
            directDependency: true,
            // RNW fuses these into the app's generated package-provider list.
            // Supplying them (rather than leaving undefined) avoids RNW's
            // autolinker crashing with "forEach on undefined" when it iterates
            // them to register the module.
            cppHeaders: ['RNLlama.h'],
            cppPackageProviders: ['RNLlama::ReactPackageProvider'],
            csNamespaces: [],
            csPackageProviders: [],
          },
        ],
      },
    },
  },
}