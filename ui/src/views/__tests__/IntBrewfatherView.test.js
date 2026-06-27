import { shallowMount } from '@vue/test-utils'
import IntBrewfatherView from '@/views/IntBrewfatherView.vue'
import { config, global } from '@/modules/pinia'
import { validateCurrentForm } from '@mp-se/espframework-ui-components'

describe('IntBrewfatherView', () => {
  beforeEach(() => {
    vi.resetAllMocks()
    config.brewfather_apikey = ''
    config.brewfather_userkey = ''
    global.disabled = false
    global.configChanged = false
  })

  it('mounts', () => {
    const wrapper = shallowMount(IntBrewfatherView)
    expect(wrapper.exists()).toBe(true)
  })

  it('does not save when validation fails', () => {
    vi.mocked(validateCurrentForm).mockReturnValue(false)
    config.saveAll = vi.fn()
    const wrapper = shallowMount(IntBrewfatherView)

    wrapper.vm.save()

    expect(config.saveAll).not.toHaveBeenCalled()
  })

  it('calls saveAll when validation passes', () => {
    vi.mocked(validateCurrentForm).mockReturnValue(true)
    config.saveAll = vi.fn()
    const wrapper = shallowMount(IntBrewfatherView)

    wrapper.vm.save()

    expect(config.saveAll).toHaveBeenCalled()
  })

  it('submitting form triggers save', async () => {
    vi.mocked(validateCurrentForm).mockReturnValue(true)
    config.saveAll = vi.fn()
    global.configChanged = true
    const wrapper = shallowMount(IntBrewfatherView)

    await wrapper.find('form').trigger('submit.prevent')

    expect(config.saveAll).toHaveBeenCalled()
  })

  it('has save function accessible on vm', () => {
    const wrapper = shallowMount(IntBrewfatherView)
    expect(typeof wrapper.vm.save).toBe('function')
  })

  it('save returns early when validation fails', () => {
    vi.mocked(validateCurrentForm).mockReturnValue(false)
    config.saveAll = vi.fn()
    const wrapper = shallowMount(IntBrewfatherView)
    const result = wrapper.vm.save()
    expect(result).toBeUndefined()
    expect(config.saveAll).not.toHaveBeenCalled()
  })

  it('save calls saveAll when validation succeeds', () => {
    vi.mocked(validateCurrentForm).mockReturnValue(true)
    config.saveAll = vi.fn()
    const wrapper = shallowMount(IntBrewfatherView)
    wrapper.vm.save()
    expect(config.saveAll).toHaveBeenCalledTimes(1)
  })

  it('button is disabled when global.disabled is true', () => {
    global.disabled = true
    const wrapper = shallowMount(IntBrewfatherView)
    expect(wrapper.find('button[type="submit"]').attributes('disabled')).toBe('')
  })

  it('button is disabled when configChanged is false', () => {
    global.configChanged = false
    const wrapper = shallowMount(IntBrewfatherView)
    expect(wrapper.find('button[type="submit"]').attributes('disabled')).toBe('')
  })

  it('button is enabled when disabled and configChanged are true', () => {
    global.disabled = false
    global.configChanged = true
    const wrapper = shallowMount(IntBrewfatherView)
    expect(wrapper.find('button[type="submit"]').attributes('disabled')).toBeUndefined()
  })

  it('v-model setters update config values when stubs emit update:modelValue', async () => {
    config.brewfather_apikey = ''
    config.brewfather_userkey = ''
    const wrapper = shallowMount(IntBrewfatherView, {
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
    expect(config.brewfather_apikey).toBe('apikey123')
    await textStubs[1].vm.$emit('update:modelValue', 'userkey456')
    expect(config.brewfather_userkey).toBe('userkey456')
  })
})
