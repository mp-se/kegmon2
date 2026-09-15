import { shallowMount } from '@vue/test-utils'
import IntBrewloggerView from '@/views/IntBrewloggerView.vue'
import { config, global } from '@/modules/pinia'
import { validateCurrentForm } from '@mp-se/espframework-ui-components'

describe('IntBrewloggerView', () => {
  beforeEach(() => {
    vi.resetAllMocks()
    // ensure default config url
    config.brewlogger_url = ''
    global.disabled = false
    global.configChanged = false
  })

  it('mounts', () => {
    const wrapper = shallowMount(IntBrewloggerView)
    expect(wrapper.exists()).toBe(true)
  })

  it('does not save when validation fails', () => {
    vi.mocked(validateCurrentForm).mockReturnValue(false)
    config.brewlogger_url = 'http://example.com/'
    const wrapper = shallowMount(IntBrewloggerView)

    wrapper.vm.save()

    expect(config.brewlogger_url).toBe('http://example.com/')
    expect(config.saveAll).not.toHaveBeenCalled()
  })

  it('trims trailing slash and calls saveAll when valid', () => {
    vi.mocked(validateCurrentForm).mockReturnValue(true)
    config.brewlogger_url = 'http://example.com/'
    config.saveAll = vi.fn()
    const wrapper = shallowMount(IntBrewloggerView)

    wrapper.vm.save()

    expect(config.brewlogger_url).toBe('http://example.com')
    expect(config.saveAll).toHaveBeenCalled()
  })

  it('submitting form triggers save when button enabled', async () => {
    vi.mocked(validateCurrentForm).mockReturnValue(true)
    config.saveAll = vi.fn()
    global.configChanged = true
    const wrapper = shallowMount(IntBrewloggerView)

    await wrapper.find('form').trigger('submit.prevent')

    expect(config.saveAll).toHaveBeenCalled()
  })

  it('button is disabled when global.disabled true', () => {
    global.disabled = true
    const wrapper = shallowMount(IntBrewloggerView)
    expect(wrapper.find('button[type="submit"]').attributes('disabled')).toBe('')
  })

  it('has save function accessible on vm', () => {
    const wrapper = shallowMount(IntBrewloggerView)
    expect(typeof wrapper.vm.save).toBe('function')
  })

  it('save returns early when validation fails', () => {
    vi.mocked(validateCurrentForm).mockReturnValue(false)
    config.saveAll = vi.fn()
    const wrapper = shallowMount(IntBrewloggerView)
    const result = wrapper.vm.save()
    expect(result).toBeUndefined()
    expect(config.saveAll).not.toHaveBeenCalled()
  })

  it('save validates form before proceeding', () => {
    vi.mocked(validateCurrentForm).mockReturnValue(false)
    config.saveAll = vi.fn()
    const wrapper = shallowMount(IntBrewloggerView)
    wrapper.vm.save()
    expect(config.saveAll).not.toHaveBeenCalled()

    vi.mocked(validateCurrentForm).mockReturnValue(true)
    wrapper.vm.save()
    expect(config.saveAll).toHaveBeenCalled()
  })

  it('does not trim URL when it has no trailing slash', () => {
    vi.mocked(validateCurrentForm).mockReturnValue(true)
    config.saveAll = vi.fn()
    config.brewlogger_url = 'http://example.com'
    const wrapper = shallowMount(IntBrewloggerView)
    wrapper.vm.save()
    expect(config.brewlogger_url).toBe('http://example.com')
  })

  it('button is enabled when global.disabled and configChanged are both true', () => {
    global.disabled = false
    global.configChanged = true
    const wrapper = shallowMount(IntBrewloggerView)
    expect(wrapper.find('button[type="submit"]').attributes('disabled')).toBeUndefined()
  })

  it('v-model setter updates config.brewlogger_url when stub emits update:modelValue', async () => {
    config.brewlogger_url = ''
    const wrapper = shallowMount(IntBrewloggerView, {
      global: {
        stubs: {
          BsInputText: {
            name: 'BsInputText',
            template: '<div />',
            props: ['modelValue'],
            emits: ['update:modelValue']
          }
        }
      }
    })
    const textStubs = wrapper.findAllComponents({ name: 'BsInputText' })
    await textStubs[0].vm.$emit('update:modelValue', 'http://logger.example.com')
    expect(config.brewlogger_url).toBe('http://logger.example.com')
  })
})
