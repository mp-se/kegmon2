# Kegmon State Machine Requirements

## Overview
Each scale has an independent state machine with 10 states. The machine tracks weight changes and detects pours, keg replacements, and weight anomalies.

**Critical Principle:** `stableWeight` is the baseline weight against which all changes are measured. It should only be SET when entering STABLE state, never during transitions.

**stableWeight Initialization:** `stableWeight = NaN` indicates the baseline has NOT been established yet. Only set to an actual weight value when transitioning TO STABLE state. Checking `isnan(stableWeight)` determines if baseline is valid.

**stableVolume:** Always calculated from `stableWeight` on-the-fly using the weight-to-volume converter. NOT stored independently. This is just an alternative representation (liters vs kilograms) of the same baseline value. Volume is calculated based on the beers FG which has a slight impact on the weight.

**currentWeight Source:** All conditions comparing `currentWeight` use the **Kalman filter output** (FILTER_KALMAN, index 12) as the source. The Kalman filter provides the most stable, noise-reduced weight estimate suitable for state machine decision-making. Other filters (EMA, Butterworth, Median) are available for display/logging but do NOT affect state transitions.

**Slope Detection Source:** Pour detection (slope calculation) uses the **fast-responding filter** (FILTER_EMA or FILTER_WMA) to detect rapid weight changes. The Kalman filter (stability filter) is used for establishing the baseline and validating the final settled weight, but is too slow for responsive pour triggering. Trigger pour if the fast filter detects a slope exceeding the threshold and within realistic flow rate bounds.

---

## Global Conditions (Checked Every Cycle - ALL States EXCEPT IDLE and DISABLED)

These conditions override state-specific logic and trigger transitions from ANY state **AFTER initialization**. Check in this priority order:

**IMPORTANT: Pre-emptive State Check**
If a scale is in the **DISABLED** state, all global conditions and state machine processing are skipped. The scale is considered non-functional for the duration of the session.

**IMPORTANT: Filter Warm-up Grace Period**
Global conditions DO NOT apply during IDLE state (first 2 seconds). This allows filters (Kalman, EMA) to initialize and converge without false error triggers from startup transients. Only basic weight validity checks apply in IDLE. Once the system transitions OUT of IDLE to SETTLING, all global conditions become active and remain active for all subsequent states.

### 1. **Scale Becomes Uncalibrated** (Exit to CALIBRATION_NEEDED)
   - Condition: `scaleFactor == 0.0 OR scaleFactor becomes invalid`
   - Action: Transition to CALIBRATION_NEEDED
   - Fire: CALIBRATION_NEEDED event
   - **Priority: HIGHEST** - cannot process readings without calibration

### 2. **Signal Quality Critical** (Exit to LOAD_CELL_ERROR)
   - Condition: `signalQualityPercent < 20 OR consecutiveErrors >= 10`
   - Action: Transition to LOAD_CELL_ERROR
   - Fire: LOAD_CELL_ERROR event with error reason details
   - **Priority: HIGH** - sensor health check
   - **Except**: If already in CALIBRATION_NEEDED (stay there)
   
   **Signal Quality Calculation:**
   ```
   signalQualityPercent = 100 - (consecutiveErrors * 5)
   Minimum: 0%, Maximum: 100%
   ```
   
   **Entry Triggers (detailed):**
   - **TIMEOUT**: HX711 not responding for X ms → enter immediately (Severity: Critical)
   - **NAN_VALUE**: Reading is NaN (invalid float) → enter immediately (Severity: Critical)
   - **STUCK_VALUE**: Same value repeated (variance = 0 for 20+ reads) → enter after 5+ consecutive (Severity: Critical)
   - **PHANTOM_SPIKE**: Single-read weight change > maxSingleReadChangeKg (e.g., > 0.5kg) → counts as error, contributes to quality score (Severity: High)
   - **Quality Threshold**: `signalQualityPercent < 20` (≥4 consecutive errors, 95% recent reads bad) → enter when crossed
   - **Error Accumulation**: `consecutiveErrors >= 10` (10+ bad reads in a row) → enter when crossed
   
   **Calibration Note**: Lack of calibration (`scaleFactor == 0`) is NOT treated as a signal error and does not affect `signalQualityPercent` or `consecutiveErrors`. This ensures a non-calibrated scale remains in the `CALIBRATION_NEEDED` state rather than showing a generic hardware error.

   **Recovery Conditions (Exit from LOAD_CELL_ERROR):**
   - Both conditions must be met:
     1. `signalQualityPercent > 80` (fewer than 4 errors in recent reads)
     2. `consecutiveErrors == 0` (confirmed clean reads)
   - Action: Transition back to previous state OR to SETTLING if previous state unknown
   - Fire: LOAD_CELL_RECOVERED event
   - Recovery Timeout: If stuck > 5 minutes with no improvement, alert user: "Scale sensor unresponsive - manual intervention may be needed"

