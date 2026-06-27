<!--
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
 -->

<template>
  <div class="container">
    <p></p>
    <p class="h2">Device - Events</p>
    <hr />

    <div class="row mb-3">
      <div class="col-md-6">
        <label for="scaleFilter" class="form-label">Filter by Scale</label>
        <select
          id="scaleFilter"
          v-model="scaleFilter"
          class="form-select"
          :disabled="!status.sd_mounted"
        >
          <option value="all">All Scales</option>
          <option v-for="i in global.feature.no_scales" :key="i" :value="i.toString()">
            Scale {{ i }}
          </option>
        </select>
      </div>
      <div class="col-md-6 d-flex align-items-end">
        <button
          @click="loadEvents"
          class="btn btn-primary"
          :disabled="eventStore.loading || global.disabled || !status.sd_mounted"
        >
          <span
            class="spinner-border spinner-border-sm"
            role="status"
            aria-hidden="true"
            v-if="eventStore.loading"
          ></span>
          Refresh Data
        </button>
      </div>
    </div>

    <div>
      <div
        v-if="eventStore.events.length === 0 && !eventStore.loading && status.sd_mounted"
        class="alert alert-info"
      >
        No events loaded. Click "Refresh Data" to fetch historical event data.
      </div>

      <div v-else-if="filteredEvents.length === 0 && status.sd_mounted" class="alert alert-warning">
        No events found for the selected scale filter.
      </div>

      <div v-else>
        <p class="h4">Event History ({{ filteredEvents.length }} events)</p>
        <div class="table-responsive">
          <div class="row fw-bold mb-2 border-bottom pb-2">
            <div class="col-md-1">Scale</div>
            <div class="col-md-2">Event Type</div>
            <div class="col-md-2">Timestamp</div>
            <div class="col-md-7">Details</div>
          </div>
          <div
            v-for="(event, idx) in filteredEvents"
            :key="`event-${idx}`"
            class="row border-bottom py-2"
          >
            <div class="col-md-1">{{ event.scale }}</div>
            <div class="col-md-2">{{ formatEventType(event.eventType) }}</div>
            <div class="col-md-2">{{ formatTimestamp(event.timestamp) }}</div>
            <div class="col-md-7 small">{{ formatEventDetails(event) }}</div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, computed, onMounted } from 'vue'
import { useEventStore } from '@/modules/eventStore'
import { global, status } from '@/modules/pinia'
import { logError } from '@mp-se/espframework-ui-components'

const eventStore = useEventStore()
const scaleFilter = ref('all')

const filteredEvents = computed(() => {
  return eventStore.getFilteredEvents(scaleFilter.value)
})

onMounted(async () => {
  await loadEvents()
})

const loadEvents = async () => {
  global.disabled = true

  if (!status.sd_mounted) {
    global.messageWarning = 'SD card not mounted. Event data is not available.'
    global.disabled = false
    return
  }

  global.messageWarning = ''
  try {
    await eventStore.loadEvents()
  } catch (err) {
    logError('DeviceEventView.loadEvents()', err)
  } finally {
    global.disabled = false
  }
}

const formatEventType = (eventType) => {
  return eventType.replace(/_/g, ' ')
}

const formatTimestamp = (timestamp) => {
  const date = new Date(timestamp)
  const year = date.getFullYear()
  const month = String(date.getMonth() + 1).padStart(2, '0')
  const day = String(date.getDate()).padStart(2, '0')
  const hours = String(date.getHours()).padStart(2, '0')
  const minutes = String(date.getMinutes()).padStart(2, '0')
  const seconds = String(date.getSeconds()).padStart(2, '0')
  return `${year}-${month}-${day} ${hours}:${minutes}:${seconds}`
}

const formatEventDetails = (event) => {
  const details = []

  switch (event.eventType) {
    case 'SYSTEM_STARTUP':
      return 'System initialization'

    case 'SETTLING_STARTED':
      return `Settling started at: ${event.currWeight.toFixed(2)} kg`

    case 'STABLE_LEVEL':
      details.push(
        `Stable: ${event.stableWeight.toFixed(2)} kg (${event.stableVolume.toFixed(1)} L)`
      )
      if (event.durationMs > 0) {
        details.push(`Stabilization: ${(event.durationMs / 1000).toFixed(1)}s`)
      }
      return details.join(', ')

    case 'POURING':
      return `Started at: ${event.prePourWeight.toFixed(2)} kg`

    case 'WEIGHT_CHANGE_DETECTED':
      return `Weight change: ${event.prevWeight.toFixed(2)} kg → ${event.currWeight.toFixed(2)} kg (variance: ${event.variance.toFixed(3)})`

    case 'POUR_COMPLETED':
      details.push(`Poured: ${event.pourWeight.toFixed(2)} kg (${event.pourVolume.toFixed(2)} L)`)
      details.push(`Duration: ${(event.durationMs / 1000).toFixed(1)}s`)
      if (event.avgSlope !== 0) {
        details.push(`Rate: ${Math.abs(event.avgSlope).toFixed(4)} kg/s`)
      }
      return details.join(', ')

    case 'KEG_REMOVED':
      return `Removed: ${event.prevWeight.toFixed(2)} kg → ${event.currWeight.toFixed(2)} kg`

    case 'KEG_REPLACED':
      return `Replaced: ${event.prevWeight.toFixed(2)} kg → ${event.currWeight.toFixed(2)} kg`

    case 'KEG_ABSENT_TIMEOUT':
      if (event.durationMs > 0) {
        details.push(`Absent for: ${(event.durationMs / 1000).toFixed(1)}s`)
      }
      if (event.currWeight > 0) {
        details.push(`Current: ${event.currWeight.toFixed(2)} kg`)
      }
      return details.length > 0 ? details.join(', ') : 'Keg absent'

    case 'INVALID_WEIGHT':
      return 'Sensor reading out of range'

    case 'LOAD_CELL_ERROR':
      if (event.signalErrorReason) {
        details.push(`Reason: ${event.signalErrorReason}`)
      }
      details.push(`Quality: ${event.signalQuality}%`)
      if (event.consecutiveErrors > 0) {
        details.push(`Errors: ${event.consecutiveErrors}`)
      }
      if (event.variance > 0) {
        details.push(`Variance: ${event.variance.toFixed(3)}`)
      }
      return details.join(', ')

    case 'LOAD_CELL_RECOVERED':
      details.push(`Quality: ${event.signalQuality}%`)
      if (event.variance > 0) {
        details.push(`Variance: ${event.variance.toFixed(3)}`)
      }
      return details.join(', ')

    case 'SENSOR_RECOVERED':
      if (event.currWeight > 0) {
        return `Recovered at: ${event.currWeight.toFixed(2)} kg`
      }
      return 'Sensor recovered'

    case 'CALIBRATION_NEEDED':
      return 'Scale calibration required'

    case 'CALIBRATION_COMPLETE':
      return 'Scale calibration completed successfully'

    case 'DISABLED':
      return 'Scale unit disabled (ADC hardware not found)'

    default:
      return 'Unknown event type'
  }
}
</script>
