import { vi } from 'vitest'
import { config as vtConfig } from '@vue/test-utils'

// Mock localStorage before anything else
class LocalStorageMock {
  constructor() {
    this.store = {}
  }

  clear() {
    this.store = {}
  }

  getItem(key) {
    return this.store[key] || null
  }
  setItem(key, value) {
    this.store[key] = String(value)
  }

  removeItem(key) {
    delete this.store[key]
  }

  get length() {
    return Object.keys(this.store).length
  }

  key(index) {
    const keys = Object.keys(this.store)
    return keys[index] || null
  }
}

Object.defineProperty(window, 'localStorage', {
  value: new LocalStorageMock()
})

// Mock pinia stores BEFORE importing anything that uses them
vi.mock('@/modules/configStore', () => ({
  useConfigStore: vi.fn(() => ({
    // Device
    id: 'test-id',
    mdns: 'kegmon-test',
    temp_unit: 'C',
    weight_unit: 'kg',
    volume_unit: 'L',
    dark_mode: false,
    display_layout: 0,
    // Hardware
    ota_url: '',
    // Wifi
    wifi_portal_timeout: 30,
    wifi_connect_timeout: 10,
    wifi_ssid: 'test-ssid',
    wifi_ssid2: '',
    wifi_pass: 'test-pass',
    wifi_pass2: '',
    // Integrations
    brewfather_apikey: '',
    brewfather_userkey: '',
    brewlogger_url: '',
    brewspy_tokens: ['', '', '', ''],
    barhelper_apikey: '',
    barhelper_monitors: ['', '', '', ''],
    // Push - Generic
    push_timeout: 10,
    // Push - Http Post
    http_post_target: '',
    http_post_header1: '',
    http_post_header2: '',
    http_post2_target: '',
    http_post2_header1: '',
    http_post2_header2: '',
    // Push - Http Get
    http_get_target: '',
    http_get_header1: '',
    http_get_header2: '',
    // Push - Influx
    influxdb2_target: '',
    influxdb2_org: '',
    influxdb2_bucket: '',
    influxdb2_token: '',
    // Push - MQTT
    mqtt_target: '',
    mqtt_port: 1883,
    mqtt_user: '',
    mqtt_pass: '',
    // Arrays
    scales: [
      {
        scale_factor: 0,
        scale_offset: 0,
        keg_weight: 0,
        keg_volume: 0,
        glass_volume: 0,
        temp_sensor_id: ''
      },
      {
        scale_factor: 0,
        scale_offset: 0,
        keg_weight: 0,
        keg_volume: 0,
        glass_volume: 0,
        temp_sensor_id: ''
      },
      {
        scale_factor: 0,
        scale_offset: 0,
        keg_weight: 0,
        keg_volume: 0,
        glass_volume: 0,
        temp_sensor_id: ''
      },
      {
        scale_factor: 0,
        scale_offset: 0,
        keg_weight: 0,
        keg_volume: 0,
        glass_volume: 0,
        temp_sensor_id: ''
      }
    ],
    beers: [],
    // Methods
    toJson: vi.fn(),
    load: vi.fn(),
    sendConfig: vi.fn(),
    sendPushTest: vi.fn(),
    sendWifiScan: vi.fn(),
    sendHardwareScan: vi.fn(),
    restart: vi.fn(),
    saveAll: vi.fn(async () => true),
    getPushTestStatus: vi.fn(),
    getWifiScanStatus: vi.fn(),
    getHardwareScanStatus: vi.fn(),
    runWifiScan: vi.fn(async () => ({ success: true, data: [] })),
    sendFilesystemRequest: vi.fn(),
    runPushTest: vi.fn(),
    $state: {},
    $subscribe: vi.fn()
  }))
}))

vi.mock('@/modules/globalStore', () => ({
  useGlobalStore: vi.fn(() => ({
    id: 'kegmon-123',
    platform: 'ESP32',
    initialized: true,
    disabled: false,
    configChanged: false,
    app_ver: '2.0.0',
    app_build: '1',
    // UI toggles specific to kegmon2-ui
    ui: {
      enableVoltageFragment: false,
      enableManualWifiEntry: false,
      enableScanForStrongestAp: false
    },
    // Feature flags specific to kegmon2-ui
    feature: {
      ble: false,
      tft: false,
      no_scales: 4,
      sd: false
    },
    // Message states
    messageError: '',
    messageWarning: '',
    messageSuccess: '',
    messageInfo: '',
    // Methods
    clearMessages: vi.fn(),
    load: vi.fn(),
    // Getters
    isError: false,
    isWarning: false,
    isSuccess: false,
    isInfo: false,
    fetchTimout: 10000,
    uiVersion: '2.0.0',
    uiBuild: '1',
    $state: {}
  }))
}))

vi.mock('@/modules/statusStore', () => ({
  useStatusStore: vi.fn(() => ({
    id: 'test-id',
    initialized: true,
    disabled: false,
    load: vi.fn(),
    $state: {}
  }))
}))

