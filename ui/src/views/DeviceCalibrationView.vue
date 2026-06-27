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
    <p class="h2">Device - Calibration</p>
    <hr />

    <div class="row">
      <div class="col-md-12">
        <p class="h3">Step 1 - Select scale</p>
      </div>
      <div class="col-md-12">
        <BsInputRadio
          v-model="scale"
          :options="scaleOptions"
          label="Select which scale to calibrate"
          width=""
          :disabled="global.disabled || state > 1"
        ></BsInputRadio>
      </div>
      <div class="row gy-2" v-if="state == 1">
        <div class="col-md-2">
          <button
            @click="step1()"
            type="button"
            class="btn btn-secondary"
            :disabled="global.disabled"
          >
            Select scale
          </button>
        </div>
      </div>
      <p></p>
    </div>

    <div class="row" v-if="state > 1">
      <hr />
      <div class="col-md-12">
        <p class="h3">Step 2 - Tare scale</p>
      </div>
      <div class="col-md-12">
        <p>Remove any object from the scale and press the tare button to reset it to zero.</p>
      </div>
      <div class="row gy-2" v-if="state == 2">
        <div class="col-md-12">
          <button
            @click="step2()"
            type="button"
            class="btn btn-secondary"
            :disabled="global.disabled"
          >
            Tare scale</button
          >&nbsp;

          <button
            @click="back()"
            type="button"
            class="btn btn-secondary"
            :disabled="global.disabled"
          >
            Back
          </button>
        </div>
      </div>
      <p></p>
    </div>

    <div class="row" v-if="state > 2">
      <hr />
      <div class="col-md-12">
        <p class="h3">Step 3 - Calibrate scale</p>
      </div>
      <div class="col-md-12">
        <p>
          Place a known weight on the scale and press the factor button. The scale will calculate a
          factor and complete calibration. Use any known weight.
        </p>
      </div>

      <div class="col-md-12">
        <BsInputNumber
          v-model="weight"
          width="5"
          label="Calibration weight"
          min="0"
          max="25"
          step=".01"
          :unit="config.weight_unit"
          :disabled="global.disabled || state != 3"
        />
      </div>

      <div class="row gy-2" v-if="state == 3">
        <div class="col-md-12">
          <button
            @click="step3()"
            type="button"
            class="btn btn-secondary"
            :disabled="global.disabled"
          >
            Calculate factor</button
          >&nbsp;

          <button
            @click="back()"
            type="button"
            class="btn btn-secondary"
            :disabled="global.disabled"
          >
            Back
          </button>
        </div>
        <p></p>
      </div>
    </div>

    <div class="row" v-if="state > 3">
      <hr />
      <div class="col-md-12">
        <p class="h3">Step 4 - Validate</p>
      </div>
      <div class="col-md-12">
        <p>
          The entered weight was <b>{{ new Number(weight).toFixed(2) }}</b> and the measured weight
          after calibration was <b>{{ new Number(scaleStatus.weight).toFixed(2) }}</b> (Deviation:
          <b>{{ new Number(((scaleStatus.weight - weight) / weight) * 100).toFixed(0) }}</b> %), if
          you want to change then use the back button.
        </p>
      </div>
      <div class="row gy-2">
        <div class="col-md-12">
          <button
            @click="back()"
            type="button"
            class="btn btn-secondary"
            :disabled="global.disabled"
          >
            Back</button
          >&nbsp;

          <button
            @click="begin()"
            type="button"
            class="btn btn-secondary"
            :disabled="global.disabled"
          >
            Start over
          </button>
        </div>
      </div>
    </div>

    <div class="row p-3">
      <div class="col-md-12">
        <p></p>
      </div>

      <div :class="getClass()">Offset: {{ scaleStatus.offset }}</div>
      <div :class="getClass()">Factor: {{ scaleStatus.factor }}</div>
      <div :class="getClass()">Raw: {{ scaleStatus.raw }}</div>
      <div :class="getClass()">Weight: {{ scaleStatus.weightString }}</div>
    </div>
  </div>
