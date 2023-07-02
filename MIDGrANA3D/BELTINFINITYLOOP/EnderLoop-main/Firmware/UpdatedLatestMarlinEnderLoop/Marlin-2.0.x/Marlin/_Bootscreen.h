/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
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
#pragma once

#define CONFIG_EXAMPLES_DIR "Creality/Ender-3/CrealityV427"

/**
 * Custom Boot Screen bitmap
 *
 * Place this file in the root with your configuration files
 * and enable SHOW_CUSTOM_BOOTSCREEN in Configuration.h.
 *
 * Use the Marlin Bitmap Converter to make your own:
 * https://marlinfw.org/tools/u8glib/converter.html
 */

#define CUSTOM_BOOTSCREEN_TIMEOUT 1000
#define CUSTOM_BOOTSCREEN_BMPWIDTH 128
#define CUSTOM_BOOTSCREEN_BMPHEIGHT 64
#define CUSTOM_BOOTSCREEN_INVERTED

const unsigned char custom_start_bmp[1024] PROGMEM = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x01, 0xF8, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x03, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x03, 0xF8, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x03, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x07, 0xF0, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x07, 0xF0, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x0F, 0xE0, 0x60, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x0F, 0xE0, 0x40, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x0F, 0xC0, 0x40, 0x7E, 0x3F, 0x00, 0x06, 0x0F, 0x80, 0x0E, 0x18, 0x03, 0xF0, 0x70,
0x00, 0x00, 0x1F, 0xC0, 0x80, 0xFC, 0x3F, 0x80, 0x1C, 0x1F, 0x80, 0x3C, 0x1C, 0x07, 0xE1, 0xE0,
0x00, 0x00, 0x1F, 0x80, 0x00, 0xFC, 0x3F, 0x00, 0x78, 0x1F, 0x00, 0xF8, 0x3E, 0x07, 0xE3, 0xE0,
0x00, 0x00, 0x3F, 0x80, 0x01, 0xF8, 0x3F, 0x00, 0xF8, 0x3F, 0x01, 0xF0, 0x3E, 0x0F, 0xC7, 0xC0,
0x00, 0x00, 0x3F, 0x00, 0x01, 0xF8, 0x3F, 0x01, 0xF0, 0x3F, 0x03, 0xF0, 0x3C, 0x0F, 0xC3, 0xC0,
0x00, 0x00, 0x7F, 0xF8, 0x03, 0xF0, 0x7E, 0x03, 0xE0, 0x7E, 0x07, 0xE0, 0x7C, 0x1F, 0x80, 0x00,
0x00, 0x00, 0x7F, 0x80, 0x03, 0xF0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x78, 0x1F, 0x80, 0x00,
0x00, 0x00, 0xFE, 0x00, 0x07, 0xE0, 0xFC, 0x07, 0xE0, 0xFC, 0x0F, 0xE0, 0xE0, 0x3F, 0x00, 0x00,
0x00, 0x00, 0xFC, 0x00, 0x07, 0xE0, 0xFC, 0x0F, 0xC0, 0xFC, 0x0F, 0xC1, 0x80, 0x3F, 0x00, 0x00,
0x00, 0x01, 0xFC, 0x04, 0x0F, 0xC1, 0xF8, 0x0F, 0xC1, 0xF8, 0x0F, 0xC0, 0x00, 0x7E, 0x00, 0x00,
0x00, 0x01, 0xF8, 0x0C, 0x0F, 0xC1, 0xF8, 0x0F, 0xC1, 0xF8, 0x0F, 0xE0, 0x00, 0x7E, 0x00, 0x00,
0x00, 0x03, 0xF8, 0x18, 0x1F, 0xC3, 0xF8, 0x0F, 0xC3, 0xF0, 0x0F, 0xE0, 0x00, 0xFE, 0x00, 0x00,
0x00, 0x03, 0xF8, 0x30, 0x1F, 0x83, 0xF8, 0x0F, 0xC3, 0xF0, 0x0F, 0xF8, 0x00, 0xFC, 0x00, 0x00,
0x00, 0x07, 0xF0, 0xF0, 0x3F, 0x87, 0xF8, 0x03, 0xC7, 0xE0, 0x03, 0xFF, 0x01, 0xFC, 0x00, 0x00,
0x00, 0x0F, 0xC1, 0xC0, 0x3E, 0x07, 0xE0, 0x00, 0x07, 0xC0, 0x00, 0xF0, 0x01, 0xF0, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x01, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x03, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x03, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x07, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x07, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x07, 0xF0, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x0F, 0xE0, 0x00, 0x0E, 0x1C, 0x00, 0x0E, 0x1C, 0x00, 0x7E, 0x3C, 0x00, 0x00,
0x00, 0x00, 0x00, 0x0F, 0xC0, 0x00, 0x3C, 0x1E, 0x00, 0x3C, 0x1E, 0x00, 0xFE, 0x3F, 0x00, 0x00,
0x00, 0x00, 0x00, 0x1F, 0xC0, 0x00, 0xFC, 0x1F, 0x00, 0x78, 0x3F, 0x00, 0xFC, 0x3F, 0x00, 0x00,
0x00, 0x00, 0x00, 0x1F, 0x80, 0x01, 0xF8, 0x3F, 0x01, 0xF8, 0x3F, 0x00, 0xF8, 0x3F, 0x00, 0x00,
0x00, 0x00, 0x00, 0x3F, 0x80, 0x03, 0xF0, 0x3F, 0x01, 0xF0, 0x3F, 0x01, 0xF8, 0x3F, 0x00, 0x00,
0x00, 0x00, 0x00, 0x3F, 0x00, 0x03, 0xF0, 0x3F, 0x03, 0xF0, 0x3F, 0x01, 0xF0, 0x3F, 0x00, 0x00,
0x00, 0x00, 0x00, 0x7F, 0x00, 0x07, 0xE0, 0x7F, 0x07, 0xE0, 0x7E, 0x03, 0xF0, 0x3F, 0x00, 0x00,
0x00, 0x00, 0x00, 0x7E, 0x01, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x03, 0xE0, 0x7E, 0x00, 0x00,
0x00, 0x00, 0x00, 0xFE, 0x02, 0x0F, 0xC0, 0x7E, 0x0F, 0xC0, 0xFC, 0x07, 0xE0, 0x7C, 0x00, 0x00,
0x00, 0x00, 0x00, 0xFE, 0x06, 0x0F, 0xC0, 0xFC, 0x0F, 0xC0, 0xFC, 0x07, 0xE0, 0xFC, 0x00, 0x00,
0x00, 0x00, 0x01, 0xFC, 0x0C, 0x0F, 0xC1, 0xF8, 0x0F, 0xC1, 0xF8, 0x0F, 0xC0, 0xF8, 0x00, 0x00,
0x00, 0x00, 0x01, 0xFC, 0x1C, 0x0F, 0xC1, 0xF0, 0x0F, 0xC1, 0xF0, 0x0F, 0xC1, 0xF0, 0x00, 0x00,
0x00, 0x00, 0x03, 0xF8, 0x38, 0x07, 0x83, 0xC0, 0x07, 0x83, 0xC0, 0x1F, 0x83, 0xC0, 0x00, 0x00,
0x00, 0x00, 0x07, 0xF8, 0x78, 0x03, 0x87, 0x00, 0x03, 0x87, 0x00, 0x1F, 0x87, 0x00, 0x00, 0x00,
0x00, 0x00, 0x07, 0xC1, 0xE0, 0x00, 0x18, 0x00, 0x00, 0x08, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xF8, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};