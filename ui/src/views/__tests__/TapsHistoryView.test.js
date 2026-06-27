import { shallowMount } from '@vue/test-utils'
import TapsHistoryView from '@/views/TapsHistoryView.vue'
import { useEventStore } from '@/modules/eventStore'
import { global, config } from '@/modules/pinia'
import { status } from '@/modules/pinia'

describe('TapsHistoryView', () => {
  it('mounts', () => {
    const wrapper = shallowMount(TapsHistoryView)
    expect(wrapper.exists()).toBe(true)
  })

  it('getAllScales respects global.feature.no_scales', () => {
    global.feature.no_scales = 3
    const wrapper = shallowMount(TapsHistoryView)
    const arr = wrapper.vm.getAllScales()
    expect(arr).toEqual([1, 2, 3])
  })

  it('getBeerName returns beer name when present', () => {
    config.beers = [{ beer_name: 'Ale' }, { beer_name: 'Lager' }]
    const wrapper = shallowMount(TapsHistoryView)
    expect(wrapper.vm.getBeerName(1)).toBe('Ale')
    expect(wrapper.vm.getBeerName(2)).toBe('Lager')
    expect(wrapper.vm.getBeerName(3)).toBe('Tap 3')
  })

  it('getPoursForScale filters and sorts events and converts volumes', () => {
    const store = useEventStore()
    store.events = [
      {
        eventType: 'POUR_COMPLETED',
        scale: 1,
        timestamp: '2020-01-02T00:00:02Z',
        pourVolume: 0.5,
        pourWeight: 1,
        durationMs: 1000,
        avgSlope: 0
      },
      {
        eventType: 'POUR_COMPLETED',
        scale: 2,
        timestamp: '2020-01-02T00:00:03Z',
        pourVolume: 0.2,
        pourWeight: 0.5,
        durationMs: 500,
        avgSlope: 0
      }
    ]
    // default volume unit is cl
    config.isVolumeCl = true
    const wrapper = shallowMount(TapsHistoryView)
    const pours1 = wrapper.vm.getPoursForScale(1)
    expect(pours1.length).toBe(1)
    expect(pours1[0].volume).toBeGreaterThan(0)
  })

  it('getVolumeUnit respects config flags', () => {
    config.isVolumeCl = true
    config.isVolumeUsOz = false
    config.isVolumeUkOz = false
    const wrapper = shallowMount(TapsHistoryView)
    expect(wrapper.vm.getVolumeUnit()).toBe('cl')
    config.isVolumeCl = false
    config.isVolumeUsOz = true
    expect(wrapper.vm.getVolumeUnit()).toBe('us-oz')
  })

  it('formatTimestamp and formatChartTimestamp produce expected strings', () => {
    const wrapper = shallowMount(TapsHistoryView)
    const ts = new Date('2020-01-02T03:04:05Z').getTime()
    const d = new Date(ts)
    const pad = (n) => String(n).padStart(2, '0')
    const expectedFull = `${d.getFullYear()}-${pad(d.getMonth() + 1)}-${pad(d.getDate())} ${pad(
      d.getHours()
    )}:${pad(d.getMinutes())}:${pad(d.getSeconds())}`
    const expectedChart = `${pad(d.getMonth() + 1)}-${pad(d.getDate())} ${pad(d.getHours())}:${pad(
      d.getMinutes()
    )}`
    expect(wrapper.vm.formatTimestamp(ts)).toBe(expectedFull)
    expect(wrapper.vm.formatChartTimestamp(ts)).toBe(expectedChart)
  })

  it('getScales returns unique sorted scales from events', () => {
    const store = useEventStore()
    store.events = [
      { eventType: 'POUR_COMPLETED', scale: 2 },
      { eventType: 'POUR_COMPLETED', scale: 1 },
      { eventType: 'STABLE_LEVEL', scale: 1 }
    ]
    const wrapper = shallowMount(TapsHistoryView)
    const scales = wrapper.vm.getScales()
    expect(scales).toEqual([1, 2])
  })

  it('createCharts with no chart element does not throw', () => {
    const store = useEventStore()
    store.events = []
    const wrapper = shallowMount(TapsHistoryView)
    // ensure no DOM canvas
    const spy = vi.spyOn(document, 'getElementById').mockReturnValue(null)
    expect(() => wrapper.vm.createCharts()).not.toThrow()
    spy.mockRestore()
  })

  it('loadHistory handles sd_mounted false and true paths', async () => {
    const store = useEventStore()
    // sd not mounted path
    status.sd_mounted = false
    global.messageWarning = ''
    const wrapper = shallowMount(TapsHistoryView)
    await wrapper.vm.loadHistory()
    expect(global.messageWarning).toMatch(/SD card not mounted/)

    // mounted path: ensure loadEvents called and charts creation attempted
    status.sd_mounted = true
    store.loadEvents = vi.fn().mockResolvedValue()
    store.events = [
      { eventType: 'POUR_COMPLETED', scale: 1, timestamp: 1000 },
      { eventType: 'STABLE_LEVEL', scale: 1, timestamp: 1001, stableVolume: 1 }
    ]

    // stub DOM to avoid real canvas; advance timers for setTimeout
    const elSpy = vi.spyOn(document, 'getElementById').mockReturnValue(null)
    vi.useFakeTimers()

    const p = wrapper.vm.loadHistory()
    // advance timers so the internal setTimeout resolves
    await vi.advanceTimersByTimeAsync(200)
    await p

    expect(store.loadEvents).toHaveBeenCalled()
    expect(global.disabled).toBe(false)

    vi.useRealTimers()
    elSpy.mockRestore()
  })
})
