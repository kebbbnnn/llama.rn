import { AppRegistry } from 'react-native'
import App from './App.tsx'
import { name as appName } from './app.json'
import { installJsi } from 'llama.rn'

// Log install success/failure so the CI run can assert it.
console.log('[llama.rn] Triggering installJsi on Windows...')
installJsi()
  .then(() => console.log('[llama.rn] JSI installed OK on Windows'))
  .catch((e) => console.error('[llama.rn] JSI install FAILED:', e && e.stack ? e.stack : e))

AppRegistry.registerComponent(appName, () => App)
