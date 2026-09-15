// Unmock the real statusStore so coverage runs against actual code
vi.doUnmock('../statusStore')

import { describe, it, expect, beforeEach, vi } from 'vitest'
import { createPinia, setActivePinia } from 'pinia'
import { sharedHttpClient as http } from '@mp-se/espframework-ui-components'

// Mock @/modules/pinia to provide the `global` object statusStore reads from
vi.mock('@/modules/pinia', () => ({
  global: {
    disabled: false,
    initialized: true,
    feature: { no_scales: 4 }
  },
  saveConfigState: vi.fn(),
  getConfigChanges: vi.fn(() => ({}))
}))

// Minimal configStore mock — statusStore only calls useConfigStore() for unit conversions
vi.mock('../configStore', () => ({
  useConfigStore: vi.fn(() => ({
    isWeightKg: true,
    isWeightLbs: false,
    isTempC: true,
    isTempF: false,
    isVolumeCl: true,
    isVolumeUsOz: false,
    isVolumeUkOz: false
  }))
}))

import { global } from '@/modules/pinia'

const makeStatusJson = () => ({
  id: 'abc123',
  mdns: 'kegmon-test',
  wifi_ssid: 'TestWifi',
  weight_unit: 'kg',
  volume_unit: 'cl',
  temp_unit: 'C',
  rssi: -55,
  total_heap: 256,
  free_heap: 128,
  ip: '192.168.1.100',
  wifi_setup: true,
  scale_busy: false,
  uptime_seconds: 30,
  uptime_minutes: 5,
  uptime_hours: 2,
  uptime_days: 1,
  sd_mounted: true,
  scales: [
    {
      scale_factor: 1,
      stable_weight: 10.5,
      stable_volume: 50.0,
      pouring_volume: 5.0,
      last_pour_volume: 3.0,
      keg_volume: 30.0,
      glass: 0.5
    },
    {
      scale_factor: 2,
      stable_weight: 8.0,
      stable_volume: 40.0,
      pouring_volume: 4.0,
      last_pour_volume: 2.0,
      keg_volume: 25.0,
      glass: 0.5
    },
    {
      scale_factor: 3,
      stable_weight: 0,
      stable_volume: 0,
      pouring_volume: 0,
      last_pour_volume: 0,
      keg_volume: 0,
      glass: 0
    },
    {
      scale_factor: 4,
      stable_weight: 0,
      stable_volume: 0,
      pouring_volume: 0,
      last_pour_volume: 0,
      keg_volume: 0,
      glass: 0
    }
  ],
  sensors: [{ id: 's1', temperature: 20.5 }],
  ha: { enabled: true },
  brewspy: { tokens: [] },
  barhelper: {},
  brewlogger: { url: '' },
  recent_events: [
    { unit: 0, name: 'pour_completed', timestamp_ms: 1000, data: { volume: 0.33 } },
    { unit: 0, name: 'stable_level', timestamp_ms: 2000, data: {} },
    { unit: 1, name: 'keg_removed', timestamp_ms: 500, data: {} }
  ]
})