</template>

<script setup>
import { onBeforeMount, ref } from 'vue'
import { weightKgToLbs, weightLbsToKg } from '@/modules/utils'
import { global, config } from '@/modules/pinia'
import {
  sharedHttpClient as http,
  logDebug,
  logError,
  logInfo
} from '@mp-se/espframework-ui-components'

const state = ref(1)
const scale = ref(1)
const weight = ref(0)
const scaleStatus = ref({
  offset: 0,
  factor: 0,
  weight: 0,
  weightString: '',
  raw: 0
})

const scaleOptions = ref([
  { label: 'Scale 1', value: 0 },
  { label: 'Scale 2', value: 1 },
  { label: 'Scale 3', value: 2 },
  { label: 'Scale 4', value: 3 }
])

function getClass() {
  if (config.dark_mode) return 'col-md-3 p-3 border bg-dark text-white'
  return 'col-md-3 p-3 border bg-light'
}

function back() {
  global.clearMessages()
  state.value = state.value - 1
}

function begin() {
  global.clearMessages()
  state.value = 1
}

function step1() {
  global.clearMessages()
  state.value = 2
}

function saveScaleValues(json) {
  const scaleData = json.scales[scale.value]

  if (scaleData) {
    logDebug('DeviceCalibrationView.saveScaleValues()', scaleData)

    scaleStatus.value.offset = scaleData.scale_offset
    scaleStatus.value.factor = scaleData.scale_factor
    scaleStatus.value.raw = scaleData.scale_raw
    scaleStatus.value.weight = scaleData.stable_weight

    scaleStatus.value.weightString =
      new Number(
        config.isWeightKg ? scaleStatus.value.weight : weightKgToLbs(scaleStatus.value.weight)
      ).toFixed(3) +
      ' ' +
      config.weight_unit
  }
}

async function step2() {
  global.disabled = true
  global.clearMessages()

  try {
    logDebug('DeviceCalibrationView.step2()', 'Sending tare request')
    await http.postJson('api/scale/tare', { scale_index: scale.value })

    // Poll until scale_busy is false
    while (true) {
      logDebug('DeviceCalibrationView.step2()', 'Checking scale status')
      const json = await http.getJson('api/scale')
      logInfo('DeviceCalibrationView.step2()', json)

      saveScaleValues(json)

      if (!json.scale_busy) {
        global.messageSuccess = 'Scale tare completed'
        state.value = 3
        global.disabled = false
        break
      }

      // wait 2s before next poll
      await new Promise((r) => setTimeout(r, 2000))
    }
  } catch (e) {
    logError('DeviceCalibrationView.step2()', e)
    global.messageError = e.message || e
    global.disabled = false
  }
}

async function step3() {
  global.clearMessages()

  if (weight.value == 0) {
    global.messageError = 'You need to supply a weight larger than 0 (zero)'
    return
  }

  global.disabled = true

  try {
    logDebug('DeviceCalibrationView.step3()', 'Sending factor request')
    const weightKg = config.isWeightLbs ? weightLbsToKg(weight.value) : weight.value
    logInfo('DeviceCalibrationView.step3()', { scale_index: scale.value, weight: weightKg })
    await http.postJson('api/scale/factor', { scale_index: scale.value, weight: weightKg })

    // Poll until scale_busy is false
    while (true) {
      logDebug('DeviceCalibrationView.step3()', 'Checking scale status')
      const json = await http.getJson('api/scale')
      logInfo('DeviceCalibrationView.step3()', json)

      if (!json.scale_busy && json.scales[scale.value].state === 'Stable') {
        global.messageSuccess = 'Scale factor calculation completed'
        state.value = 4
        global.disabled = false
        saveScaleValues(json)
        break
      }

      // wait 2s before next poll
      await new Promise((r) => setTimeout(r, 2000))
    }
  } catch (e) {
    logError('DeviceCalibrationView.step3()', e)
    global.messageError = e.message || e
    global.disabled = false
  }
}

onBeforeMount(() => {
  state.value = 1
})
</script>
