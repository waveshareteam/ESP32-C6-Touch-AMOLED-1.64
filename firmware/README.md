# Factory Firmware

[中文](./README_ZH.md)

| File | Purpose |
| --- | --- |
| [ESP32-C6-Touch-AMOLED-1.64_FactoryOnly_20260317.bin](ESP32-C6-Touch-AMOLED-1.64_FactoryOnly_20260317.bin) | Factory recovery firmware dated 2026-03-17. Flash from `0x0`. |

Before flashing:

- Back up device-specific data that must be preserved.
- Confirm the board model and hardware revision.
- Follow the offset and flashing procedure in the product documentation for this image. Do not infer the factory image from a source build.
- Do not mix this image with a bootloader or partition file built for another project.

This image was verified on the corresponding product with Flash Download Tool. SHA-256: `1E200A79A883FC22F48C44F540B479F54E1F732019BC866C9C49F6B09734BCA6`.

See the [firmware guide](https://github.com/waveshareteam/esp32-repo-template/blob/main/docs/spec.md) for the difference between factory firmware and source-build firmware.

## Update the Factory Image

Replacing a factory binary is a development operation. Record its source, expected hardware revision, flashing procedure, checksum, and user-visible version in the Pull Request. Never overwrite an existing image with an unrelated local build.
