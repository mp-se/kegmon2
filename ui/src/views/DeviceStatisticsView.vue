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
    <p class="h2">Device - Statistics</p>
    <hr />

    <div class="row" v-if="loaded">
      <div class="col-md-12">
        <p class="h4">Recent Events</p>
        <div class="table-responsive">
          <table class="table table-sm table-striped">
            <thead>
              <tr>
                <th style="width: 8%">Unit</th>
                <th style="width: 15%">Event Name</th>
                <th class="text-end" style="width: 12%">Timestamp</th>
                <th style="width: 65%">Details</th>
              </tr>
            </thead>
            <tbody>
              <tr v-for="(event, idx) in status.events" :key="`event-${idx}`">
                <td>{{ event.unit }}</td>
                <td>{{ formatEventName(event.name) }}</td>
                <td class="text-end">{{ formatEventTime(event.timestamp_ms) }}</td>
                <td class="small">{{ formatEventData(event) }}</td>
              </tr>
            </tbody>
          </table>
        </div>
      </div>

      <div class="col-md-12">
        <p class="h4">Level Statistics</p>
        <div class="table-responsive">
          <table class="table table-sm table-striped" style="table-layout: fixed">
            <thead>
              <tr>
                <th>Index</th>
                <th>State</th>
                <th class="text-end">Confidence</th>
                <th class="text-end">Total Pours</th>
                <th class="text-end">Total Volume</th>
                <th class="text-end">Avg Pour Volume</th>
                <th class="text-end">Max Pour Volume</th>
                <th class="text-end">Min Pour Volume</th>
                <th class="text-end">Avg Duration (sec)</th>
                <th class="text-end">Keg Replacements</th>
                <th class="text-end">Current Age (hrs)</th>
              </tr>
            </thead>
            <tbody>
              <tr v-for="stat in statistics.level_statistics" :key="`level-${stat.index}`">
                <td>{{ stat.index }}</td>
                <td>{{ stat.state }}</td>
                <td class="text-end">{{ stat.confidence }}%</td>
                <td class="text-end">{{ stat.total_pours }}</td>
                <td class="text-end">{{ stat.total_pour_volume }}</td>
                <td class="text-end">{{ Number(stat.avg_pour_volume).toFixed(2) }}</td>
                <td class="text-end">{{ Number(stat.max_pour_volume).toFixed(2) }}</td>
                <td class="text-end">{{ Number(stat.min_pour_volume).toFixed(2) }}</td>
                <td class="text-end">{{ Number(stat.avg_pour_duration_sec).toFixed(2) }}</td>
                <td class="text-end">{{ stat.keg_replacements }}</td>
                <td class="text-end">{{ stat.current_keg_age_hours }}</td>
              </tr>
            </tbody>
          </table>
        </div>
      </div>

      <div class="col-md-12">
        <p class="h4">Scale Statistics</p>
        <div class="table-responsive">
          <table class="table table-sm table-striped" style="table-layout: fixed">
            <thead>
              <tr>
                <th>Index</th>
                <th class="text-end">Total Readings</th>
                <th class="text-end">Valid</th>
                <th class="text-end">Invalid</th>
                <th class="text-end">Quality (%)</th>
                <th class="text-end">Frequency (Hz)</th>
                <th class="text-end">Raw Avg</th>
                <th class="text-end">Raw Min</th>
                <th class="text-end">Raw Max</th>
                <th class="text-end">Current Variance</th>
                <th class="text-end">Stable Variance</th>
                <th class="text-end">Calibration Drift/hr</th>
              </tr>
            </thead>
            <tbody>
              <tr v-for="stat in statistics.scale_statistics" :key="`scale-${stat.index}`">
                <td>{{ stat.index }}</td>
                <td class="text-end">{{ stat.total_readings }}</td>
                <td class="text-end">{{ stat.valid_readings }}</td>
                <td class="text-end">{{ stat.invalid_readings }}</td>
                <td class="text-end">{{ Number(stat.reading_quality).toFixed(1) }}</td>
                <td class="text-end">{{ Number(stat.reading_frequency).toFixed(2) }}</td>
                <td class="text-end">{{ Number(stat.raw_average).toFixed(0) }}</td>
                <td class="text-end">{{ Number(stat.raw_min).toFixed(0) }}</td>
                <td class="text-end">{{ Number(stat.raw_max).toFixed(0) }}</td>
                <td class="text-end">{{ Number(stat.current_variance).toFixed(2) }}</td>
                <td class="text-end">{{ Number(stat.stable_state_variance).toFixed(2) }}</td>
                <td class="text-end">{{ Number(stat.calibration_drift_per_hour).toFixed(3) }}</td>
              </tr>
            </tbody>
          </table>
        </div>
      </div>
    </div>

    <div class="row" v-else>
      <div class="col-md-3">
        <p>Unable to load statistics data</p>
      </div>
    </div>

    <div class="row gy-2">
      <div class="col-md-12">
        <hr />
      </div>
      <div class="col-sm-12">
        <button
          @click="clearStatistics()"
          type="button"
          class="btn btn-secondary"
          :disabled="global.disabled"
        >
          Clear statistics
        </button>
      </div>
    </div>
  </div>
