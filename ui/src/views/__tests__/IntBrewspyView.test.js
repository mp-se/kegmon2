import { shallowMount } from '@vue/test-utils'
import IntBrewspyView from '@/views/IntBrewspyView.vue'
import { config, global } from '@/modules/pinia'
import { validateCurrentForm } from '@mp-se/espframework-ui-components'

describe('IntBrewspyView', () => {
  beforeEach(() => {
    vi.resetAllMocks()
    config.brewspy_tokens = ['', '', '', '']
    global.disabled = false
    global.configChanged = false
  })

  it('mounts', () => {
    const wrapper = shallowMount(IntBrewspyView)
    expect(wrapper.exists()).toBe(true)
  })

  it('does not save when validation fails', () => {
    vi.mocked(validateCurrentForm).mockReturnValue(false)
    config.brewspy_tokens = ['a', 'b', 'c', 'd']
    config.saveAll = vi.fn()

    const wrapper = shallowMount(IntBrewspyView)
    wrapper.vm.save()

    expect(config.saveAll).not.toHaveBeenCalled()
    expect(config.brewspy_tokens[0]).toBe('a')
  })

  it('calls saveAll when validation passes', () => {
    vi.mocked(validateCurrentForm).mockReturnValue(true)
    config.saveAll = vi.fn()
    const wrapper = shallowMount(IntBrewspyView)

    wrapper.vm.save()

    expect(config.saveAll).toHaveBeenCalled()
  })

  it('submitting form triggers save', async () => {
    vi.mocked(validateCurrentForm).mockReturnValue(true)
    config.saveAll = vi.fn()
    global.configChanged = true
    const wrapper = shallowMount(IntBrewspyView)

    await wrapper.find('form').trigger('submit.prevent')

    expect(config.saveAll).toHaveBeenCalled()
  })

  it('has save function accessible on vm', () => {
    const wrapper = shallowMount(IntBrewspyView)
    expect(typeof wrapper.vm.save).toBe('function')
  })

  it('save returns early when validation fails', () => {
    vi.mocked(validateCurrentForm).mockReturnValue(false)
    config.saveAll = vi.fn()
    const wrapper = shallowMount(IntBrewspyView)
    const result = wrapper.vm.save()
    expect(result).toBeUndefined()
    expect(config.saveAll).not.toHaveBeenCalled()
  })

  it('save invokes saveAll when form is valid', () => {
    vi.mocked(validateCurrentForm).mockReturnValue(true)
    config.saveAll = vi.fn()
    const wrapper = shallowMount(IntBrewspyView)
    wrapper.vm.save()
    expect(config.saveAll).toHaveBeenCalledTimes(1)
  })

  it('button is disabled when global.disabled true', () => {
    global.disabled = true
    const wrapper = shallowMount(IntBrewspyView)
    expect(wrapper.find('button[type="submit"]').attributes('disabled')).toBe('')
  })

  it('button is disabled when configChanged false', () => {
    global.configChanged = false
    const wrapper = shallowMount(IntBrewspyView)
    expect(wrapper.find('button[type="submit"]').attributes('disabled')).toBe('')
  })

  it('button is enabled when both disabled and configChanged conditions met', () => {
    global.disabled = false
    global.configChanged = true
    const wrapper = shallowMount(IntBrewspyView)
    expect(wrapper.find('button[type="submit"]').attributes('disabled')).toBeUndefined()
  })

  it('v-model setters update config.brewspy_tokens when stubs emit update:modelValue', async () => {
    config.brewspy_tokens = ['', '', '', '']
    const wrapper = shallowMount(IntBrewspyView, {
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
    await textStubs[0].vm.$emit('update:modelValue', 'token0')
    expect(config.brewspy_tokens[0]).toBe('token0')
    await textStubs[1].vm.$emit('update:modelValue', 'token1')
    expect(config.brewspy_tokens[1]).toBe('token1')
  })
})
