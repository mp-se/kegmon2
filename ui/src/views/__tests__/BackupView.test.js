import { shallowMount } from '@vue/test-utils'
import BackupView from '@/views/BackupView.vue'
import { global, config } from '@/modules/pinia'

describe('BackupView', () => {
  beforeEach(() => {
    global.messageSuccess = ''
    global.messageFailed = ''
    global.disabled = false
    config.mdns = 'keg'
    config.toJson = () => JSON.stringify({ a: 1 })
    config.saveAll = vi.fn()
  })

  it('mounts', () => {
    const wrapper = shallowMount(BackupView)
    expect(wrapper.exists()).toBe(true)
  })

  it('backup creates download and sets success message', () => {
    const wrapper = shallowMount(BackupView)

    const clickSpy = vi.fn()
    const aMock = {
      setAttribute: vi.fn(),
      click: clickSpy
    }

    const createSpy = vi.spyOn(document, 'createElement').mockReturnValue(aMock)
    const urlSpy = vi.spyOn(URL, 'createObjectURL').mockReturnValue('blob:fake')

    wrapper.vm.backup()

    expect(createSpy).toHaveBeenCalledWith('a')
    expect(urlSpy).toHaveBeenCalled()
    expect(clickSpy).toHaveBeenCalled()
    expect(global.messageSuccess).toMatch(/Backup file created and downloaded/)

    createSpy.mockRestore()
    urlSpy.mockRestore()
  })

  it('restore sets messageFailed when no file selected', () => {
    const wrapper = shallowMount(BackupView)
    const el = { files: [] }
    const getSpy = vi.spyOn(document, 'getElementById').mockReturnValue(el)

    wrapper.vm.restore()
    expect(global.messageFailed).toMatch(/You need to select one file/)

    getSpy.mockRestore()
  })

  it('restore parses valid v1 backup and calls saveAll', async () => {
    const wrapper = shallowMount(BackupView)
    const file = { name: 'b.txt' }
    const el = { files: [file] }
    const getSpy = vi.spyOn(document, 'getElementById').mockReturnValue(el)

    // mock FileReader to synchronously invoke load with v1 payload
    const payload = JSON.stringify({
      meta: { software: 'KegMon', version: '1.0.0' },
      config: { foo: 'bar' }
    })
    const mockReader = {
      addEventListener(evt, cb) {
        if (evt === 'load') this._cb = cb
      },
      readAsText() {
        this._cb({ target: { result: payload } })
      }
    }

    // stub global FileReader
    vi.stubGlobal('FileReader', function () {
      return mockReader
    })

    await wrapper.vm.restore()

    expect(config.saveAll).toHaveBeenCalled()

    // restore
    vi.unstubAllGlobals()
    getSpy.mockRestore()
  })

  it('restore handles malformed JSON', async () => {
    const wrapper = shallowMount(BackupView)
    const file = { name: 'bad.txt' }
    const el = { files: [file] }
    const getSpy = vi.spyOn(document, 'getElementById').mockReturnValue(el)

    const mockReader = {
      addEventListener(evt, cb) {
        if (evt === 'load') this._cb = cb
      },
      readAsText() {
        this._cb({ target: { result: 'not-json' } })
      }
    }

    vi.stubGlobal('FileReader', function () {
      return mockReader
    })

    await wrapper.vm.restore()
    expect(global.messageFailed).toMatch(/Unable to parse configuration file/)

    vi.unstubAllGlobals()
    getSpy.mockRestore()
  })

  it('restore calls doRestore1 when meta.software is KegMon and version is 1.0.0', async () => {
    config.saveAll = vi.fn()
    const wrapper = shallowMount(BackupView)
    const file = { name: 'kegmon.json' }
    const el = { files: [file] }
    const getSpy = vi.spyOn(document, 'getElementById').mockReturnValue(el)

    const backupData = {
      meta: { software: 'KegMon', version: '1.0.0' },
      config: { mdns: 'kegmon-test', temp_unit: 'C' }
    }

    const mockReader = {
      addEventListener(evt, cb) {
        if (evt === 'load') this._cb = cb
      },
      readAsText() {
        this._cb({ target: { result: JSON.stringify(backupData) } })
      }
    }

    vi.stubGlobal('FileReader', function () {
      return mockReader
    })

    await wrapper.vm.restore()
    expect(config.saveAll).toHaveBeenCalled()

    vi.unstubAllGlobals()
    getSpy.mockRestore()
  })

  it('restore calls doRestore when meta.software is KegMon but version differs', async () => {
    config.saveAll = vi.fn()
    const wrapper = shallowMount(BackupView)
    const file = { name: 'kegmon.json' }
    const el = { files: [file] }
    const getSpy = vi.spyOn(document, 'getElementById').mockReturnValue(el)

    const backupData = {
      meta: { software: 'KegMon', version: '2.0.0' },
      config: { mdns: 'kegmon-test' }
    }

    const mockReader = {
      addEventListener(evt, cb) {
        if (evt === 'load') this._cb = cb
      },
      readAsText() {
        this._cb({ target: { result: JSON.stringify(backupData) } })
      }
    }

    vi.stubGlobal('FileReader', function () {
      return mockReader
    })

    await wrapper.vm.restore()
    expect(config.saveAll).toHaveBeenCalled()

    vi.unstubAllGlobals()
    getSpy.mockRestore()
  })

  it('restore sets messageFailed when meta.software is unknown', async () => {
    const wrapper = shallowMount(BackupView)
    const file = { name: 'other.json' }
    const el = { files: [file] }
    const getSpy = vi.spyOn(document, 'getElementById').mockReturnValue(el)

    const backupData = {
      meta: { software: 'OtherApp', version: '1.0' },
      config: {}
    }

    const mockReader = {
      addEventListener(evt, cb) {
        if (evt === 'load') this._cb = cb
      },
      readAsText() {
        this._cb({ target: { result: JSON.stringify(backupData) } })
      }
    }

    vi.stubGlobal('FileReader', function () {
      return mockReader
    })

    await wrapper.vm.restore()
    expect(global.messageFailed).toMatch(/Unknown format/)

    vi.unstubAllGlobals()
    getSpy.mockRestore()
  })
})