</template>

<script setup>
import { onMounted, onUnmounted, ref } from 'vue'
import { global } from '@/modules/pinia'
import { useStatusStore } from '@/modules/statusStore'
import { logError, logInfo, sharedHttpClient as http } from '@mp-se/espframework-ui-components'

const status = useStatusStore()
const statistics = ref({
  level_statistics: [],
  scale_statistics: []
})
const loaded = ref(false)
const timer = ref(null)

onMounted(() => {
  getStatistics()
  timer.value = setInterval(() => {
    getStatistics()
  }, 4000)
})

onUnmounted(() => {
  clearInterval(timer.value)
})

const getStatistics = async () => {
  global.clearMessages()
  logInfo('DeviceStatisticsView.getStatistics()', 'Fetching /api/statistics')
  global.disabled = true
  try {
    const json = await http.getJson('api/statistics')
    statistics.value = json
    global.disabled = false
    loaded.value = true
  } catch (err) {
    logError('DeviceStatisticsView.getStatistics()', err)
    global.disabled = false
  }
}

const clearStatistics = async () => {
  global.clearMessages()
  logInfo('DeviceStatisticsView.clearStatistics()', 'Sending /api/statistics/clear')
  global.disabled = true
  try {
    await http.getJson('api/statistics/clear')
    global.disabled = false
  } catch (err) {
    logError('DeviceStatisticsView.clearStatistics()', err)
    global.disabled = false
  }
}

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

const formatEventName = (name) => {
  return eventNameMap[name] || name
}

const getRelativeTime = (timestamp_ms) => {
  const now = Date.now()
  const diff = now - timestamp_ms
  const seconds = Math.floor(diff / 1000)
  const minutes = Math.floor(seconds / 60)
  const hours = Math.floor(minutes / 60)
  const days = Math.floor(hours / 24)

  if (seconds < 60) return 'just now'
  if (minutes < 60) return `${minutes}m ago`
  if (hours < 24) return `${hours}h ago`
  return `${days}d ago`
}

const formatEventTime = (timestamp_ms) => {
  return getRelativeTime(timestamp_ms)
}

const formatEventData = (event) => {
  if (!event.data || Object.keys(event.data).length === 0) return '-'
  const data = event.data
  const parts = []

  // Volume data
  if (data.volume_l) parts.push(`Volume: ${data.volume_l}L`)
  if (data.stable_volume_l) parts.push(`Stable Vol: ${data.stable_volume_l}L`)
  if (data.total_pour_volume) parts.push(`Total Vol: ${data.total_pour_volume}L`)

  // Weight data
  if (data.weight_kg) parts.push(`Weight: ${data.weight_kg}kg`)
  if (data.stable_weight_kg) parts.push(`Stable Wt: ${data.stable_weight_kg}kg`)
  if (data.pre_weight_kg && data.post_weight_kg) {
    parts.push(`Weight: ${data.pre_weight_kg} → ${data.post_weight_kg}kg`)
  } else {
    if (data.pre_weight_kg) parts.push(`Pre: ${data.pre_weight_kg}kg`)
    if (data.post_weight_kg) parts.push(`Post: ${data.post_weight_kg}kg`)
  }
  if (data.previous_weight_kg && data.current_weight_kg) {
    parts.push(`${data.previous_weight_kg} → ${data.current_weight_kg}kg`)
  }

  // Duration data
  if (data.duration_ms) parts.push(`Duration: ${(data.duration_ms / 1000).toFixed(1)}s`)

  // Other data
  if (data.avg_slope_kg_sec) parts.push(`Slope: ${data.avg_slope_kg_sec}kg/s`)
  if (data.min_valid_weight_kg)
    parts.push(`Valid: ${data.min_valid_weight_kg}-${data.max_valid_weight_kg}kg`)

  return parts.length > 0 ? parts.join(' | ') : '-'
}
</script>
