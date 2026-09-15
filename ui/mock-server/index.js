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

import { createRequire } from "module";
import { registerEspFwk } from './espfwk.js'
import { statisticsData, scaleData } from './data.js'

const require = createRequire(import.meta.url);
const express = require('express')
var cors = require('cors')

const app = express()
const port = 3000

app.use(cors())
app.use(express.json())

registerEspFwk(app)

var testRunning = false

app.post('/api/push', (req, res) => {
  console.log('GET: /api/push')
  /* 
   * Description:    Initiate the push test for a defined target
   * Authentication: Required
   * Limitation:     - 
   * Note:           Use /api/test/push/status to check for completion
   * Return:         200 OK, 401 Access Denied, 422 Content Invalid
   * Request body:
     {
       push_format: "http_format|http_format2|http_format3|influxdb2_format|mqtt_format"
     }
   */
  if(!Object.prototype.hasOwnProperty.call(req.body, "push_format")) {
    res.sendStatus(422)
    return
  }  
  testRunning = true
  setTimeout(() => { testRunning = false }, 5000)
  var data = {
    success: true,
    message: "Test scheduled."
  }
  res.type('application/json')
  res.send(data)
})

app.get('/api/push/status', (req, res) => {
  console.log('GET: /api/push/status')
  /* 
   * Description:    Return status of the current gyro calibration process. 
   * Authentication: Required
   * Limitation:     - 
   * Note:           -
   * Return:         200 OK, 401 Access Denied
   */
  var data = {}
  if (testRunning) {
    data = {
      status: testRunning,
      success: false,
      message: "Push test running..."
    }
  } else {
    data = {
      status: false,
      success: true,
      message: "Push test completed...",
      push_return_code: 200,
      push_enabled: true
    }
  }
  res.type('application/json')
  res.send(data)
})


app.get('/levels', (req, res) => {
  console.log('GET: /levels')

  var levels = "2024-07-11 13:12:07;nan;16.587868;nan;nan\n\
2024-07-11 13:30:14;nan;16.488798;nan;0.099069 \n\
2024-07-11 13:30:15;nan;16.488798;nan;nan\n\
2024-07-11 14:01:24;nan;16.291916;nan;0.196884\n\
2024-07-11 14:01:24;nan;16.291916;nan;nan\n\
2024-07-11 17:22:55;nan;16.192898;nan;0.099018\n\
2024-07-11 17:22:55;nan;16.192898;nan;nan\n\
2024-07-11 19:34:53;nan;15.863690;nan;0.329206\n\
2024-07-11 19:34:53;nan;15.863690;nan;nan\n\
2024-07-11 22:14:11;nan;15.768347;nan;0.190148\n\
2024-07-11 22:14:11;nan;15.768347;nan;nan\n\
2024-07-11 22:17:53;nan;15.668524;nan;0.099822\n\
2024-07-11 22:17:53;nan;15.668524;nan;nan\n\
2024-07-11 22:19:23;nan;15.539115;nan;0.129409\n\
2024-07-11 22:19:24;nan;15.539115;nan;nan\n\
2024-07-12 00:03:40;8.623783;nan;nan;nan\n\
2024-07-12 00:03:40;nan;15.826628;nan;nan\n\
2024-07-12 01:48:25;nan;15.726063;nan;0.100565\n\
2024-07-12 01:48:26;nan;15.726063;nan;nan\n\
2024-07-12 01:50:02;nan;15.582197;nan;0.143866\n\
2024-07-12 01:50:02;nan;15.582197;nan;nan\n\
2024-07-12 05:12:30;nan;15.482185;nan;0.100012\n\
2024-07-12 05:12:30;nan;15.482185;nan;nan\n\
2024-07-12 23:25:21;8.524719;nan;0.099064;nan\n\
2024-07-12 23:25:21;8.524719;nan;nan;nan\n\
2024-07-14 04:58:30;8.670466;nan;nan;nan\n\
2024-07-14 04:58:30;nan;16.128782;nan;nan\n\
2024-07-14 05:07:22;nan;15.972906;nan;0.155876\n\
2024-07-14 05:07:22;nan;15.972906;nan;nan\n\
2024-07-14 05:11:04;nan;15.788408;nan;0.184498\n\
2024-07-14 05:11:05;nan;15.788408;nan;nan\n\
2024-07-14 05:17:51;nan;15.688248;nan;0.100161\n\
2024-07-14 05:17:51;nan;15.688248;nan;nan\n\
2024-07-14 19:57:58;8.662101;nan;nan;nan\n\
2024-07-14 19:57:59;nan;16.060747;nan;nan\n\
2024-07-14 20:34:13;nan;15.960558;nan;0.100189\n\
2024-07-14 20:34:13;nan;15.960558;nan;nan\n\
2024-07-14 20:36:31;nan;15.815523;nan;0.145035\n\
2024-07-14 20:36:31;nan;15.815523;nan;nan\n\
2024-07-14 20:40:11;nan;15.715315;nan;0.100208\n\
2024-07-14 20:40:11;nan;15.715315;nan;nan\n\
2024-07-15 09:53:54;nan;15.615223;nan;0.100092\n\
2024-07-15 09:53:54;nan;15.615223;nan;nan\n\
2024-07-16 14:37:44;nan;15.156647;nan;0.458576\n\
2024-07-16 14:37:45;nan;15.156647;nan;nan"
  res.send(levels)
})

