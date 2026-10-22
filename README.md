# ESP32 Digital Clock (Modular OOP)

นาฬิกาดิจิทัลบน **ESP32 DevKit C V4** ด้วย **ESP-IDF v6 + FreeRTOS** เขียนแบบ **OOP แยกโมดูล** (Bottom-Up) รันจำลองบน **Wokwi** ครบทุกฟังก์ชัน (นาฬิกา/ปฏิทิน, ตั้งปลุก, จับเวลา, ตั้งเวลาเองผ่าน NTP)

> รายงานฉบับเต็ม (สเปก → สถาปัตยกรรม → ทุก spiral step → ตาราง task/pin/test) อยู่ที่ [`docs/REPORT.md`](docs/REPORT.md)

## ทำอะไรไปบ้าง

- แยก 2 ชั้น: `main/core/` = domain logic ล้วน (C++17, ไม่มี ESP-IDF, เทสต์บน PC ได้) และ `main/drivers/` = adapter ฝั่ง ESP-IDF v6
- ใช้ 4 FreeRTOS task (Display P3 / Button P5 / Alarm P4 / NTP P1) + I2C mutex กันข้อมูลชนกัน
- UI 4 หน้า: **CLOCK / ALARM / STOPWATCH / SET TIME** (สลับด้วยปุ่ม MODE)
- RTC **DS1307** (0x68) + OLED **SSD1306** (0x3C, buffer 1024 B) บน I²C #0 @ 400 kHz
- ซิงก์เวลาจาก **NTP** (`pool.ntp.org`, TZ +7) แล้วเขียนกลับลง RTC อัตโนมัติ — ถ้าเน็ตล่มใช้เวลา RTC ต่อ
- ปุ่ม 4 ตัว (GPIO 12/13/14/27) ผ่าน ISR + **software debounce 50 ms** แยก **short/long press**
- ปลุกเสียงผ่าน **LEDC PWM** (piezo GPIO 25) และ **stopwatch จับ lap** ด้วย `esp_timer` ความละเอียด µs
- Unit test **22 เคส** (ครบทั้งชื่อแบบ Suite 10 ชื่อ และ Dashboard 12 ชื่อ) + Wokwi `diagram.json` ให้ paste ได้ทันที

## โครงสร้าง

```
main/core/      domain logic ล้วน (Bcd, DateTime, Debounce, TimeManager,
                Alarm/Stopwatch, UiStateMachine/Controller/Renderer)
main/drivers/   adapter ESP-IDF v6 (I2CBus, DS1307, SSD1306, Button(+Manager),
                Buzzer, NTPManager, AlarmManager, Stopwatch, SystemClock)
main/main.cpp   ต่อทุกอย่างเข้าด้วยกัน (object graph + 4 task)
main/ClockConfig.hpp  pin / credential / task parameter ทั้งหมด (จุดเดียว)
diagram.json         ไดอะแกรม Wokwi (pin 12/13/14/27) — อยู่รากโปรเจกต์ตามที่ Wokwi ต้องการ
wokwi.toml           ชี้ firmware ที่ build ได้ให้ Wokwi (ESP-IDF → build/flasher_args.json)
wokwi/               diagram-suite-pins.json = ชุด pin แบบ Suite (18/19/4/5)
test/host/      runner เทสต์บน PC + ESP-IDF stub (เขียนเอง ไม่ต้องมี ESP-IDF)
test/test_clock_core/  โปรเจกต์ ESP-IDF สำหรับรัน Unity test บนบอร์ด
docs/REPORT.md  รายงานฉบับเต็ม
```

## การ Build และรัน

1. เปิดโปรเจกต์ด้วย VS Code + ESP-IDF Extension (v6)

2. Build firmware: `idf.py build` (หรือ `idf.py set-target esp32` ครั้งแรก)

3. Flash + monitor: `idf.py -p /dev/ttyUSB0 flash monitor`

4. รันจำลอง: `idf.py wokwi` (ESP-IDF ≥ 6.0) หรือกด **Wokwi: Start Simulator** ใน VS Code — ไฟล์ `diagram.json` + `wokwi.toml` อยู่ที่รากโปรเจกต์พร้อมแล้ว