### 2. **Scale Becomes Uncalibrated** (Exit to CALIBRATION_NEEDED)
   - Condition: `scaleFactor == 0.0 OR scaleFactor becomes invalid`
   - Action: Transition to CALIBRATION_NEEDED
   - Fire: CALIBRATION_NEEDED event
   - **Priority: HIGH** - cannot process readings without calibration
   - **Except**: If already in LOAD_CELL_ERROR (stay there)

### 3. **Weight Invalid** (Exit to INVALID_WEIGHT)
   - Condition: `currentWeight < -0.1kg OR currentWeight > (kegWeight + beerCapacity * 1.1)`
   - Action: Transition to INVALID_WEIGHT
   - Fire: INVALID_WEIGHT event
   - **Priority: HIGH** - sensor reading out of bounds
   - **Except**: If already in INVALID_WEIGHT (stay there) or LOAD_CELL_ERROR (stay there)

### 4. **Weight Absent** (Exit to KEG_ABSENT)
   - Condition: `-0.1kg <= currentWeight < (kegWeight * 0.9)` (weight below minimum valid keg weight)
   - Action: Transition to KEG_ABSENT
   - Fire: KEG_REMOVED event (if coming from a state that should report removal)
   - **Priority: HIGH** - no valid weight to process
   - **Except**: If already in KEG_ABSENT (stay there)

**Weight Range Summary (NO OVERLAP):**
- **INVALID_WEIGHT**: `currentWeight < -0.1kg` OR `currentWeight > (kegWeight + beerCapacity * 1.1)` (out of bounds)
- **KEG_ABSENT**: `-0.1kg <= currentWeight < (kegWeight * 0.9)` (weight present but keg too light)
- **VALID FOR PROCESSING**: `(kegWeight * 0.9) <= currentWeight <= (kegWeight + beerCapacity * 1.1)` (keg present with reasonable load)

**Note:** State-specific exit conditions only apply after global conditions are checked and false.

---

## Anomalous Data Detection

**Phantom/Spike Detection (Global, all states except IDLE):**

Between consecutive reads, weight changes are expected to be gradual during normal operation (settling, pouring, or equilibrium). Sudden large changes between individual reads indicate sensor artifacts or noise spikes and should be treated as signal quality issues.

**Detection Logic:**
- Monitor `|currentWeight(t) - currentWeight(t-1)|` between consecutive sensor reads
- Threshold: If single-cycle change > `maxSingleReadChangeKg` (e.g., 0.5kg in 100ms), it's physically unrealistic for a keg and indicates phantom data
- Action: Count as signal error without triggering LOAD_CELL_ERROR immediately
- Recovery: If the next read is normal, the spike is isolated; if multiple spikes occur, quality score drops

**Impact on Signal Quality:**
- Each phantom spike increments `consecutiveErrors` (same as other error types)
- Contributes to `signalQualityPercent = 100 - (consecutiveErrors * 5)` calculation
- If spikes accumulate: quality < 20% OR consecutiveErrors >= 10 → LOAD_CELL_ERROR

**Rationale:**
- Filters (Kalman, EMA) help smooth phantom data, but don't catch all spikes before they affect slope calculations
- Detection at read-level provides early warning of sensor degradation
- Distinguishes between normal variance (expected) and anomalous jumps (artifact)
- Prevents false pour detection from phantom drops in STABLE state

---

## Validation Criteria Principle

**ALL state transition conditions require explicit validation thresholds** to prevent false triggers from transient sensor noise and single-sample anomalies. Each condition must specify either:

1. **Reading Count Validation:** Minimum number of consecutive readings meeting the condition
2. **Time-Based Validation:** Minimum elapsed time the condition must be true
3. **Combination:** Both reading count AND time duration required

**Why This Matters:**
- Single-sample changes (1 reading showing an anomaly) should NOT trigger state transitions
- Transient sensor noise, EMI spikes, and filter initialization artifacts can produce 1-2 odd readings
- Multiple consecutive readings confirm a real state change vs. noise
- Typical sensor reads at ~10Hz, so 3 readings ≈ 300ms confirmation window

**Validation Summary Table (by state transition):**
| Transition | Condition | Validation | Threshold |
|-----------|-----------|-----------|-----------|
| STABLE → POURING | Negative slope detected | Reading count | ≥3 readings (Fast Filter) |
| STABLE → SETTLING | Weight change (inc/dec) | Reading count | ≥3 readings (±0.4kg/±0.1kg) |
| POURING → STABLE (normal) | Slope positive | Reading count | ≥1 reading confirms |
| POURING → SETTLING (weight removal) | Steep drop rate | Reading count | ≥3 consecutive readings (slope < -0.15 kg/s) |
| POURING → STABLE (timeout) | Time elapsed | Time-based | ≥2 seconds |
| SETTLING → STABLE | Weight settled | Time-based + slope | ≥6 seconds WITH slope < threshold |
| IDLE → SETTLING | Weight present + filters ready | Time-based | ≥2 seconds |
| KEG_ABSENT → KEG_REPLACED | Weight reappears | Reading count | ≥1 reading (immediate detection) |
| KEG_ABSENT → timeout event | Time in absence | Time-based | ≥30 seconds (periodic: 30, 60, 90...) |

