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
  <button
    :id="id"
    type="button"
    class="btn btn-secondary"
    hidden
    data-bs-toggle="modal"
    :data-bs-target="'#modal' + $.uid"
  >
    Testing
  </button>
  <div class="modal fade modal-lg" :id="'modal' + $.uid" tabindex="-1" aria-hidden="true">
    <div class="modal-dialog">
      <div class="modal-content p-4">
        <div class="modal-header">
          <h1 class="modal-title fs-5">{{ disabled ? 'Processing...' : title }}</h1>
        </div>
        <div class="modal-body">
          <p v-if="message">{{ message }}</p>
          <BsSelect :disabled="disabled" v-model="result" :options="options"> </BsSelect>
        </div>
        <div class="modal-footer">
          <button
            :disabled="disabled"
            @click="confirm"
            type="button"
            class="btn btn-primary"
            data-bs-dismiss="modal"
          >
            Confirm
          </button>
          <button @click="cancel" type="button" class="btn btn-secondary" data-bs-dismiss="modal">
            Cancel
          </button>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref } from 'vue'

/*
 * Purpose: Show a select dialog
 */
defineOptions({
  inheritAttrs: false
})

const props = defineProps({
  callback: Function,
  id: String,
  disabled: Boolean,
  title: String,
  options: Array,
  message: String
})

const emit = defineEmits(['update:modelValue'])

const result = ref('')

const confirm = () => {
  emit('update:modelValue', result.value)
  props.callback(true, result.value)
}

const cancel = () => {
  props.callback(false, '')
}
</script>
