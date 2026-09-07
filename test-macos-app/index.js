import { AppRegistry, NativeModules, TurboModuleRegistry } from 'react-native'
import App from './App.tsx'
import { name as appName } from './app.json'
import { installJsi } from 'llama.rn'

// Mirror the Windows test app: report progress over HTTP + write a marker so
// CI (or a local run) can assert the JSI bindings actually install on macOS.
function report(msg) {
  console.log('[llama.rn]', msg)
  try {
    fetch('http://127.0.0.1:8083/log', {
      method: 'POST',
      headers: { 'Content-Type': 'text/plain' },
      body: String(msg),
    }).catch(() => {})
  } catch (_) {}
}

report('index.js loaded')
report('TurboModuleRegistry.get(RNLlama): ' + (TurboModuleRegistry.get('RNLlama') ? 'found' : 'null'))
report('NativeModules.RNLlama: ' + (NativeModules && NativeModules.RNLlama ? 'found' : 'null'))

installJsi()
  .then(() => {
    report('installJsi() SUCCEEDED!')
    report('llamaInitContext type after install: ' + typeof global.llamaInitContext)
    try {
      fetch('http://127.0.0.1:8083/success', { method: 'POST', body: 'ok' }).catch(() => {})
    } catch (_) {}
  })
  .catch((e) => {
    report('installJsi() FAILED: ' + (e && e.stack ? e.stack : e))
  })

AppRegistry.registerComponent(appName, () => App)