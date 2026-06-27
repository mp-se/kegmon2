import { beforeEach, describe, expect, it, vi } from 'vitest'
import { shallowMount } from '@vue/test-utils'
import App from '@/App.vue'
import { createTestingPinia } from '@/tests/testUtils'
import { global, status, config } from '@/modules/pinia'
import * as piniaModule from '@/modules/pinia'
import { sharedHttpClient as http } from '@mp-se/espframework-ui-components'

vi.mock('@mp-se/espframework-ui-components', () => ({
  sharedHttpClient: { auth: vi.fn(), ping: vi.fn() },
  logError: vi.fn(),
  logInfo: vi.fn(),
  version: 'test-version'
}))

describe('App.vue unit', () => {
  beforeEach(() => {
    // reset stores
    global.initialized = true
    global.disabled = false
    global.messageError = ''
    global.messageWarning = ''
    global.messageSuccess = ''
    global.messageInfo = ''
    status.connected = true
    config.dark_mode = false
  })

  it('close clears the correct alert message', () => {
    const pinia = createTestingPinia()
    const wrapper = shallowMount(App, { global: { plugins: [pinia] } })

    global.messageError = 'err'
    wrapper.vm.close('danger')
    expect(global.messageError).toBe('')

    global.messageWarning = 'w'
    wrapper.vm.close('warning')
    expect(global.messageWarning).toBe('')

    global.messageSuccess = 's'
    wrapper.vm.close('success')
    expect(global.messageSuccess).toBe('')

    global.messageInfo = 'i'
    wrapper.vm.close('info')
    expect(global.messageInfo).toBe('')
  })

  it('handleDarkModeUpdate sets attribute on documentElement', () => {
    const pinia = createTestingPinia()
    const wrapper = shallowMount(App, { global: { plugins: [pinia] } })

    const setSpy = vi.spyOn(document.documentElement, 'setAttribute')
    wrapper.vm.handleDarkModeUpdate(true)
    expect(config.dark_mode).toBe(true)
    expect(setSpy).toHaveBeenCalledWith('data-bs-theme', 'dark')
    wrapper.vm.handleDarkModeUpdate(false)
    expect(config.dark_mode).toBe(false)
    expect(setSpy).toHaveBeenCalledWith('data-bs-theme', 'light')
    setSpy.mockRestore()
  })

  it('showSpinner and hideSpinner call dialog methods', () => {
    const pinia = createTestingPinia()
    const show = vi.fn()
    const close = vi.fn()
    const el = { showModal: show, close }
    const qSpy = vi.spyOn(document, 'querySelector').mockReturnValue(el)

    const wrapper = shallowMount(App, { global: { plugins: [pinia] } })
    wrapper.vm.showSpinner()
    expect(show).toHaveBeenCalled()
    wrapper.vm.hideSpinner()
    expect(close).toHaveBeenCalled()

    qSpy.mockRestore()
  })

  it('initializeApp success path sets global.initialized', async () => {
    const pinia = createTestingPinia()

    // prepare mocks BEFORE mounting so onMounted initializeApp uses them
    http.auth.mockResolvedValue(true)
    const globalLoad = vi.spyOn(global, 'load').mockResolvedValue(true)
    const statusLoad = vi.spyOn(status, 'load').mockResolvedValue(true)
    const configLoad = vi.spyOn(config, 'load').mockResolvedValue(true)
    const saveState = vi.spyOn(piniaModule, 'saveConfigState').mockImplementation(vi.fn())

    // stub spinner DOM to avoid errors
    const qSpy = vi
      .spyOn(document, 'querySelector')
      .mockReturnValue({ showModal: vi.fn(), close: vi.fn() })
    const setSpy = vi.spyOn(document.documentElement, 'setAttribute')

    // allow initialize to run
    global.initialized = false
    shallowMount(App, { global: { plugins: [pinia] } })

    // wait for mounted hook to finish
    await new Promise((r) => setTimeout(r, 0))

    expect(http.auth).toHaveBeenCalled()
    expect(globalLoad).toHaveBeenCalled()
    expect(statusLoad).toHaveBeenCalled()
    expect(configLoad).toHaveBeenCalled()
    expect(saveState).toHaveBeenCalled()
    expect(global.initialized).toBe(true)

    setSpy.mockRestore()
    qSpy.mockRestore()
    globalLoad.mockRestore()
    statusLoad.mockRestore()
    configLoad.mockRestore()
    saveState.mockRestore()
  })

  it('initializeApp handles auth failure', async () => {
    const pinia = createTestingPinia()
    http.auth.mockResolvedValue(false)
    const qSpy = vi
      .spyOn(document, 'querySelector')
      .mockReturnValue({ showModal: vi.fn(), close: vi.fn() })

    global.initialized = false
    shallowMount(App, { global: { plugins: [pinia] } })
    await Promise.resolve()
    expect(global.messageError).toMatch(/Failed to authenticate/)
    qSpy.mockRestore()
  })
})
