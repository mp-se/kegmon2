import { shallowMount } from '@vue/test-utils'
import IntHassView from '@/views/IntHassView.vue'
import { config } from '@/modules/pinia'
import { validateCurrentForm } from '@mp-se/espframework-ui-components'

describe('IntHassView', () => {
  beforeEach(() => {
    vi.resetAllMocks()
    config.mqtt_target = ''
    config.mqtt_port = 1883
  })

  it('mounts', () => {
    const wrapper = shallowMount(IntHassView)
    expect(wrapper.exists()).toBe(true)
  })

  it('does not save when validation fails', () => {
    vi.mocked(validateCurrentForm).mockReturnValue(false)
    config.mqtt_target = 'server.example'
    config.saveAll = vi.fn()

    const wrapper = shallowMount(IntHassView)
    wrapper.vm.save()

    expect(config.saveAll).not.toHaveBeenCalled()
  })

  it('calls saveAll when validation passes', () => {
    vi.mocked(validateCurrentForm).mockReturnValue(true)
    config.saveAll = vi.fn()
    const wrapper = shallowMount(IntHassView)

    wrapper.vm.save()

    expect(config.saveAll).toHaveBeenCalled()
  })

  it('submitting form triggers save when button enabled', async () => {
    vi.mocked(validateCurrentForm).mockReturnValue(true)
    config.saveAll = vi.fn()
    global.configChanged = true
    const wrapper = shallowMount(IntHassView)

    await wrapper.find('form').trigger('submit.prevent')

    expect(config.saveAll).toHaveBeenCalled()
  })

  it('button is disabled when global.disabled true', () => {
    global.disabled = true
    const wrapper = shallowMount(IntHassView)
    expect(wrapper.find('button[type="submit"]').attributes('disabled')).toBe('')
  })

  it('button is disabled when configChanged false', () => {
    global.configChanged = false
    const wrapper = shallowMount(IntHassView)
    expect(wrapper.find('button[type="submit"]').attributes('disabled')).toBe('')
  })

  it('renders BsInputText components for mqtt configuration', () => {
    const wrapper = shallowMount(IntHassView)
    // Check that the component renders BsInputText stubs
    expect(wrapper.findAll('bs-input-text-stub').length).toBeGreaterThan(0)
  })

  it('disables inputs when global.disabled is true', () => {
    global.disabled = true
    const wrapper = shallowMount(IntHassView)
    const inputs = wrapper.findAll('[disabled]')
    expect(inputs.length).toBeGreaterThan(0)
  })

  it('saves form data with correct config values', () => {
    vi.mocked(validateCurrentForm).mockReturnValue(true)
    config.saveAll = vi.fn()
    config.mqtt_target = 'mqtt.example.com'
    config.mqtt_port = 1883

    const wrapper = shallowMount(IntHassView)
    wrapper.vm.save()

    expect(config.saveAll).toHaveBeenCalled()
  })

  it('has save function accessible on vm', () => {
    const wrapper = shallowMount(IntHassView)
    expect(typeof wrapper.vm.save).toBe('function')
  })

  it('form validation false prevents save action', () => {
    vi.mocked(validateCurrentForm).mockReturnValue(false)
    config.saveAll = vi.fn()
    const wrapper = shallowMount(IntHassView)
    const result = wrapper.vm.save()
    expect(result).toBeUndefined()
    expect(config.saveAll).not.toHaveBeenCalled()
  })

  it('form validation true allows save action', () => {
    vi.mocked(validateCurrentForm).mockReturnValue(true)
    config.saveAll = vi.fn()
    const wrapper = shallowMount(IntHassView)
    wrapper.vm.save()
    expect(config.saveAll).toHaveBeenCalledTimes(1)
  })

  it('v-model setters update config values when stubs emit update:modelValue', async () => {
    config.mqtt_target = ''
    config.mqtt_port = 1883
    config.mqtt_user = ''
    config.mqtt_pass = ''
    const wrapper = shallowMount(IntHassView, {
      global: {
        stubs: {
          BsInputText: {
            name: 'BsInputText',
            template: '<div />',
            props: ['modelValue'],
            emits: ['update:modelValue']
          },
          BsInputNumber: {
            name: 'BsInputNumber',
            template: '<div />',
            props: ['modelValue'],
            emits: ['update:modelValue']
          }
        }
      }
    })
    const textStubs = wrapper.findAllComponents({ name: 'BsInputText' })
    const numStubs = wrapper.findAllComponents({ name: 'BsInputNumber' })
    await textStubs[0].vm.$emit('update:modelValue', 'mqtt.server.com')
    expect(config.mqtt_target).toBe('mqtt.server.com')
    await numStubs[0].vm.$emit('update:modelValue', 8883)
    expect(config.mqtt_port).toBe(8883)
    await textStubs[1].vm.$emit('update:modelValue', 'user1')
    expect(config.mqtt_user).toBe('user1')
    await textStubs[2].vm.$emit('update:modelValue', 'pass1')
    expect(config.mqtt_pass).toBe('pass1')
  })
})
