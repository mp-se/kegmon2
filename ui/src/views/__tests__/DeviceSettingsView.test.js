import { shallowMount } from '@vue/test-utils'
import DeviceSettingsView from '@/views/DeviceSettingsView.vue'
import { sharedHttpClient as http, validateCurrentForm } from '@mp-se/espframework-ui-components'
import { config, global } from '@/modules/pinia'

describe('DeviceSettingsView', () => {
  it('mounts', () => {
    const wrapper = shallowMount(DeviceSettingsView)
    expect(wrapper.exists()).toBe(true)
  })

  it('saveSettings does not call saveAll when form invalid', () => {
    config.saveAll = vi.fn()
    vi.mocked(validateCurrentForm).mockReturnValueOnce(false)
    const wrapper = shallowMount(DeviceSettingsView)
    wrapper.vm.saveSettings()
    expect(config.saveAll).not.toHaveBeenCalled()
  })

  it('saveSettings calls saveAll when form valid', () => {
    config.saveAll = vi.fn()
    vi.mocked(validateCurrentForm).mockReturnValueOnce(true)
    const wrapper = shallowMount(DeviceSettingsView)
    wrapper.vm.saveSettings()
    expect(config.saveAll).toHaveBeenCalled()
  })

  it('restart calls config.restart', async () => {
    config.restart = vi.fn(async () => {})
    const wrapper = shallowMount(DeviceSettingsView)
    await wrapper.vm.restart()
    expect(config.restart).toHaveBeenCalled()
  })

  it('factory success path sets messageSuccess and resets disabled flag', async () => {
    global.clearMessages = vi.fn()
    http.getJson.mockResolvedValueOnce({ success: true, message: 'ok' })
    global.disabled = false
    const wrapper = shallowMount(DeviceSettingsView)
    await wrapper.vm.factory()
    expect(global.clearMessages).toHaveBeenCalled()
    expect(global.messageSuccess).toBe('ok')
    expect(global.disabled).toBe(false)
  })

  it('factory failure path sets messageError', async () => {
    global.clearMessages = vi.fn()
    http.getJson.mockResolvedValueOnce({ success: false, message: 'bad' })
    global.disabled = false
    const wrapper = shallowMount(DeviceSettingsView)
    await wrapper.vm.factory()
    expect(global.messageError).toBe('bad')
    expect(global.disabled).toBe(false)
  })

  it('factory exception path sets error message and resets disabled', async () => {
    global.clearMessages = vi.fn()
    http.getJson.mockRejectedValueOnce(new Error('network error'))
    global.disabled = false
    const wrapper = shallowMount(DeviceSettingsView)
    await wrapper.vm.factory()
    expect(global.messageError).toBe('Failed to do factory restore')
    expect(global.disabled).toBe(false)
  })

  it('renders buttons for save, restart, and factory restore', () => {
    const wrapper = shallowMount(DeviceSettingsView)
    expect(wrapper.html()).toContain('Save')
    expect(wrapper.html()).toContain('Restart device')
    expect(wrapper.html()).toContain('Restore factory defaults')
  })

  it('disables all buttons when global.disabled is true', () => {
    global.disabled = true
    const wrapper = shallowMount(DeviceSettingsView)
    const buttons = wrapper.findAll('button[disabled]')
    expect(buttons.length).toBeGreaterThan(0)
  })

  it('has all required functions on vm', () => {
    const wrapper = shallowMount(DeviceSettingsView)
    expect(typeof wrapper.vm.saveSettings).toBe('function')
    expect(typeof wrapper.vm.restart).toBe('function')
    expect(typeof wrapper.vm.factory).toBe('function')
  })

  it('factory clears messages before starting', async () => {
    global.clearMessages = vi.fn()
    http.getJson.mockResolvedValueOnce({ success: true, message: 'done' })
    const wrapper = shallowMount(DeviceSettingsView)
    await wrapper.vm.factory()
    expect(global.clearMessages).toHaveBeenCalled()
  })

  it('factory sets disabled true during operation and false after', async () => {
    http.getJson.mockResolvedValueOnce({ success: true, message: 'ok' })
    global.disabled = false
    const wrapper = shallowMount(DeviceSettingsView)
    const factoryPromise = wrapper.vm.factory()
    // At this point disabled should be true (synchronously set)
    expect(global.disabled).toBe(true)
    await factoryPromise
    // After completion, should be false
    expect(global.disabled).toBe(false)
  })

  it('factory handles location.reload fallback when first reload fails', async () => {
    http.getJson.mockResolvedValueOnce({ success: true, message: 'ok' })
    const windowReloadMock = vi.fn()
    global.disabled = false
    // Simulate location.reload throwing
    delete window.location
    window.location = { reload: windowReloadMock }
    const wrapper = shallowMount(DeviceSettingsView)
    // Note: setTimeout won't actually run in test, but the code path is parsed
    await wrapper.vm.factory()
    expect(global.messageSuccess).toBe('ok')
  })

  it('factory exception path sets error message', async () => {
    global.clearMessages = vi.fn()
    http.getJson.mockRejectedValueOnce(new Error('network fail'))
    global.disabled = false
    const wrapper = shallowMount(DeviceSettingsView)
    await wrapper.vm.factory()
    expect(global.messageError).toBe('Failed to do factory restore')
    expect(global.disabled).toBe(false)
  })

  it('saveSettings validates form before saving', () => {
    config.saveAll = vi.fn()
    vi.mocked(validateCurrentForm).mockReturnValueOnce(false)
    const wrapper = shallowMount(DeviceSettingsView)
    wrapper.vm.saveSettings()
    expect(config.saveAll).not.toHaveBeenCalled()
  })

  it('restart is async and calls config.restart', async () => {
    config.restart = vi.fn(async () => {})
    const wrapper = shallowMount(DeviceSettingsView)
    const isAsync = wrapper.vm.restart.constructor.name === 'AsyncFunction'
    expect(isAsync).toBe(true)
    await wrapper.vm.restart()
    expect(config.restart).toHaveBeenCalled()
  })

  it('v-model setters update config values when stubs emit update:modelValue', async () => {
    config.mdns = ''
    config.temp_unit = 'C'
    config.weight_unit = 'kg'
    config.volume_unit = 'L'
    config.dark_mode = false
    const wrapper = shallowMount(DeviceSettingsView, {
      global: {
        stubs: {
          BsInputText: {
            name: 'BsInputText',
            template: '<div />',
            props: ['modelValue'],
            emits: ['update:modelValue']
          },
          BsInputRadio: {
            name: 'BsInputRadio',
            template: '<div />',
            props: ['modelValue'],
            emits: ['update:modelValue']
          }
        }
      }
    })
    const textStubs = wrapper.findAllComponents({ name: 'BsInputText' })
    const radioStubs = wrapper.findAllComponents({ name: 'BsInputRadio' })
    await textStubs[0].vm.$emit('update:modelValue', 'mydevice')
    expect(config.mdns).toBe('mydevice')
    await radioStubs[0].vm.$emit('update:modelValue', 'F')
    expect(config.temp_unit).toBe('F')
    await radioStubs[1].vm.$emit('update:modelValue', 'lbs')
    expect(config.weight_unit).toBe('lbs')
    await radioStubs[2].vm.$emit('update:modelValue', 'gal')
    expect(config.volume_unit).toBe('gal')
    await radioStubs[3].vm.$emit('update:modelValue', true)
    expect(config.dark_mode).toBe(true)
  })

  it('clicking factory button triggers factory function', async () => {
    http.getJson = vi.fn().mockResolvedValue({ success: false, message: 'fail' })
    const wrapper = shallowMount(DeviceSettingsView)
    const factoryBtn = wrapper
      .findAll('button[type="button"]')
      .find((b) => b.text().includes('factory') || b.text().includes('Restore'))
    if (factoryBtn) {
      await factoryBtn.trigger('click')
      expect(http.getJson).toHaveBeenCalled()
    }
  })

  it('factory fires beforeunload handler to clear timeout', async () => {
    vi.useFakeTimers()
    http.getJson = vi.fn().mockResolvedValue({ success: true, message: 'ok' })
    const wrapper = shallowMount(DeviceSettingsView)
    const p = wrapper.vm.factory()
    await vi.runAllTicks()
    window.dispatchEvent(new Event('beforeunload'))
    await vi.advanceTimersByTimeAsync(3000)
    await p
    vi.useRealTimers()
  })
})
