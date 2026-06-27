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
import {
  sharedHttpClient as http,
  logDebug,
  logError,
  logInfo
} from '@mp-se/espframework-ui-components'
import { status } from '@/modules/pinia'

const MAX_EVENT_FILES = 4

export const useEventStore = defineStore('event', {
  state: () => {
    return {
      events: [],
      loading: false
    }
  },
  getters: {
    getFilteredEvents: (state) => (scaleFilter) => {
      if (scaleFilter === 'all') {
        return state.events
      }
      const scaleNum = parseInt(scaleFilter)
      return state.events.filter((event) => event.scale === scaleNum)
    }
  },
  actions: {
    async loadEvents() {
      logInfo('eventStore.loadEvents()', 'Fetching event files')

      // Check if SD card is mounted
      if (!status.sd_mounted) {
        logError('eventStore.loadEvents()', 'SD card not mounted')
        return false
      }

      this.loading = true
      try {
        const allEvents = []
        const filenames = [
          'data.csv',
          ...Array.from({ length: MAX_EVENT_FILES - 1 }, (_, i) => `data${i + 1}.csv`)
        ]

        // Try to load each file - process only those that exist
        for (const filename of filenames) {
          try {
            logInfo('eventStore.loadEvents()', `Fetching event file: ${filename}`)
            const resp = await http.request(`sd/${filename}`, { method: 'GET' })
            const csvText = await resp.text()
            const fileEvents = this._parseCsv(csvText)
            allEvents.push(...fileEvents)
            logDebug('eventStore.loadEvents()', `Loaded ${filename}: ${fileEvents.length} events`)
          } catch (err) {
            logDebug('eventStore.loadEvents()', `File not found: ${filename}`, err)
          }
        }

        // Sort events by timestamp descending (newest first)
        this.events = allEvents.sort((a, b) => new Date(b.timestamp) - new Date(a.timestamp))

        logInfo('eventStore.loadEvents()', `Loaded ${this.events.length} events`)
        return true
      } catch (err) {
        logError('eventStore.loadEvents()', err)
        return false
      } finally {
        this.loading = false
      }
    },
    _parseCsv(csvText) {
      const lines = csvText.trim().split('\n')
      const events = []

      for (const line of lines) {
        if (!line.trim()) continue

        const cols = line.split(',')
        // Expect exactly 18 columns per the new logging format
        if (cols.length !== 18) {
          logDebug(
            'eventStore._parseCsv()',
            `Skipping line with ${cols.length} columns (expected 18): ${line.substring(0, 50)}`
          )
          continue
        }

        // Parse all columns, allowing empty strings for optional fields
        const event = {
          version: parseInt(cols[0]) || 1,
          timestamp: cols[1].trim(),
          scale: parseInt(cols[2]) || 0,
          eventType: cols[3].trim(),
          // Weight and volume measurements
          stableWeight: cols[4] ? parseFloat(cols[4]) : 0,
          stableVolume: cols[5] ? parseFloat(cols[5]) : 0,
          prePourWeight: cols[6] ? parseFloat(cols[6]) : 0,
          postPourWeight: cols[7] ? parseFloat(cols[7]) : 0,
          pourWeight: cols[8] ? parseFloat(cols[8]) : 0,
          pourVolume: cols[9] ? parseFloat(cols[9]) : 0,
          // Timing and rate measurements
          durationMs: cols[10] ? parseInt(cols[10]) : 0,
          avgSlope: cols[11] ? parseFloat(cols[11]) : 0,
          // Weight change tracking
          prevWeight: cols[12] ? parseFloat(cols[12]) : 0,
          currWeight: cols[13] ? parseFloat(cols[13]) : 0,
          // Signal quality and error info
          signalErrorReason: cols[14] ? cols[14].trim() : '',
          signalQuality: cols[15] ? parseInt(cols[15]) : 0,
          variance: cols[16] ? parseFloat(cols[16]) : 0,
          consecutiveErrors: cols[17] ? parseInt(cols[17]) : 0
        }

        // Only add valid events
        if (event.timestamp && event.eventType) {
          events.push(event)
        } else {
          logDebug('eventStore._parseCsv()', `Skipping invalid event: missing timestamp or type`)
        }
      }

      logDebug('eventStore._parseCsv()', `Parsed ${events.length} events from CSV`)
      return events
    }
  }
})
