import type { TurboModule } from 'react-native'
import { NativeModules, TurboModuleRegistry } from 'react-native'

export interface Spec extends TurboModule {
  install(): Promise<boolean>
}

const getRNLlama = (): Spec | null =>
  (TurboModuleRegistry.get<Spec>('RNLlama') ?? (NativeModules as any)?.RNLlama) ?? null

export default {
  install: () => {
    const mod = getRNLlama()
    if (!mod) {
      return Promise.reject(
        new Error(
          'RNLlama native module is not available (neither TurboModule nor NativeModule found)',
        ),
      )
    }
    return mod.install()
  },
} as Spec