---

## State Definitions & Data Management

### 1. IDLE State
**Purpose:** Initialization - no baseline established yet. Filters are initializing and converging.

**Filter Warm-up Grace Period:** During IDLE state, global error conditions (Signal Quality, Calibration, Weight Invalid) are NOT enforced. This allows the system to initialize without false errors from startup transients. Only weight presence/absence is checked for transition eligibility.

**Data State:**
- `stableWeight = NaN` (not set - use `isnan(stableWeight)` to check)
- `state = Idle`
- All slope tracking reset
- Filters initializing (Kalman, EMA, etc. beginning to converge)

**Minimum Duration:** Must remain in IDLE for at least 2 seconds after valid weight detected to allow filters to initialize and converge. This prevents using noisy or stale readings as the baseline.

**Exit Conditions:** (Global conditions NOT checked - see above)

1. **Weight Present & Filters Initialized** (Exit to SETTLING)
   - Conditions (ALL must be true):
     - `currentWeight >= (kegWeight * 0.9)` (weight at least 90% of empty keg)
     - `currentWeight <= (kegWeight + beerCapacity * 1.1)` (weight not exceeding max)
     - `timeInState >= 2000ms` (filters have had time to initialize)
   - Action: Transition to SETTLING
   - Fire: SETTLING_STARTED event
   - Why: Weight is valid and filters are initialized, safe to establish baseline
   - **NOTE:** After this transition, all global conditions become active for all remaining states
   - **EXPLICIT:** Global Condition #4 (Weight Absent: currentWeight < kegWeight * 0.9) is NOT checked in IDLE, but exit condition requires weight >= (kegWeight * 0.9) to proceed
   - **State Transition:** IDLE → SETTLING (not STABILIZING - SETTLING is the single unified state for both initial and re-stabilization)

---

### 2. SETTLING State
**Purpose:** Wait for weight to settle into a stable baseline. Handles initial stabilization (from IDLE) and re-stabilization after pours/changes (from POURING or STABLE).

**Data State:**
- `stableWeight` NOT YET SET (or preserved from before)
- Moving average of weight readings
- Tracking time in state
- Variance calculation for stability detection
- Slope tracking to detect settlement

**Settlement Detection Logic (unified):**
A weight is "stabilized" when the Kalman filter slope remains within acceptable range for a sustained period:

- **Slope Within Acceptable Range:** `avgSlope > minPourSlope/100` (extremely small: > -0.0015 kg/s or near-zero)
  - **CRITICAL:** Slope is calculated from the **Kalman/stability filter**, not the fast EMA filter
  - This ensures we're checking when the actual baseline has stabilized, not just the fast-responding filter
  - This is 10x stricter than before (old: 0.015 kg/s, new: -0.0015 kg/s)
  - Filters must show nearly zero rate of change before declaring stability

- **AND** sustained stability: `timeWithinSlopeThreshold >= 6000ms` (slope stays in range for continuous 6 seconds)
  - Unlike a fixed timer that waits 6 seconds then checks slope once, this requires **sustained convergence**
  - If slope exceeds threshold at ANY point, the timer resets to 0
  - Only when slope remains within acceptable range for 6 **consecutive** seconds does stabilization trigger
  - This prevents locking while filter is still drifting or oscillating
  - Ensures filter has truly reached equilibrium, not just temporarily within threshold
  - More adaptive than fixed time: fast-settling weights stabilize quickly, slow-settling weights take longer

- **OR** extended timeout reached (6 sec × 5 = 30 sec fallback) with weight within stability window
  - Prevents indefinite settling if filter cannot converge
  - Triggers after 30 seconds regardless of slope behavior

**Exit Conditions:** (Global conditions checked first - see above)

1. **Weight Stabilized** (Exit to STABLE)
   - Condition: Weight stabilization detected (see logic above - ALL conditions must be met)
   - Action: Set `stableWeight = currentWeight`, Transition to STABLE
   - Fire: STABLE_LEVEL event with newly settled weight
   - Why: Settlement confirmed with stricter criteria (filters truly at rest), new baseline locked in
 
---

### 3. STABLE State
**Purpose:** Baseline locked, monitoring for pours or weight changes

**Data State:**
- `stableWeight` = locked baseline (DO NOT MODIFY)
- Continuously calculating slope of weight changes

**Exit Conditions:**

1. **Pour Detected** (weight dropping with sustained negative slope)
   - Condition: `avgSlope < pourSlopeThreshold` AND within realistic range (-0.15 to -0.008)
   - Validation: Requires ≥3 consecutive readings in the fast filter showing negative slope before triggering
   - Action: Transition to POURING
   - Fire: POURING event
   - Rationale: 3-reading requirement prevents false detection from transient 1-2 sample weight fluctuations. Realistic range ensures we only trigger on actual pours, not slow drift or sudden removal.