// Mock eventStore used by many views
// Mock eventStore used by many views (singleton so component and tests share same instance)
const __fakeEventStore = {
  events: [],
  loading: false,
  loadHistory: vi.fn(),
  loadEvents: vi.fn(),
  getFilteredEvents: vi.fn(() => []),
  $state: {}
}
vi.mock('@/modules/eventStore', () => ({
  useEventStore: vi.fn(() => __fakeEventStore)
}))

// Mock external dependencies (provide commonly used exports)
vi.mock('@mp-se/espframework-ui-components', () => ({
  // logging
  logError: vi.fn(),
  logDebug: vi.fn(),
  logInfo: vi.fn(),

  // util conversions
  tempToF: (c) => (c * 9) / 5 + 32,
  tempToC: (f) => ((f - 32) * 5) / 9,

  // form util
  validateCurrentForm: vi.fn(() => true),

  // timer helpers
  useTimers: () => ({
    createInterval: () => ({
      start: () => {},
      stop: () => {}
    }),
    createTimeout: vi.fn((cb, t) => setTimeout(cb, t))
  }),
  useFetch: () => ({
    managedFetch: () => ({})
  }),

  // HTTP helpers used across the app
  sharedHttpClient: {
    getJson: vi.fn(),
    postJson: vi.fn(),
    putJson: vi.fn(),
    deleteJson: vi.fn(),
    request: vi.fn(async () => ({ ok: true })),
    filesystemRequest: vi.fn(async () => ({ success: false })),
    uploadFile: vi.fn(async () => ({ success: true })),
    // websocket helper used by SerialView — returns object with close and socketGetter()
    createWebSocket: vi.fn((path, handlers = {}) => {
      const ws = {
        close: () => {
          if (handlers.onClose) handlers.onClose()
        },
        socketGetter: () => ({
          close: () => {
            if (handlers.onClose) handlers.onClose()
          },
          send: (data) => {
            if (handlers.onMessage) handlers.onMessage({ data })
          }
        })
      }
      // call onOpen immediately to simulate an open socket
      if (handlers.onOpen) handlers.onOpen()
      return ws
    }),
    restart: vi.fn(),
    getErrorString: vi.fn((c) => `Error ${c}`),
    timeout: 10000
  },

  // UI component placeholders (exported names used in main.js and views)
  BsMessage: {},
  BsCard: {},
  BsFileUpload: {},
  BsProgress: {},
  BsInputBase: {},
  BsInputText: {},
  BsInputReadonly: {},
  BsSelect: {},
  BsInputTextArea: {},
  BsInputNumber: {},
  BsInputSwitch: {},
  BsInputRadio: {},
  BsDropdown: {},
  BsModal: {},
  BsModalConfirm: {},
  BsInputTextAreaFormat: {},
  BsMenuBar: {},
  BsFooter: {},
  BsInput: {},
  BsButton: {},

  // Icons (placeholders)
  IconHome: {},
  IconTools: {},
  IconGraphUpArrow: {},
  IconCloudUpArrow: {},
  IconUpArrow: {},
  IconCpu: {},
  IconWifi: {},
  IconEye: {},
  IconEyeSlash: {},
  IconCheckCircle: {},
  IconXCircle: {},
  IconExclamationTriangle: {},
  IconInfoCircle: {},
  // export a version string used by App.onMounted()
  version: 'test-2.0.0',
  // convenience helpers used by views
  createTimeout: vi.fn((cb, t) => setTimeout(cb, t)),
  createInterval: vi.fn(() => ({ start: () => {}, stop: () => {} }))
}))

// Stub UI components from the espframework UI library used across all tests
vtConfig.global = vtConfig.global || {}
vtConfig.global.components = vtConfig.global.components || {}
const uiStubs = [
  'BsInputReadonly',
  'BsInputNumber',
  'BsProgress',
  'BsFileUpload',
  'BsModalConfirm',
  'BsInput',
  'BsButton',
  'BsMessage',
  'BsCard',
  'BsInputText',
  'BsInputSwitch',
  'BsInputRadio',
  'BsDropdown',
  'BsModal',
  'BsInputTextAreaFormat',
  'BsSelect',
  'BsMenuBar',
  'BsFooter',
  'router-link',
  'router-view',
  'VoltageFragment',
  'ListFilesFragment',
  'AdvancedFilesFragment',
  'EnableCorsFragment'
]
uiStubs.forEach((name) => {
  vtConfig.global.components[name] = {
    template: '<div />'
  }
})

// Mock Chart.js to avoid canvas/context issues in JSDOM
vi.mock('chart.js', () => {
  const Chart = class {
    constructor(_ctx, _cfg) {
      this.ctx = null
      this.config = _cfg
    }
    update() {}
    destroy() {}
    static register() {}
  }
  return {
    Chart,
    CategoryScale: {},
    LinearScale: {},
    PointElement: {},
    LineElement: {},
    LineController: {},
    Title: {},
    Tooltip: {},
    Legend: {}
  }
})

vi.mock('chart.js/auto', () => ({}))
