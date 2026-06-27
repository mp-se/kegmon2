import { describe, it, expect } from 'vitest'
import { weightKgToLbs, weightLbsToKg, volumeCLtoUSOZ, volumeCLtoUKOZ } from '../utils'

describe('utils', () => {
  // --- weightKgToLbs ---
  it('weightKgToLbs converts 1 kg to ~2.2046 lbs', () => {
    expect(weightKgToLbs(1)).toBeCloseTo(2.2046, 3)
  })

  it('weightKgToLbs converts 0 kg to 0 lbs', () => {
    expect(weightKgToLbs(0)).toBe(0)
  })

  it('weightKgToLbs converts 10 kg correctly', () => {
    expect(weightKgToLbs(10)).toBeCloseTo(22.046, 2)
  })

  it('weightKgToLbs handles negative values', () => {
    expect(weightKgToLbs(-1)).toBeCloseTo(-2.2046, 3)
  })

  // --- weightLbsToKg ---
  it('weightLbsToKg converts 1 lbs to ~0.4536 kg', () => {
    expect(weightLbsToKg(1)).toBeCloseTo(0.4536, 3)
  })

  it('weightLbsToKg converts 0 lbs to 0 kg', () => {
    expect(weightLbsToKg(0)).toBe(0)
  })

  it('weightLbsToKg is inverse of weightKgToLbs', () => {
    expect(weightLbsToKg(weightKgToLbs(5))).toBeCloseTo(5, 5)
  })

  // --- volumeCLtoUSOZ ---
  it('volumeCLtoUSOZ converts 100 cl to ~33.8 US fl oz', () => {
    expect(volumeCLtoUSOZ(100)).toBeCloseTo(33.814, 2)
  })

  it('volumeCLtoUSOZ converts 0 cl to 0', () => {
    expect(volumeCLtoUSOZ(0)).toBe(0)
  })

  it('volumeCLtoUSOZ converts 33 cl (a standard can) to ~11.16 oz', () => {
    expect(volumeCLtoUSOZ(33)).toBeCloseTo(11.16, 1)
  })

  // --- volumeCLtoUKOZ ---
  it('volumeCLtoUKOZ converts 100 cl to ~35.12 UK fl oz', () => {
    expect(volumeCLtoUKOZ(100)).toBeCloseTo(35.12, 1)
  })

  it('volumeCLtoUKOZ converts 0 cl to 0', () => {
    expect(volumeCLtoUKOZ(0)).toBe(0)
  })

  it('volumeCLtoUKOZ result is larger than US oz for same volume', () => {
    expect(volumeCLtoUKOZ(100)).toBeGreaterThan(volumeCLtoUSOZ(100))
  })
})
