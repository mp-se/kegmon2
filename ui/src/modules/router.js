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

import { ref } from 'vue'
import { createRouter, createWebHistory } from 'vue-router'
import { validateCurrentForm } from '@mp-se/espframework-ui-components'
import * as badge from '@/modules/badge'
import { global } from '@/modules/pinia'

import HomeView from '@/views/HomeView.vue'
import DeviceSettingsView from '@/views/DeviceSettingsView.vue'
import DeviceStatisticsView from '@/views/DeviceStatisticsView.vue'
import DeviceHardwareView from '@/views/DeviceHardwareView.vue'
import DeviceCalibrationView from '@/views/DeviceCalibrationView.vue'
import DeviceWifiView from '@/views/DeviceWifiView.vue'
import DeviceEventView from '@/views/DeviceEventView.vue'
import TapsSettingsView from '@/views/TapsSettingsView.vue'
import TapsBeerView from '@/views/TapsBeerView.vue'
import TapsHistoryView from '@/views/TapsHistoryView.vue'
import IntInfluxdbView from '@/views/IntInfluxdbView.vue'
import IntHassView from '@/views/IntHassView.vue'
import IntBrewfatherView from '@/views/IntBrewfatherView.vue'
import IntBrewspyView from '@/views/IntBrewspyView.vue'
import IntBarhelperView from '@/views/IntBarhelperView.vue'
import IntBrewloggerView from '@/views/IntBrewloggerView.vue'
import AboutView from '@/views/AboutView.vue'
import BackupView from '@/views/BackupView.vue'
import FirmwareView from '@/views/FirmwareView.vue'
import SupportView from '@/views/SupportView.vue'
import SerialView from '@/views/SerialView.vue'
import ToolsView from '@/views/ToolsView.vue'
import NotFoundView from '@/views/NotFoundView.vue'

const routes = [
  {
    path: '/',
    name: 'home',
    component: HomeView
  },
  {
    path: '/device/settings',
    name: 'device-settings',
    component: DeviceSettingsView
  },
  {
    path: '/device/hardware',
    name: 'device-hardware',
    component: DeviceHardwareView
  },
  {
    path: '/device/calibration',
    name: 'device-calibration',
    component: DeviceCalibrationView
  },
  {
    path: '/device/statistics',
    name: 'device-statistics',
    component: DeviceStatisticsView
  },
  {
    path: '/device/wifi',
    name: 'device-wifi',
    component: DeviceWifiView
  },
  {
    path: '/device/events',
    name: 'device-events',
    component: DeviceEventView
  },
  {
    path: '/other/backup',
    name: 'backup',
    component: BackupView
  },
  {
    path: '/other/firmware',
    name: 'firmware',
    component: FirmwareView
  },
  {
    path: '/taps/settings',
    name: 'taps-settings',
    component: TapsSettingsView
  },
  {
    path: '/taps/beer',
    name: 'taps-beer',
    component: TapsBeerView
  },
  {
    path: '/taps/history',
    name: 'taps-history',
    component: TapsHistoryView
  },
  {
    path: '/integration/influxdb',
    name: 'integration-influxdb',
    component: IntInfluxdbView
  },
  {
    path: '/integration/homeassistant',
    name: 'integration-homeassistant',
    component: IntHassView
  },
  {
    path: '/integration/brewfather',
    name: 'integration-brewfather',
    component: IntBrewfatherView
  },
  {
    path: '/integration/brewspy',
    name: 'integration-brewspy',
    component: IntBrewspyView
  },
  {
    path: '/integration/barhelper',
    name: 'integration-barhelper',
    component: IntBarhelperView
  },
  {
    path: '/integration/brewlogger',
    name: 'integration-brewlogger',
    component: IntBrewloggerView
  },
  {
    path: '/other/support',
    name: 'support',
    component: SupportView
  },
  {
    path: '/other/tools',
    name: 'tools',
    component: ToolsView
  },
  {
    path: '/other/serial',
    name: 'serial',
    component: SerialView
  },
  {
    path: '/other/about',
    name: 'about',
    component: AboutView
  },
  {
    path: '/:catchAll(.*)',
    name: '404',
    component: NotFoundView
  }
]

const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),
  routes: routes
})

export default router

router.beforeEach(() => {
  if (global.disabled) return false

  if (!validateCurrentForm()) return false

  global.clearMessages()
  return true
})

const items = ref([
  {
    label: 'Home',
    icon: 'IconHome',
    path: '/',
    subs: []
  },
  {
    label: 'Device',
    icon: 'IconCpu',
    path: '/device',
    badge: badge.deviceBadge,
    subs: [
      {
        label: 'Settings',
        badge: badge.deviceSettingBadge,
        path: '/device/settings'
      },
      /*{
        label: 'Hardware',
        badge: badge.deviceHardwareBadge,
        path: '/device/hardware'
      },*/
      {
        label: 'Calibration',
        badge: badge.deviceCalibrationBadge,
        path: '/device/calibration'
      },
      {
        label: 'Events',
        path: '/device/events'
      },
      {
        label: 'Statistics',
        path: '/device/statistics'
      },
      {
        label: 'Wifi',
        badge: badge.deviceWifiBadge,
        path: '/device/wifi'
      }
    ]
  },
  {
    label: 'Taps',
    icon: 'IconJustify',
    path: '/taps',
    subs: [
      {
        label: 'Settings',
        path: '/taps/settings'
      },
      {
        label: 'Beers',
        path: '/taps/beer'
      },
      {
        label: 'History',
        path: '/taps/history'
      }
    ]
  },
  {
    label: 'Integrations',
    icon: 'IconCloudUpArrow',
    path: '/integration',
    subs: [
      {
        label: 'Home Assistant',
        path: '/integration/homeassistant'
      },
      {
        label: 'Brewfather',
        path: '/integration/brewfather'
      },
      {
        label: 'Brewlogger',
        path: '/integration/brewlogger'
      },
      {
        label: 'Brewspy',
        path: '/integration/brewspy'
      },
      {
        label: 'Barhelper',
        path: '/integration/barhelper'
      },
      {
        label: 'Influx Db',
        path: '/integration/influxdb'
      }
    ]
  },
  {
    label: 'Other',
    icon: 'IconTools',
    path: '/other',
    subs: [
      {
        label: 'Serial console',
        path: '/other/serial'
      },
      {
        label: 'Backup & Restore',
        path: '/other/backup'
      },
      {
        label: 'Firmware update',
        path: '/other/firmware'
      },
      {
        label: 'Support',
        path: '/other/support'
      },
      {
        label: 'Tools',
        path: '/other/tools'
      },
      {
        label: 'About',
        path: '/other/about'
      }
    ]
  }
])

export { items }
