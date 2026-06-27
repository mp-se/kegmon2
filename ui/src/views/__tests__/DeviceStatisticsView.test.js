// Mock the status store used by the view before importing the component
vi.mock('@/modules/statusStore', () => ({
  useStatusStore: () => ({ events: [], $state: {} })
}))

import { shallowMount } from '@vue/test-utils'
import { nextTick } from 'vue'
import DeviceStatisticsView from '@/views/DeviceStatisticsView.vue'
import { sharedHttpClient as http } from '@mp-se/espframework-ui-components'
import { global } from '@/modules/pinia'

const flush = () => new Promise((r) => setTimeout(r, 0))

describe('DeviceStatisticsView', () => {
  beforeEach(() => {
    http.getJson = vi
      .fn()
      .mockResolvedValue({ level_statistics: [], scale_statistics: [], events: [] })
    global.disabled = false
  })

  afterEach(() => {
    vi.restoreAllMocks()
  })

  it('mounts and fetches statistics', async () => {
    const sample = { level_statistics: [], scale_statistics: [], events: [] }
    http.getJson.mockResolvedValueOnce(sample)

    const origSetInterval = globalThis.setInterval
    vi.spyOn(globalThis, 'setInterval').mockImplementation((cb) => {
      void cb
      return 42
    })

    const wrapper = shallowMount(DeviceStatisticsView)
    await nextTick()
    await flush()

    expect(http.getJson).toHaveBeenCalledWith('api/statistics')
    expect(wrapper.vm.statistics).toEqual(sample)
    expect(wrapper.vm.loaded).toBe(true)
    expect(global.disabled).toBe(false)

    vi.restoreAllMocks()
    globalThis.setInterval = origSetInterval
    wrapper.unmount()
  })

  it('interval callback triggers fetch when manually invoked', async () => {
    let captured = null
    vi.spyOn(globalThis, 'setInterval').mockImplementation((cb) => {
      captured = cb
      return 99
    })
    http.getJson.mockResolvedValue({ level_statistics: [], scale_statistics: [], events: [] })

    const wrapper = shallowMount(DeviceStatisticsView)
    await nextTick()
    await flush()

    expect(typeof captured).toBe('function')
    // simulate the interval callback
    await captured()
    await flush()
    expect(http.getJson).toHaveBeenCalled()
    wrapper.unmount()
  })

  it('formatEventName maps known names', () => {
    const wrapper = shallowMount(DeviceStatisticsView)
    expect(wrapper.vm.formatEventName('system_startup')).toBe('System Startup')
    expect(wrapper.vm.formatEventName('unknown_event')).toBe('unknown_event')
  })

  it('getRelativeTime and formatEventTime produce expected outputs', () => {
    const now = 1_000_000_000_000
    vi.spyOn(Date, 'now').mockReturnValue(now)
    const wrapper = shallowMount(DeviceStatisticsView)
    expect(wrapper.vm.getRelativeTime(now - 30 * 1000)).toBe('just now')
    expect(wrapper.vm.getRelativeTime(now - 5 * 60 * 1000)).toBe('5m ago')
    expect(wrapper.vm.getRelativeTime(now - 2 * 60 * 60 * 1000)).toBe('2h ago')
    expect(wrapper.vm.getRelativeTime(now - 2 * 24 * 60 * 60 * 1000)).toBe('2d ago')
    expect(wrapper.vm.formatEventTime(now - 2 * 60 * 1000)).toBe('2m ago')
  })

  it('formatEventData composes details for events', () => {
    const wrapper = shallowMount(DeviceStatisticsView)
    const event = {
      data: {
        volume_l: 1.2,
        pre_weight_kg: 1,
        post_weight_kg: 3,
        duration_ms: 1200
      }
    }
    const out = wrapper.vm.formatEventData(event)
    expect(out).toContain('Volume')
    expect(out).toContain('Weight')
    expect(out).toContain('Duration')
  })

  it('clearStatistics calls api and restores disabled', async () => {
    http.getJson.mockResolvedValueOnce({})
    const wrapper = shallowMount(DeviceStatisticsView)
    global.disabled = false
    await wrapper.vm.clearStatistics()
    expect(global.disabled).toBe(false)
    expect(http.getJson).toHaveBeenCalledWith('api/statistics/clear')
    wrapper.unmount()
  })

  it('formatEventData covers previous_weight_kg and min_valid_weight_kg branches', () => {
    const wrapper = shallowMount(DeviceStatisticsView)
    const event = {
      data: {
        previous_weight_kg: 10,
        current_weight_kg: 8,
        min_valid_weight_kg: 1,
        max_valid_weight_kg: 20
      }
    }
    const out = wrapper.vm.formatEventData(event)
    expect(out).toContain('10')
    expect(out).toContain('8')
    expect(out).toContain('Valid')
  })
})
