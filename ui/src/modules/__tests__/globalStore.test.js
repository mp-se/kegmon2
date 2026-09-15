// Unmock the real globalStore so coverage runs against actual code
// (setup.js mocks @/modules/globalStore globally for component tests)
vi.doUnmock('../globalStore')

import { describe, it, expect, beforeEach, vi } from 'vitest'
import { createPinia, setActivePinia } from 'pinia'
import { sharedHttpClient as http } from '@mp-se/espframework-ui-components'

describe('globalStore', () => {
  let store
  let useGlobalStore

  beforeEach(async () => {
    setActivePinia(createPinia())
    const module = await import('../globalStore')
    useGlobalStore = module.useGlobalStore
    store = useGlobalStore()
    vi.clearAllMocks()
  })

  // --- Store creation ---
  it('can create store instance', () => {
    expect(store).toBeDefined()
    expect(store.$id).toBe('global')
  })

  // --- Default state ---
  it('initializes with empty id and platform', () => {
    expect(store.id).toBe('')
    expect(store.platform).toBe('')
  })

  it('initializes with initialized false', () => {
    expect(store.initialized).toBe(false)
  })

  it('initializes with disabled false', () => {
    expect(store.disabled).toBe(false)
  })

  it('initializes with configChanged false', () => {
    expect(store.configChanged).toBe(false)
  })

  it('initializes with empty messages', () => {
    expect(store.messageError).toBe('')
    expect(store.messageWarning).toBe('')
    expect(store.messageSuccess).toBe('')
    expect(store.messageInfo).toBe('')
  })

  it('initializes ui feature flags as false', () => {
    expect(store.ui.enableVoltageFragment).toBe(false)
    expect(store.ui.enableManualWifiEntry).toBe(false)
    expect(store.ui.enableScanForStrongestAp).toBe(false)
  })

  it('initializes device feature flags as false', () => {
    expect(store.feature.ble).toBe(false)
    expect(store.feature.tft).toBe(false)
    expect(store.feature.sd).toBe(false)
  })

  it('initializes no_scales to 4', () => {
    expect(store.feature.no_scales).toBe(4)
  })

  // --- clearMessages action ---
  it('clearMessages clears all message fields', () => {
    store.messageError = 'error'
    store.messageWarning = 'warn'
    store.messageSuccess = 'ok'
    store.messageInfo = 'info'
    store.clearMessages()
    expect(store.messageError).toBe('')
    expect(store.messageWarning).toBe('')
    expect(store.messageSuccess).toBe('')
    expect(store.messageInfo).toBe('')
  })

  it('clearMessages works when all messages are already empty', () => {
    store.clearMessages()
    expect(store.messageError).toBe('')
  })

  // --- Getters: isError / isWarning / isSuccess / isInfo ---
  it('isError returns true when messageError is set', () => {
    store.messageError = 'something went wrong'
    expect(store.isError).toBe(true)
  })

  it('isError returns false when messageError is empty', () => {
    expect(store.isError).toBe(false)
  })

  it('isWarning returns true when messageWarning is set', () => {
    store.messageWarning = 'be careful'
    expect(store.isWarning).toBe(true)
  })

  it('isWarning returns false when empty', () => {
    expect(store.isWarning).toBe(false)
  })

  it('isSuccess returns true when messageSuccess is set', () => {
    store.messageSuccess = 'all good'
    expect(store.isSuccess).toBe(true)
  })

  it('isSuccess returns false when empty', () => {
    expect(store.isSuccess).toBe(false)
  })

  it('isInfo returns true when messageInfo is set', () => {
    store.messageInfo = 'fyi'
    expect(store.isInfo).toBe(true)
  })

  it('isInfo returns false when empty', () => {
    expect(store.isInfo).toBe(false)
  })

  // --- Getter: fetchTimout ---
  it('fetchTimout returns a number from http.timeout', () => {
    expect(typeof store.fetchTimout).toBe('number')
  })

  // --- Getter: uiVersion / uiBuild ---
  it('uiVersion returns undefined or a string', () => {
    expect(store.uiVersion === undefined || typeof store.uiVersion === 'string').toBe(true)
  })

  it('uiBuild returns undefined or a string', () => {
    expect(store.uiBuild === undefined || typeof store.uiBuild === 'string').toBe(true)
  })

  // --- load action ---
  it('load succeeds and updates state fields', async () => {
    const mockData = {
      board: 'kegmon',
      app_ver: '2.0.0',
      app_build: '42',
      platform: 'esp32',
      firmware_file: 'Kegmon.BIN',
      ble: true,
      no_scales: 4,
      tft: false,
      sd_mounted: true
    }
    http.getJson.mockResolvedValueOnce(mockData)
    const result = await store.load()
    expect(result).toBe(true)
    expect(store.board).toBe('KEGMON')
    expect(store.app_ver).toBe('2.0.0')
    expect(store.app_build).toBe('42')
    expect(store.platform).toBe('ESP32')
    expect(store.firmware_file).toBe('kegmon.bin')
    expect(store.feature.ble).toBe(true)
    expect(store.feature.no_scales).toBe(4)
    expect(store.feature.tft).toBe(false)
    expect(store.feature.sd).toBe(true)
  })

  it('load returns false when http call throws', async () => {
    http.getJson.mockRejectedValueOnce(new Error('network error'))
    const result = await store.load()
    expect(result).toBe(false)
  })
})
