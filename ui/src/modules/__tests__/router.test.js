import { describe, it, expect, vi, beforeEach } from 'vitest'
import { validateCurrentForm } from '@mp-se/espframework-ui-components'
import { global } from '@/modules/pinia'
import router from '@/modules/router'

describe('router navigation guard', () => {
  beforeEach(async () => {
    vi.resetAllMocks()
    global.disabled = false
    global.clearMessages = vi.fn()
    vi.mocked(validateCurrentForm).mockReturnValue(true)
    // Always start from home so we have somewhere to navigate from
    await router.push('/')
  })

  it('allows navigation and calls clearMessages when conditions pass', async () => {
    global.disabled = false
    vi.mocked(validateCurrentForm).mockReturnValue(true)
    const result = await router.push('/about')
    expect(result).toBeUndefined() // no NavigationFailure = success
    expect(global.clearMessages).toHaveBeenCalled()
  })

  it('blocks navigation when global.disabled is true', async () => {
    global.disabled = true
    const result = await router.push('/about')
    // NavigationFailure has a truthy type property
    expect(result).toBeTruthy()
  })

  it('blocks navigation when validateCurrentForm returns false', async () => {
    global.disabled = false
    vi.mocked(validateCurrentForm).mockReturnValue(false)
    const result = await router.push('/about')
    expect(result).toBeTruthy()
    expect(global.clearMessages).not.toHaveBeenCalled()
  })
})
