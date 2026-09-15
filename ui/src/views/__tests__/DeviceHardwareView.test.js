import { shallowMount } from '@vue/test-utils'
vi.mock('@mp-se/espframework-ui-components', () => ({
  validateCurrentForm: vi.fn(),
  sharedHttpClient: { postJson: vi.fn(), getJson: vi.fn() },
  logDebug: vi.fn(),
  logError: vi.fn(),
  logInfo: vi.fn()
}))
import DeviceHardwareView from '@/views/DeviceHardwareView.vue'
import { global, config } from '@/modules/pinia'
import { validateCurrentForm } from '@mp-se/espframework-ui-components'

describe('DeviceHardwareView', () => {
  beforeEach(() => {
    global.clearMessages = vi.fn()
    global.disabled = false
    global.configChanged = true
    config.saveAll = vi.fn()
    config.restart = vi.fn().mockResolvedValue()
  })

  it('mounts', () => {
    const wrapper = shallowMount(DeviceHardwareView)
    expect(wrapper.exists()).toBe(true)
  })

  it('does not save when form invalid', () => {
    validateCurrentForm.mockReturnValue(false)
    const wrapper = shallowMount(DeviceHardwareView)
    wrapper.vm.save()
    expect(global.clearMessages).not.toHaveBeenCalled()
    expect(config.saveAll).not.toHaveBeenCalled()
    vi.restoreAllMocks()
  })

  it('clears messages and calls saveAll when form valid', () => {
    validateCurrentForm.mockReturnValue(true)
    const wrapper = shallowMount(DeviceHardwareView)
    wrapper.vm.save()
    expect(global.clearMessages).toHaveBeenCalled()
    expect(config.saveAll).toHaveBeenCalled()
    vi.restoreAllMocks()
  })

  it('restart calls config.restart', async () => {
    const wrapper = shallowMount(DeviceHardwareView)
    await wrapper.vm.restart()
    expect(config.restart).toHaveBeenCalled()
  })
})