2. **Weight Increase > 0.4kg** (Weight addition)
   - Condition: `currentWeight >= stableWeight + 0.4kg`
   - Validation: Requires 0.4kg increase sustained for at least 3 consecutive readings (≥ 300ms at typical 10Hz rate)
   - Action: Transition to SETTLING
   - Fire: WEIGHT_CHANGE_DETECTED event
   - Rationale: DO NOT modify stableWeight - SETTLING will determine true baseline. Multiple readings confirm real weight addition, not sensor spike.

3. **Weight Decrease > 0.1kg** (Passive/Slow Pour Detection)
   - Condition: `currentWeight <= stableWeight - 0.1kg`
   - Validation: Requires 0.1kg decrease sustained for at least 3 consecutive readings.
   - Action: Transition to SETTLING
   - Fire: WEIGHT_CHANGE_DETECTED event
   - Rationale: Catches slow pours or weight drops that were too slow for the slope trigger. SETTLING will eventually return to STABLE.

---

### 4. POURING State
**Purpose:** Detect and measure active pour event

**Data State:**
- `prePourWeight = stableWeight` (weight before pour started)
- `stableWeight` NOT MODIFIED during pour
- Slope tracking continuing
- `pourVolume` accumulating

**Exit Conditions:**

1. **Pour Completes** (weight stabilized at new level with minimum hold time)
   - Condition: ALL of the following must be true:
     - `timeInState >= 1000ms` (minimum 1 second to allow weight to physically settle)
     - `kalmanSlope > pourSlopeThreshold` (Kalman filter confirms slope positive/near-zero - stable baseline, not noisy fast filter)
     - `(prePourWeight - currentWeight) >= levelDecreaseThreshold` (minimum 0.1 kg weight drop confirmed)
   - Validation: 
     - **Filter Source:** Use **Kalman filter slope**, not EMA or WMA. The Kalman filter provides the stable, noise-reduced baseline confirmation that weight has truly settled to its final level
     - **Time Gate:** `timeInState >= 1000ms` ensures scale has had time to physically dampen oscillations and stabilize around the new equilibrium weight
     - **Slope Confirmation:** Slope must be positive/near-zero (not just for 1 sample, but consistently) to confirm weight is no longer dropping
     - **Weight Drop Validation:** Confirms actual pour occurred (>= 0.1 kg minimum)
   - Action: Set `stableWeight = currentWeight`, Transition to STABLE
   - Fire: POUR_COMPLETED event (if weight drop >= threshold) OR STABLE_LEVEL event (if weight drop < threshold)
   - Why: 
     - **Time buffer prevents premature exit:** Hardware load cells exhibit mechanical settling time (~0.5-1.5 seconds). Exiting before this completes captures intermediate weight as final, not the true settled weight
     - **Kalman filter ensures true stabilization:** The Kalman filter (stable baseline) shows the actual equilibrium. Fast filters (EMA, WMA) can show false stability while still trending. Entry uses WMA for fast detection (catch pours quickly), but exit must use Kalman for confirmation (ensure weight truly settled)
     - **Prevents false pours:** Without time gate and Kalman validation, pour completes while scale still oscillating toward final weight
     - **Real-world example:** 1.014 kg drop over 4 minutes (slow pour) needs 1+ second post-pour for load cell to stop moving and settle to its new equilibrium (~6.7374 kg). Exiting at 1.4 seconds before settlement complete captured intermediate weight instead of final level

2. **Unrealistic Drop Rate** (slope too steep)
   - Condition: `avgSlope < minPourSlope` (e.g., < -0.15 kg/s)
   - Validation: Requires ≥3 consecutive readings with slope < minPourSlope before confirming weight removal (prevents single-read transient spikes)
   - Action: Transition to SETTLING
   - Fire: SETTLING_STARTED event
   - Why: Sudden drop indicates weight removal, not a pour. Pours are gradual. Multiple confirmations prevent false detection. Transition to SETTLING to re-verify stability.

3. **Pour Timeout**
   - Condition: `timeInState >= pourDurationMs` (2 seconds elapsed)
   - Validation: Time-based - absolute 2-second threshold (no reading count needed, time is the gate)
   - Action: Set `stableWeight = currentWeight`, Transition to STABLE
   - Fire: POUR_COMPLETED event
   - Why: Timeout ensures baseline locked for next pour, regardless of slope. Prevents indefinite POURING state

---

### 5. KEG_ABSENT State
**Purpose:** Waiting for keg to be replaced (30 second timeout)

**Data State:**
- `stableWeight` unchanged from when keg was removed
- Waiting for weight to reappear

**Exit Conditions:** (Global conditions checked first - see above)

1. **Weight Reappears** (weight >= min keg weight)
   - Condition: `currentWeight >= (kegWeight * 0.9)` (weight at least 90% of empty keg)
   - Action: Set `stableWeight = currentWeight`, Transition to REPLACING_KEG
   - Fire: KEG_REPLACED event
   - Why: New keg detected
   - Priority: Checked BEFORE timeout event; if weight recovers, exit KEG_ABSENT immediately

