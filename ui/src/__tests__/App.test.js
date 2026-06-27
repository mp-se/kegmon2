import { describe, it, expect } from 'vitest'
import { shallowMount } from '@vue/test-utils'
import App from '../App.vue'
import { createTestingPinia } from '../tests/testUtils'

describe('App.vue (smoke)', () => {
  it('mounts without error', () => {
    const pinia = createTestingPinia()
    const wrapper = shallowMount(App, { global: { plugins: [pinia] } })
    expect(wrapper.exists()).toBe(true)
  })
})