// --- Scale calibration endpoints ---
app.post('/api/scale/tare', (req, res) => {
  console.log('POST: /api/scale/tare', req.body)
  res.type('application/json')
  res.send({ success: true, message: 'Tare operation initiated' })
})

app.post('/api/scale/factor', (req, res) => {
  console.log('POST: /api/scale/factor', req.body)
  res.type('application/json')
  res.send({ success: true, message: 'Factor calculation initiated' })
})

app.get('/api/scale', (req, res) => {
  console.log('GET: /api/scale')
  res.type('application/json')
  res.send(scaleData)
})

app.get('/api/stability', (req, res) => {
  console.log('GET: /api/stability')
  /*
   * Description:    Return stability data as json document.
   * Authentication: Required
   * Note:           -
   * Return:         200 OK, 401 Access Denied
   */
  const statsData = {
    stability_sum1: 0,
    stability_min1: 0,
    stability_max1: 0,
    stability_ave1: 0,
    stability_var1: 0,
    stability_popdev1: 0,
    stability_ubiasdev1: 0,
    stability_count1: 0,    
    stability_sum2: 0,
    stability_min2: 0,
    stability_max2: 0,
    stability_ave2: 0,
    stability_var2: 0,
    stability_popdev2: 0,
    stability_ubiasdev2: 0,
    stability_count2: 0,    
    stability_sum3: 0,
    stability_min3: 0,
    stability_max3: 0,
    stability_ave3: 0,
    stability_var3: 0,
    stability_popdev3: 0,
    stability_ubiasdev3: 0,
    stability_count3: 0,
    stability_sum4: 0,
    stability_min4: 0,
    stability_max4: 0,
    stability_ave4: 0,
    stability_var4: 0,
    stability_popdev4: 0,
    stability_ubiasdev4: 0,
    stability_count4: 0,    }

  res.type('application/json')
  res.send(statsData)
})

app.get('/api/statistics', (req, res) => {
  console.log('GET: /api/statistics')
  res.type('application/json')
  res.send(statisticsData)
})

app.get('/api/statistics/clear', (req, res) => {
  console.log('GET: /api/statistics/clear')
  res.type('application/json')
  res.send({ success: true, message: 'Statistics cleared' })
})

