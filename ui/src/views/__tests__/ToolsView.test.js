import { shallowMount } from '@vue/test-utils'
import ToolsView from '@/views/ToolsView.vue'
import { global } from '@/modules/pinia'

describe('ToolsView', () => {
  beforeEach(() => {
    vi.resetAllMocks()
    // default UI flags
    global.ui = { enableVoltageFragment: false }
  })

  it('mounts', () => {
    const wrapper = shallowMount(ToolsView)
    expect(wrapper.exists()).toBe(true)
  })

  it('hideAdvanced toggles when enableAdvanced called', () => {
    const wrapper = shallowMount(ToolsView)
    // initial is true
    expect(wrapper.vm.hideAdvanced).toBe(true)
    wrapper.vm.enableAdvanced()
    expect(wrapper.vm.hideAdvanced).toBe(false)
    wrapper.vm.enableAdvanced()
    expect(wrapper.vm.hideAdvanced).toBe(true)
  })

  it('renders VoltageFragment when ui flag enabled', () => {
    global.ui.enableVoltageFragment = true
    const wrapper = shallowMount(ToolsView)
    // VoltageFragment is stubbed; ensure template branch exists by checking stub tag
    expect(wrapper.find('voltage-fragment-stub').exists()).toBe(true)
  })

  it('does not render VoltageFragment when ui flag disabled', () => {
    global.ui.enableVoltageFragment = false
    const wrapper = shallowMount(ToolsView)
    expect(wrapper.find('voltage-fragment-stub').exists()).toBe(false)
  })

  it('renders EnableAdvanced button when hideAdvanced is true', () => {
    const wrapper = shallowMount(ToolsView)
    expect(wrapper.find('button').text()).toContain('Enable Advanced')
  })

  it('button is disabled when global.disabled is true', () => {
    global.disabled = true
    const wrapper = shallowMount(ToolsView)
    expect(wrapper.find('button').attributes('disabled')).toBe('')
  })

  it('button is enabled when global.disabled is false', () => {
    global.disabled = false
    const wrapper = shallowMount(ToolsView)
    expect(wrapper.find('button').attributes('disabled')).toBeUndefined()
  })

  it('renders ListFilesFragment always', () => {
    const wrapper = shallowMount(ToolsView)
    expect(wrapper.find('list-files-fragment-stub').exists()).toBe(true)
  })

  it('renders AdvancedFilesFragment when hideAdvanced becomes false', async () => {
    const wrapper = shallowMount(ToolsView)
    expect(wrapper.vm.hideAdvanced).toBe(true)
    wrapper.vm.hideAdvanced = false
    await wrapper.vm.$nextTick()
    expect(wrapper.find('advanced-files-fragment-stub').exists()).toBe(true)
  })

  it('renders EnableCorsFragment when hideAdvanced becomes false', async () => {
    const wrapper = shallowMount(ToolsView)
    wrapper.vm.hideAdvanced = false
    await wrapper.vm.$nextTick()
    expect(wrapper.find('enable-cors-fragment-stub').exists()).toBe(true)
  })

  it('clicking the enableAdvanced button toggles hideAdvanced', async () => {
    const wrapper = shallowMount(ToolsView)
    expect(wrapper.vm.hideAdvanced).toBe(true)
    await wrapper.find('button').trigger('click')
    expect(wrapper.vm.hideAdvanced).toBe(false)
  })
})
