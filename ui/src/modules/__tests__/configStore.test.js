// Unmock the real configStore so coverage runs against actual code
vi.doUnmock('../configStore')

import { describe, it, expect, beforeEach, vi } from 'vitest'
import { createPinia, setActivePinia } from 'pinia'
import { sharedHttpClient as http } from '@mp-se/espframework-ui-components'

// Mock @/modules/pinia with a minimal implementation to avoid circular deps
// and provide the global state that configStore actions write to
vi.mock('@/modules/pinia', () => ({
  global: {
    disabled: false,
    configChanged: false,
    initialized: true,
    clearMessages: vi.fn(),
    messageSuccess: '',
    messageError: '',
    messageWarning: '',
    feature: { no_scales: 4 }
  },
  saveConfigState: vi.fn(),
  getConfigChanges: vi.fn(() => ({}))
}))

import { global, getConfigChanges } from '@/modules/pinia'

const makeMockConfig = () => ({
  id: 'abc',
  mdns: 'kegmon',
  temp_unit: 'C',
  weight_unit: 'kg',
  volume_unit: 'cl',
  dark_mode: false,
  display_layout: 0,
  ota_url: 'http://ota.example.com',
  wifi_portal_timeout: 30,
  wifi_connect_timeout: 10,
  wifi_ssid: 'MyWifi',
  wifi_ssid2: '',
  wifi_pass: 'secret',
  wifi_pass2: '',
  brewfather_apikey: 'bf_key',
  brewfather_userkey: 'bf_user',
  brewlogger_url: 'http://log',
  brewspy_tokens: ['t1', 't2', 't3', 't4'],
  barhelper_apikey: 'bh_key',
  barhelper_monitors: ['m1', 'm2', 'm3', 'm4'],
  push_timeout: 15,
  http_post_target: 'http://post1',
  http_post_header1: 'h1',
  http_post_header2: 'h2',
  http_post2_target: 'http://post2',
  http_post2_header1: 'h3',
  http_post2_header2: 'h4',
  http_get_target: 'http://get',
  http_get_header1: 'hg1',
  http_get_header2: 'hg2',
  influxdb2_target: 'http://influx',
  influxdb2_org: 'myorg',
  influxdb2_bucket: 'mybucket',
  influxdb2_token: 'mytoken',
  mqtt_target: 'mqtt.host',
  mqtt_port: 1883,
  mqtt_user: 'mqttuser',
  mqtt_pass: 'mqttpass',
  scales: [
    {
      scale_factor: 1,
      scale_offset: 0,
      keg_weight: 10,
      keg_volume: 20,
      glass_volume: 0.5,
      temp_sensor_id: 's1'
    },
    {
      scale_factor: 2,
      scale_offset: 0,
      keg_weight: 10,
      keg_volume: 20,
      glass_volume: 0.5,
      temp_sensor_id: 's2'
    },
    {
      scale_factor: 3,
      scale_offset: 0,
      keg_weight: 10,
      keg_volume: 20,
      glass_volume: 0.5,
      temp_sensor_id: 's3'
    },
    {
      scale_factor: 4,
      scale_offset: 0,
      keg_weight: 10,
      keg_volume: 20,
      glass_volume: 0.5,
      temp_sensor_id: 's4'
    }
  ],
  beers: [
    { beer_name: 'IPA', beer_id: 'b1', beer_abv: 5.5, beer_fg: 1.01, beer_ebc: 8, beer_ibu: 40 },
    { beer_name: 'Lager', beer_id: 'b2', beer_abv: 4.5, beer_fg: 1.01, beer_ebc: 4, beer_ibu: 20 },
    { beer_name: '', beer_id: '', beer_abv: 0, beer_fg: 0, beer_ebc: 0, beer_ibu: 0 },
    { beer_name: '', beer_id: '', beer_abv: 0, beer_fg: 0, beer_ebc: 0, beer_ibu: 0 }
  ]
})

