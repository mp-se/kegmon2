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
    <p class="h3">Taps - Beers</p>
    <hr />

    <form @submit.prevent="save" class="needs-validation" novalidate>
      <div class="row">
        <template v-for="(beer, index) in config.beers" :key="`name-${index}`">
          <div :class="getTapClass()">
            <BsInputText
              v-model="config.beers[index].beer_name"
              maxlength="30"
              :label="`Beer ${index + 1} - Name`"
              :disabled="global.disabled"
            />
          </div>
        </template>

        <template v-for="(beer, index) in config.beers" :key="`fg-${index}`">
          <div :class="getTapClass()">
            <BsInputNumber
              v-model="config.beers[index].beer_fg"
              :label="`Beer ${index + 1} - FG`"
              min="0"
              max="2"
              step="0.0001"
              width="5"
              :disabled="global.disabled"
            />
          </div>
        </template>

        <template v-for="(beer, index) in config.beers" :key="`ebc-${index}`">
          <div :class="getTapClass()">
            <BsInputNumber
              v-model="config.beers[index].beer_ebc"
              :label="`Beer ${index + 1} - EBC`"
              min="0"
              max="100"
              step="1"
              width="5"
              :disabled="global.disabled"
            />
          </div>
        </template>

        <template v-for="(beer, index) in config.beers" :key="`ibu-${index}`">
          <div :class="getTapClass()">
            <BsInputNumber
              v-model="config.beers[index].beer_ibu"
              :label="`Beer ${index + 1} - IBU`"
              min="0"
              max="100"
              step="1"
              width="5"
              :disabled="global.disabled"
            />
          </div>
        </template>

        <template v-for="(beer, index) in config.beers" :key="`abv-${index}`">
          <div :class="getTapClass()">
            <BsInputNumber
              v-model="config.beers[index].beer_abv"
              :label="`Beer ${index + 1} - ABV`"
              min="0"
              max="20"
              step="0.1"
              unit="%"
              width="5"
              :disabled="global.disabled"
            />
          </div>
        </template>
      </div>

      <div class="row">
        <div class="col-md-12">
          <p></p>
        </div>
        <template v-for="(beer, index) in config.beers" :key="`buttons-${index}`">
          <div :class="getTapClass()">
            <template v-if="config.brewfather_apikey != '' && config.brewfather_userkey != ''">
              <button
                @click="fetchBrewfather(index + 1)"
                type="button"
                class="btn btn-secondary w-2 m-2"
                :disabled="global.disabled"
              >
                <span
                  class="spinner-border spinner-border-sm"
                  role="status"
                  aria-hidden="true"
                  :hidden="!global.disabled"
                ></span>
                &nbsp;Fetch from Brewfather</button
              >&nbsp;
            </template>
            <template v-if="config.brewlogger_url != ''">
              <button
                @click="fetchBrewlogger(index + 1)"
                type="button"
                class="btn btn-secondary w-2 m-2"
                :disabled="global.disabled"
              >
                <span
                  class="spinner-border spinner-border-sm"
                  role="status"
                  aria-hidden="true"
                  :hidden="!global.disabled"
                ></span>
                &nbsp;Fetch from BrewLogger</button
              >&nbsp;
            </template>
            <template v-if="config.brewspy_tokens[index] != ''">
              <button
                @click="fetchBrewspy(index + 1)"
                type="button"
                class="btn btn-secondary w-2 m-2"
                :disabled="global.disabled"
              >
                <span
                  class="spinner-border spinner-border-sm"
                  role="status"
                  aria-hidden="true"
                  :hidden="!global.disabled"
                ></span>
                &nbsp;Fetch from Brewspy
              </button>
            </template>
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

    <BsModalSelect
      v-model="beer"
      :callback="confirmBeerCallback"
      message="Select a beer to use for the tap"
      id="beer"
      title="Select beer"
      :options="beerOptions"
    />
  </div>
</template>

<script setup>
import { ref } from 'vue'
import { validateCurrentForm, sharedHttpClient as http } from '@mp-se/espframework-ui-components'
import { global, config } from '@/modules/pinia'
import { logDebug, logError, logInfo } from '@mp-se/espframework-ui-components'

const beerOptions = ref([])
const beer = ref('')
const tapNo = ref(0)

const getTapClass = () => {
  const noKegs = global.feature.no_scales
  if (noKegs >= 4) return 'col-md-3'
  if (noKegs === 3) return 'col-md-4'
  if (noKegs === 2) return 'col-md-6'
  return 'col-md-12'
}