describe('statusStore', () => {
  let store
  let useStatusStore

  beforeEach(async () => {
    setActivePinia(createPinia())
    const module = await import('../statusStore')
    useStatusStore = module.useStatusStore
    store = useStatusStore()
    global.feature.no_scales = 4
    vi.clearAllMocks()
  })

  // --- Store creation ---
  it('can create store instance', () => {
    expect(store).toBeDefined()
    expect(store.$id).toBe('status')
  })

  // --- Default state ---
  it('initializes connected as false', () => {
    expect(store.connected).toBe(false)
  })

  it('initializes id as empty string', () => {
    expect(store.id).toBe('')
  })

  it('initializes scales as empty array', () => {
    expect(store.scales).toEqual([])
  })

  it('initializes sensors as empty array', () => {
    expect(store.sensors).toEqual([])
  })

  it('initializes events as empty array', () => {
    expect(store.events).toEqual([])
  })

  // --- Getters: weight unit ---
  it('isWeightKg returns true when weight_unit is kg', () => {
    store.weight_unit = 'kg'
    expect(store.isWeightKg).toBe(true)
  })

  it('isWeightLbs returns true when weight_unit is lbs', () => {
    store.weight_unit = 'lbs'
    expect(store.isWeightLbs).toBe(true)
  })

  // --- Getters: temperature unit ---
  it('isTempC returns true when temp_unit is C', () => {
    store.temp_unit = 'C'
    expect(store.isTempC).toBe(true)
  })

  it('isTempF returns true when temp_unit is F', () => {
    store.temp_unit = 'F'
    expect(store.isTempF).toBe(true)
  })

  // --- Getters: volume unit ---
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

  // --- load action ---
  it('load populates state from api/status', async () => {
    http.getJson.mockResolvedValueOnce(makeStatusJson())
    const result = await store.load()
    expect(result).toBe(true)
    expect(store.connected).toBe(true)
    expect(store.id).toBe('abc123')
    expect(store.mdns).toBe('kegmon-test')
    expect(store.wifi_ssid).toBe('TestWifi')
    expect(store.rssi).toBe(-55)
    expect(store.ip).toBe('192.168.1.100')
    expect(store.wifi_setup).toBe(true)
    expect(store.sd_mounted).toBe(true)
    expect(store.uptime_days).toBe(1)
    expect(store.uptime_hours).toBe(2)
    expect(store.uptime_minutes).toBe(5)
    expect(store.uptime_seconds).toBe(30)
  })

  it('load trims scales to no_scales from global.feature', async () => {
    global.feature.no_scales = 2
    http.getJson.mockResolvedValueOnce(makeStatusJson())
    await store.load()
    expect(store.scales).toHaveLength(2)
  })

  it('load populates scales with converted weight values', async () => {
    http.getJson.mockResolvedValueOnce(makeStatusJson())
    await store.load()
    // kg mode: value is same, fixed to 2 decimals
    expect(store.scales[0].stable_weight).toBe('10.50')
  })

  it('load populates scales volume values', async () => {
    http.getJson.mockResolvedValueOnce(makeStatusJson())
    await store.load()
    // cl mode: value is same, fixed to 0 decimals
    expect(store.scales[0].stable_volume).toBe('50')
  })

  it('load populates sensors array', async () => {
    http.getJson.mockResolvedValueOnce(makeStatusJson())
    await store.load()
    expect(store.sensors).toHaveLength(1)
    expect(store.sensors[0].id).toBe('s1')
  })

  it('load populates push status objects when present', async () => {
    http.getJson.mockResolvedValueOnce(makeStatusJson())
    await store.load()
    expect(store.ha).toEqual({ enabled: true })
    expect(store.sd_mounted).toBe(true)
  })

  it('load populates events from recent_events', async () => {
    http.getJson.mockResolvedValueOnce(makeStatusJson())
    await store.load()
    expect(store.events).toHaveLength(3)
  })

  it('load returns false when http call throws', async () => {
    http.getJson.mockRejectedValueOnce(new Error('network error'))
    const result = await store.load()
    expect(result).toBe(false)
  })

  it('load handles missing ip gracefully', async () => {
    const json = makeStatusJson()
    delete json.ip
    http.getJson.mockResolvedValueOnce(json)
    await store.load()
    expect(store.ip).toBe('')
  })

  // --- _convertWeight ---
  it('_convertWeight converts kg to lbs when config.isWeightLbs', () => {
    const configLbs = { isWeightLbs: true }
    const result = store._convertWeight(1, configLbs)
    expect(parseFloat(result)).toBeCloseTo(2.2, 1)
  })

  it('_convertWeight returns same value for kg', () => {
    const configKg = { isWeightLbs: false }
    const result = store._convertWeight(5, configKg)
    expect(parseFloat(result)).toBe(5)
  })

  it('_convertWeight returns 0 for null input', () => {
    expect(store._convertWeight(null, { isWeightLbs: false })).toBe(0)
  })

  // --- _convertVolume ---
  it('_convertVolume converts to US oz when config.isVolumeUsOz', () => {
    const configUsOz = { isVolumeUsOz: true, isVolumeUkOz: false }
    const result = store._convertVolume(100, configUsOz)
    expect(parseFloat(result)).toBeCloseTo(33.8, 0)
  })

  it('_convertVolume converts to UK oz when config.isVolumeUkOz', () => {
    const configUkOz = { isVolumeUsOz: false, isVolumeUkOz: true }
    const result = store._convertVolume(100, configUkOz)
    expect(parseFloat(result)).toBeCloseTo(35.1, 0)
  })

  it('_convertVolume returns cl unchanged', () => {
    const configCl = { isVolumeUsOz: false, isVolumeUkOz: false }
    const result = store._convertVolume(50, configCl)
    expect(parseFloat(result)).toBe(50)
  })

  it('_convertVolume returns 0 for null input', () => {
    expect(store._convertVolume(null, { isVolumeUsOz: false, isVolumeUkOz: false })).toBe(0)
  })

  // --- _convertTemperature ---
  it('_convertTemperature converts C to F when config.isTempF', () => {
    const configF = { isTempF: true }
    const result = store._convertTemperature(0, configF)
    expect(parseFloat(result)).toBeCloseTo(32, 1)
  })

  it('_convertTemperature returns C unchanged', () => {
    const configC = { isTempF: false }
    const result = store._convertTemperature(20, configC)
    expect(parseFloat(result)).toBe(20)
  })

  it('_convertTemperature returns 0 for null input', () => {
    expect(store._convertTemperature(null, { isTempF: false })).toBe(0)
  })

  // --- getLastEventsForScale ---
  it('getLastEventsForScale returns events for the given scale index', () => {
    store.events = makeStatusJson().recent_events
    const events = store.getLastEventsForScale(0)
    expect(events.length).toBe(2)
    events.forEach((e) => expect(e).toHaveProperty('name'))
  })

  it('getLastEventsForScale returns at most limit events', () => {
    store.events = makeStatusJson().recent_events
    const events = store.getLastEventsForScale(0, 1)
    expect(events).toHaveLength(1)
  })

  it('getLastEventsForScale sorts events newest first', () => {
    store.events = makeStatusJson().recent_events
    const events = store.getLastEventsForScale(0)
    expect(events[0].timestamp_ms).toBeGreaterThan(events[1].timestamp_ms)
  })

  it('getLastEventsForScale maps known event names to readable labels', () => {
    store.events = [{ unit: 0, name: 'pour_completed', timestamp_ms: 1000, data: {} }]
    const events = store.getLastEventsForScale(0)
    expect(events[0].name).toBe('Pour Completed')
  })

  it('getLastEventsForScale falls back to raw name for unknown event types', () => {
    store.events = [{ unit: 0, name: 'some_unknown_event', timestamp_ms: 1000, data: {} }]
    const events = store.getLastEventsForScale(0)
    expect(events[0].name).toBe('some_unknown_event')
  })

  it('getLastEventsForScale returns empty array when no events match', () => {
    store.events = []
    expect(store.getLastEventsForScale(0)).toEqual([])
  })

  it('getLastEventsForScale handles null events gracefully', () => {
    store.events = null
    const events = store.getLastEventsForScale(0)
    expect(events).toEqual([])
  })

  it('getLastEventsForScale returns empty data object when event has no data', () => {
    store.events = [{ unit: 0, name: 'pouring', timestamp_ms: 1000 }]
    const events = store.getLastEventsForScale(0)
    expect(events[0].data).toEqual({})
  })

  it('load skips temperature conversion when sensors array is empty', async () => {
    const json = makeStatusJson()
    json.sensors = []
    http.getJson.mockResolvedValueOnce(json)
    await store.load()
    expect(store.sensors).toEqual([])
  })

  // --- getRelativeTime ---
  it('getRelativeTime returns "just now" for recent events', () => {
    store.uptime_seconds = 30
    store.uptime_minutes = 0
    store.uptime_hours = 0
    store.uptime_days = 0
    // event happened 10 seconds ago (uptime 30s, event at 20000ms)
    expect(store.getRelativeTime(20000)).toBe('just now')
  })

  it('getRelativeTime returns minutes ago for events within an hour', () => {
    store.uptime_seconds = 0
    store.uptime_minutes = 10
    store.uptime_hours = 0
    store.uptime_days = 0
    // event happened 5 minutes ago (uptime 10min, event at 5min in ms)
    expect(store.getRelativeTime(5 * 60 * 1000)).toBe('5 min ago')
  })

  it('getRelativeTime returns hours ago for events older than 60 minutes', () => {
    store.uptime_seconds = 0
    store.uptime_minutes = 0
    store.uptime_hours = 3
    store.uptime_days = 0
    // event happened 2 hours ago
    expect(store.getRelativeTime(1 * 60 * 60 * 1000)).toBe('2h ago')
  })

  it('getRelativeTime returns days ago for events older than 24 hours', () => {
    store.uptime_seconds = 0
    store.uptime_minutes = 0
    store.uptime_hours = 0
    store.uptime_days = 3
    // event happened 2 days ago
    expect(store.getRelativeTime(1 * 24 * 60 * 60 * 1000)).toBe('2d ago')
  })
})
