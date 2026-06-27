/*
 * Kegmon
 * Copyright (c) 2024-2026 Magnus
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Alternatively, this software may be used under the terms of a
 * commercial license. See LICENSE_COMMERCIAL for details.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

import { defineStore } from 'pinia'
import { global, saveConfigState, getConfigChanges } from '@/modules/pinia'
import {
  logDebug,
  logError,
  logInfo,
  sharedHttpClient as http
} from '@mp-se/espframework-ui-components'

export const useConfigStore = defineStore('config', {
  state: () => {
    return {
      // Device
      id: '',
      mdns: '',
      temp_unit: '',
      weight_unit: '',
      volume_unit: '',
      dark_mode: false,
      display_layout: 0,
      // Hardware
      ota_url: '',
      // Wifi
      wifi_portal_timeout: 0,
      wifi_connect_timeout: 0,
      wifi_ssid: '',
      wifi_ssid2: '',
      wifi_pass: '',
      wifi_pass2: '',
      // Integrations
      brewfather_apikey: '',
      brewfather_userkey: '',
      brewlogger_url: '',
      brewspy_tokens: ['', '', '', ''],
      barhelper_apikey: '',
      barhelper_monitors: ['', '', '', ''],
      // Push - Generic
      push_timeout: 10,
      // Push - Http Post 1
      http_post_target: '',
      http_post_header1: '',
      http_post_header2: '',
      // Push - Http Post 2
      http_post2_target: '',
      http_post2_header1: '',
      http_post2_header2: '',
      // Push - Http Get
      http_get_target: '',
      http_get_header1: '',
      http_get_header2: '',
      // Push - Influx
      influxdb2_target: '',
      influxdb2_org: '',
      influxdb2_bucket: '',
      influxdb2_token: '',
      // Push - MQTT
      mqtt_target: '',
      mqtt_port: 1883,
      mqtt_user: '',
      mqtt_pass: '',
      // Arrays
      scales: [
        {
          scale_factor: 0,
          scale_offset: 0,
          keg_weight: 0,
          keg_volume: 0,
          glass_volume: 0,
          temp_sensor_id: ''
        },
        {
          scale_factor: 0,
          scale_offset: 0,
          keg_weight: 0,
          keg_volume: 0,
          glass_volume: 0,
          temp_sensor_id: ''
        },
        {
          scale_factor: 0,
          scale_offset: 0,
          keg_weight: 0,
          keg_volume: 0,
          glass_volume: 0,
          temp_sensor_id: ''
        },
        {
          scale_factor: 0,
          scale_offset: 0,
          keg_weight: 0,
          keg_volume: 0,
          glass_volume: 0,
          temp_sensor_id: ''
        }
      ],
      beers: [
        {
          beer_name: '',
          beer_id: '',
          beer_abv: 0,
          beer_fg: 0,
          beer_ebc: 0,
          beer_ibu: 0
        },
        {
          beer_name: '',
          beer_id: '',
          beer_abv: 0,
          beer_fg: 0,
          beer_ebc: 0,
          beer_ibu: 0
        },
        {
          beer_name: '',
          beer_id: '',
          beer_abv: 0,
          beer_fg: 0,
          beer_ebc: 0,
          beer_ibu: 0
        },
        {
          beer_name: '',
          beer_id: '',
          beer_abv: 0,
          beer_fg: 0,
          beer_ebc: 0,
          beer_ibu: 0
        }
      ]
    }
  },
  getters: {
    getVolumeUnit() {
      return this.volume_unit == 'cl' ? 'cl' : 'fl. oz'
    },
    getWeightUnit() {
      return this.weight_unit == 'kg' ? 'kg' : 'lbs'
    },
    getTempUnit() {
      return this.temp_unit == 'C' ? '°C' : '°F'
    },
    // Weight unit helpers
    isWeightKg() {
      return this.weight_unit === 'kg'
    },
    isWeightLbs() {
      return this.weight_unit === 'lbs'
    },
    // Temperature unit helpers
    isTempC() {
      return this.temp_unit === 'C'
    },
    isTempF() {
      return this.temp_unit === 'F'
    },
    // Volume unit helpers
    isVolumeCl() {
      return this.volume_unit === 'cl'
    },
    isVolumeUsOz() {
      return this.volume_unit === 'us-oz'
    },
    isVolumeUkOz() {
      return this.volume_unit === 'uk-oz'
    }
  },
  actions: {
    toJson() {
      logInfo('configStore.toJSON()')
      var dest = {}

      for (var key in this.$state) {
        if (!key.startsWith('$')) {
          dest[key] = this[key]
        }
      }

      logInfo('configStore.toJSON()', dest)
      return JSON.stringify(dest, null, 2)
    },
    async load() {
      global.disabled = true
      logInfo('configStore.load()', 'Fetching /api/config')
      try {
        const json = await http.getJson('api/config')
        logDebug('configStore.load()', json)
        global.disabled = false

        const maxKegs = global.feature.no_scales

        this.id = json.id
        this.mdns = json.mdns
        this.temp_unit = json.temp_unit
        this.weight_unit = json.weight_unit
        this.volume_unit = json.volume_unit
        this.dark_mode = json.dark_mode
        this.display_layout = json.display_layout

        // Hardware
        this.ota_url = json.ota_url

        // Wifi
        this.wifi_portal_timeout = json.wifi_portal_timeout
        this.wifi_connect_timeout = json.wifi_connect_timeout
        this.wifi_ssid = json.wifi_ssid
        this.wifi_ssid2 = json.wifi_ssid2
        this.wifi_pass = json.wifi_pass
        this.wifi_pass2 = json.wifi_pass2

        // Integrations
        this.brewfather_apikey = json.brewfather_apikey
        this.brewfather_userkey = json.brewfather_userkey
        this.brewlogger_url = json.brewlogger_url
        this.brewspy_tokens = (json.brewspy_tokens || this.brewspy_tokens).slice(0, maxKegs)
        this.barhelper_apikey = json.barhelper_apikey
        this.barhelper_monitors = (json.barhelper_monitors || this.barhelper_monitors).slice(
          0,
          maxKegs
        )

        // Push - Generic
        this.push_timeout = json.push_timeout

        // Push - Http Post 1
        this.http_post_target = json.http_post_target
        this.http_post_header1 = json.http_post_header1
        this.http_post_header2 = json.http_post_header2

        // Push - Http Post 2
        this.http_post2_target = json.http_post2_target
        this.http_post2_header1 = json.http_post2_header1
        this.http_post2_header2 = json.http_post2_header2

        // Push - Http Get
        this.http_get_target = json.http_get_target
        this.http_get_header1 = json.http_get_header1
        this.http_get_header2 = json.http_get_header2

        // Push - Influx
        this.influxdb2_target = json.influxdb2_target
        this.influxdb2_org = json.influxdb2_org
        this.influxdb2_bucket = json.influxdb2_bucket
        this.influxdb2_token = json.influxdb2_token

        // Push - MQTT
        this.mqtt_target = json.mqtt_target
        this.mqtt_port = json.mqtt_port
        this.mqtt_user = json.mqtt_user
        this.mqtt_pass = json.mqtt_pass

        // Arrays - trim to configured number of kegs
        this.scales = (json.scales || this.scales).slice(0, maxKegs)
        this.beers = (json.beers || this.beers).slice(0, maxKegs)

        return true
      } catch (err) {
        global.disabled = false
        logError('configStore.load()', err)
        return false
      }
    },
    async sendConfig() {
      global.disabled = true
      logInfo('configStore.sendConfig()', 'Sending /api/config')

      var data = getConfigChanges()
      logDebug('configStore.sendConfig()', data)

      if (JSON.stringify(data).length == 2) {
        logInfo('configStore.sendConfig()', 'No config data to store, skipping step')
        global.disabled = false
        return true
      }

      try {
        await http.postJson('api/config', data)
        global.disabled = false
        logInfo('configStore.sendConfig()', 'Sending /api/config completed')
        return true
      } catch (err) {
        logError('configStore.sendConfig()', err)
        global.disabled = false
        return false
      }
    },
    async sendPushTest(data) {
      global.disabled = true
      logInfo('configStore.sendPushTest()', 'Sending /api/push')
      try {
        await http.postJson('api/push', data)
        return true
      } catch (err) {
        logError('configStore.sendPushTest()', err)
        return false
      }
    },
    async restart() {
      global.clearMessages()
      global.disabled = true
      try {
        const res = await http.restart(this.mdns, { redirectDelayMs: 8000 })
        if (res.success && res.json && res.json.status === true) {
          global.messageSuccess =
            (res.json.message || '') +
            ' Redirecting to http://' +
            this.mdns +
            '.local in 8 seconds.'
          logInfo('configStore.restart()', 'Restart requested, redirect scheduled')
        } else if (res.success && res.json) {
          global.messageError = res.json.message || 'Failed to restart device'
        } else {
          global.messageError = 'Failed to request restart'
        }
      } catch (err) {
        logError('configStore.restart()', err)
        global.messageError = 'Failed to do restart'
      } finally {
        global.disabled = false
      }
    },
    async getPushTestStatus() {
      logInfo('configStore.getPushTest()', 'Fetching /api/push/status')
      try {
        const json = await http.getJson('api/push/status')
        logDebug('configStore.getPushTest()', json)
        logInfo('configStore.getPushTest()', 'Fetching /api/push/status completed')
        return { success: true, data: json }
      } catch (err) {
        logError('configStore.getPushTest()', err)
        return { success: false, data: null }
      }
    },
    async sendWifiScan() {
      global.disabled = true
      logInfo('configStore.sendWifiScan()', 'Sending /api/wifi')
      try {
        await http.request('api/wifi')
        logInfo('configStore.sendWifiScan()', 'Sending /api/wifi completed')
        return true
      } catch (err) {
        logError('configStore.sendWifiScan()', err)
        return false
      }
    },
    async getWifiScanStatus() {
      logInfo('configStore.getWifiScanStatus()', 'Fetching /api/wifi/status')
      try {
        const json = await http.getJson('api/wifi/status')
        logDebug('configStore.getWifiScanStatus()', json)
        logInfo('configStore.getWifiScanStatus()', 'Fetching /api/wifi/status completed')
        return { success: true, data: json }
      } catch (err) {
        logError('configStore.getWifiScanStatus()', err)
        return { success: false, data: null }
      }
    },
    async sendHardwareScan() {
      global.disabled = true
      logInfo('configStore.sendHardwareScan()', 'Sending /api/hardware')
      try {
        await http.request('api/hardware')
        logInfo('configStore.sendHardwareScan()', 'Sending /api/hardware completed')
        return true
      } catch (err) {
        logError('configStore.sendHardwareScan()', err)
        return false
      }
    },
    async getHardwareScanStatus() {
      logInfo('configStore.getHardwareScanStatus()', 'Fetching /api/hardware/status')
      try {
        const json = await http.getJson('api/hardware/status')
        logDebug('configStore.getHardwareScanStatus()', json)
        logInfo('configStore.getHardwareScanStatus()', 'Fetching /api/hardware/status completed')
        return { success: true, data: json }
      } catch (err) {
        logError('configStore.getHardwareScanStatus()', err)
        return { success: false, data: null }
      }
    },
    async saveAll() {
      global.clearMessages()
      global.disabled = true
      const success = await this.sendConfig()
      if (!success) {
        global.disabled = false
        global.messageError = 'Failed to store configuration to device'
        return false
      } else {
        global.messageSuccess = 'Configuration has been saved to device'
        saveConfigState()
        return true
      }
    },
    async sendFilesystemRequest(data) {
      global.disabled = true
      logInfo('configStore.sendFilesystemRequest()', 'Sending /api/filesystem')
      try {
        const result = await http.filesystemRequest(data)
        logDebug('configStore.sendFilesystemRequest()', result)
        return result
      } catch (err) {
        logError('configStore.sendFilesystemRequest()', err)
        return { success: false, text: '' }
      }
    },
    async runPushTest(data) {
      global.disabled = true
      const started = await this.sendPushTest(data)
      if (!started) {
        global.messageError = 'Failed to start push test'
        global.disabled = false
        return { success: false }
      }

      const wait = (ms) => new Promise((res) => setTimeout(res, ms))

      while (true) {
        await wait(2000)
        const resp = await this.getPushTestStatus()
        if (!resp.success) {
          global.disabled = false
          global.messageError = 'Failed to get push test status'
          return { success: false }
        }

        const d = resp.data
        if (d.status) {
          // still running
          continue
        }

        global.disabled = false
        if (!d.push_enabled) {
          global.messageWarning = 'No endpoint is defined for this target. Cannot run test.'
        } else if (!d.success) {
          global.messageError = 'Test failed with error code ' + http.getErrorString(d.last_error)
        } else {
          global.messageSuccess = 'Test was successful'
        }

        return { success: true }
      }
    },
    async runWifiScan() {
      global.disabled = true
      const started = await this.sendWifiScan()
      if (!started) {
        global.disabled = false
        global.messageError = 'Failed to start wifi scan'
        return { success: false }
      }

      const wait = (ms) => new Promise((res) => setTimeout(res, ms))

      while (true) {
        await wait(2000)
        const resp = await this.getWifiScanStatus()
        if (!resp.success) {
          global.disabled = false
          global.messageError = 'Failed to get wifi scan status'
          return { success: false, data: null }
        }

        const d = resp.data
        if (d.status) {
          continue
        }

        global.disabled = false
        return { success: d.success, data: d }
      }
    },
    async runHardwareScan() {
      global.disabled = true
      const started = await this.sendHardwareScan()
      if (!started) {
        global.disabled = false
        global.messageError = 'Failed to start hardware scan'
        return { success: false }
      }

      const wait = (ms) => new Promise((res) => setTimeout(res, ms))

      while (true) {
        await wait(2000)
        const resp = await this.getHardwareScanStatus()
        if (!resp.success) {
          global.disabled = false
          global.messageError = 'Failed to get hardware scan status'
          return { success: false, data: null }
        }

        const d = resp.data
        if (d.status) {
          continue
        }

        global.disabled = false
        return { success: d.success, data: d }
      }
    }
  }
})
