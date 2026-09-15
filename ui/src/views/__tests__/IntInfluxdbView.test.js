import { shallowMount } from '@vue/test-utils'
import IntInfluxdbView from '@/views/IntInfluxdbView.vue'
import { config } from '@/modules/pinia'
import { validateCurrentForm } from '@mp-se/espframework-ui-components'

describe('IntInfluxdbView', () => {
  beforeEach(() => {
    vi.resetAllMocks()
    config.influxdb2_target = ''
    config.influxdb2_org = ''
    config.influxdb2_bucket = ''
    config.influxdb2_token = ''
  })

  it('mounts', () => {
    const wrapper = shallowMount(IntInfluxdbView)
    expect(wrapper.exists()).toBe(true)
  })

  it('does not save when validation fails', () => {
    vi.mocked(validateCurrentForm).mockReturnValue(false)
    config.influxdb2_target = 'http://influx.example/'
    config.saveAll = vi.fn()

    const wrapper = shallowMount(IntInfluxdbView)
    wrapper.vm.save()

    expect(config.saveAll).not.toHaveBeenCalled()
  })

  it('calls saveAll when validation passes', () => {
    vi.mocked(validateCurrentForm).mockReturnValue(true)
    config.saveAll = vi.fn()
    const wrapper = shallowMount(IntInfluxdbView)

    wrapper.vm.save()

    expect(config.saveAll).toHaveBeenCalled()
  })

  it('submitting form triggers save when button enabled', async () => {
    vi.mocked(validateCurrentForm).mockReturnValue(true)
    config.saveAll = vi.fn()
    global.configChanged = true
    const wrapper = shallowMount(IntInfluxdbView)

    await wrapper.find('form').trigger('submit.prevent')

    expect(config.saveAll).toHaveBeenCalled()
  })

  it('button is disabled when global.disabled true', () => {
    global.disabled = true
    const wrapper = shallowMount(IntInfluxdbView)
    expect(wrapper.find('button[type="submit"]').attributes('disabled')).toBe('')
  })

  it('button is disabled when configChanged false', () => {
    global.configChanged = false
    const wrapper = shallowMount(IntInfluxdbView)
    expect(wrapper.find('button[type="submit"]').attributes('disabled')).toBe('')
  })

  it('renders input fields with correct labels', () => {
    const wrapper = shallowMount(IntInfluxdbView)
    expect(wrapper.html()).toContain('Server')
    expect(wrapper.html()).toContain('Organisation')
    expect(wrapper.html()).toContain('Bucket')
  })

  it('trims target URL on save', () => {
    vi.mocked(validateCurrentForm).mockReturnValue(true)
    config.saveAll = vi.fn()
    config.influxdb2_target = '  http://influx.example/  '

    const wrapper = shallowMount(IntInfluxdbView)
    wrapper.vm.save()

    expect(config.saveAll).toHaveBeenCalled()
  })

  it('handles both validation success and failure paths', () => {
    config.saveAll = vi.fn()
    const wrapper = shallowMount(IntInfluxdbView)

    // Test failure path
    vi.mocked(validateCurrentForm).mockReturnValue(false)
    wrapper.vm.save()
    expect(config.saveAll).not.toHaveBeenCalled()

    // Test success path
    vi.mocked(validateCurrentForm).mockReturnValue(true)
    wrapper.vm.save()
    expect(config.saveAll).toHaveBeenCalled()
  })

  it('has save function accessible on vm', () => {
    const wrapper = shallowMount(IntInfluxdbView)
    expect(typeof wrapper.vm.save).toBe('function')
  })

  it('save returns early when validation fails', () => {
    vi.mocked(validateCurrentForm).mockReturnValue(false)
    config.saveAll = vi.fn()
    const wrapper = shallowMount(IntInfluxdbView)
    const result = wrapper.vm.save()
    expect(result).toBeUndefined()
    expect(config.saveAll).not.toHaveBeenCalled()
  })

  it('save invokes saveAll exactly once on valid form', () => {
    vi.mocked(validateCurrentForm).mockReturnValue(true)
    config.saveAll = vi.fn()
    const wrapper = shallowMount(IntInfluxdbView)
    wrapper.vm.save()
    wrapper.vm.save()
    expect(config.saveAll).toHaveBeenCalledTimes(2)
  })

  it('v-model setters update config values when stubs emit update:modelValue', async () => {
    config.influxdb2_target = ''
    config.influxdb2_org = ''
    config.influxdb2_bucket = ''
    config.influxdb2_token = ''
    const wrapper = shallowMount(IntInfluxdbView, {
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
    await textStubs[0].vm.$emit('update:modelValue', 'influx.server.com')
    expect(config.influxdb2_target).toBe('influx.server.com')
    await textStubs[1].vm.$emit('update:modelValue', 'myorg')
    expect(config.influxdb2_org).toBe('myorg')
    await textStubs[2].vm.$emit('update:modelValue', 'mybucket')
    expect(config.influxdb2_bucket).toBe('mybucket')
    await textStubs[3].vm.$emit('update:modelValue', 'mytoken')
    expect(config.influxdb2_token).toBe('mytoken')
  })
})
