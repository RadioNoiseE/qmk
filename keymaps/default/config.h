/* Copyright 2025 Jing Huang
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

// safe bet for atmega32u2
#define DYNAMIC_MACRO_NO_NESTING
#define DYNAMIC_MACRO_SIZE 32

// force nkey rollover
#define FORCE_NKRO

// disable tapping features and one-shot modifiers
#define NO_ACTION_TAPPING
#define NO_ACTION_ONESHOT