describe('configStore', () => {
  let store
  let useConfigStore

  beforeEach(async () => {
    setActivePinia(createPinia())
    const configModule = await import('../configStore')
    useConfigStore = configModule.useConfigStore
    store = useConfigStore()
    // Reset global mock state between tests
    global.disabled = false
    global.messageError = ''
    global.messageWarning = ''
    global.messageSuccess = ''
    vi.clearAllMocks()
    // Default: no pending changes
    getConfigChanges.mockReturnValue({})
  })

  // --- Store creation ---
  it('can create store instance', () => {
    expect(store).toBeDefined()
    expect(store.$id).toBe('config')
  })

  // --- Default state ---
  it('initializes with empty string fields', () => {
    expect(store.id).toBe('')
    expect(store.mdns).toBe('')
    expect(store.temp_unit).toBe('')
    expect(store.weight_unit).toBe('')
    expect(store.volume_unit).toBe('')
    expect(store.ota_url).toBe('')
    expect(store.wifi_ssid).toBe('')
    expect(store.mqtt_target).toBe('')
  })

  it('initializes dark_mode as false', () => {
    expect(store.dark_mode).toBe(false)
  })

  it('initializes mqtt_port as 1883', () => {
    expect(store.mqtt_port).toBe(1883)
  })

  it('initializes brewspy_tokens as array of 4 empty strings', () => {
    expect(store.brewspy_tokens).toEqual(['', '', '', ''])
  })

  it('initializes barhelper_monitors as array of 4 empty strings', () => {
    expect(store.barhelper_monitors).toEqual(['', '', '', ''])
  })

  it('initializes scales as array of 4 entries with zero values', () => {
    expect(store.scales).toHaveLength(4)
    expect(store.scales[0].scale_factor).toBe(0)
  })

  it('initializes beers as array of 4 entries', () => {
    expect(store.beers).toHaveLength(4)
    expect(store.beers[0].beer_name).toBe('')
  })

  // --- Getters ---
  it('getVolumeUnit returns "cl" when volume_unit is cl', () => {
    store.volume_unit = 'cl'
    expect(store.getVolumeUnit).toBe('cl')
  })

  it('getVolumeUnit returns "fl. oz" for non-cl unit', () => {
    store.volume_unit = 'us-oz'
    expect(store.getVolumeUnit).toBe('fl. oz')
  })

  it('getWeightUnit returns "kg" when weight_unit is kg', () => {
    store.weight_unit = 'kg'
    expect(store.getWeightUnit).toBe('kg')
  })

  it('getWeightUnit returns "lbs" for non-kg unit', () => {
    store.weight_unit = 'lbs'
    expect(store.getWeightUnit).toBe('lbs')
  })

  it('getTempUnit returns "°C" when temp_unit is C', () => {
    store.temp_unit = 'C'
    expect(store.getTempUnit).toBe('°C')
  })

  it('getTempUnit returns "°F" for non-C unit', () => {
    store.temp_unit = 'F'
    expect(store.getTempUnit).toBe('°F')
  })

  it('isWeightKg returns true when weight_unit is kg', () => {
    store.weight_unit = 'kg'
    expect(store.isWeightKg).toBe(true)
  })

  it('isWeightLbs returns true when weight_unit is lbs', () => {
    store.weight_unit = 'lbs'
    expect(store.isWeightLbs).toBe(true)
  })

  it('isTempC returns true when temp_unit is C', () => {
    store.temp_unit = 'C'
    expect(store.isTempC).toBe(true)
  })

  it('isTempF returns true when temp_unit is F', () => {
    store.temp_unit = 'F'
    expect(store.isTempF).toBe(true)
  })

  it('isVolumeCl returns true when volume_unit is cl', () => {
    store.volume_unit = 'cl'
    expect(store.isVolumeCl).toBe(true)
  })

  it('isVolumeUsOz returns true when volume_unit is us-oz', () => {
    store.volume_unit = 'us-oz'
    expect(store.isVolumeUsOz).toBe(true)
  })

  it('isVolumeUkOz returns true when volume_unit is uk-oz', () => {
    store.volume_unit = 'uk-oz'
    expect(store.isVolumeUkOz).toBe(true)
  })

  // --- toJson action ---
  it('toJson returns a JSON string of state', () => {
    store.mdns = 'kegmon-test'
    const json = store.toJson()
    const parsed = JSON.parse(json)
    expect(parsed.mdns).toBe('kegmon-test')
  })

  it('toJson includes all state keys', () => {
    const json = store.toJson()
    const parsed = JSON.parse(json)
    expect(parsed).toHaveProperty('scales')
    expect(parsed).toHaveProperty('beers')
    expect(parsed).toHaveProperty('wifi_ssid')
  })

  // --- load action ---
  it('load populates state from api/config response', async () => {
    const mockData = makeMockConfig()
    http.getJson.mockResolvedValueOnce(mockData)
    const result = await store.load()
    expect(result).toBe(true)
    expect(store.id).toBe('abc')
    expect(store.mdns).toBe('kegmon')
    expect(store.temp_unit).toBe('C')
    expect(store.weight_unit).toBe('kg')
    expect(store.volume_unit).toBe('cl')
    expect(store.wifi_ssid).toBe('MyWifi')
    expect(store.mqtt_port).toBe(1883)
    expect(store.brewspy_tokens).toEqual(['t1', 't2', 't3', 't4'])
    expect(store.scales).toHaveLength(4)
    expect(store.scales[0].scale_factor).toBe(1)
    expect(store.beers[0].beer_name).toBe('IPA')
  })

  it('load returns false when http call throws', async () => {
    http.getJson.mockRejectedValueOnce(new Error('network error'))
    const result = await store.load()
    expect(result).toBe(false)
  })

  // --- sendConfig action ---
  it('sendConfig posts changed data and returns true', async () => {
    getConfigChanges.mockReturnValue({ mdns: 'kegmon' })
    http.postJson.mockResolvedValueOnce({})
    const result = await store.sendConfig()
    expect(result).toBe(true)
    expect(http.postJson).toHaveBeenCalledWith('api/config', { mdns: 'kegmon' })
  })

  it('sendConfig returns true when there are no changes (empty diff)', async () => {
    getConfigChanges.mockReturnValue({})
    const result = await store.sendConfig()
    expect(result).toBe(true)
    expect(http.postJson).not.toHaveBeenCalled()
  })

  it('sendConfig returns false when http call throws', async () => {
    getConfigChanges.mockReturnValue({ mdns: 'changed' })
    http.postJson.mockRejectedValueOnce(new Error('post failed'))
    const result = await store.sendConfig()
    expect(result).toBe(false)
  })

  // --- sendPushTest action ---
  it('sendPushTest posts to api/push and returns true', async () => {
    http.postJson.mockResolvedValueOnce({})
    const result = await store.sendPushTest({ target: 'http' })
    expect(result).toBe(true)
    expect(http.postJson).toHaveBeenCalledWith('api/push', { target: 'http' })
  })

  it('sendPushTest returns false when http throws', async () => {
    http.postJson.mockRejectedValueOnce(new Error('fail'))
    const result = await store.sendPushTest({})
    expect(result).toBe(false)
  })

  // --- getPushTestStatus action ---
  it('getPushTestStatus returns success with data', async () => {
    const mockStatus = { status: false, success: true, push_enabled: true }
    http.getJson.mockResolvedValueOnce(mockStatus)
    const result = await store.getPushTestStatus()
    expect(result.success).toBe(true)
    expect(result.data).toEqual(mockStatus)
  })

  it('getPushTestStatus returns false on error', async () => {
    http.getJson.mockRejectedValueOnce(new Error('fail'))
    const result = await store.getPushTestStatus()
    expect(result.success).toBe(false)
    expect(result.data).toBeNull()
  })

  // --- sendWifiScan action ---
  it('sendWifiScan calls api/wifi and returns true', async () => {
    http.request.mockResolvedValueOnce({})
    const result = await store.sendWifiScan()
    expect(result).toBe(true)
    expect(http.request).toHaveBeenCalledWith('api/wifi')
  })

  it('sendWifiScan returns false on error', async () => {
    http.request.mockRejectedValueOnce(new Error('fail'))
    const result = await store.sendWifiScan()
    expect(result).toBe(false)
  })

  // --- getWifiScanStatus action ---
  it('getWifiScanStatus returns success with data', async () => {
    const mockData = { status: false, success: true, networks: [] }
    http.getJson.mockResolvedValueOnce(mockData)
    const result = await store.getWifiScanStatus()
    expect(result.success).toBe(true)
    expect(result.data).toEqual(mockData)
  })

  it('getWifiScanStatus returns false on error', async () => {
    http.getJson.mockRejectedValueOnce(new Error('fail'))
    const result = await store.getWifiScanStatus()
    expect(result.success).toBe(false)
  })

  // --- sendHardwareScan action ---
  it('sendHardwareScan calls api/hardware and returns true', async () => {
    http.request.mockResolvedValueOnce({})
    const result = await store.sendHardwareScan()
    expect(result).toBe(true)
    expect(http.request).toHaveBeenCalledWith('api/hardware')
  })

  it('sendHardwareScan returns false on error', async () => {
    http.request.mockRejectedValueOnce(new Error('fail'))
    const result = await store.sendHardwareScan()
    expect(result).toBe(false)
  })

  // --- getHardwareScanStatus action ---
  it('getHardwareScanStatus returns success with data', async () => {
    const mockData = { status: false, success: true, sensors: [] }
    http.getJson.mockResolvedValueOnce(mockData)
    const result = await store.getHardwareScanStatus()
    expect(result.success).toBe(true)
    expect(result.data).toEqual(mockData)
  })

  it('getHardwareScanStatus returns false on error', async () => {
    http.getJson.mockRejectedValueOnce(new Error('fail'))
    const result = await store.getHardwareScanStatus()
    expect(result.success).toBe(false)
  })

  // --- sendFilesystemRequest action ---
  it('sendFilesystemRequest returns success with text', async () => {
    http.filesystemRequest.mockResolvedValueOnce({ success: true, text: 'file content' })
    const result = await store.sendFilesystemRequest({ action: 'list' })
    expect(result.success).toBe(true)
    expect(result.text).toBe('file content')
  })

  it('sendFilesystemRequest returns false on error', async () => {
    http.filesystemRequest.mockRejectedValueOnce(new Error('fail'))
    const result = await store.sendFilesystemRequest({})
    expect(result.success).toBe(false)
    expect(result.text).toBe('')
  })

  // --- saveAll action ---
  it('saveAll returns true and sets messageSuccess on sendConfig success', async () => {
    // sendConfig will see empty changes → returns true immediately
    const result = await store.saveAll()
    expect(result).toBe(true)
    expect(global.messageSuccess).toBeTruthy()
  })

  it('saveAll returns false and sets messageError when sendConfig fails', async () => {
    // Force a real POST that fails by returning non-empty changes
    getConfigChanges.mockReturnValue({ mdns: 'changed' })
    http.postJson.mockRejectedValueOnce(new Error('fail'))
    const result = await store.saveAll()
    expect(result).toBe(false)
    expect(global.messageError).toBeTruthy()
  })

  // --- restart action ---
  it('restart sets messageSuccess when res.success and res.json.status is true', async () => {
    http.restart.mockResolvedValueOnce({
      success: true,
      json: { status: true, message: 'Restarting' }
    })
    store.mdns = 'kegmon'
    await store.restart()
    expect(global.messageSuccess).toContain('Redirecting to http://kegmon.local')
  })

  it('restart sets messageError from json.message when res.success but status is not true', async () => {
    http.restart.mockResolvedValueOnce({
      success: true,
      json: { status: false, message: 'Not ready' }
    })
    await store.restart()
    expect(global.messageError).toBe('Not ready')
  })

  it('restart sets fallback messageError when res.json.message is missing', async () => {
    http.restart.mockResolvedValueOnce({ success: true, json: { status: false } })
    await store.restart()
    expect(global.messageError).toBe('Failed to restart device')
  })

  it('restart sets messageError when res.success is false', async () => {
    http.restart.mockResolvedValueOnce({ success: false })
    await store.restart()
    expect(global.messageError).toBe('Failed to request restart')
  })

  it('restart sets messageError when http.restart throws', async () => {
    http.restart.mockRejectedValueOnce(new Error('timeout'))
    await store.restart()
    expect(global.messageError).toBe('Failed to do restart')
  })

  it('restart always re-enables global after completion', async () => {
    http.restart.mockResolvedValueOnce({ success: true, json: { status: true, message: '' } })
    await store.restart()
    expect(global.disabled).toBe(false)
  })

  // --- runPushTest action ---
  it('runPushTest returns false when sendPushTest fails', async () => {
    http.postJson.mockRejectedValueOnce(new Error('fail'))
    const result = await store.runPushTest({ target: 'http' })
    expect(result.success).toBe(false)
    expect(global.messageError).toBeTruthy()
  })

  it('runPushTest sets messageSuccess on successful push test', async () => {
    vi.useFakeTimers()
    http.postJson.mockResolvedValueOnce({})
    http.getJson.mockResolvedValueOnce({ status: false, success: true, push_enabled: true })
    const promise = store.runPushTest({ target: 'http' })
    await vi.runAllTimersAsync()
    const result = await promise
    vi.useRealTimers()
    expect(result.success).toBe(true)
    expect(global.messageSuccess).toBeTruthy()
  })

  it('runPushTest sets messageWarning when push_enabled is false', async () => {
    vi.useFakeTimers()
    http.postJson.mockResolvedValueOnce({})
    http.getJson.mockResolvedValueOnce({ status: false, success: false, push_enabled: false })
    const promise = store.runPushTest({})
    await vi.runAllTimersAsync()
    const result = await promise
    vi.useRealTimers()
    expect(result.success).toBe(true)
    expect(global.messageWarning).toBeTruthy()
  })

  it('runPushTest sets messageError when push test fails with error code', async () => {
    vi.useFakeTimers()
    http.postJson.mockResolvedValueOnce({})
    http.getJson.mockResolvedValueOnce({
      status: false,
      success: false,
      push_enabled: true,
      last_error: 5
    })
    http.getErrorString = vi.fn(() => 'Error 5')
    const promise = store.runPushTest({})
    await vi.runAllTimersAsync()
    const result = await promise
    vi.useRealTimers()
    expect(result.success).toBe(true)
    expect(global.messageError).toBeTruthy()
  })

  it('runPushTest returns false when getPushTestStatus fails', async () => {
    vi.useFakeTimers()
    http.postJson.mockResolvedValueOnce({})
    http.getJson.mockRejectedValueOnce(new Error('status fail'))
    const promise = store.runPushTest({})
    await vi.runAllTimersAsync()
    const result = await promise
    vi.useRealTimers()
    expect(result.success).toBe(false)
    expect(global.messageError).toBeTruthy()
  })

  it('runPushTest polls until status is false before resolving', async () => {
    vi.useFakeTimers()
    http.postJson.mockResolvedValueOnce({})
    // First poll: still running (status: true); second: done
    http.getJson
      .mockResolvedValueOnce({ status: true })
      .mockResolvedValueOnce({ status: false, success: true, push_enabled: true })
    const promise = store.runPushTest({})
    // advance past both 2s waits
    await vi.runAllTimersAsync()
    const result = await promise
    vi.useRealTimers()
    expect(result.success).toBe(true)
    expect(http.getJson).toHaveBeenCalledTimes(2)
  })

  // --- runWifiScan action ---
  it('runWifiScan returns false when sendWifiScan fails', async () => {
    http.request.mockRejectedValueOnce(new Error('fail'))
    const result = await store.runWifiScan()
    expect(result.success).toBe(false)
    expect(global.messageError).toBeTruthy()
  })

  it('runWifiScan returns data on success', async () => {
    vi.useFakeTimers()
    http.request.mockResolvedValueOnce({})
    http.getJson.mockResolvedValueOnce({ status: false, success: true, networks: ['net1'] })
    const promise = store.runWifiScan()
    await vi.runAllTimersAsync()
    const result = await promise
    vi.useRealTimers()
    expect(result.success).toBe(true)
    expect(result.data.networks).toEqual(['net1'])
  })

  it('runWifiScan returns false when getWifiScanStatus fails', async () => {
    vi.useFakeTimers()
    http.request.mockResolvedValueOnce({})
    http.getJson.mockRejectedValueOnce(new Error('status fail'))
    const promise = store.runWifiScan()
    await vi.runAllTimersAsync()
    const result = await promise
    vi.useRealTimers()
    expect(result.success).toBe(false)
    expect(global.messageError).toBeTruthy()
  })

  it('runWifiScan polls until status is false', async () => {
    vi.useFakeTimers()
    http.request.mockResolvedValueOnce({})
    http.getJson
      .mockResolvedValueOnce({ status: true })
      .mockResolvedValueOnce({ status: false, success: true, networks: [] })
    const promise = store.runWifiScan()
    await vi.runAllTimersAsync()
    const result = await promise
    vi.useRealTimers()
    expect(result.success).toBe(true)
    expect(http.getJson).toHaveBeenCalledTimes(2)
  })

  // --- runHardwareScan action ---
  it('runHardwareScan returns false when sendHardwareScan fails', async () => {
    http.request.mockRejectedValueOnce(new Error('fail'))
    const result = await store.runHardwareScan()
    expect(result.success).toBe(false)
    expect(global.messageError).toBeTruthy()
  })

  it('runHardwareScan returns data on success', async () => {
    vi.useFakeTimers()
    http.request.mockResolvedValueOnce({})
    http.getJson.mockResolvedValueOnce({ status: false, success: true, sensors: ['s1'] })
    const promise = store.runHardwareScan()
    await vi.runAllTimersAsync()
    const result = await promise
    vi.useRealTimers()
    expect(result.success).toBe(true)
    expect(result.data.sensors).toEqual(['s1'])
  })

  it('runHardwareScan returns false when getHardwareScanStatus fails', async () => {
    vi.useFakeTimers()
    http.request.mockResolvedValueOnce({})
    http.getJson.mockRejectedValueOnce(new Error('status fail'))
    const promise = store.runHardwareScan()
    await vi.runAllTimersAsync()
    const result = await promise
    vi.useRealTimers()
    expect(result.success).toBe(false)
    expect(global.messageError).toBeTruthy()
  })

  it('runHardwareScan polls until status is false', async () => {
    vi.useFakeTimers()
    http.request.mockResolvedValueOnce({})
    http.getJson
      .mockResolvedValueOnce({ status: true })
      .mockResolvedValueOnce({ status: false, success: true, sensors: [] })
    const promise = store.runHardwareScan()
    await vi.runAllTimersAsync()
    const result = await promise
    vi.useRealTimers()
    expect(result.success).toBe(true)
    expect(http.getJson).toHaveBeenCalledTimes(2)
  })
})
