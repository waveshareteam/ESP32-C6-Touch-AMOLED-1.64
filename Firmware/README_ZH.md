# 出厂固件

[English](README.md) | [简体中文](README_ZH.md)

本目录保存 ESP32-C6-Touch-AMOLED-1.64 开发板的实测出厂镜像。

| 文件 | 用途 |
| --- | --- |
| [ESP32-C6-Touch-AMOLED-1.64_FactoryOnly_20260317.bin](ESP32-C6-Touch-AMOLED-1.64_FactoryOnly_20260317.bin) | ESP32-C6-Touch-AMOLED-1.64 出厂恢复固件，日期为 2026-03-17，从 `0x0` 地址开始烧录。 |

烧录前：

- 备份需要保留的设备专用数据。
- 确认开发板型号和硬件版本。
- 烧录该合并镜像前先整片擦除目标设备。
- 按产品文档中的实测流程操作，不要与其他项目的文件混用。

SHA-256：`1E200A79A883FC22F48C44F540B479F54E1F732019BC866C9C49F6B09734BCA6`

该文件是用于恢复出厂功能的单个合并 BIN。
