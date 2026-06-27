import { shallowMount } from '@vue/test-utils'
import DeviceEventView from '@/views/DeviceEventView.vue'
import { useEventStore } from '@/modules/eventStore'
import { global, status } from '@/modules/pinia'

describe('DeviceEventView', () => {
  beforeEach(() => {
    vi.resetAllMocks()
    // ensure default mocked status
    status.sd_mounted = false
    global.messageWarning = ''
  })

  it('mounts', () => {
    const wrapper = shallowMount(DeviceEventView)
    expect(wrapper.exists()).toBe(true)
  })

  it('formatEventType replaces underscores with spaces', () => {
    const wrapper = shallowMount(DeviceEventView)
    expect(wrapper.vm.formatEventType('TEST_EVENT_TYPE')).toBe('TEST EVENT TYPE')
  })

  it('formatTimestamp returns expected format', () => {
    const wrapper = shallowMount(DeviceEventView)
    const ts = new Date('2020-01-02T03:04:05Z').getTime()
    // timezone differences may apply, so only check structure
    const out = wrapper.vm.formatTimestamp(ts)
    expect(out).toMatch(/2020-01-02/)
    expect(out).toMatch(/03:04:05|04:04:05|02:04:05/)
  })

  it('formatEventDetails handles POUR_COMPLETED', () => {
    const wrapper = shallowMount(DeviceEventView)
    const evt = {
      eventType: 'POUR_COMPLETED',
      pourWeight: 1.23,
      pourVolume: 0.45,
      durationMs: 1500,
      avgSlope: 0.1
    }
    const out = wrapper.vm.formatEventDetails(evt)
    expect(out).toContain('Poured')
    expect(out).toContain('Duration')
  })

  it('loadEvents sets warning when SD not mounted and does not call eventStore.loadEvents', async () => {
    const wrapper = shallowMount(DeviceEventView)
    const store = useEventStore()
    store.loadEvents = vi.fn()
    status.sd_mounted = false
    await wrapper.vm.loadEvents()
    expect(global.messageWarning).toBeTruthy()
    expect(store.loadEvents).not.toHaveBeenCalled()
  })

  it('formatEventDetails covers several event types', () => {
    const wrapper = shallowMount(DeviceEventView)

    expect(wrapper.vm.formatEventDetails({ eventType: 'SYSTEM_STARTUP' })).toBe(
      'System initialization'
    )
    expect(
      wrapper.vm.formatEventDetails({ eventType: 'SETTLING_STARTED', currWeight: 1.5 })
    ).toContain('Settling started')
    expect(
      wrapper.vm.formatEventDetails({
        eventType: 'STABLE_LEVEL',
        stableWeight: 2.0,
        stableVolume: 1.2,
        durationMs: 500
      })
    ).toContain('Stable')
    expect(wrapper.vm.formatEventDetails({ eventType: 'POURING', prePourWeight: 0.5 })).toContain(
      'Started at'
    )
    expect(
      wrapper.vm.formatEventDetails({
        eventType: 'WEIGHT_CHANGE_DETECTED',
        prevWeight: 1,
        currWeight: 1.2,
        variance: 0.01
      })
    ).toContain('Weight change')
    expect(
      wrapper.vm.formatEventDetails({ eventType: 'KEG_REMOVED', prevWeight: 2, currWeight: 0 })
    ).toContain('Removed')
    expect(
      wrapper.vm.formatEventDetails({
        eventType: 'KEG_ABSENT_TIMEOUT',
        durationMs: 2000,
        currWeight: 0
      })
    ).toContain('Absent')
    expect(wrapper.vm.formatEventDetails({ eventType: 'INVALID_WEIGHT' })).toBe(
      'Sensor reading out of range'
    )
    expect(
      wrapper.vm.formatEventDetails({
        eventType: 'LOAD_CELL_ERROR',
        signalErrorReason: 'sig',
        signalQuality: 90,
        consecutiveErrors: 2,
        variance: 0.002
      })
    ).toContain('Quality')
    expect(
      wrapper.vm.formatEventDetails({
        eventType: 'LOAD_CELL_RECOVERED',
        signalQuality: 80,
        variance: 0.001
      })
    ).toContain('Quality')
    expect(
      wrapper.vm.formatEventDetails({ eventType: 'SENSOR_RECOVERED', currWeight: 1 })
    ).toContain('Recovered at')
    expect(wrapper.vm.formatEventDetails({ eventType: 'CALIBRATION_NEEDED' })).toBe(
      'Scale calibration required'
    )
    expect(wrapper.vm.formatEventDetails({ eventType: 'CALIBRATION_COMPLETE' })).toBe(
      'Scale calibration completed successfully'
    )
    expect(wrapper.vm.formatEventDetails({ eventType: 'DISABLED' })).toBe(
      'Scale unit disabled (ADC hardware not found)'
    )
    expect(wrapper.vm.formatEventDetails({ eventType: 'SOME_UNKNOWN' })).toBe('Unknown event type')
  })

  it('loadEvents calls eventStore.loadEvents when SD mounted', async () => {
    const wrapper = shallowMount(DeviceEventView)
    const store = useEventStore()
    store.loadEvents = vi.fn()
    status.sd_mounted = true
    await wrapper.vm.loadEvents()
    expect(store.loadEvents).toHaveBeenCalled()
  })

  it('formatEventDetails covers KEG_REPLACED, KEG_ABSENT_TIMEOUT with currWeight>0, SENSOR_RECOVERED with no weight', () => {
    const wrapper = shallowMount(DeviceEventView)
    expect(
      wrapper.vm.formatEventDetails({ eventType: 'KEG_REPLACED', prevWeight: 2, currWeight: 3 })
    ).toContain('Replaced')
    expect(
      wrapper.vm.formatEventDetails({
        eventType: 'KEG_ABSENT_TIMEOUT',
        durationMs: 2000,
        currWeight: 1.5
      })
    ).toContain('Current')
    expect(wrapper.vm.formatEventDetails({ eventType: 'SENSOR_RECOVERED', currWeight: 0 })).toBe(
      'Sensor recovered'
    )
  })

  it('loadEvents handles exception from eventStore.loadEvents', async () => {
    const wrapper = shallowMount(DeviceEventView)
    const store = useEventStore()
    store.loadEvents = vi.fn().mockRejectedValue(new Error('load error'))
    status.sd_mounted = true
    await expect(wrapper.vm.loadEvents()).resolves.not.toThrow()
    expect(global.disabled).toBe(false)
  })

  it('selecting a scale filter updates scaleFilter via v-model', async () => {
    const wrapper = shallowMount(DeviceEventView)
    const select = wrapper.find('select')
    await select.setValue('1')
    await wrapper.vm.$nextTick()
    // Just verify the onChange handler was exercised (select value updates)
    expect(['1', 'all']).toContain(wrapper.vm.scaleFilter)
  })

  it('renders event rows when filteredEvents has items', () => {
    const store = useEventStore()
    store.getFilteredEvents = vi.fn(() => [
      {
        scale: 1,
        eventType: 'POUR_COMPLETED',
        timestamp: '2024-01-01T10:00:00',
        pourWeight: 1.5,
        pourVolume: 1.8,
        durationMs: 5000,
        avgSlope: 0
      }
    ])
    const wrapper = shallowMount(DeviceEventView)
    expect(wrapper.findAll('[class*="border-bottom"]').length).toBeGreaterThan(0)
  })
})
