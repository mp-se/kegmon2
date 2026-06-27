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
import { useConfigStore } from './configStore'
import {
  sharedHttpClient as http,
  logDebug,
  logError,
  logInfo,
  tempToF
} from '@mp-se/espframework-ui-components'
import { weightKgToLbs, volumeCLtoUSOZ, volumeCLtoUKOZ } from '@/modules/utils.js'
import { global } from './pinia'

export const useStatusStore = defineStore('status', {
  state: () => {
    return {
      connected: false,

      // Status
      id: '',
      mdns: '',
      wifi_ssid: '',
      weight_unit: '',
      volume_unit: '',
      temp_unit: '',
      rssi: 0,
      total_heap: 0,
      free_heap: 0,
      ip: '',
      wifi_setup: false,
      scale_busy: false,
      uptime_seconds: 0,
      uptime_minutes: 0,
      uptime_hours: 0,
      uptime_days: 0,
      scales: [],
      sensors: [],
      events: [],
      ha: {},
      brewspy: {},
      brewlogger: {},
      barhelper: {},
      sd_mounted: false
    }
  },
  getters: {
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
    async load() {
      logInfo('statusStore.load()', 'Fetching /api/status')
      try {
        const json = await http.getJson('api/status')
        logDebug('statusStore.load()', json)
        const config = useConfigStore()

        this.connected = true

        // General properties
        this.id = json.id
        this.mdns = json.mdns
        this.wifi_ssid = json.wifi_ssid
        this.weight_unit = json.weight_unit
        this.volume_unit = json.volume_unit
        this.temp_unit = json.temp_unit
        this.rssi = json.rssi
        this.total_heap = json.total_heap
        this.free_heap = json.free_heap
        this.ip = json.ip || ''
        this.wifi_setup = json.wifi_setup
        this.scale_busy = json.scale_busy
        this.uptime_seconds = json.uptime_seconds
        this.uptime_minutes = json.uptime_minutes
        this.uptime_hours = json.uptime_hours
        this.uptime_days = json.uptime_days
        this.sd_mounted = json.sd_mounted

        // Parse scales array and trim to configured number of kegs
        const maxKegs = global.feature.no_scales
        this.scales = (json.scales || []).slice(0, maxKegs).map((scale) => ({
          ...scale,
          stable_weight: this._convertWeight(scale.stable_weight, config),
          stable_volume: this._convertVolume(scale.stable_volume, config),
          pouring_volume: this._convertVolume(scale.pouring_volume, config),
          last_pour_volume: this._convertVolume(scale.last_pour_volume, config),
          keg_volume: this._convertVolume(scale.keg_volume, config),
          glass: scale.glass
        }))

        // Parse sensors array
        this.sensors = json.sensors || []
        if (this.sensors.length > 0 && this.sensors[0]) {
          this.sensors[0].temperature = this._convertTemperature(
            this.sensors[0].temperature,
            config
          )
        }

        // Push status
        if (Object.prototype.hasOwnProperty.call(json, 'ha')) this.ha = json.ha
        if (Object.prototype.hasOwnProperty.call(json, 'brewspy')) this.brewspy = json.brewspy
        if (Object.prototype.hasOwnProperty.call(json, 'barhelper')) this.barhelper = json.barhelper
        if (Object.prototype.hasOwnProperty.call(json, 'brewlogger'))
          this.brewlogger = json.brewlogger

        // Parse events array
        this.events = json.recent_events || []

        logInfo('statusStore.load()', 'Fetching /api/status completed')
        return true
      } catch (err) {
        logError('statusStore.load()', err)
        return false
      }
    },
    _convertWeight(weight, config) {
      if (weight == null) return 0
      const converted = config.isWeightLbs ? weightKgToLbs(weight) : weight
      return Number(converted).toFixed(2)
    },
    _convertVolume(volume, config) {
      if (volume == null) return 0
      let converted = volume
      if (config.isVolumeUsOz) {
        converted = volumeCLtoUSOZ(volume)
      } else if (config.isVolumeUkOz) {
        converted = volumeCLtoUKOZ(volume)
      }
      return Number(converted).toFixed(0)
    },
    _convertTemperature(temp, config) {
      if (temp == null) return 0
      const converted = config.isTempF ? tempToF(temp) : temp
      return Number(converted).toFixed(2)
    },
    getLastEventsForScale(scaleIndex, limit = 3) {
      const eventNameMap = {
        system_startup: 'System Startup',
        settling_started: 'Settling Started',
        stable_level: 'Stable Level',
        pouring: 'Pouring',
        weight_change_detected: 'Weight Change Detected',
        pour_completed: 'Pour Completed',
        keg_removed: 'Keg Removed',
        keg_replaced: 'Keg Replaced',
        keg_absent_timeout: 'Keg Absent',
        invalid_weight: 'Invalid Weight',
        load_cell_error: 'Load Cell Error',
        load_cell_recovered: 'Load Cell Recovered',
        sensor_recovered: 'Sensor Recovered',
        calibration_needed: 'Calibration Needed',
        calibration_complete: 'Calibration Complete',
        disabled: 'Disabled'
      }

      const filteredEvents = (this.events || [])
        .filter((event) => event.unit === scaleIndex)
        .sort((a, b) => b.timestamp_ms - a.timestamp_ms)
        .slice(0, limit)

      return filteredEvents.map((event) => ({
        name: eventNameMap[event.name] || event.name,
        timestamp_ms: event.timestamp_ms,
        data: event.data || {}
      }))
    },
    getRelativeTime(timestamp_ms) {
      const totalUptimeMs =
        this.uptime_days * 24 * 60 * 60 * 1000 +
        this.uptime_hours * 60 * 60 * 1000 +
        this.uptime_minutes * 60 * 1000 +
        this.uptime_seconds * 1000
      const diff = totalUptimeMs - timestamp_ms
      const seconds = Math.floor(diff / 1000)
      const minutes = Math.floor(seconds / 60)
      const hours = Math.floor(minutes / 60)
      const days = Math.floor(hours / 24)

      if (seconds < 60) return 'just now'
      if (minutes < 60) return `${minutes} min ago`
      if (hours < 24) return `${hours}h ago`
      return `${days}d ago`
    }
  }
})
