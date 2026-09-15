import { shallowMount } from '@vue/test-utils'
import HomeView from '@/views/HomeView.vue'
import { status, global, config } from '@/modules/pinia'

describe('HomeView', () => {
  beforeEach(() => {
    // reset mock stores
    status.scales = [
      {
        stable_volume: 5,
        keg_volume: 20,
        glass: 0.5,
        sampling_rate: 80,
        stable_weight: 2,
        last_pour_volume: 0.5
      },
      { stable_volume: 2, keg_volume: 10, glass: 0.33, sampling_rate: 10, stable_weight: 1 }
    ]
    status.sensors = [{ id: 't1', temperature: 12.3 }]
    status.getLastEventsForScale = () => [{ name: 'pouring', timestamp_ms: Date.now() - 1000 }]
    status.getRelativeTime = () => 'just now'

    config.beers = [{ beer_name: 'A', beer_abv: 5, beer_ebc: 10, beer_ibu: 20 }, { beer_name: 'B' }]
    config.scales = [{ temp_sensor_id: 't1' }, {}]
    global.feature = { no_scales: 4 }
    global.app_ver = '1.2.3'
    global.app_build = '100'
    global.uiVersion = 'v'
    global.uiBuild = 'b'
    global.platform = 'esp32'
  })

  it('mounts', () => {
    const wrapper = shallowMount(HomeView)
    expect(wrapper.exists()).toBe(true)
  })

  it('getTapClass responds to feature.no_scales', () => {
    global.feature.no_scales = 2
    const wrapper = shallowMount(HomeView)
    expect(wrapper.vm.getTapClass()).toBe('col-md-6')
  })

  it('getTapClass covers other values', () => {
    global.feature.no_scales = 4
    const wrapper4 = shallowMount(HomeView)
    expect(wrapper4.vm.getTapClass()).toBe('col-md-3')

    global.feature.no_scales = 3
    const wrapper3 = shallowMount(HomeView)
    expect(wrapper3.vm.getTapClass()).toBe('col-md-4')

    global.feature.no_scales = 1
    const wrapper1 = shallowMount(HomeView)
    expect(wrapper1.vm.getTapClass()).toBe('col-md-12')
  })

  it('calculateGlassesLeft computes correct value', () => {
    const wrapper = shallowMount(HomeView)
    expect(wrapper.vm.calculateGlassesLeft({ stable_volume: 5, glass: 0.5 })).toBe(10)
    expect(wrapper.vm.calculateGlassesLeft({})).toBe(0)
  })

  it('getScaleTemperature returns sensor temperature or null', () => {
    const wrapper = shallowMount(HomeView)
    expect(wrapper.vm.getScaleTemperature(0)).toBe(12.3)
    expect(wrapper.vm.getScaleTemperature(1)).toBe(null)
  })

  it('tapProgressArray computes percentages and clamps', () => {
    const wrapper = shallowMount(HomeView)
    const arr = wrapper.vm.tapProgressArray
    expect(Array.isArray(arr)).toBe(true)
    expect(arr[0]).toBeGreaterThan(0)
  })

  it('tapProgressArray returns 0 when keg_volume missing or zero', () => {
    status.scales = [{ stable_volume: 5, keg_volume: 0 }, {}]
    const wrapper = shallowMount(HomeView)
    const arr = wrapper.vm.tapProgressArray
    expect(arr[0]).toBe(0)
  })

  it('push computed properties show Not updated when push_used false', () => {
    status.ha = { push_used: false }
    status.brewspy = { push_used: false }
    status.brewlogger = { push_used: false }
    status.barhelper = { push_used: false }
    const wrapper = shallowMount(HomeView)
    expect(wrapper.vm.pushHomeAssistant).toBe('Not updated')
    expect(wrapper.vm.pushBrewspy).toBe('Not updated')
    expect(wrapper.vm.pushBrewLogger).toBe('Not updated')
    expect(wrapper.vm.pushBarhelper).toBe('Not updated')
  })

  it('clampProgress handles edge values', () => {
    const wrapper = shallowMount(HomeView)
    expect(wrapper.vm.clampProgress(NaN)).toBe(0)
    expect(wrapper.vm.clampProgress(Infinity)).toBe(0)
    expect(wrapper.vm.clampProgress(-10)).toBe(0)
    expect(wrapper.vm.clampProgress(150)).toBe(100)
    expect(wrapper.vm.clampProgress(42.4)).toBe(42)
  })

  it('push computed properties show success and failure messages', () => {
    status.ha = { push_used: true, push_age: 5000, push_status: true }
    status.brewspy = { push_used: true, push_age: 2000, push_status: false, push_code: 12 }
    status.brewlogger = { push_used: true, push_age: 3000, push_status: true }
    status.barhelper = {
      push_used: true,
      push_age: 3000,
      push_status: false,
      push_code: 7,
      push_response: '{"message":"ok"}'
    }
    const wrapper = shallowMount(HomeView)
    expect(wrapper.vm.pushHomeAssistant).toMatch(/Updated 5s ago/)
    expect(wrapper.vm.pushBrewspy).toMatch(/Failed, error 12/)
    expect(wrapper.vm.pushBrewLogger).toMatch(/Updated 3s ago/)
    expect(wrapper.vm.pushBarhelper).toMatch(/ok/)
  })

  it('pushBarhelper handles malformed payloads without throwing', () => {
    status.barhelper = {
      push_used: true,
      push_age: 2000,
      push_status: false,
      push_code: 7,
      push_response: 'not-json'
    }
    const wrapper = shallowMount(HomeView)
    expect(wrapper.vm.pushBarhelper).toContain('Failed')
  })

  it('pushBarhelper fixes payloads containing broken volume field', () => {
    // craft a payload that matches the broken pattern: contains '"volume:' and '"\n'
    const broken = '{"message":"x","volume:1.23"\n}'
    status.barhelper = {
      push_used: true,
      push_age: 12000,
      push_status: true,
      push_code: 0,
      push_response: broken
    }
    const wrapper = shallowMount(HomeView)
    // should return a string containing 'Updated' and not throw
    expect(wrapper.vm.pushBarhelper).toMatch(/Updated/)
  })

  it('getLastEvents and getRelativeTime proxy to status helpers', () => {
    const wrapper = shallowMount(HomeView)
    const events = wrapper.vm.getLastEvents(0)
    expect(events[0].name).toBe('pouring')
    expect(wrapper.vm.getRelativeTime(Date.now())).toBe('just now')
  })

  it('refresh calls status.load', async () => {
    const spy = vi.spyOn(status, 'load').mockResolvedValue()
    const wrapper = shallowMount(HomeView)
    await wrapper.vm.refresh()
    expect(spy).toHaveBeenCalled()
    spy.mockRestore()
  })

  it('onBeforeUnmount clears the polling interval', () => {
    const clearSpy = vi.spyOn(window, 'clearInterval')
    const wrapper = shallowMount(HomeView)
    wrapper.unmount()
    expect(clearSpy).toHaveBeenCalled()
    clearSpy.mockRestore()
  })

  it('renders device info section slot content when BsCard has slot', async () => {
    const wrapper = shallowMount(HomeView, {
      global: {
        stubs: {
          BsCard: { name: 'BsCard', template: '<div><slot /></div>' }
        }
      }
    })
    expect(wrapper.text()).toContain('esp32')
  })

  it('sampling_rate undefined renders Unknown ADC badge', async () => {
    status.scales = [
      { stable_volume: 5, keg_volume: 20, glass: 0.5, sampling_rate: undefined, stable_weight: 2 }
    ]
    config.beers = [{ beer_name: 'A', beer_abv: 5, beer_ebc: 10, beer_ibu: 20 }]
    const wrapper = shallowMount(HomeView, {
      global: {
        stubs: {
          BsCard: { name: 'BsCard', template: '<div><slot /></div>' }
        }
      }
    })
    expect(wrapper.text()).toContain('Unknown')
  })

  it('sampling_rate 0 renders Unknown ADC badge', async () => {
    status.scales = [
      { stable_volume: 5, keg_volume: 20, glass: 0.5, sampling_rate: 0, stable_weight: 2 }
    ]
    config.beers = [{ beer_name: 'A', beer_abv: 5, beer_ebc: 10, beer_ibu: 20 }]
    const wrapper = shallowMount(HomeView, {
      global: {
        stubs: {
          BsCard: { name: 'BsCard', template: '<div><slot /></div>' }
        }
      }
    })
    expect(wrapper.text()).toContain('Unknown')
  })
})
