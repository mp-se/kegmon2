#!/usr/bin/env python3
"""Fetch and display recent core metrics from InfluxDB for debugging"""

import influxdb_client
from influxdb_client.client.write_api import SYNCHRONOUS

# Configuration
URL = "http://192.168.1.89:8086"
TOKEN = "K_mo_oBg2jOUuein420L9YGl5uG3QVtXG7eh_dlCLPwchVUboKIaKgYvd5qoDme-RojxqMUo1hhI6wUsaC1DHw=="
ORG = "home"
BUCKET = "kegmon2"

client = influxdb_client.InfluxDBClient(
    url=URL,
    token=TOKEN,
    org=ORG
)

query_api = client.query_api()

# Define state names for readability
# Maps both numeric and string representations
STATES = {
    0: "IDLE", 1: "SETTLING", 2: "STABLE", 3: "POURING",
    4: "KEG_ABSENT", 5: "REPLACING", 6: "INVALID", 7: "ERROR", 8: "CALIB",
    "0": "IDLE", "1": "SETTLING", "2": "STABLE", "3": "POURING",
    "4": "KEG_ABSENT", "5": "REPLACING", "6": "INVALID", "7": "ERROR", "8": "CALIB",
    "Idle": "IDLE", "Settling": "SETTLING", "Stable": "STABLE", "Pouring": "POURING",
    "KegAbsent": "KEG_ABSENT", "ReplacingKeg": "REPLACING", "InvalidWeight": "INVALID",
    "LoadCellError": "ERROR", "CalibrationNeeded": "CALIB"
}

# Query to pivot relevant fields for scale 1 into a single row per timestamp
query = f'''
from(bucket: "{BUCKET}")
  |> range(start: -10m)
  |> filter(fn: (r) => r._measurement == "scale")
  |> filter(fn: (r) => r._field == "kalman1" or r._field == "ema1" or r._field == "avg_slope1" or r._field == "state1" or r._field == "stable_wgt1" or r._field == "raw1")
  |> pivot(rowKey:["_time"], columnKey: ["_field"], valueColumn: "_value")
  |> sort(columns: ["_time"], desc: false)
'''

print(f"\nCore metrics for Scale 1 (Last 10 minutes):")
print("-" * 115)
header = f"{'Time (UTC)':<25} {'Raw':<10} {'Kalman':<12} {'EMA':<12} {'Slope':<10} {'StableW':<10} {'State':<15}"
print(header)
print("-" * 115)

try:
    tables = query_api.query(query)
    for table in tables:
        for record in table.records:
            t = str(record.get_time())[:23] # Truncate sub-second precision for readability
            raw = record.values.get("raw1", 0.0)
            kalman = record.values.get("kalman1", 0.0)
            ema = record.values.get("ema1", 0.0)
            slope = record.values.get("avg_slope1", 0.0)
            stable = record.values.get("stable_wgt1", 0.0)
            state_val = record.values.get("state1", "?")
            
            # Map state value (tries float -> int -> string)
            state_display = STATES.get(state_val, str(state_val))
            if isinstance(state_val, float):
                state_display = STATES.get(int(state_val), str(state_val))
            
            print(f"{t:<25} {raw:<10.4f} {kalman:<12.4f} {ema:<12.4f} {slope:<10.5f} {stable:<10.3f} {state_display:<15}")
except Exception as e:
    print(f"Error querying InfluxDB: {e}")

client.close()
