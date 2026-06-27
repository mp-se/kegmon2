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
    <p class="h3">Links and device logs</p>
    <hr />
    <div class="row">
      <p>
        If you need support, want to discuss the software or request any new features you can do
        that on github.com or homebrewtalk.com.
      </p>
    </div>
    <div class="row">
      <div class="col-md-4">
        <a
          class="link-primary link-offset-2 link-underline-opacity-25 link-underline-opacity-100-hover"
          href="https://github.com/mp-se/kegmon"
          target="_blank"
          >Report issues on github.com</a
        >
      </div>
      <div class="col-md-4">
        <a
          class="link-primary link-offset-2 link-underline-opacity-25 link-underline-opacity-100-hover"
          href="https://www.homebrewtalk.com/"
          target="_blank"
          >Discuss on homebrewtalk.com</a
        >
      </div>
      <!--
            <div class="col-md-4">
                <a class="link-primary link-offset-2 link-underline-opacity-25 link-underline-opacity-100-hover"
                    href="https://www.gravitymon.com/" target="_blank">Read docs on
                    gravitymon.com</a>
            </div>
            -->
    </div>

    <hr />
    <div class="row">
      <div class="col">
        <p>
          Platform: <span class="badge bg-secondary">{{ global.platform }}</span> Firmware:
          <span class="badge bg-secondary">{{ global.app_ver }} ({{ global.app_build }})</span>
          User interface:
          <span class="badge bg-secondary">{{ global.uiVersion }} ({{ global.uiBuild }})</span>
        </p>
      </div>
    </div>
    <hr />

    <div class="row">
      <div class="col-md-12">
        <button @click="viewLogs" type="button" class="btn btn-primary" :disabled="global.disabled">
          <span
            class="spinner-border spinner-border-sm"
            role="status"
            aria-hidden="true"
            :hidden="!global.disabled"
          ></span>
          &nbsp;View device logs</button
        >&nbsp;

        <button
          @click="removeLogs"
          type="button"
          class="btn btn-secondary"
          :disabled="global.disabled"
        >
          <span
            class="spinner-border spinner-border-sm"
            role="status"
            aria-hidden="true"
            :hidden="!global.disabled"
          ></span>
          &nbsp;Erase device logs</button
        >&nbsp;

        <button
          @click="hardwareScan"
          type="button"
          class="btn btn-secondary"
          :disabled="global.disabled"
        >
          <span
            class="spinner-border spinner-border-sm"
            role="status"
            aria-hidden="true"
            :hidden="!global.disabled"
          ></span>
          &nbsp;Hardware scan</button
        >&nbsp;

        <button
          @click="showHelp = !showHelp"
          type="button"
          class="btn btn-secondary"
          :disabled="global.disabled"
        >
          <span
            class="spinner-border spinner-border-sm"
            role="status"
            aria-hidden="true"
            :hidden="!global.disabled"
          ></span>
          &nbsp;Toggle error help</button
        >&nbsp;
        <template v-if="status.ispindel_config">
          <button
            @click="removeLegacy"
            type="button"
            class="btn btn-secondary"
            :disabled="global.disabled"
          >
            <span
              class="spinner-border spinner-border-sm"
              role="status"
              aria-hidden="true"
              :hidden="!global.disabled"
            ></span>
            &nbsp;Erase iSpindel config
          </button>
        </template>
      </div>
    </div>

    <div class="row">
      <div class="col">
        <p></p>
      </div>
    </div>
    <div class="row">
      <div class="col">
        <pre>{{ logData }}</pre>
      </div>
      <div class="form-text">Starts with the latest log entry first.</div>
    </div>

    <div class="row" v-if="showHelp">
      <div class="col-md-12">
        <p></p>
      </div>
      <div class="col-md-12">
        Common HTTP error codes:
        <ul>
          <li>
            400 - Bad request. Probably an issue with the post format. Do a preview of the format to
            identify the issue.
          </li>
          <li>
            401 - Unauthorized. The service needs an token or other means to authenticate the
            device.
          </li>
          <li>403 - Forbidden. Could be an issue with token or URL.</li>
          <li>404 - Not found. Probably a wrong URL.</li>
        </ul>
        <br />
        MQTT connection errors:
        <ul>
          <li>-1 - Connection refused</li>
          <li>-2 - Send header failed</li>
          <li>-3 - Send payload failed</li>
          <li>-4 - Not connected</li>
          <li>-5 - Connection lost</li>
          <li>-6 - No stream</li>
          <li>-7 - No HTTP server</li>
          <li>-8 - Too little RAM available</li>
          <li>-9 - Error encoding</li>
          <li>-10 - Error writing to stream</li>
          <li>-11 - Read timeout</li>
          <li>-100 - Endpoint skipped since its SSL and the device is in gravity mode</li>
        </ul>
        <br />
        MQTT push on topic errors:
        <ul>
          <li>-1 - Buffer to short</li>
          <li>-2 - Overflow</li>
          <li>-3 - Network failed connected</li>
          <li>-4 - Network timeout</li>
          <li>-5 - Network read failed</li>
          <li>-6 - Network write failed</li>
          <li>-10 - Connection denied</li>
          <li>-11 - Failed subscription</li>
        </ul>
        <br />
        WIFI error codes
        <ul>
          <li>1 - No SSID found.</li>
          <li>4 - Connection failed.</li>
          <li>5 - Connection lost.</li>
          <li>6 - Wrong password.</li>
          <li>7 - Disconnected by AP.</li>
        </ul>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref } from 'vue'
import { status, config, global } from '@/modules/pinia'

const logData = ref('')
const showHelp = ref(false)

async function fetchLog(file) {
  const res = await config.sendFilesystemRequest({ command: 'get', file })
  if (res.success && res.text) {
    res.text.split('\n').forEach((item) => {
      if (item.length) logData.value = item + '\n' + logData.value
    })
  }
}

async function removeLog(file) {
  await config.sendFilesystemRequest({ command: 'del', file })
}

async function viewLogs() {
  global.clearMessages()
  global.disabled = true
  logData.value = ''

  await fetchLog('/error2.log')
  await fetchLog('/error.log')
  global.disabled = false
}

async function removeLogs() {
  global.clearMessages()
  global.disabled = true
  logData.value = ''

  await removeLog('/error2.log')
  await removeLog('/error.log')
  global.messageSuccess = 'Requested logs to be deleted'
  global.disabled = false
}

async function removeLegacy() {
  global.clearMessages()
  global.disabled = true
  logData.value = ''

  await removeLog('/config.json')
  await removeLog('/gravitymon.json')
  global.messageSuccess = 'Requested old configuration files to be deleted'
  global.disabled = false
}

async function hardwareScan() {
  global.clearMessages()
  global.disabled = true
  logData.value = ''

  const result = await config.runHardwareScan()
  if (result.success) {
    logData.value = result.data
  }
  global.disabled = false
}
</script>
