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

x
<template>
  <div class="container">
    <p></p>
    <p class="h3">Taps - Settings</p>
    <hr />

    <form @submit.prevent="save" class="needs-validation" novalidate>
      <div class="row">
        <template v-for="(scale, index) in config.scales" :key="index">
          <div :class="getTapClass()">
            <BsInputNumber
              :model-value="getKegWeight(index)"
              @update:model-value="setKegWeight(index, $event)"
              width="7"
              min="0.0"
              max="10.0"
              step="0.01"
              :label="`Tap ${index + 1} - Empty keg weight`"
              :unit="config.weight_unit"
              :disabled="global.disabled"
            />
          </div>
        </template>

        <template v-for="(scale, index) in config.scales" :key="`glass-${index}`">
          <div :class="getTapClass()">
            <BsSelect
              v-model="config.scales[index].glass_volume"
              width="12"
              :options="glassOptions"
              :label="`Tap ${index + 1} - Glass size`"
              :disabled="global.disabled"
            />
          </div>
        </template>

        <template v-for="(scale, index) in config.scales" :key="`keg-${index}`">
          <div :class="getTapClass()">
            <BsSelect
              v-model="config.scales[index].keg_volume"
              width="12"
              :options="kegOptions"
              :label="`Tap ${index + 1} - Keg volume`"
              :disabled="global.disabled"
            />
          </div>
        </template>
        <template v-for="(scale, index) in config.scales" :key="`keg-${index}`">
          <div :class="getTapClass()">
            <BsSelect
              v-model="config.scales[index].temp_sensor_id"
              width="12"
              :options="tempSensorOptions"
              :label="`Tap ${index + 1} - Temperature sensor`"
              :disabled="global.disabled"
            />
          </div>
        </template>
      </div>

      <div class="row gy-2">
        <div class="col-md-12">
          <hr />
        </div>
        <div class="col-md-12">
          <button
            type="submit"
            class="btn btn-primary w-2"
            :disabled="global.disabled || !global.configChanged"
          >
            <span
              class="spinner-border spinner-border-sm"
              role="status"
              aria-hidden="true"
              :hidden="!global.disabled"
            ></span>
            &nbsp;Save
          </button>
        </div>
      </div>
    </form>
  </div>
</template>

<script setup>
import { ref, onMounted, computed } from 'vue'
import { validateCurrentForm } from '@mp-se/espframework-ui-components'
import { weightKgToLbs, weightLbsToKg } from '@/modules/utils'
import { global, config, status } from '@/modules/pinia'
import { logDebug } from '@mp-se/espframework-ui-components'

const getTapClass = () => {
  const noKegs = global.feature.no_scales

  logDebug('TapsSettingsView::getTapClass', 'Number of kegs:', noKegs) // Debug log

  if (noKegs >= 4) return 'col-md-3'
  if (noKegs === 3) return 'col-md-4'
  if (noKegs === 2) return 'col-md-6'
  return 'col-md-12'
}

// Helper to get displayed keg weight (convert from kg to user's selected unit)
const getKegWeight = (scaleIndex) => {
  const kegWeightKg = config.scales[scaleIndex].keg_weight
  return config.isWeightLbs ? weightKgToLbs(kegWeightKg) : kegWeightKg
}

// Helper to set keg weight (convert from user's selected unit to kg)
const setKegWeight = (scaleIndex, value) => {
  config.scales[scaleIndex].keg_weight = config.isWeightLbs ? weightLbsToKg(value) : value
}

const tempSensorOptions = computed(() => [
  { label: 'None', value: '' },
  ...status.sensors.map((sensor) => {
    // Find all taps that use this sensor
    const assignedTaps = config.scales
      .map((scale, index) => (scale.temp_sensor_id === sensor.id ? index + 1 : null))
      .filter((tap) => tap !== null)

    const label =
      assignedTaps.length > 0 ? `${sensor.id} (Tap ${assignedTaps.join(', Tap ')})` : sensor.id
    return { label, value: sensor.id }
  })
])

const glassOptions = ref([
  { label: '20 cl / 7.0 imperial ounces', value: 0.2 },
  { label: '25 cl / 8.8 imperial ounces', value: 0.25 },
  { label: '30 cl / 11 imperial ounces', value: 0.3 },
  { label: '33 cl / 12 imperial ounces', value: 0.33 },
  { label: '40 cl / 14 imperial ounces', value: 0.4 },
  { label: '50 cl / 18 imperial ounces', value: 0.5 },
  { label: 'Imperial pint, 568 ml / 20 imp fl oz', value: 0.568 },
  { label: 'Half Imperial pint, 284 ml / 10 imp fl oz', value: 0.284 },
  { label: 'Quater Imperial pint, 142 ml / 5 imp fl oz', value: 0.142 },
  { label: 'US pint, 468 ml / 16 US fl oz', value: 0.468 }
])

const kegOptions = ref([
  { label: 'Oxebar 4l', value: 4 },
  { label: 'Mini 5l / 1.32gal / 169oz', value: 5 },
  { label: 'Oxebar 8l', value: 8 },
  { label: 'Cornelious 9l', value: 9 },
  { label: 'Mini 10l', value: 10 },
  { label: 'Cornelius 18l', value: 18 },
  { label: 'Cornelius 19l / 5gal / 640oz', value: 19 },
  { label: 'Keykeg 20l', value: 20 },
  { label: 'Sixth Barrel 5.16gal / 640oz', value: 19.5 },
  { label: 'Quarter Barrel 7.75gal / 992oz', value: 29.3 },
  { label: 'Unittank 29l', value: 29 },
  { label: 'Half Barrel 15.5gal / 1984oz', value: 58.7 },
  { label: 'Kegmenter 58l', value: 58 }
])

onMounted(() => {
  // Additional initialization if needed
})

const save = () => {
  if (!validateCurrentForm()) return

  config.saveAll()
}
</script>