const mockEventData = {
  'data.csv': `1,2026-01-19 14:30:00,1,SYSTEM_STARTUP,0,0,0,0,0,0,0,0,0,0,,0,0,0
1,2026-01-19 14:30:05,1,STABLE_LEVEL,25.0,75.0,0,0,0,0,5000,0,0,0,,0,0,0
1,2026-01-19 14:32:15,1,POURING,25.0,0,25.0,0,0,0,0,0.0250,0,0,,0,0,0
1,2026-01-19 14:32:35,1,POUR_COMPLETED,24.2,72.6,25.0,24.2,0.8,0.24,20000,0.0040,0,0,,0,0,0
1,2026-01-19 14:32:36,1,STABLE_LEVEL,24.2,72.6,0,0,0,0,1000,0,0,0,,0,0,0
1,2026-01-19 15:45:20,1,POURING,24.2,0,24.2,0,0,0,0,0.0242,0,0,,0,0,0
1,2026-01-19 15:45:50,1,POUR_COMPLETED,23.1,69.3,24.2,23.1,1.1,0.33,30000,0.0037,0,0,,0,0,0
1,2026-01-19 15:45:51,1,STABLE_LEVEL,23.1,69.3,0,0,0,0,1100,0,0,0,,0,0,0
1,2026-01-19 17:20:10,1,POURING,23.1,0,23.1,0,0,0,0,0.0231,0,0,,0,0,0
1,2026-01-19 17:20:45,1,POUR_COMPLETED,21.8,65.4,23.1,21.8,1.3,0.39,35000,0.0037,0,0,,0,0,0
1,2026-01-19 17:20:46,1,STABLE_LEVEL,21.8,65.4,0,0,0,0,1200,0,0,0,,0,0,0
1,2026-01-19 19:15:30,1,POURING,21.8,0,21.8,0,0,0,0,0.0218,0,0,,0,0,0
1,2026-01-19 19:16:10,1,POUR_COMPLETED,20.2,60.6,21.8,20.2,1.6,0.48,40000,0.0040,0,0,,0,0,0
1,2026-01-19 19:16:11,1,STABLE_LEVEL,20.2,60.6,0,0,0,0,1300,0,0,0,,0,0,0
1,2026-01-19 21:30:15,1,POURING,20.2,0,20.2,0,0,0,0,0.0202,0,0,,0,0,0
1,2026-01-19 21:31:05,1,POUR_COMPLETED,18.5,55.5,20.2,18.5,1.7,0.51,50000,0.0034,0,0,,0,0,0
1,2026-01-19 21:31:06,1,STABLE_LEVEL,18.5,55.5,0,0,0,0,1400,0,0,0,,0,0,0
1,2026-01-19 23:10:40,1,POURING,18.5,0,18.5,0,0,0,0,0.0185,0,0,,0,0,0
1,2026-01-19 23:11:20,1,POUR_COMPLETED,16.8,50.4,18.5,16.8,1.7,0.51,40000,0.0425,0,0,,0,0,0
1,2026-01-19 23:11:21,1,STABLE_LEVEL,16.8,50.4,0,0,0,0,1500,0,0,0,,0,0,0
1,2026-01-20 01:05:15,1,POURING,16.8,0,16.8,0,0,0,0,0.0168,0,0,,0,0,0
1,2026-01-20 01:05:55,1,POUR_COMPLETED,14.9,44.7,16.8,14.9,1.9,0.57,40000,0.0475,0,0,,0,0,0
1,2026-01-20 01:05:56,1,STABLE_LEVEL,14.9,44.7,0,0,0,0,1600,0,0,0,,0,0,0
1,2026-01-20 03:20:30,1,POURING,14.9,0,14.9,0,0,0,0,0.0149,0,0,,0,0,0
1,2026-01-20 03:21:25,1,POUR_COMPLETED,12.5,37.5,14.9,12.5,2.4,0.72,55000,0.0436,0,0,,0,0,0
1,2026-01-20 03:21:26,1,STABLE_LEVEL,12.5,37.5,0,0,0,0,1700,0,0,0,,0,0,0
1,2026-01-20 06:15:45,1,POURING,12.5,0,12.5,0,0,0,0,0.0125,0,0,,0,0,0
1,2026-01-20 06:16:45,1,POUR_COMPLETED,9.8,29.4,12.5,9.8,2.7,0.81,60000,0.0450,0,0,,0,0,0
1,2026-01-20 06:16:46,1,STABLE_LEVEL,9.8,29.4,0,0,0,0,1800,0,0,0,,0,0,0
1,2026-01-20 08:45:20,1,POURING,9.8,0,9.8,0,0,0,0,0.0098,0,0,,0,0,0
1,2026-01-20 08:46:10,1,POUR_COMPLETED,6.5,19.5,9.8,6.5,3.3,0.99,60000,0.0550,0,0,,0,0,0
1,2026-01-20 08:46:11,1,STABLE_LEVEL,6.5,19.5,0,0,0,0,1900,0,0,0,,0,0,0
1,2026-01-20 10:30:50,1,POURING,6.5,0,6.5,0,0,0,0,0.0065,0,0,,0,0,0
1,2026-01-20 10:31:35,1,POUR_COMPLETED,3.0,9.0,6.5,3.0,3.5,1.05,70000,0.0500,0,0,,0,0,0
1,2026-01-20 10:31:36,1,STABLE_LEVEL,3.0,9.0,0,0,0,0,2000,0,0,0,,0,0,0
1,2026-01-20 11:45:15,1,KEG_REMOVED,0,0,0,0,0,0,0,0,3.0,0.1,,0,0,0
1,2026-01-20 11:45:25,1,KEG_REPLACED,0,0,0,0,0,0,0,0,0.1,25.0,,0,0,0
1,2026-01-20 11:45:30,1,STABLE_LEVEL,25.0,75.0,0,0,0,0,5000,0,0,0,,0,0,0
1,2026-01-20 13:20:15,1,SETTLING_STARTED,0,0,0,0,0,0,0,0,0,25.0,,0,0,0
1,2026-01-20 14:15:45,1,LOAD_CELL_ERROR,0,0,0,0,0,0,0,0,0,0,TIMEOUT,25,0.85,3
1,2026-01-20 14:15:50,1,LOAD_CELL_RECOVERED,0,0,0,0,0,0,0,0,0,0,,95,0.02,0
1,2026-01-19 14:35:10,2,SYSTEM_STARTUP,0,0,0,0,0,0,0,0,0,0,,0,0,0
1,2026-01-19 14:35:15,2,SETTLING_STARTED,0,0,0,0,0,0,0,0,0,22.5,,0,0,0
1,2026-01-19 14:35:20,2,STABLE_LEVEL,22.5,67.5,0,0,0,0,4500,0,0,0,,0,0,0
1,2026-01-19 14:50:20,2,POURING,22.5,0,22.5,0,0,0,0,-0.0225,0,0,,0,0,0
1,2026-01-19 14:50:50,2,POUR_COMPLETED,21.1,63.3,22.5,21.1,1.4,0.42,30000,-0.0047,0,0,,0,0,0
1,2026-01-19 14:50:51,2,STABLE_LEVEL,21.1,63.3,0,0,0,0,1100,0,0,0,,0,0,0
1,2026-01-19 16:30:10,2,POURING,21.1,0,21.1,0,0,0,0,-0.0211,0,0,,0,0,0
1,2026-01-19 16:31:00,2,POUR_COMPLETED,19.5,58.5,21.1,19.5,1.6,0.48,50000,-0.0032,0,0,,0,0,0
1,2026-01-19 16:31:01,2,STABLE_LEVEL,19.5,58.5,0,0,0,0,1200,0,0,0,,0,0,0
1,2026-01-19 16:35:20,2,WEIGHT_CHANGE_DETECTED,0,0,0,0,0,0,0,0,19.5,20.2,,0,0.70,0
1,2026-01-19 18:45:40,2,POURING,19.5,0,19.5,0,0,0,0,-0.0195,0,0,,0,0,0
1,2026-01-19 18:46:35,2,POUR_COMPLETED,17.8,53.4,19.5,17.8,1.7,0.51,55000,-0.0031,0,0,,0,0,0
1,2026-01-19 18:46:36,2,STABLE_LEVEL,17.8,53.4,0,0,0,0,1300,0,0,0,,0,0,0
1,2026-01-19 20:15:25,2,POURING,17.8,0,17.8,0,0,0,0,-0.0178,0,0,,0,0,0
1,2026-01-19 20:16:45,2,POUR_COMPLETED,15.9,47.7,17.8,15.9,1.9,0.57,80000,-0.0024,0,0,,0,0,0
1,2026-01-19 20:16:46,2,STABLE_LEVEL,15.9,47.7,0,0,0,0,1400,0,0,0,,0,0,0`,

  'data1.csv': `1,2026-01-18 10:00:00,2,SYSTEM_STARTUP,0,0,0,0,0,0,0,0,0,0,,0,0,0
1,2026-01-18 10:00:05,2,SETTLING_STARTED,0,0,0,0,0,0,0,0,0,22.5,,0,0,0
1,2026-01-18 10:00:10,2,STABLE_LEVEL,22.5,67.5,0,0,0,0,4500,0,0,0,,0,0,0
1,2026-01-18 10:05:20,2,POURING,22.5,0,22.5,0,0,0,0,-0.0225,0,0,,0,0,0
1,2026-01-18 10:05:45,2,POUR_COMPLETED,21.1,63.3,22.5,21.1,1.4,0.42,25000,-0.0056,0,0,,0,0,0
1,2026-01-18 10:05:46,2,STABLE_LEVEL,21.1,63.3,0,0,0,0,1100,0,0,0,,0,0,0
1,2026-01-18 12:30:15,2,POURING,21.1,0,21.1,0,0,0,0,-0.0211,0,0,,0,0,0
1,2026-01-18 12:31:00,2,POUR_COMPLETED,19.5,58.5,21.1,19.5,1.6,0.48,45000,-0.0036,0,0,,0,0,0
1,2026-01-18 12:31:01,2,STABLE_LEVEL,19.5,58.5,0,0,0,0,1200,0,0,0,,0,0,0
1,2026-01-18 15:20:45,2,POURING,19.5,0,19.5,0,0,0,0,-0.0195,0,0,,0,0,0
1,2026-01-18 15:21:35,2,POUR_COMPLETED,17.8,53.4,19.5,17.8,1.7,0.51,50000,-0.0034,0,0,,0,0,0
1,2026-01-18 15:21:36,2,STABLE_LEVEL,17.8,53.4,0,0,0,0,1300,0,0,0,,0,0,0
1,2026-01-18 15:22:00,2,INVALID_WEIGHT,0,0,0,0,0,0,0,0,0,0,,0,0,0
1,2026-01-18 15:22:10,2,SENSOR_RECOVERED,0,0,0,0,0,0,0,0,0,17.8,,0,0,0
1,2026-01-18 18:15:10,2,POURING,17.8,0,17.8,0,0,0,0,-0.0178,0,0,,0,0,0
1,2026-01-18 18:16:15,2,POUR_COMPLETED,15.9,47.7,17.8,15.9,1.9,0.57,65000,-0.0029,0,0,,0,0,0
1,2026-01-18 18:16:16,2,STABLE_LEVEL,15.9,47.7,0,0,0,0,1400,0,0,0,,0,0,0
1,2026-01-18 20:45:30,2,POURING,15.9,0,15.9,0,0,0,0,-0.0159,0,0,,0,0,0
1,2026-01-18 20:46:50,2,POUR_COMPLETED,13.5,40.5,15.9,13.5,2.4,0.72,80000,-0.0030,0,0,,0,0,0
1,2026-01-18 20:46:51,2,STABLE_LEVEL,13.5,40.5,0,0,0,0,1500,0,0,0,,0,0,0`,

  'data2.csv': `1,2026-01-17 09:00:00,1,SYSTEM_STARTUP,0,0,0,0,0,0,0,0,0,0,,0,0,0
1,2026-01-17 09:00:05,1,SETTLING_STARTED,0,0,0,0,0,0,0,0,0,26.0,,0,0,0
1,2026-01-17 09:00:10,1,STABLE_LEVEL,26.0,78.0,0,0,0,0,4800,0,0,0,,0,0,0
1,2026-01-17 09:10:20,1,POURING,26.0,0,26.0,0,0,0,0,-0.0260,0,0,,0,0,0
1,2026-01-17 09:10:45,1,POUR_COMPLETED,24.9,74.7,26.0,24.9,1.1,0.33,25000,-0.0044,0,0,,0,0,0
1,2026-01-17 09:10:46,1,STABLE_LEVEL,24.9,74.7,0,0,0,0,1000,0,0,0,,0,0,0
1,2026-01-17 09:15:30,1,KEG_ABSENT_TIMEOUT,0,0,0,0,0,0,30000,0,0,0.5,,0,0,0
1,2026-01-17 11:45:30,1,POURING,24.9,0,24.9,0,0,0,0,-0.0249,0,0,,0,0,0
1,2026-01-17 11:46:10,1,POUR_COMPLETED,23.5,70.5,24.9,23.5,1.4,0.42,40000,-0.0035,0,0,,0,0,0
1,2026-01-17 11:46:11,1,STABLE_LEVEL,23.5,70.5,0,0,0,0,1100,0,0,0,,0,0,0
1,2026-01-17 14:20:45,1,POURING,23.5,0,23.5,0,0,0,0,-0.0235,0,0,,0,0,0
1,2026-01-17 14:21:30,1,POUR_COMPLETED,21.8,65.4,23.5,21.8,1.7,0.51,45000,-0.0378,0,0,,0,0,0
1,2026-01-17 14:21:31,1,STABLE_LEVEL,21.8,65.4,0,0,0,0,1200,0,0,0,,0,0,0
1,2026-01-17 14:25:00,1,CALIBRATION_NEEDED,0,0,0,0,0,0,0,0,0,0,,0,0,0
1,2026-01-17 14:25:30,1,CALIBRATION_COMPLETE,0,0,0,0,0,0,0,0,0,0,,0,0,0
1,2026-01-17 17:00:10,1,POURING,21.8,0,21.8,0,0,0,0,-0.0218,0,0,,0,0,0
1,2026-01-17 17:01:05,1,POUR_COMPLETED,19.9,59.7,21.8,19.9,1.9,0.57,55000,-0.0035,0,0,,0,0,0
1,2026-01-17 17:01:06,1,STABLE_LEVEL,19.9,59.7,0,0,0,0,1300,0,0,0,,0,0,0
1,2026-01-17 19:30:40,1,POURING,19.9,0,19.9,0,0,0,0,-0.0199,0,0,,0,0,0
1,2026-01-17 19:31:50,1,POUR_COMPLETED,17.6,52.8,19.9,17.6,2.3,0.69,70000,-0.0033,0,0,,0,0,0
1,2026-01-17 19:31:51,1,STABLE_LEVEL,17.6,52.8,0,0,0,0,1400,0,0,0,,0,0,0
1,2026-01-17 22:15:20,1,KEG_REMOVED,0,0,0,0,0,0,0,0,17.6,0.2,,0,0,0
1,2026-01-17 22:15:30,1,KEG_REPLACED,0,0,0,0,0,0,0,0,0.2,25.0,,0,0,0`
}

app.get('/sd/:filename', (req, res) => {
  console.log(`GET: /sd/${req.params.filename}`)
  const filename = req.params.filename
  const csvData = mockEventData[filename]

  if (!csvData) {
    res.status(404).send('File not found')
    return
  }

  // Add 500ms delay per file
  setTimeout(() => {
    res.type('text/csv')
    res.send(csvData)
  }, 500)
})

app.listen(port, () => {
  console.log(`Kegmon API simulator port http://localhost:${port}/`)
})