2. **Timeout Expired** (stays in KEG_ABSENT, awaiting keg)
   - Condition: `timeInState >= 30 seconds` (allow extended wait for replacement)
   - Action: Stay in KEG_ABSENT (do not exit state, but log timeout)
   - Fire: KEG_ABSENT_TIMEOUT event (reminder that keg has been missing for 30+ seconds)
   - Firing Pattern: Event fires once when timeInState crosses 30 seconds, then again at 60 seconds, 90 seconds, etc. (every 30-second boundary)
   - Why: Keg missing for extended period - user notification/reminder; periodic events for extended monitoring

---

### 6. REPLACING_KEG State
**Purpose:** Wait for new keg weight to stabilize (3 second timeout)

**Data State:**
- `stableWeight = currentWeight` (new keg baseline being established)
- Updated with each reading

**Exit Conditions:** (Global conditions checked first - see above)

1. **Timeout reached with stable readings** (Exit to STABLE)
   - Conditions (ALL must be true):
     - `timeInState >= 3000ms` (3 second minimum duration)
     - `weightVariance < stabilityThreshold` (readings are stable/converged)
   - Action: Transition to STABLE
   - Fire: STABLE_LEVEL event with newly settled weight
   - Why: New keg baseline confirmed and stable

2. **Weight drops** (keg removed again) (Exit to KEG_ABSENT)
   - Condition: `currentWeight < 0.1kg` (covered by global weight absent condition)
   - Action: Transition to KEG_ABSENT
   - Fire: KEG_REMOVED event
   - Why: Keg removal detected during new keg stabilization

---

### 7. INVALID_WEIGHT State
**Purpose:** Sensor is reporting invalid readings (out of valid weight range)

**Data State:**
- `stableWeight` unchanged
- No weight processing

**Exit Conditions:** (Global conditions checked first - see above)

1. **Weight Returns to Valid Range** (Exit to SETTLING)
   - Condition: `currentWeight >= (kegWeight * 0.9) AND currentWeight <= (kegWeight + beerCapacity * 1.1)`
   - Action: Transition to SETTLING
   - Fire: SENSOR_RECOVERED event (sensor reading returned to valid range)
   - Why: Sensor recovered, need to re-establish baseline

---

### 8. LOAD_CELL_ERROR State
**Purpose:** ADC/Load cell signal is lost, unresponsive, or too unreliable (sensor is dead)

**Data State:**
- `stableWeight` unchanged but not used
- `signalQualityPercent` < 20 (very poor signal)
- `consecutiveErrors` high (multiple bad reads)
- Scale should be treated as offline/unusable

**Entry:** Triggered by Global Condition #1 (Signal Quality Critical). See Global Conditions section for detailed entry triggers.

**Exit Condition (Recovery):**
   - Conditions (ALL must be true):
     - `signalQualityPercent > 80` (signal quality recovered)
     - `consecutiveErrors == 0` (confirmed clean reads)
   - Action: Transition back to previous state OR to SETTLING if previous state unknown
   - Fire: LOAD_CELL_RECOVERED event
   - Why: Sensor has recovered, can resume monitoring

---

### 10. DISABLED State
**Purpose:** Scale unit is permanently inactive because ADC hardware (HX711) was not detected at startup.

**Data State:**
- `stableWeight = NaN`
- No weight processing, no GPIO polling, no background loop execution.
- This state is terminal for the current session (hardware is only scanned at boot).

**Differentiator:**
- **DISABLED**: Hardware chip (ADC) is physically missing or failed core initialization. Permanent for session.
- **LOAD_CELL_ERROR**: Hardware chip is present, but the 4-wire load cell signal is lost, noisy, or disconnected. Recoverable if signal returns.

**Exit Condition:**
- None (requires system reboot to re-scan hardware).

---

## Error Reason Details

| Reason | Cause | Recoverable | Action |
|--------|-------|------------|--------|
| **TIMEOUT** | HX711 SPI timeout, clock issues, wiring loose | Yes | Resume on next successful read |
| **NAN_VALUE** | ADC returned invalid float, memory corruption | Maybe | Check wiring, restart sensor |
| **OUT_OF_RANGE** | Reading > ±50kg (physical sensor limit) | Yes | This is INVALID_WEIGHT, not LoadCellError |
| **STUCK_VALUE** | Sensor frozen at one value, mechanical jam | No | Requires physical intervention |
| **PHANTOM_SPIKE** | Unexplained large jump between reads (> 0.5kg), EMI artifact, loose connection | Yes | Check wiring, shield from EMI, restart |
| **EXCESSIVE_NOISE** | Electromagnetic interference, bad power supply | Yes | Shield wiring, check PSU |
| CALIBRATION_INVALID | ScaleFactor = 0 or offset not set | No | This is CALIBRATION_NEEDED, not LoadCellError |

**Note on AvgSlope Logging:** When firing pour-related events (POURING, POUR_COMPLETED), the `AvgSlope` CSV field should use the **Kalman filter slope** for consistency. The Kalman filter provides the stable, noise-reduced slope suitable for historical analysis. EMA slope is used internally for fast detection but not logged.

---

## Key Principles

