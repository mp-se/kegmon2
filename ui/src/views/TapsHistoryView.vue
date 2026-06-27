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
    <p class="h3">Taps - History</p>
    <hr />

    <div class="row mb-3">
      <div class="col-md-12">
        <button
          @click="loadHistory"
          class="btn btn-primary"
          :disabled="eventStore.loading || global.disabled || !status.sd_mounted"
        >
          <span
            class="spinner-border spinner-border-sm"
            role="status"
            aria-hidden="true"
            v-if="eventStore.loading"
          ></span>
          Refresh History
        </button>
      </div>
    </div>

    <!-- Charts Section -->
    <div class="row">
      <div class="col-md-12">
        <div class="card mb-3">
          <div class="card-body">
            <h6 class="card-title">Beer Level Over Time</h6>
            <canvas id="allTapsChart" style="max-height: 400px"></canvas>
          </div>
        </div>
      </div>
    </div>

    <hr />

    <!-- Tables Section -->
    <div class="row">
      <template v-for="scale in getAllScales()" :key="`scale-table-${scale}`">
        <div :class="getTapClass()">
          <h5>{{ getBeerName(scale) }} (Scale {{ scale }})</h5>
          <div class="table-responsive">
            <table class="table table-sm table-striped">
              <thead>
                <tr>
                  <th>Date & Time</th>
                  <th>Volume</th>
                </tr>
              </thead>
              <tbody>
                <tr v-if="getPoursForScale(scale).length === 0">
                  <td colspan="2" class="text-center text-muted">No pours recorded</td>
                </tr>
                <tr v-for="(pour, idx) in getPoursForScale(scale)" :key="`pour-${scale}-${idx}`">
                  <td>{{ formatTimestamp(pour.timestamp) }}</td>
                  <td>{{ pour.volume.toFixed(2) }} {{ getVolumeUnit() }}</td>
                </tr>
              </tbody>
            </table>
          </div>
        </div>
      </template>
    </div>
  </div>
</template>

<script setup>
import { onMounted, ref } from 'vue'
import {
  Chart as ChartJS,
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  LineController,
  Title,
  Tooltip,
  Legend
} from 'chart.js'
import { useEventStore } from '@/modules/eventStore'
import { global, config, status } from '@/modules/pinia'
import { logError, logInfo } from '@mp-se/espframework-ui-components'
import { volumeCLtoUSOZ, volumeCLtoUKOZ } from '@/modules/utils'

ChartJS.register(
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  LineController,
  Title,
  Tooltip,
  Legend
)

const eventStore = useEventStore()
const charts = ref({})

