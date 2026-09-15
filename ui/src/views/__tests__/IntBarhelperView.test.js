import { shallowMount } from '@vue/test-utils'
import IntBarhelperView from '@/views/IntBarhelperView.vue'
import { config, global } from '@/modules/pinia'
import { validateCurrentForm } from '@mp-se/espframework-ui-components'

describe('IntBarhelperView', () => {
  beforeEach(() => {
    vi.resetAllMocks()
    config.barhelper_apikey = ''
    config.barhelper_monitors = ['', '', '', '']
    global.disabled = false
    global.configChanged = false
  })

  it('mounts', () => {
    const wrapper = shallowMount(IntBarhelperView)
    expect(wrapper.exists()).toBe(true)
  })

  it('does not save when validation fails', () => {
    vi.mocked(validateCurrentForm).mockReturnValue(false)
    config.barhelper_apikey = 'key'
    config.saveAll = vi.fn()

    const wrapper = shallowMount(IntBarhelperView)
    wrapper.vm.save()

    expect(config.saveAll).not.toHaveBeenCalled()
  })

  it('calls saveAll when validation passes', () => {
    vi.mocked(validateCurrentForm).mockReturnValue(true)
    config.saveAll = vi.fn()
    const wrapper = shallowMount(IntBarhelperView)

    wrapper.vm.save()

    expect(config.saveAll).toHaveBeenCalled()
  })

  it('submitting form triggers save', async () => {
    vi.mocked(validateCurrentForm).mockReturnValue(true)
    config.saveAll = vi.fn()
    global.configChanged = true
    const wrapper = shallowMount(IntBarhelperView)

    await wrapper.find('form').trigger('submit.prevent')

    expect(config.saveAll).toHaveBeenCalled()
  })

  it('has save function accessible on vm', () => {
    const wrapper = shallowMount(IntBarhelperView)
    expect(typeof wrapper.vm.save).toBe('function')
  })

  it('save returns early when validation fails', () => {
    vi.mocked(validateCurrentForm).mockReturnValue(false)
    config.saveAll = vi.fn()
    const wrapper = shallowMount(IntBarhelperView)
    const result = wrapper.vm.save()
    expect(result).toBeUndefined()
    expect(config.saveAll).not.toHaveBeenCalled()
  })

  it('save invokes saveAll when validation passes', () => {
    vi.mocked(validateCurrentForm).mockReturnValue(true)
    config.saveAll = vi.fn()
    const wrapper = shallowMount(IntBarhelperView)
    wrapper.vm.save()
    expect(config.saveAll).toHaveBeenCalledTimes(1)
  })

  it('button is disabled when global.disabled true', () => {
    global.disabled = true
    const wrapper = shallowMount(IntBarhelperView)
    expect(wrapper.find('button[type="submit"]').attributes('disabled')).toBe('')
  })

  it('button is disabled when configChanged false', () => {
    global.configChanged = false
    const wrapper = shallowMount(IntBarhelperView)
    expect(wrapper.find('button[type="submit"]').attributes('disabled')).toBe('')
  })

  it('button is enabled when both disabled=false and configChanged=true', () => {
    global.disabled = false
    global.configChanged = true
    const wrapper = shallowMount(IntBarhelperView)
    expect(wrapper.find('button[type="submit"]').attributes('disabled')).toBeUndefined()
  })

  it('v-model setters update config values when stubs emit update:modelValue', async () => {
    config.barhelper_apikey = ''
    config.barhelper_monitors = ['', '', '', '']
    const wrapper = shallowMount(IntBarhelperView, {
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
    await textStubs[0].vm.$emit('update:modelValue', 'apikey123')
    expect(config.barhelper_apikey).toBe('apikey123')
    await textStubs[1].vm.$emit('update:modelValue', 'monitor0')
    expect(config.barhelper_monitors[0]).toBe('monitor0')
  })
})
