# Factory Firmware

[English](README.md) | [简体中文](README_ZH.md)

This directory stores the tested factory image for the ESP32-C6-Touch-AMOLED-1.64 development board.

| File | Purpose |
| --- | --- |
| [ESP32-C6-Touch-AMOLED-1.64_FactoryOnly_20260317.bin](ESP32-C6-Touch-AMOLED-1.64_FactoryOnly_20260317.bin) | Factory recovery firmware for the ESP32-C6-Touch-AMOLED-1.64 board, dated 2026-03-17. Flash from `0x0`. |

Before flashing:

- Back up any device-specific data that must be preserved.
- Confirm the board model and hardware revision.
- Erase the target device before writing this merged image.
- Follow the verified flashing procedure in the product documentation and do not combine this image with files from another project.

SHA-256: `1E200A79A883FC22F48C44F540B479F54E1F732019BC866C9C49F6B09734BCA6`

This file is a single merged BIN for factory recovery. The checksum and hardware applicability should be recorded in the pull request when the image is updated.

## Updating the Factory Image

When replacing the factory image, verify the new BIN on the target hardware, preserve older dates, and update both language versions of this README.