5. (ทางเลือก) เทสต์บนบอร์ด: `cd test/test_clock_core && idf.py build flash monitor`

6. (ทางเลือก) เทสต์บน PC: `bash test/host/run_host_tests.sh` — ไม่ต้องมี ESP-IDF

## ทดสอบใน Wokwi

1. ติดตั้ง extension **Wokwi for VS Code** (`wokwi.wokwi-vscode`) + **ESP-IDF** (`espressif.esp-idf-extension`)
   แล้ว `F1` → *Wokwi: Request a new License* → กด *GET YOUR LICENSE* ในเบราว์เซอร์
2. `idf.py set-target esp32` (ครั้งแรก) แล้ว `idf.py build` — **ต้อง build ก่อนเสมอ** เพราะ `wokwi.toml`
   ชี้ไปที่ `build/flasher_args.json`
3. `F1` → **Wokwi: Start Simulator** → เปิด Serial Monitor (115200) แก้โค้ดแล้ว build ใหม่ simulator จะ reload เอง

ทางลัดแบบคำสั่งเดียว: `pip install idf-wokwi` + `WOKWI_CLI_TOKEN` แล้ว `idf.py wokwi` (ESP-IDF ≥ 6.0)
รายละเอียดทุกวิธี + checklist + เรื่อง Wi-Fi/NTP อยู่ที่ [`docs/REPORT.md` หัวข้อ 5.4](docs/REPORT.md)

## ปุ่มควบคุม

หน้าจอมี 4 โหมด: **CLOCK → ALARM → STOPWATCH → SET TIME** (หมุนวงด้วยปุ่ม MODE)

| ปุ่ม | GPIO | กดครั้งเดียว | กดค้าง 1 วินาที |
| --- | --- | --- | --- |
| MODE | 12 | เปลี่ยนหน้า | สั่ง sync NTP ใหม่ทันที |
| INC | 13 | ในหน้า ALARM/SET TIME = +1 ที่ฟิลด์ที่เลือก (ชั่วโมง/นาที) · ในหน้า CLOCK = sync NTP | +5 |
| SET | 14 | ALARM = บันทึก + เปิด/ปิดปลุก · SET TIME = เขียนเวลาลง RTC · STOPWATCH = เริ่ม/หยุด | เลื่อน cursor ระหว่างชั่วโมง/นาที |
| LAP | 27 | STOPWATCH = บันทึก lap (ถ้าหยุดอยู่ = รีเซ็ต) · อื่น ๆ = ปิดเสียงปลุก | เหมือนกดครั้งเดียว |

คีย์บอร์ดใน Wokwi: **M / I / S / L** ตรงกับปุ่มด้านบน (ผูกไว้ด้วย attribute `key` ใน `diagram.json`)

## ผลการทดสอบ

- **Host unit test:** ผ่านหมด 16 เคส / 158 assertions (`bash test/host/run_host_tests.sh`) ครอบคลุม BCD conversion, framebuffer 1024 B, debounce 50 ms, long press, RTC fallback, NTP offset +7, alarm comparator, lap split
- **Type-check:** `main/main.cpp` + `core/*.cpp` + `drivers/*.cpp` + `unit_tests.cpp` ผ่านหมดด้วย `-Wall -Wextra` **ไม่มี warning** (`bash test/host/typecheck_target.sh`)
- **Unity บน target:** 22 เคส (10 ชื่อแบบ Suite + 12 ชื่อแบบ Dashboard) ลงทะเบียนครบใน `test/test_clock_core/main/unit_tests.cpp`
- **Wokwi:** `diagram.json` ทั้งสองเวอร์ชันผ่านการตรวจครบ 5 ข้อ — part type ถูกต้อง, ไม่มีอุปกรณ์ทับกัน, ขาทุกเส้นมีอยู่จริงบน part, ทุกอุปกรณ์ถูกต่อสายครบ, ขา ESP32 ที่ใช้มีจริงบนบอร์ด
- **คีย์ลัด Wokwi:** ผูกปุ่มคีย์บอร์ดไว้แล้วใน `diagram.json` (M / I / S / L)