### 1. **stableWeight Management**
- **Initialization:** `stableWeight = NaN` (indicates "not set")
- **Set:** Only when transitioning TO STABLE state
- **Check:** Use `isnan(stableWeight)` to verify if baseline is valid
- **Preserve:** Never modified during transitions
- **Reference:** Used to detect significant changes (pours, additions)

### 2. **Slope Tracking and Filter Selection**
- **Reset:** On transitions from STABLE (to POURING or SETTLING)
- **Purpose:** Detect whether weight is changing or settled
- **Filter Selection (CRITICAL):**
  - **Entry to POURING (Pour Detection):** Use **WMA (Weighted Moving Average)** or fast-response filter for slope detection. Fast filters catch rapid weight changes quickly. Entry should trigger as soon as slope exceeds threshold to detect pours responsively.
  - **Exit from POURING (Pour Completion):** Use **Kalman filter** for slope validation. Kalman provides the stable, noise-reduced baseline. Exit requires confirmation that weight has truly settled to its equilibrium, not just a transient dip from noise or fast-filter overshoot.
  - **SETTLING state:** Use **Kalman filter** for slope checks to confirm stable baseline before locking into STABLE state
  - **Rationale:** Entry and exit use different filters because they serve different purposes:
    - Entry: Detect changes fast (WMA responsive, low latency)
    - Exit: Confirm true stability (Kalman stable baseline, filtered noise)
- **Threshold Rationale:** Slope threshold `avgSlope > -minPourSlope/100` (~0.0015 kg/s) is necessary because:
  - Raw readings can fluctuate ±0.001-0.002 kg due to sensor noise
  - Kalman filter can show micro-movements as it reaches steady state
  - Previous threshold (0.015 kg/s) allowed declarations of stability while filter still settling
  - Stricter threshold ensures filters have truly reached equilibrium before locking baseline
  - Empirical InfluxDB data shows filters converge within ~6 seconds at this threshold

### 4. **stableVolume Calculation**
- **Calculation:** `stableVolume = weightToVolume(stableWeight)`
- **Storage:** NOT stored independently - always calculated on-the-fly
- **Timing:** Only valid when `stableWeight` is valid (not NaN)
- **Purpose:** Alternative representation of baseline for display/reporting

### 5. **Global Conditions** (see above section)
- **Signal Quality Critical:** Always checked first across all states
- **Calibration:** Always checked for scale validity
- **Weight Invalid:** Always checked for sensor range validity
- **Weight Absent:** Always checked to detect missing kegs
- **Priority:** Global conditions override state-specific logic
- **Reference:** See "Global Conditions" section for exact thresholds and priority order

### 6. **Event Firing - Always on State Transitions**
**CRITICAL:** Every state transition MUST fire an event. This maintains a clear, unambiguous state history for logging, UI updates, and external integrations. No silent transitions.

**State Entry Events:**
- SETTLING_STARTED: Fired when entering SETTLING state (initial baseline or re-stabilization)
- STABLE_LEVEL: Fired when entering STABLE state
- POURING: Fired when entering POURING state
- KEG_REMOVED: Fired when entering KEG_ABSENT
- KEG_REPLACED: Fired when entering REPLACING_KEG
- INVALID_WEIGHT: Fired when entering INVALID_WEIGHT state
- LOAD_CELL_ERROR: Fired when entering LOAD_CELL_ERROR state
- CALIBRATION_NEEDED: Fired when entering CALIBRATION_NEEDED state
- LOAD_CELL_RECOVERED: Fired when exiting LOAD_CELL_ERROR state (recovery)
- SENSOR_RECOVERED: Fired when exiting INVALID_WEIGHT state
- POUR_COMPLETED: Fired when exiting POURING state

**State-Specific Events:**
- WEIGHT_CHANGE_DETECTED: Fired when large weight increase (>0.4kg) detected in STABLE (awaiting settlement confirmation)
- KEG_ABSENT_TIMEOUT: Fired when KEG_ABSENT state exceeds 30 seconds (reminder for user)

---

## Event Specifications

All events follow the CSV logging format defined in [EVENT_LOGGING_FORMAT.md](EVENT_LOGGING_FORMAT.md). This section specifies which parameters should be populated for each event type.

### 1. SYSTEM_STARTUP
**When**: System initialization or reboot. Use this to identify session boundaries.

**CSV Parameters**:
- **Timestamp**: Event time
- **Scale**: Scale/unit number (1-4)
- **EventType**: `SYSTEM_STARTUP`

**Example**:
```
1,2026-01-18 10:00:00,1,SYSTEM_STARTUP,0,0,0,0,0,0,0,0,0,0,,0,0,0
```

---

### 2. SETTLING_STARTED
**When**: Transition from IDLE → SETTLING (initial stabilization) OR from STABLE/POURING → SETTLING (re-stabilization)

**CSV Parameters**:
- **Timestamp**: Event time
- **Scale**: Scale/unit number (1-4)
- **EventType**: `SETTLING_STARTED`
- **DurationMs**: 0 (event just started)
- **CurrWeight**: Current weight when settling begins

