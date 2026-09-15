import { shallowMount } from '@vue/test-utils'
import SupportView from '@/views/SupportView.vue'
import { global, config } from '@/modules/pinia'

describe('SupportView', () => {
  beforeEach(() => {
    // reset globals
    global.disabled = false
    global.messageSuccess = ''
    config.sendFilesystemRequest = vi.fn(async () => ({ success: true, text: 'line1\nline2' }))
    config.runHardwareScan = vi.fn(async () => ({ success: true, data: 'scan-results' }))
  })

  it('mounts', () => {
    const wrapper = shallowMount(SupportView)
    expect(wrapper.exists()).toBe(true)
  })

  it('viewLogs populates logData and toggles disabled', async () => {
    const wrapper = shallowMount(SupportView)
    expect(global.disabled).toBe(false)
    await wrapper.vm.viewLogs()
    expect(global.disabled).toBe(false)
    expect(wrapper.vm.logData).toContain('line1')
  })

  it('removeLogs sets messageSuccess and resets disabled', async () => {
    const wrapper = shallowMount(SupportView)
    await wrapper.vm.removeLogs()
    expect(wrapper.vm.logData).toBe('')
    expect(global.messageSuccess).toBe('Requested logs to be deleted')
    expect(global.disabled).toBe(false)
  })

  it('removeLegacy sets messageSuccess when called', async () => {
    const wrapper = shallowMount(SupportView)
    await wrapper.vm.removeLegacy()
    expect(global.messageSuccess).toBe('Requested old configuration files to be deleted')
    expect(global.disabled).toBe(false)
  })

  it('hardwareScan calls config.runHardwareScan and sets logData', async () => {
    const wrapper = shallowMount(SupportView)
    await wrapper.vm.hardwareScan()
    expect(config.runHardwareScan).toHaveBeenCalled()
    expect(wrapper.vm.logData).toBe('scan-results')
    expect(global.disabled).toBe(false)
  })

  it('toggle showHelp shows help section', () => {
    const wrapper = shallowMount(SupportView)
    expect(wrapper.vm.showHelp).toBe(false)
    wrapper.vm.showHelp = true
    expect(wrapper.vm.showHelp).toBe(true)
  })
})
