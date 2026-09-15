import { shallowMount } from '@vue/test-utils'
import AboutView from '@/views/AboutView.vue'

describe('AboutView', () => {
  it('mounts', () => {
    const wrapper = shallowMount(AboutView)
    expect(wrapper.exists()).toBe(true)
  })
})