const confirmBeerCallback = (result, value) => {
  logDebug('TapsBeerView.confirmSearchCallback()', result, value)

  if (result) {
    global.clearMessages()
    global.disabled = true
    if (value != '') {
      beerOptions.value.forEach((e) => {
        if (e.value == value) {
          logDebug('TapsBeerView.confirmBeerCallback()', e)

          const tapIndex = tapNo.value - 1
          config.beers[tapIndex].beer_abv = parseFloat(parseFloat(e.abv).toFixed(1))
          config.beers[tapIndex].beer_ebc = Math.ceil(e.ebc)
          config.beers[tapIndex].beer_ibu = Math.ceil(e.ibu)
          config.beers[tapIndex].beer_name = e.label
          config.beers[tapIndex].beer_id = e.id

          global.disabled = false
          return
        }
      })
    }
  } else {
    global.disabled = false
  }
}

const fetchBrewfather = (tap) => {
  logInfo('TapsBeerView.fetchBrewfather()', tap)

  global.disabled = true
  var auth = btoa(config.brewfather_userkey + ':' + config.brewfather_apikey)
  beerOptions.value = []
  beer.value = ''

  fetch(
    'https://api.brewfather.app/v2/batches?status=Completed&include=recipe.name,estimatedFg,estimatedColor,estimatedIbu,measuredAbv&complete=false',
    {
      method: 'GET',
      headers: { Authorization: 'Basic ' + auth },
      signal: AbortSignal.timeout(global.fetchTimout)
    }
  )
    .then((res) => res.json())
    .then((json) => {
      for (var i = 0; i < json.length; i++) {
        logInfo('TapsBeerView.fetchBrewfather()', json[i])

        var abv = Object.prototype.hasOwnProperty.call(json[i], 'measuredAbv')
          ? json[i].measuredAbv
          : 0
        var ibu = Object.prototype.hasOwnProperty.call(json[i], 'estimatedIbu')
          ? json[i].estimatedIbu
          : 0
        var ebc = Object.prototype.hasOwnProperty.call(json[i], 'estimatedColor')
          ? json[i].estimatedColor
          : 0
        var fg = Object.prototype.hasOwnProperty.call(json[i], 'estimatedFg')
          ? json[i].estimatedFg
          : 1

        var beer = {
          label: json[i].recipe.name,
          value: json[i].batchNo,
          abv: abv,
          ibu: ibu,
          ebc: ebc,
          fg: fg,
          id: ''
        }
        beerOptions.value.push(beer)
      }

      logInfo('TapsBeerView.fetchBrewfather()', beerOptions.value)
      tapNo.value = tap
      document.getElementById('beer').click()
    })
    .catch((err) => {
      logError('TapsBeerView.fetchBrewfather()', err)
      global.messageError = 'Failed to fetch data from brewfather'
      global.disabled = false
    })
}

const fetchBrewlogger = (tap) => {
  logInfo('TapsBeerView.fetchBrewlogger()', tap)

  global.disabled = true
  beerOptions.value = []
  beer.value = ''

  fetch(config.brewlogger_url + '/api/batch/taplist', {
    method: 'GET',
    signal: AbortSignal.timeout(global.fetchTimout)
  })
    .then((res) => res.json())
    .then((json) => {
      for (var i = 0; i < json.length; i++) {
        logInfo('TapsBeerView.fetchBrewlogger()', json[i])

        var beer = {
          label: json[i].name + ' (' + json[i].brewDate + ')',
          value: json[i].id,
          abv: json[i].abv,
          ibu: json[i].ibu,
          ebc: json[i].ebc,
          id: String(json[i].id),
          fg: 1 // TODO: Missing FG in brewlogger
        }
        beerOptions.value.push(beer)
      }

      logInfo('TapsBeerView.fetchBrewlogger()', beerOptions.value)
      tapNo.value = tap
      document.getElementById('beer').click()
    })
    .catch((err) => {
      logError('TapsBeerView.fetchBrewlogger()', err)
      global.messageError = 'Failed to fetch data from brewlogger'
      global.disabled = false
    })
}

const fetchBrewspy = async (tap) => {
  logInfo('TapsBeerView.fetchBrewspy()', tap)

  global.disabled = true
  const tapIndex = tap - 1

  try {
    const res = await http.postJson('api/brewspy/tap', { token: config.brewspy_tokens[tapIndex] })
    const json = await res.json()

    logDebug('TapsBeerView.fetchBrewspy()', json)

    config.beers[tapIndex].beer_abv = parseFloat(parseFloat(json.abv).toFixed(1))
    config.beers[tapIndex].beer_ebc = 0
    config.beers[tapIndex].beer_ibu = 0
    config.beers[tapIndex].beer_name = json.recipe
  } catch (err) {
    logError('TapsBeerView.fetchBrewspy()', err)
    global.messageError = 'Failed to fetch data from brewspy'
  } finally {
    global.disabled = false
  }
}

const save = () => {
  if (!validateCurrentForm()) return

  config.saveAll()
}
</script>
