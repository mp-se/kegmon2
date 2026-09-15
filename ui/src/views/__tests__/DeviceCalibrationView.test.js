import { shallowMount } from '@vue/test-utils'
import DeviceCalibrationView from '@/views/DeviceCalibrationView.vue'
import { global, config } from '@/modules/pinia'
import { sharedHttpClient as http } from '@mp-se/espframework-ui-components'

describe('DeviceCalibrationView', () => {
  beforeEach(() => {
    // reset globals and config used by the view
    global.clearMessages = vi.fn()
    global.disabled = false
    global.messageError = ''
    global.messageSuccess = ''
    config.dark_mode = false
    config.isWeightKg = true
    config.isWeightLbs = false
    config.weight_unit = 'kg'
  })

  it('mounts', () => {
    const wrapper = shallowMount(DeviceCalibrationView)
    expect(wrapper.exists()).toBe(true)
  })

  it('navigation helpers update state and clear messages', () => {
    const wrapper = shallowMount(DeviceCalibrationView)
    wrapper.vm.state = 3
    wrapper.vm.back()
    expect(global.clearMessages).toHaveBeenCalled()
    expect(wrapper.vm.state).toBe(2)

    wrapper.vm.begin()
    expect(wrapper.vm.state).toBe(1)

    wrapper.vm.step1()
    expect(wrapper.vm.state).toBe(2)
  })

  it('getClass respects dark_mode', () => {
    const wrapper = shallowMount(DeviceCalibrationView)
    config.dark_mode = true
    expect(wrapper.vm.getClass()).toMatch(/bg-dark/)
    config.dark_mode = false
    expect(wrapper.vm.getClass()).toMatch(/bg-light/)
  })

  it('saveScaleValues populates scaleStatus and formats weight', () => {
    const wrapper = shallowMount(DeviceCalibrationView)

    // set scale index to 1
    wrapper.vm.scale = 1

    const json = {
      scales: [{}, { scale_offset: 5, scale_factor: 2, scale_raw: 123, stable_weight: 3.141 }]
    }

    wrapper.vm.saveScaleValues(json)

    expect(wrapper.vm.scaleStatus.offset).toBe(5)
    expect(wrapper.vm.scaleStatus.factor).toBe(2)
    expect(wrapper.vm.scaleStatus.raw).toBe(123)
    expect(wrapper.vm.scaleStatus.weight).toBeCloseTo(3.141)
    expect(wrapper.vm.scaleStatus.weightString).toMatch(/3.141/) // includes unit
  })

  it('step2 posts tare and reads scale status (success path)', async () => {
    const wrapper = shallowMount(DeviceCalibrationView)

    // stub http methods
    vi.spyOn(http, 'postJson').mockResolvedValue({})
    vi.spyOn(http, 'getJson').mockResolvedValueOnce({
      scale_busy: false,
      scales: [{}, { scale_offset: 1, scale_factor: 1, scale_raw: 0, stable_weight: 2 }]
    })

    await wrapper.vm.step2()

    expect(http.postJson).toHaveBeenCalled()
    expect(http.getJson).toHaveBeenCalled()
    expect(global.messageSuccess).toMatch(/Scale tare completed/)
    expect(wrapper.vm.state).toBe(3)

    vi.restoreAllMocks()
  })

  it('step2 handles errors', async () => {
    const wrapper = shallowMount(DeviceCalibrationView)
    vi.spyOn(http, 'postJson').mockRejectedValue(new Error('network'))

    await wrapper.vm.step2()

    expect(global.messageError).toMatch(/network/)
    expect(global.disabled).toBe(false)

    vi.restoreAllMocks()
  })

  it('step3 validates weight and performs factor calculation (success)', async () => {
    const wrapper = shallowMount(DeviceCalibrationView)
    wrapper.vm.weight = 2.5
    wrapper.vm.scale = 1

    vi.spyOn(http, 'postJson').mockResolvedValue({})
    vi.spyOn(http, 'getJson').mockResolvedValueOnce({
      scale_busy: false,
      scales: [
        {},
        { state: 'Stable', scale_offset: 1, scale_factor: 1, scale_raw: 0, stable_weight: 2.5 }
      ]
    })

    await wrapper.vm.step3()

    expect(http.postJson).toHaveBeenCalled()
    expect(http.getJson).toHaveBeenCalled()
    expect(global.messageSuccess).toMatch(/Scale factor calculation completed/)
    expect(wrapper.vm.state).toBe(4)
    expect(wrapper.vm.scaleStatus.weight).toBeCloseTo(2.5)

    vi.restoreAllMocks()
  })

  it('step3 errors when weight is zero', async () => {
    const wrapper = shallowMount(DeviceCalibrationView)
    wrapper.vm.weight = 0
    await wrapper.vm.step3()
    expect(global.messageError).toMatch(/You need to supply a weight larger than 0/)
  })

  it('clicking begin button triggers begin function', async () => {
    const wrapper = shallowMount(DeviceCalibrationView)
    // Set state > 3 so the "Start over" button renders
    wrapper.vm.state = 4
    await wrapper.vm.$nextTick()
    const buttons = wrapper.findAll('button[type="button"]')
    const beginBtn = buttons.find((b) => b.text().includes('Start over'))
    if (beginBtn) {
      await beginBtn.trigger('click')
      expect(wrapper.vm.state).toBe(1)
    }
  })

  it('clicking back button in state > 3 triggers back function', async () => {
    const wrapper = shallowMount(DeviceCalibrationView)
    wrapper.vm.state = 4
    await wrapper.vm.$nextTick()
    const backButtons = wrapper.findAll('button[type="button"]')
    const backBtn = backButtons.find((b) => b.text().includes('Back'))
    if (backBtn) {
      await backBtn.trigger('click')
      // back() should decrement state or reset
      expect(wrapper.vm.state).not.toBe(4)
    }
  })

  it('clicking step3 button in state == 3 calls step3', async () => {
    vi.spyOn(http, 'postJson').mockResolvedValue({})
    vi.spyOn(http, 'getJson').mockResolvedValue({
      scale_busy: false,
      scales: [
        { state: 'Stable', scale_offset: 1, scale_factor: 1, scale_raw: 0, stable_weight: 1 }
      ]
    })
    const wrapper = shallowMount(DeviceCalibrationView)
    wrapper.vm.state = 3
    wrapper.vm.weight = 1.0
    await wrapper.vm.$nextTick()
    const buttons = wrapper.findAll('button[type="button"]')
    const step3Btn = buttons.find((b) => b.text().includes('Calculate factor'))
    if (step3Btn) {
      await step3Btn.trigger('click')
      expect(http.postJson).toHaveBeenCalled()
    }
    vi.restoreAllMocks()
  })

  it('step2 polling loops when scale_busy on first call', async () => {
    vi.useFakeTimers()
    const wrapper = shallowMount(DeviceCalibrationView)
    wrapper.vm.scale = 0

    vi.spyOn(http, 'postJson').mockResolvedValue({})
    vi.spyOn(http, 'getJson')
      .mockResolvedValueOnce({ scale_busy: true, scales: [{}] })
      .mockResolvedValueOnce({
        scale_busy: false,
        scales: [
          { state: 'Stable', scale_offset: 1, scale_factor: 1, scale_raw: 0, stable_weight: 1 }
        ]
      })

    const p = wrapper.vm.step2()
    await vi.advanceTimersByTimeAsync(3000)
    await p

    expect(global.messageSuccess).toMatch(/tare completed|Tare/i)
    vi.useRealTimers()
    vi.restoreAllMocks()
  })

  it('step3 handles errors from http calls', async () => {
    const wrapper = shallowMount(DeviceCalibrationView)
    wrapper.vm.weight = 2.5
    vi.spyOn(http, 'postJson').mockRejectedValue(new Error('network error'))

    await wrapper.vm.step3()

    expect(global.messageError).toMatch(/network error/)
    expect(global.disabled).toBe(false)
    vi.restoreAllMocks()
  })

  it('step3 polling loops when scale_busy on first call', async () => {
    vi.useFakeTimers()
    const wrapper = shallowMount(DeviceCalibrationView)
    wrapper.vm.weight = 2.5
    wrapper.vm.scale = 0

    vi.spyOn(http, 'postJson').mockResolvedValue({})
    vi.spyOn(http, 'getJson')
      .mockResolvedValueOnce({ scale_busy: true, scales: [{}] })
      .mockResolvedValueOnce({
        scale_busy: false,
        scales: [
          { state: 'Stable', scale_offset: 1, scale_factor: 1, scale_raw: 0, stable_weight: 2.5 }
        ]
      })

    const p = wrapper.vm.step3()
    await vi.advanceTimersByTimeAsync(3000)
    await p

    expect(global.messageSuccess).toMatch(/factor calculation completed/)
    vi.useRealTimers()
    vi.restoreAllMocks()
  })
})