## ปัญหาที่เจอ และวิธีแก้

| ปัญหา | วิธีแก้ |
| --- | --- |
| `esp_netif_t` ใน IDF v6 เป็น **struct** ไม่ใช่ pointer typedef → ส่ง `esp_netif_t*` ไม่ผ่าน | เปลี่ยน signature เป็น `esp_netif_t*` ทุกฟังก์ชันของ esp_netif |
| `esp_netif_sntp_sync_wait()` ใช้ไม่ได้ | ต้องตั้ง `wait_for_sync = true` ใน `esp_sntp_config_t` และเรียก `esp_netif_sntp_init()` แค่ครั้งเดียว (sync ใหม่ใช้ `esp_netif_sntp_start()`) |
| ใส่ NTP server สำรองไม่ได้ | ค่า default ของ IDF คือ `CONFIG_LWIP_SNTP_MAX_SERVERS=1` → ตั้งเป็น 3 ใน `sdkconfig.defaults` |
| `i2c_master` / `ledc` / `gpio` หาไม่เจอตอน build | IDF v6 แยก `driver` ออกมาเป็น component → ระบุ `REQUIRES esp_driver_i2c esp_driver_gpio esp_driver_ledc` ใน `main/CMakeLists.txt` |
| frame buffer กลายเป็นขนาด 0 | `kBufferSize` ถูกประกาศเป็น `uint8_t` (1024 ถูกตัด) → เปลี่ยนเป็น `uint16_t` |
| `TimeManager::now()` compile ไม่ผ่าน | ฟังก์ชันเป็น `const` แต่ต้องอัปเดต cache → ประกาศ `m_lastKnown` เป็น `mutable` |
| ค่า NTP epoch เทียบผิดสัญญาณ | `kNtpToUnixEpochDelta` เป็น `uint64` → ต้อง cast เป็น `int64` ก่อนเทียบ |
| `struct Clock::timeval` compile ไม่ผ่าน | ใน `namespace Clock` การเขียน `struct timeval` โดยยังไม่มี `<sys/time.h>` จะ **ประกาศชนิดใหม่ `Clock::timeval`** ที่ชนกับ `esp_sntp_time_cb_t` → ใส่ `#include <sys/time.h>` ไว้บนสุดไฟล์และใช้ `::timeval` แบบ fully qualified + เพิ่ม `static_assert` เทียบกับ `esp_sntp_time_cb_t` |
| `esp_sntp_config_t` ไม่มีฟิลด์ `renew_servers_after_new_ip` | ฟิลด์นี้ใน ESP-IDF v6 สะกดด้วย **ตัวพิมพ์ใหญ่** คือ `renew_servers_after_new_IP` → แก้ใน `main/drivers/NTPManager.cpp` และปรับ stub ให้ตรงชื่อจริง |
| `ESP_LOGI(...)` error เรื่อง type conversion (`i2c_port_num_t` → `int`) | cast argument ทุกตัวให้เป็น `int`/`unsigned` ให้ชัดเจน (`static_cast<int>(Config::kI2cPort)`, `%u` + `static_cast<unsigned>`) และตัด dependency ของ `PRIu32` ออกทั้งโปรเจกต์ → ทำกับทุก log call ใน `main.cpp` และ `main/drivers/*` |
| `unknown type name 'i2c_port_num_t'` / `use of undeclared identifier 'I2C_NUM_0'` ใน `main/ClockConfig.hpp` | header ที่ใช้ประกาศค่า I²C ต้อง include driver ของมันเอง → เพิ่ม `#include "driver/i2c_master.h"` (ไฟล์นี้ include `driver/i2c_types.h` → `hal/i2c_types.h` ซึ่งประกาศ `i2c_port_num_t` และ `I2C_NUM_0`) รวมถึงต้องมี `REQUIRES esp_driver_i2c` ใน `main/CMakeLists.txt` |
| `use of undeclared identifier 'esp_timer_get_time'` ใน `main/drivers/EspTimerSource.hpp` | ไฟล์นี้เป็น header-only จึงต้อง include `esp_timer.h` เอง (`#include "esp_timer.h"`) |
| `'struct Clock::Config' has no member named 'kI2cPort'` | เป็น **อาการล้าน (cascade)** จาก 2 error ข้างบน — GCC พิมพ์ namespace เป็น "struct" ในข้อความ "has no member named" เสมอ → แก้ที่ต้นเหตุ (include) เท่านั้น error นี้จะหายเอง; เพิ่มการเปลี่ยน `namespace Config` เป็น `struct Config` (สมาชิกเป็น `static constexpr`) เพื่อให้ชื่อสมาชิกเป็นของจริงและ `Config::kXxx` ทุกจุดยังใช้ได้เหมือนเดิม |
| ปุ่ม/อุปกรณ์ใน `diagram.json` ต่อสายไม่ได้ | `diagram.json` ที่ให้มาในสเปกมีชื่อ part และชื่อขาผิด (`wokwi-esp32-devkit-v4`, `board-ds1307`, ขาปุ่ม `1.1`/`1.A`) → แก้เป็น `board-esp32-devkit-c-v4`, `wokwi-ds1307`, `board-ssd1306` และขาปุ่ม `1.l`/`2.l` (ชื่อขาจริงตามคู่มือ Wokwi) |
| ปุ่มทับตัวบอร์ด ESP32 ในหน้า Wokwi | บอร์ด DevKit C V4 กว้าง 105 × สูง 214 px แต่สเปกวางปุ่มที่ `top: 150` → ย้ายชุดปุ่มลงใต้บอร์ด (`top: 280`) และย้าย RTC/OLED ไปด้านขวา (`left: 280`) |
| Wokwi ต่อ Wi-Fi ไม่ได้ (SSID `iotexpo` ไม่มีในตัวจำลอง) | เพิ่ม SSID สำรอง `Wokwi-GUEST` (ไม่มีรหัส) ใน `main/ClockConfig.hpp` → ถ้าต่อหลักไม่สำเร็จจะสลับไปใช้อัตโนมัติ |
| ไม่มี ESP-IDF/บอร์ดในเครื่องพัฒนา | เขียน ESP-IDF stub เองที่ `test/host/stubs/` (อ้างอิง header จริงของ v6.0) + ใช้ `python -m ziglang c++` เป็น host compiler |
| กดปุ่มใน Wokwi แล้วไม่มีอะไรเกิดขึ้นเลย | 3 จุดที่แก้แล้ว: (1) log ของปุ่มใช้ `ESP_LOGD` ซึ่งถูกซ่อนที่ระดับ log เริ่มต้นของ ESP-IDF (INFO) → เปลี่ยนเป็น `ESP_LOGI`; (2) ตัวกรอง debounce **ทิ้ง** release edge ที่มาถึงภายใน 50 ms ของ press edge → การคลิกเมาส์ที่สั้นกว่า 50 ms (แบบที่กดใน Wokwi) จะหายไปทั้งคลิป → เปลี่ยนเป็น "เลื่อนการตัดสินใจ" (defer) แล้วค่อยยืนยันใน `update()`; (3) เพิ่มการ poll ระดับสัญญาณ GPIO เป็น fallback สำหรับกรณี edge หาย → ดูหัวข้อ 13 ใน `docs/REPORT.md` |

## หมายเหตุ

- โค้ดยังไม่ได้ถูก flash ลงฮาร์ดแวร์จริง (เครื่องพัฒนาไม่มี ESP-IDF และไม่มีบอร์ด) — การตรวจสอบทำได้ 2 ทางคือ host unit test และ type-check ด้วย stub ดังกล่าว
- ค่าเริ่มต้นใช้ pin ปุ่มของ Dashboard (12/13/14/27); ชุด pin ของ Suite (18/19/4/5) อยู่ใน `wokwi/diagram-suite-pins.json`
- ถ้าต้องการ sync NTP จริงใน Wokwi ฟิร์มแวร์จะต่อ `iotexpo` ก่อน แล้วสลับไปใช้ `Wokwi-GUEST` (รหัสว่าง) ให้อัตโนมัติ — ถ้าเน็ตล่มจริงระบบจะ fallback ไปใช้เวลาจาก RTC
