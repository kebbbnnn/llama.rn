import { AppRegistry } from 'react-native'
import App from './App.tsx'
import { name as appName } from './app.json'
import { installJsi } from '../src/index.ts' // llama.rn public API (local root source)

// Log install success/failure so the CI run can assert it.
installJsi()
  .then(() => console.log('[llama.rn] JSI installed OK on Windows'))
  .catch((e) => console.error('[llama.rn] JSI install FAILED:', e))

AppRegistry.registerComponent(appName, () => App)