const getTapClass = () => {
  const noKegs = global.feature.no_scales
  if (noKegs >= 4) return 'col-md-3'
  if (noKegs === 3) return 'col-md-4'
  if (noKegs === 2) return 'col-md-6'
  return 'col-md-12'
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

const formatChartTimestamp = (timestamp) => {
  const date = new Date(timestamp)
  const month = String(date.getMonth() + 1).padStart(2, '0')
  const day = String(date.getDate()).padStart(2, '0')
  const hours = String(date.getHours()).padStart(2, '0')
  const minutes = String(date.getMinutes()).padStart(2, '0')
  return `${month}-${day} ${hours}:${minutes}`
}

const getVolumeUnit = () => {
  if (config.isVolumeCl) return 'cl'
  if (config.isVolumeUsOz) return 'us-oz'
  if (config.isVolumeUkOz) return 'uk-oz'
  return 'cl'
}

const convertVolume = (volumeCl) => {
  if (config.isVolumeCl) return volumeCl
  if (config.isVolumeUsOz) return volumeCLtoUSOZ(volumeCl)
  if (config.isVolumeUkOz) return volumeCLtoUKOZ(volumeCl)
  return volumeCl
}

const getScales = () => {
  const scales = new Set()
  eventStore.events.forEach((event) => {
    if (event.eventType === 'POUR_COMPLETED') {
      scales.add(event.scale)
    }
  })
  return Array.from(scales).sort((a, b) => a - b)
}

const getAllScales = () => {
  const noScales = global.feature.no_scales || 1
  const scales = []
  for (let i = 1; i <= noScales; i++) {
    scales.push(i)
  }
  return scales
}

const getPoursForScale = (scale) => {
  return eventStore.events
    .filter((event) => event.scale === scale && event.eventType === 'POUR_COMPLETED')
    .sort((a, b) => new Date(b.timestamp) - new Date(a.timestamp))
    .map((event) => ({
      timestamp: event.timestamp,
      volume: convertVolume(event.pourVolume * 100),
      weight: event.pourWeight,
      duration: event.durationMs,
      rate: event.avgSlope
    }))
}

const getBeerName = (scale) => {
  if (config.beers && config.beers[scale - 1]) {
    return config.beers[scale - 1].beer_name || `Tap ${scale}`
  }
  return `Tap ${scale}`
}

const createCharts = () => {
  const scales = getScales()
  if (scales.length === 0) return

  // Collect all unique timestamps across all scales
  const allTimestamps = new Set()
  const scaleData = {}

  scales.forEach((scale) => {
    const beerName = getBeerName(scale)

    // Get all POUR_COMPLETED and STABLE_LEVEL events for this scale
    const relevantEvents = eventStore.events.filter(
      (event) =>
        event.scale === scale &&
        (event.eventType === 'POUR_COMPLETED' || event.eventType === 'STABLE_LEVEL')
    )

    scaleData[scale] = {
      beerName,
      points: []
    }

    // For each POUR_COMPLETED, find the following STABLE_LEVEL to get remaining volume
    relevantEvents.forEach((event, idx) => {
      if (event.eventType === 'POUR_COMPLETED') {
        // Find the next STABLE_LEVEL event
        const nextStableEvent = relevantEvents
          .slice(idx + 1)
          .find((e) => e.eventType === 'STABLE_LEVEL')
        if (nextStableEvent) {
          scaleData[scale].points.push({
            timestamp: event.timestamp,
            volume: convertVolume(nextStableEvent.stableVolume * 100)
          })
          allTimestamps.add(event.timestamp)
        }
      }
    })

    // Also add the initial volume if we have STABLE_LEVEL events
    const firstStableEvent = relevantEvents.find((e) => e.eventType === 'STABLE_LEVEL')
    if (firstStableEvent && scaleData[scale].points.length === 0) {
      scaleData[scale].points.push({
        timestamp: new Date(firstStableEvent.timestamp).getTime() - 1000,
        volume: convertVolume(firstStableEvent.stableVolume)
      })
    }
  })

  // Create sorted timestamp array for x-axis labels
  const sortedTimestamps = Array.from(allTimestamps).sort()
  const labels = sortedTimestamps.map((ts) => formatChartTimestamp(ts))

  // Generate datasets for each scale
  const colors = ['#0d6efd', '#198754', '#fd7e14', '#dc3545']
  const datasets = scales.map((scale, idx) => {
    const { points } = scaleData[scale]

    // Create data array aligned with sorted timestamps
    const data = sortedTimestamps.map((ts) => {
      const point = points.find((p) => p.timestamp === ts)
      return point ? point.volume : null
    })

    return {
      label: scaleData[scale].beerName,
      data,
      borderColor: colors[idx % colors.length],
      backgroundColor: colors[idx % colors.length] + '33',
      borderWidth: 2,
      tension: 0.4,
      fill: false,
      pointRadius: 5,
      pointBackgroundColor: colors[idx % colors.length],
      spanGaps: true
    }
  })

  const chartElement = document.getElementById('allTapsChart')
  if (chartElement && !charts.value['combined']) {
    const ctx = chartElement.getContext('2d')
    charts.value['combined'] = new ChartJS(ctx, {
      type: 'line',
      data: {
        labels,
        datasets
      },
      options: {
        responsive: true,
        maintainAspectRatio: true,
        plugins: {
          legend: {
            display: true,
            position: 'top'
          }
        },
        scales: {
          y: {
            title: {
              display: true,
              text: `Beer Level (${getVolumeUnit()})`
            }
          },
          x: {
            title: {
              display: true,
              text: 'Time'
            }
          }
        }
      }
    })
  }
}

const loadHistory = async () => {
  global.disabled = true

  if (!status.sd_mounted) {
    global.messageWarning = 'SD card not mounted. Event history is not available.'
    global.disabled = false
    return
  }

  global.messageWarning = ''
  try {
    await eventStore.loadEvents()
    logInfo('TapsHistoryView.loadHistory()', 'History loaded')

    // Destroy existing charts before creating new ones
    Object.values(charts.value).forEach((chart) => chart.destroy())
    charts.value = {}

    // Create charts after DOM is updated
    await new Promise((resolve) => setTimeout(resolve, 100))
    createCharts()
  } catch (err) {
    logError('TapsHistoryView.loadHistory()', err)
  } finally {
    global.disabled = false
  }
}

onMounted(async () => {
  logInfo('TapsHistoryView.onMounted()', 'Loading history')
  await loadHistory()
})
</script>
