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
wokwi/          diagram.json (pin 12/13/14/27) + diagram-suite-pins.json (18/19/4/5)
                + wokwi.toml (ชี้ path ของ firmware .bin ที่ build ได้)
test/host/      runner เทสต์บน PC + ESP-IDF stub (เขียนเอง ไม่ต้องมี ESP-IDF)
test/test_clock_core/  โปรเจกต์ ESP-IDF สำหรับรัน Unity test บนบอร์ด
docs/REPORT.md  รายงานฉบับเต็ม
```

## การ Build และรัน

1. เปิดโปรเจกต์ด้วย VS Code + ESP-IDF Extension (v6)

2. Build firmware: `idf.py build` (หรือ `idf.py set-target esp32` ครั้งแรก)

3. Flash + monitor: `idf.py -p /dev/ttyUSB0 flash monitor`

4. รันจำลอง: กด **Start Simulation** ของ Wokwi โดยวาง `wokwi/diagram.json` ลงแท็บ `diagram.json`

5. (ทางเลือก) เทสต์บนบอร์ด: `cd test/test_clock_core && idf.py build flash monitor`

6. (ทางเลือก) เทสต์บน PC: `bash test/host/run_host_tests.sh` — ไม่ต้องมี ESP-IDF

## ปุ่มควบคุม

หน้าจอมี 4 โหมด: **CLOCK → ALARM → STOPWATCH → SET TIME** (หมุนวงด้วยปุ่ม MODE)

| ปุ่ม | GPIO | กดครั้งเดียว | กดค้าง 1 วินาที |
| --- | --- | --- | --- |
| MODE | 12 | เปลี่ยนหน้า | สั่ง sync NTP ใหม่ทันที |
| INC | 13 | ในหน้า ALARM/SET TIME = +1 ที่ฟิลด์ที่เลือก (ชั่วโมง/นาที) · ในหน้า CLOCK = sync NTP | +5 |
| SET | 14 | ALARM = บันทึก + เปิด/ปิดปลุก · SET TIME = เขียนเวลาลง RTC · STOPWATCH = เริ่ม/หยุด | เลื่อน cursor ระหว่างชั่วโมง/นาที |
| LAP | 27 | STOPWATCH = บันทึก lap (ถ้าหยุดอยู่ = รีเซ็ต) · อื่น ๆ = ปิดเสียงปลุก | เหมือนกดครั้งเดียว |

คีย์บอร์ดใน Wokwi: **M / S / A / L** ตรงกับปุ่มด้านบน

## ผลการทดสอบ

- **Host unit test:** ผ่านหมด 16 เคส / 158 assertions (`bash test/host/run_host_tests.sh`) ครอบคลุม BCD conversion, framebuffer 1024 B, debounce 50 ms, long press, RTC fallback, NTP offset +7, alarm comparator, lap split
- **Type-check:** `main/main.cpp` + `core/*.cpp` + `drivers/*.cpp` + `unit_tests.cpp` ผ่านหมดด้วย `-Wall -Wextra` **ไม่มี warning** (`bash test/host/typecheck_target.sh`)
- **Unity บน target:** 22 เคส (10 ชื่อแบบ Suite + 12 ชื่อแบบ Dashboard) ลงทะเบียนครบใน `test/test_clock_core/main/unit_tests.cpp`
- **Wokwi:** `diagram.json` ทั้งสองเวอร์ชันผ่านการตรวจ syntax และใช้ชื่อ part มาตรฐานของ Wokwi

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
| ปุ่มใน `diagram.json` ต่อสายไม่ได้ | ขา pushbutton ของ Wokwi คือ `1.1` / `1.2` (ไม่ใช่ `1.A`) และ part ต้องเป็น `wokwi-ds1307`, `board-esp32-devkit-c-v4` |
| ไม่มี ESP-IDF/บอร์ดในเครื่องพัฒนา | เขียน ESP-IDF stub เองที่ `test/host/stubs/` (อ้างอิง header จริงของ v6.0) + ใช้ `python -m ziglang c++` เป็น host compiler |

## หมายเหตุ

- โค้ดยังไม่ได้ถูก flash ลงฮาร์ดแวร์จริง (เครื่องพัฒนาไม่มี ESP-IDF และไม่มีบอร์ด) — การตรวจสอบทำได้ 2 ทางคือ host unit test และ type-check ด้วย stub ดังกล่าว
- ค่าเริ่มต้นใช้ pin ปุ่มของ Dashboard (12/13/14/27); ชุด pin ของ Suite (18/19/4/5) อยู่ใน `wokwi/diagram-suite-pins.json`
- ถ้าต้องการ sync NTP จริงใน Wokwi ให้เปลี่ยน SSID เป็น `Wokwi-GUEST` (รหัสว่าง) — ถ้าไม่ได้ระบบจะ fallback ไปใช้เวลาจาก RTC อัตโนมัติ