**Example**:
```
1,2026-01-18 10:00:05,1,SETTLING_STARTED,0,0,0,0,0,0,0,0,0,23.5,,0,0,0
```

---

### 3. STABLE_LEVEL
**When**: Transition from SETTLING → STABLE (baseline confirmed and locked)

**CSV Parameters**:
- **Timestamp**: Event time
- **Scale**: Scale/unit number (1-4)
- **EventType**: `STABLE_LEVEL`
- **StableWeight**: New baseline weight (just locked)
- **StableVolume**: Calculated volume at this baseline
- **DurationMs**: Time spent in SETTLING state before stabilization
- **Variance**: Final variance at stabilization time

**Example**:
```
1,2026-01-18 10:00:10,1,STABLE_LEVEL,23.5,70.5,0,0,0,0,5000,0,0,0,,0,0.15,0
```

---

### 4. POURING
**When**: Transition from STABLE → POURING (slope exceeds threshold)

**CSV Parameters**:
- **Timestamp**: Event time
- **Scale**: Scale/unit number (1-4)
- **EventType**: `POURING`
- **PrePourWeight**: Weight before pour (stableWeight at moment of detection)
- **AvgSlope**: Initial slope/rate that triggered pour detection
- **CurrWeight**: Current weight when pour detected

**Example**:
```
1,2026-01-18 10:03:15,1,POURING,0,23.5,23.5,0,0,0,0,-0.245,0,0,,0,0,0
```

---

### 5. WEIGHT_CHANGE_DETECTED
**When**: Transition from STABLE → SETTLING (large weight increase > 0.4kg detected)

**CSV Parameters**:
- **Timestamp**: Event time
- **Scale**: Scale/unit number (1-4)
- **EventType**: `WEIGHT_CHANGE_DETECTED`
- **PrevWeight**: Previous stable weight (baseline before increase)
- **CurrWeight**: Current weight that triggered detection
- **Variance**: Current variance (may be high during transient)

**Example**:
```
1,2026-01-18 10:15:20,1,WEIGHT_CHANGE_DETECTED,0,0,0,0,0,0,0,0,23.2,23.7,,0,0.45,0
```

---

### 6. POUR_COMPLETED
**When**: Transition from POURING → SETTLING (any exit condition: normal completion, timeout, or impossible rate)

**CSV Parameters**:
- **Timestamp**: Event time
- **Scale**: Scale/unit number (1-4)
- **EventType**: `POUR_COMPLETED`
- **PrePourWeight**: Weight at pour start (stableWeight before pour)
- **PostPourWeight**: Weight when pour ended (current weight)
- **PourWeight**: Weight difference (PrePourWeight - PostPourWeight)
- **PourVolume**: Volume equivalent of weight difference
- **DurationMs**: Time in POURING state
- **AvgSlope**: Average pour rate during pour

**Example**:
```
1,2026-01-18 10:03:25,1,POUR_COMPLETED,23.5,23.1,0.4,0.12,10000,-0.040,0,0,,0,0,0
```

---

### 7. KEG_REMOVED
**When**: Transition from STABLE/other → KEG_ABSENT (weight drops below kegWeight * 0.9)

**CSV Parameters**:
- **Timestamp**: Event time
- **Scale**: Scale/unit number (1-4)
- **EventType**: `KEG_REMOVED`
- **PrevWeight**: Weight before removal (last known keg weight)
- **CurrWeight**: Current weight (now below minimum)

**Example**:
```
1,2026-01-18 10:15:30,1,KEG_REMOVED,0,0,0,0,0,0,0,0,23.5,0.5,,0,0,0
```

---

### 8. KEG_REPLACED
**When**: Transition from KEG_ABSENT → REPLACING_KEG (weight reappears >= kegWeight * 0.9)

**CSV Parameters**:
- **Timestamp**: Event time
- **Scale**: Scale/unit number (1-4)
- **EventType**: `KEG_REPLACED`
- **PrevWeight**: Weight when absent (0 or last low reading)
- **CurrWeight**: New keg weight detected

**Example**:
```
1,2026-01-18 10:15:35,1,KEG_REPLACED,0,0,0,0,0,0,0,0,0.5,20.2,,0,0,0
```

---

### 9. KEG_ABSENT_TIMEOUT
**When**: Periodic event in KEG_ABSENT state every time timeInState >= 30 seconds

**CSV Parameters**:
- **Timestamp**: Event time (when timeout period exceeded)
- **Scale**: Scale/unit number (1-4)
- **EventType**: `KEG_ABSENT_TIMEOUT`
- **DurationMs**: Time keg has been absent (timeInState)
- **CurrWeight**: Current weight (should be below minimum)

**Note**: This event may fire multiple times if keg remains absent (every 30 second cycle)

**Example**:
```
1,2026-01-18 10:16:00,1,KEG_ABSENT_TIMEOUT,0,0,0,0,0,0,30000,0,0,0.3,,0,0,0
```

---

### 10. INVALID_WEIGHT
**When**: Transition to INVALID_WEIGHT state (weight outside valid range: < -0.1kg OR > kegWeight + beerCapacity * 1.1)

