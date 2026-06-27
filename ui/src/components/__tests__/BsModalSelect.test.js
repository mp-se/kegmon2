import { shallowMount } from '@vue/test-utils'
import { describe, it, expect, vi } from 'vitest'
import BsModalSelect from '@/components/BsModalSelect.vue'

describe('BsModalSelect', () => {
  it('renders message and uses provided title', () => {
    const cb = vi.fn()
    const wrapper = shallowMount(BsModalSelect, {
      props: { callback: cb, id: 'btn1', title: 'Pick', message: 'choose one', options: [] }
    })

    expect(wrapper.text()).toContain('choose one')
    const btn = wrapper.find('button')
    expect(btn.attributes('id')).toBe('btn1')
  })

  it('shows Processing... when disabled', () => {
    const cb = vi.fn()
    const wrapper = shallowMount(BsModalSelect, {
      props: { callback: cb, id: 'btn2', title: 'Pick', disabled: true, options: [] }
    })

    expect(wrapper.text()).toContain('Processing...')
  })

  it('confirm emits update and calls callback with true and value', () => {
    const cb = vi.fn()
    const wrapper = shallowMount(BsModalSelect, {
      props: { callback: cb, id: 'btn3', title: 'Pick', options: [] }
    })

    // set the selected result and call confirm
    wrapper.vm.result = 'opt1'
    wrapper.vm.confirm()

    expect(wrapper.emitted()['update:modelValue'][0]).toEqual(['opt1'])
    expect(cb).toHaveBeenCalledWith(true, 'opt1')
  })

  it('cancel calls callback with false and empty value', () => {
    const cb = vi.fn()
    const wrapper = shallowMount(BsModalSelect, {
      props: { callback: cb, id: 'btn4', title: 'Pick', options: [] }
    })

    wrapper.vm.cancel()
    expect(cb).toHaveBeenCalledWith(false, '')
  })
})
