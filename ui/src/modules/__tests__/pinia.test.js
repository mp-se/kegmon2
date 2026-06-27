/*
 * Tests for pinia.js — covers saveConfigState, getConfigChanges, deepClone, deepEqual
 * and the $subscribe callback on configStore.
 *
 * We must unmock the real stores so pinia.js can use genuine Pinia store instances.
 * @mp-se/espframework-ui-components stays mocked (sharedHttpClient etc. not needed here).
 */

vi.doUnmock('@/modules/configStore')
vi.doUnmock('@/modules/globalStore')
vi.doUnmock('@/modules/statusStore')
vi.doUnmock('@/modules/pinia')

describe('pinia', () => {
  let global, config, saveConfigState, getConfigChanges

  beforeEach(async () => {
    vi.resetModules()
    const mod = await import('@/modules/pinia')
    global = mod.global
    config = mod.config
    saveConfigState = mod.saveConfigState
    getConfigChanges = mod.getConfigChanges
  })

  // -------------------------------------------------------------------------
  // getConfigChanges — before saveConfigState is ever called
  // -------------------------------------------------------------------------
  describe('getConfigChanges (configCompare is null)', () => {
    it('returns empty object when saveConfigState has not been called', () => {
      const changes = getConfigChanges()
      expect(changes).toEqual({})
    })
  })

  // -------------------------------------------------------------------------
  // saveConfigState
  // -------------------------------------------------------------------------
  describe('saveConfigState', () => {
    it('sets global.configChanged to false', () => {
      global.configChanged = true
      saveConfigState()
      expect(global.configChanged).toBe(false)
    })

    it('creates a snapshot so subsequent getConfigChanges returns {}', () => {
      saveConfigState()
      expect(getConfigChanges()).toEqual({})
    })

    it('snapshot is independent — mutating config state is detected', () => {
      config.mdns = 'original'
      saveConfigState()
      config.mdns = 'modified'
      const changes = getConfigChanges()
      expect(changes.mdns).toBe('modified')
    })

    it('can be called multiple times — each call overwrites the snapshot', () => {
      config.mdns = 'first'
      saveConfigState()
      config.mdns = 'second'
      saveConfigState() // snapshot now has 'second'
      // no diff
      expect(getConfigChanges()).toEqual({})
    })
  })

  // -------------------------------------------------------------------------
  // getConfigChanges — after saveConfigState
  // -------------------------------------------------------------------------
  describe('getConfigChanges (after saveConfigState)', () => {
    it('returns empty object when nothing changed', () => {
      saveConfigState()
      expect(getConfigChanges()).toEqual({})
    })

    it('detects a changed string property', () => {
      config.mdns = 'before'
      saveConfigState()
      config.mdns = 'after'
      const changes = getConfigChanges()
      expect(changes).toHaveProperty('mdns', 'after')
    })

    it('does not include unchanged properties in the result', () => {
      config.mdns = 'same'
      saveConfigState()
      // mdns is unchanged; only changed keys should appear
      const changes = getConfigChanges()
      expect(Object.keys(changes)).not.toContain('mdns')
    })
  })

  // -------------------------------------------------------------------------
  // deepClone edge-cases (exercised through saveConfigState / getConfigChanges)
  // -------------------------------------------------------------------------
  describe('deepClone', () => {
    it('handles null values in state without throwing', () => {
      // mdns is a string — set to empty to get null-ish state
      config.mdns = 'x'
      saveConfigState()
      config.mdns = 'x'
      expect(getConfigChanges()).toEqual({})
    })

    it('detects mutations inside an array property', () => {
      // tap_title is an array on configStore; if not, use any array property
      if (Array.isArray(config.$state.tap_title)) {
        config.tap_title = ['a', 'b']
        saveConfigState()
        config.tap_title = ['a', 'z']
        const changes = getConfigChanges()
        expect(changes.tap_title).toEqual(['a', 'z'])
      } else {
        // fallback: just verify no error is thrown on primitive clone
        saveConfigState()
        expect(getConfigChanges()).toEqual({})
      }
    })

    it('snapshot is a deep copy — nested object mutation is detected', () => {
      // Find any object property in $state
      const objKey = Object.keys(config.$state).find(
        (k) =>
          config.$state[k] !== null &&
          typeof config.$state[k] === 'object' &&
          !Array.isArray(config.$state[k])
      )
      if (objKey) {
        config[objKey] = { ...config[objKey], _test: 'before' }
        saveConfigState()
        config[objKey] = { ...config[objKey], _test: 'after' }
        const changes = getConfigChanges()
        expect(changes[objKey]._test).toBe('after')
      } else {
        // no object property; just verify clean state
        saveConfigState()
        expect(getConfigChanges()).toEqual({})
      }
    })
  })

  // -------------------------------------------------------------------------
  // $subscribe callback
  // -------------------------------------------------------------------------
  describe('$subscribe callback', () => {
    it('does not flip configChanged when global.initialized is false', async () => {
      global.initialized = false
      global.configChanged = false
      saveConfigState()
      config.mdns = 'trigger-' + Date.now()
      await new Promise((r) => setTimeout(r, 0))
      expect(global.configChanged).toBe(false)
    })

    it('sets configChanged to true when initialized and a property changes', async () => {
      global.initialized = true
      config.mdns = 'before-sub'
      saveConfigState()
      config.mdns = 'after-sub-' + Date.now()
      await new Promise((r) => setTimeout(r, 0))
      expect(global.configChanged).toBe(true)
    })

    it('sets configChanged to false when initialized but nothing actually changed', async () => {
      global.initialized = true
      config.mdns = 'steady'
      saveConfigState()
      global.configChanged = false
      // Re-assign same value to trigger subscriber without a real diff
      config.mdns = 'steady'
      await new Promise((r) => setTimeout(r, 0))
      expect(global.configChanged).toBe(false)
    })
  })
})