**CSV Parameters**:
- **Timestamp**: Event time
- **Scale**: Scale/unit number (1-4)
- **EventType**: `INVALID_WEIGHT`
- **CurrWeight**: Weight that triggered error (out of bounds)

**Example**:
```
1,2026-01-18 10:25:48,2,INVALID_WEIGHT,0,0,0,0,0,0,0,0,0,52.3,,0,0,0
```

---

### 11. LOAD_CELL_ERROR
**When**: Transition to LOAD_CELL_ERROR state (signal quality critical)

**CSV Parameters**:
- **Timestamp**: Event time
- **Scale**: Scale/unit number (1-4)
- **EventType**: `LOAD_CELL_ERROR`
- **SignalErrorReason**: Error type (TIMEOUT, NAN_VALUE, STUCK_VALUE, CALIBRATION_INVALID, or empty for quality threshold)
- **SignalQuality**: Current signal quality percentage (< 20 or > 80 at recovery)
- **Variance**: Current variance reading
- **ConsecutiveErrors**: Count of consecutive errors

**Example**:
```
1,2026-01-18 10:22:15,2,LOAD_CELL_ERROR,0,0,0,0,0,0,0,0,0,0,TIMEOUT,15,0.85,6
```

---

### 12. LOAD_CELL_RECOVERED
**When**: Transition from LOAD_CELL_ERROR → previous state (signal quality recovered)

**CSV Parameters**:
- **Timestamp**: Event time
- **Scale**: Scale/unit number (1-4)
- **EventType**: `LOAD_CELL_RECOVERED`
- **SignalQuality**: Recovered signal quality percentage (> 80)
- **Variance**: Variance at recovery
- **ConsecutiveErrors**: Should be 0 at recovery

**Example**:
```
1,2026-01-18 10:22:25,2,LOAD_CELL_RECOVERED,0,0,0,0,0,0,0,0,0,0,,92,0.12,0
```

---

### 13. SENSOR_RECOVERED
**When**: Transition from INVALID_WEIGHT → SETTLING (weight returns to valid range)

**CSV Parameters**:
- **Timestamp**: Event time
- **Scale**: Scale/unit number (1-4)
- **EventType**: `SENSOR_RECOVERED`
- **CurrWeight**: Weight that returned to valid range

**Example**:
```
1,2026-01-18 10:26:05,2,SENSOR_RECOVERED,0,0,0,0,0,0,0,0,0,21.5,,0,0,0
```

---

### 14. CALIBRATION_NEEDED
**When**: Transition to CALIBRATION_NEEDED state (scale not calibrated: scaleFactor == 0.0)

**CSV Parameters**:
- **Timestamp**: Event time
- **Scale**: Scale/unit number (1-4)
- **EventType**: `CALIBRATION_NEEDED`

**Example**:
```
1,2026-01-18 10:26:00,3,CALIBRATION_NEEDED,0,0,0,0,0,0,0,0,0,0,,0,0,0
```

---

### 15. CALIBRATION_COMPLETE
**When**: Transition from CALIBRATION_NEEDED → IDLE (calibration set successfully)

**CSV Parameters**:
- **Timestamp**: Event time
- **Scale**: Scale/unit number (1-4)
- **EventType**: `CALIBRATION_COMPLETE`

**Example**:
```
1,2026-01-18 10:26:10,3,CALIBRATION_COMPLETE,0,0,0,0,0,0,0,0,0,0,,0,0,0
```

---

### 16. DISABLED
**When**: Transition to DISABLED state at startup when ADC hardware is not found.

**CSV Parameters**:
- **Timestamp**: Event time
- **Scale**: Scale/unit number (1-4)
- **EventType**: `DISABLED`

**Example**:
```
1,2026-01-18 10:00:01,4,DISABLED,0,0,0,0,0,0,0,0,0,0,,0,0,0
```

---

## Scenario Examples

### Scenario A: Normal Pour
```
STABLE (weight=7.0kg, stableWeight=7.0kg)
  → slope becomes negative
  → POURING (weight=6.5kg, stableWeight=7.0kg[unchanged])
  → slope becomes positive
  → STABLE (weight=6.4kg, stableWeight=6.4kg[NOW set])
```

### Scenario B: Weight Spike (Transient)
```
STABLE (weight=7.0kg, stableWeight=7.0kg)
  → weight spikes to 7.5kg (> 0.4kg increase)
  → SETTLING (weight=7.5kg, stableWeight=7.0kg[unchanged])
  → weight settles back at 7.0kg
  → STABLE (weight=7.0kg, stableWeight=7.0kg[confirmed])
```

### Scenario C: Keg Replacement
```
STABLE (weight=7.0kg, stableWeight=7.0kg)
  → weight drops to 0kg
  → KEG_ABSENT (weight=0kg, stableWeight=7.0kg)
  → new weight appears at 8.0kg
  → REPLACING_KEG (weight=8.0kg, stableWeight=8.0kg)
  → settles at 8.0kg
  → STABLE (weight=8.0kg, stableWeight=8.0kg[set])
```

