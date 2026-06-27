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

export function weightKgToLbs(w) {
  return w * 2.2046226218
}

export function weightLbsToKg(w) {
  return w / 2.2046226218
}

export function volumeCLtoUSOZ(cl) {
  // centiliter to US fluid ounces
  // 1 cl = 0.338140225 US fl oz
  return cl * 0.338140225
}

export function volumeCLtoUKOZ(cl) {
  // centiliter to UK (imperial) fluid ounces
  // 1 cl = 0.351195720 UK fl oz
  return cl * 0.35119572
}
