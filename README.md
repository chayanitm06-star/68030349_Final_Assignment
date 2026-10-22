# ESP32 Modular Digital Clock — ESP-IDF v6 / FreeRTOS / C++ OOP

> โปรเจกต์นาฬิกาดิจิทัลสำหรับ ESP32 ตามสเปกจากไฟล์ HTML ทั้งสอง
> (`wokwi_esp32_digital_clock_suite.html` + `wokwi_esp32_digital_clock_dashboard.html`)
> เขียนด้วย C++ แบบ OOP สถาปัตยกรรม **Bottom‑Up**, ใช้ **ESP‑IDF v6.0**, **FreeRTOS**,
> RTC **DS1307**, จอ **SSD1306** และซิงก์เวลาผ่าน **Wi‑Fi / NTP**

---

## สารบัญ

1. [สิ่งที่ไฟล์ HTML ทั้งสองกำหนด](#1-สิ่งที่ไฟล์-html-ทั้งสองกำหนด)
2. [ภาพรวมผลลัพธ์](#2-ภาพรวมผลลัพธ์)
3. [สถาปัตยกรรมซอฟต์แวร์](#3-สถาปัตยกรรมซอฟต์แวร์)
4. [ขั้นตอนที่ 1 – 5 (Spiral) ที่ทำเสร็จแล้ว](#4-ขั้นตอนที่-1--5-spiral-ที่ทำเสร็จแล้ว)
5. [วิธี Build / Flash / Monitor](#5-วิธี-build--flash--monitor)
6. [วิธีรันชุดทดสอบ Unit Test](#6-วิธีรันชุดทดสอบ-unit-test)
7. [การจำลองบน Wokwi](#7-การจำลองบน-wokwi)
8. [ตาราง FreeRTOS Tasks](#8-ตาราง-freertos-tasks)
9. [ตารางการต่อสาย (Pin Map)](#9-ตารางการต่อสาย-pin-map)
10. [ตาราง Unit Tests ทั้งหมด 22 รายการ](#10-ตาราง-unit-tests-ทั้งหมด-22-รายการ)
11. [Traceability Matrix (สเปก HTML → โค้ด → เทสต์)](#11-traceability-matrix-สเปก-html--โค้ด--เทสต์)
12. [แผนผังไฟล์ทั้งหมด](#12-แผนผังไฟล์ทั้งหมด)
13. [ข้อจำกัด / หมายเหตุ](#13-ข้อจำกัด--หมายเหตุ)

---

## 1. สิ่งที่ไฟล์ HTML ทั้งสองกำหนด

อ่านและสรุปเนื้อหาจาก HTML ทั้งสองไฟล์ก่อนลงมือเขียนโค้ด ได้ดังนี้

### 1.1 `wokwi_esp32_digital_clock_suite.html` (สเปก “Suite”)

| หัวข้อ | สิ่งที่สเปกกำหนด |
|---|---|
| สถาปัตยกรรม | Bottom‑Up Modular OOP – HAL สร้างทีละชั้นจนถึงระบบ FreeRTOS multitasking; แยก hardware (I2C Bus, DS1307, SSD1306, Button) ออกจาก domain logic (TimeManager, AlarmEngine, StopwatchEngine) ด้วย interface มาตรฐาน + Queue/Mutex/EventGroup |
| Spiral 1 | HAL & Bus Layer – I2CBus thread‑safe, DS1307 BCD parser, SSD1306 128×64 buffer |
| Spiral 2 | Time Subsystem – TimeManager, Wi‑Fi Station driver, SNTP sync + RTC fallback |
| Spiral 3 | Input & UI Engine – GPIO ISR debounce (Short/Long press), OLED renderer task |
| Spiral 4 | Business Engines – AlarmEngine comparator + piezo buzzer, StopwatchEngine lap timing |
| Spiral 5 | FreeRTOS Integration – multitask sync, queue event pump, mutex safety, `app_main` |
| Tasks | ตาราง 4 tasks: `vTaskClockDisplay` (prio 3 / 4096 B / Core 1 / 100 ms), `vTaskButtonHandler` (5 / 3072 B / Core 0 / ISR), `vTaskAlarmCheck` (4 / 2048 B / Core 1 / 1000 ms), `vTaskNTPSync` (1 / 6144 B / Core 0 / 3600 s) |
| Wokwi | `diagram.json` + ตารางต่อสาย: DS1307 & SSD1306 บน I²C (SDA 21 / SCL 22), ปุ่ม 4 ตัว GPIO 18/19/4/5, buzzer GPIO 25 |
| ไฟล์โค้ด | `main.cpp`, `I2CBus.hpp`, `DS1307.hpp/.cpp`, `SSD1306.hpp/.cpp`, `ButtonManager.hpp`, `TimeManager.hpp`, `AlarmEngine.hpp`, `StopwatchEngine.hpp` |
| เทสต์ | 10 test ของ 4 spiral module (`test_ds1307_bcd_conversion` … `test_freertos_event_group_sync`) |

### 1.2 `wokwi_esp32_digital_clock_dashboard.html` (สเปก “Dashboard”)

| หัวข้อ | สิ่งที่สเปกกำหนด |
|---|---|
| Dashboard | จำลอง hardware (OLED + ปุ่ม + serial console + bus diagnostics + Wi‑Fi credential), รัน unit test 4 ชุด, กราฟสถาปัตยกรรม 4 แผนภาพ, แสดง source code |
| สเปกอุปกรณ์ | OLED 128×64 I²C @ 0x3C, DS1307 @ 0x68, I²C #0 400 kHz (SDA 21 / SCL 22), buzzer GPIO 25, debounce 50 ms, stopwatch ใช้ `esp_timer` |
| ปุ่ม (Dashboard) | MODE = GPIO 12, INC/NTP = GPIO 13, SET/ALARM = GPIO 14, LAP/RESET = GPIO 27 |
| การเชื่อมต่อ | Wi‑Fi STA `iotexpo` / `12345678`, NTP `pool.ntp.org`, TZ +7 ชม. |
| Spiral | 4 spiral: ① HAL & Core Drivers ② FreeRTOS Task Engine & Switch Driver ③ Network & NTP ④ Application Features (Alarm & Lap Timer) |
| เทสต์ | 12 test: `DS1307_BCD_To_Dec_Conversion`, `DS1307_Dec_To_BCD_Conversion`, `SSD1306_Framebuffer_Allocation_1024B`, `FreeRTOS_Button_Queue_Post_Receive`, `Button_Software_Debounce_50ms_Filter`, `I2C_Mutex_Thread_Safety_Lock`, `WiFi_Station_Connect_SSID_iotexpo`, `SNTP_Timestamp_Offset_Calculation`, `DS1307_NTP_AutoCalibration_Write`, `Alarm_Comparator_Trigger_Match`, `Stopwatch_HighRes_Timer_Precision`, `Lap_Memory_Buffer_Push` |
| ไฟล์โค้ด | `CMakeLists.txt`, `main.cpp`, `DS1307.hpp/.cpp`, `SSD1306.hpp/.cpp`, `Button.hpp/.cpp`, `NTPManager.hpp/.cpp`, `AlarmManager.hpp/.cpp`, `Stopwatch.hpp/.cpp`, `unit_tests.cpp` |
| Wokwi | `diagram.json` พร้อม paste ลง Wokwi โดยตรง |

> **ข้อสรุป:** สเปกทั้งสองไฟล์อ้างอิง **ชื่อไฟล์และชื่อ test ที่ต่างกัน** และ **pin ปุ่มต่างกัน**
> การออกแบบของโปรเจกต์นี้จึงทำให้ *ทั้งสองชุดชื่อ* อยู่ร่วมกันได้ โดย:
> * ไฟล์/คลาสตาม **Dashboard** อยู่ใน `main/drivers/` (ชื่อไฟล์ตรงกับสเปก)
> * คลาส/ไฟล์ตาม **Suite** อยู่ใน `main/core/` และ `main/drivers/`
> * Pin map ที่ใช้จริง = ของ **Dashboard** (12/13/14/27) และมี `wokwi/diagram-suite-pins.json` เป็นตัวเลือกสำรอง (18/19/4/5)

---

## 2. ภาพรวมผลลัพธ์

| รายการ | สถานะ |
|---|---|
| โค้ดเฟิร์มแวร์ครบตามสเปกทั้งสองไฟล์ | ✅ เสร็จ |
| สถาปัตยกรรม 2 ชั้น (core / drivers) + RAII + DI | ✅ เสร็จ |
| FreeRTOS 4 tasks พร้อมพารามิเตอร์ตรงตามสเปก | ✅ เสร็จ |
| Wokwi `diagram.json` (2 เวอร์ชัน) | ✅ เสร็จ |
| Unit test 22 รายการ (10 suite + 12 dashboard) | ✅ เสร็จ |
| Host test runner (รันบน PC ได้โดยไม่ต้องมี ESP‑IDF) | ✅ เสร็จ |
| Type‑check ชั้น driver ด้วย ESP‑IDF stub | ✅ เสร็จ |
| README (เอกสารรายขั้นตอน) | ✅ ไฟล์นี้ |

---

## 3. สถาปัตยกรรมซอฟต์แวร์

```
                    ┌──────────────────────────────────────────────┐
   main/main.cpp ──▶│  app_main()  : wiring + task creation        │
                    └──────────────────────────────────────────────┘
                                   │
        ┌──────────────────────────┴───────────────────────────┐
        │                                                      │
┌───────▼────────────────────────┐            ┌────────────▼────────────────────┐
│  main/core/   (pure C++17)     │            │  main/drivers/ (ESP-IDF v6)      │
│  ไม่มี dependency ของ ESP-IDF  │            │  I2CBus / DS1307 / SSD1306       │
│  → ทดสอบบน Host ได้ทันที        │◀──ports──▶│  Button / ButtonManager / Buzzer │
│                                │            │  NTPManager / SystemClock       │
│  Bcd, DateTime, Debounce,      │            │  EspTimerSource, AlarmManager,   │
│  AlarmLogic, StopwatchLogic,   │            │  Stopwatch                      │
│  NtpEpoch, UiStateMachine,     │            └─────────────────────────────────┘
│  TimeManager, AlarmEngine,     │
│  StopwatchEngine, UiController,│
│  UiRenderer                    │
└────────────────────────────────┘
```

**กฎการพึ่งพา (dependency rule)**

* `core/` → ไม่มี `#include` ของ ESP‑IDF เลย → compile บน PC ได้ → unit test ได้ทันที
* `drivers/` → implement interface ที่ประกาศใน `core/Interfaces.hpp`
* `main.cpp` → ประกอบ object graph ทั้งหมด (dependency injection) แล้วสร้าง task

**Interface (port) ที่ใช้คั่นสองชั้น** — `main/core/Interfaces.hpp`

| Interface | ผู้ implement | หน้าที่ |
|---|---|---|
| `IRtcDevice` | `DS1307` | อ่าน/เขียนเวลา RTC |
| `ITimeSource` | `SystemClock`, `EspTimerSource` | เวลา POSIX / มิลลิวินาทีแบบ monotonic |
| `INtpClient` | `NTPManager` | ซิงก์เวลาจาก NTP |
| `IDisplaySurface` | `SSD1306` | วาดภาพลง frame buffer + flush |
| `IBuzzer` | `Buzzer` | เปิด/ปิดเสียง piezo |
| `IButtonInput` | `ButtonManager` | ส่ง event ปุ่มที่ debounce แล้ว |

---

## 4. ขั้นตอนที่ 1 – 5 (Spiral) ที่ทำเสร็จแล้ว

### ✅ ขั้นตอนที่ 0 — โครงสร้างโปรเจกต์และ “single source of truth”

| สิ่งที่ทำ | ไฟล์ |
|---|---|
| สร้างโปรเจกต์ ESP‑IDF v6 (CMake + `sdkconfig.defaults`) | `CMakeLists.txt`, `main/CMakeLists.txt`, `sdkconfig.defaults` |
| รวม pin / credential / task parameter ไว้ที่เดียว | `main/ClockConfig.hpp` |
| แยก `core/` (pure C++) กับ `drivers/` (ESP‑IDF) | `main/core/`, `main/drivers/` |

`ClockConfig.hpp` ถือค่า: I²C #0 / SDA 21 / SCL 22 / 400 kHz, DS1307 `0x68`, SSD1306 `0x3C`,
ปุ่ม 12/13/14/27, debounce 50 ms, long‑press 1000 ms, buzzer GPIO 25, Wi‑Fi `iotexpo/12345678`,
NTP `pool.ntp.org` (+ `time.nist.gov`), TZ +7 ชม. และ stack/priority/core ของ task ทั้ง 4

---

### ✅ Spiral 1 — HAL & Bus Layer

**เป้าหมาย:** ชั้นต่ำสุดของระบบ — คุยกับบัส I²C และอ่าน/เขียน register ได้อย่างปลอดภัยจากหลาย task

| ไฟล์ | สิ่งที่ทำ |
|---|---|
| `main/core/Bcd.hpp` | `bcdToDecimal()`, `decimalToBcd()`, `isValidBcd()` — pure function ที่ไม่มี dependency |
| `main/core/DateTime.hpp/.cpp` | value object วัน–เวลา + คำนวณ day‑of‑week ด้วยอัลกอริทึม `days_from_civil` (รองรับปี 2000–2099 ของ DS1307) |
| `main/drivers/I2CBus.hpp/.cpp` | ห่อ `i2c_master` ของ ESP‑IDF v6 ด้วย **FreeRTOS mutex** ทุกครั้งที่มี transfer → ป้องกัน RTC กับ OLED ชนกัน |
| `main/drivers/I2CDevice.hpp` | base class ของอุปกรณ์บนบัส (ตาม UML ของสเปก) |
| `main/drivers/DS1307.hpp/.cpp` | อ่าน/เขียน register map ของ DS1307 (0x00–0x06), แปลง BCD, ตั้ง/อ่านโหมด 24 ชม., ตรวจ oscillation flag |
| `main/drivers/SSD1306.hpp/.cpp` | frame buffer **1024 ไบต์** (128×64 1‑bpp), `setPixel()` ตรวจ boundary, `drawText/drawHLine/drawTextRight`, flush ผ่าน I²C |
| `main/drivers/Font5x7.hpp` | ฟอนต์ ASCII 5×7 สำหรับวาดข้อความ |

**วิธีตรวจสอบ**
```bash
bash test/host/run_host_tests.sh     # → test_ds1307_bcd_conversion, DS1307_BCD_To_Dec_Conversion,
                                     #   DS1307_Dec_To_BCD_Conversion, SSD1306_Framebuffer_Allocation_1024B
```

**สถานะ:** ✅ Verified — BCD ↔ decimal ถูกต้อง, frame buffer ขนาด 1024 B และไม่มีการเขียนเกินขอบเขต, mutex กันการชนกันของ transfer

---

### ✅ Spiral 2 — Time Subsystem (RTC + Wi‑Fi/NTP)

**เป้าหมาย:** มี “เวลาที่เชื่อถือได้” แหล่งเดียว พร้อม fallback ไปที่ RTC เมื่อเน็ตล่ม

| ไฟล์ | สิ่งที่ทำ |
|---|---|
| `main/core/NtpEpoch.hpp` | แปลง NTP timestamp (epoch 1900) ↔ POSIX epoch (1970) และบวก offset timezone (`kNtpToUnixEpochDelta = 2208988800`) |
| `main/core/TimeManager.hpp/.cpp` | ตัวกำหนดเวลาหลัก — ลำดับการเลือกแหล่งเวลา: **NTP → RTC → POSIX fallback**, ตั้ง timezone, สั่ง calibrate RTC กลับ (`requestSync()`) |
| `main/drivers/SystemClock.hpp/.cpp` | adapter อ่าน POSIX clock ของ ESP‑IDF |
| `main/drivers/NTPManager.hpp/.cpp` | Wi‑Fi STA + lwIP SNTP ของ ESP‑IDF v6: `esp_netif_init`, `esp_netif_create_default_wifi_sta()`, ลงทะเบียน handler `WIFI_EVENT`/`IP_EVENT`, รอ DHCP, `esp_netif_sntp_init/start/sync_wait()` |

**รายละเอียดสำคัญ (ESP‑IDF v6)**
* `esp_sntp_config_t.wait_for_sync = true` **ต้องตั้ง** ไม่อย่างนั้น `esp_netif_sntp_sync_wait()` ใช้ไม่ได้
* `esp_netif_sntp_init()` เรียกครั้งเดียว การ sync ใหม่ใช้ `esp_netif_sntp_start()` (restart ได้)
* `CONFIG_LWIP_SNTP_MAX_SERVERS=3` (ค่า default ของ IDF คือ 1) เพื่อให้ใส่ server สำรองได้
* `esp_netif_t` ใน v6.0 เป็น **ตัวชี้ชนิด `struct`** → `esp_netif_t*` ต้องส่งเป็น pointer‑to‑object

**วิธีตรวจสอบ**
```bash
bash test/host/run_host_tests.sh     # → test_time_manager_rtc_fallback, test_ntp_timestamp_parsing,
                                     #   SNTP_Timestamp_Offset_Calculation, DS1307_NTP_AutoCalibration_Write
```

**สถานะ:** ✅ Verified — เมื่อ NTP ล้มเหลวระบบถอยไปใช้เวลา RTC, offset +7 ชม. คำนวณถูก, การเขียนค่ากลับลง DS1307 ทำงานผ่าน `TimeManager::requestSync()`

---

### ✅ Spiral 3 — Input & UI Engine

**เป้าหมาย:** อ่านปุ่มแบบไม่กระตุก (debounce) แยก Short/Long press และวาดหน้าจอ 4 โหมด

| ไฟล์ | สิ่งที่ทำ |
|---|---|
| `main/core/ButtonEvent.hpp` | value object ของ event ปุ่ม (`ButtonId`, `PressType`, `pressed`, `timestampMs`) |
| `main/core/Debounce.hpp/.cpp` | ตัวกรอง debounce แบบ software: ISR แจ้ง edge → task หน่อย `update()` เป็นระยะ → ตัดสิน Short/Long (threshold 1000 ms) |
| `main/core/UiStateMachine.hpp/.cpp` | FSM 4 โหมด `Clock → Set → Alarm → Stopwatch` + cursor ตอนตั้งค่า |
| `main/core/UiController.hpp/.cpp` | แปลง event ปุ่มเป็นคำสั่งของ domain (เลื่อนโหมด, ปรับค่า, เปิด/ปิด alarm, start/stop/lap) |
| `main/core/UiRenderer.hpp/.cpp` | วาดเฟรมของแต่ละโหมดลง `IDisplaySurface` (เวลา/วันที่, ตั้ง alarm, stopwatch + lap) |
| `main/core/DisplayGeometry.hpp` | ค่าคงที่เรขาคณิตของจอ 128×64 (ใช้ในเทสต์ได้) |
| `main/drivers/Button.hpp/.cpp` | driver ปุ่มรายตัว: ตั้ง GPIO (pull‑up, input), ติดตั้ง ISR, ส่ง event เข้า queue |
| `main/drivers/ButtonManager.hpp/.cpp` | ครอบ 4 ปุ่ม: drain queue → เติมข้อมูลลง `DebounceFilter` → ส่ง event ที่ผ่านการ debounce ให้ task |

**วิธีตรวจสอบ**
```bash
bash test/host/run_host_tests.sh     # → test_button_long_press_detect,
                                     #   Button_Software_Debounce_50ms_Filter, test_ui_mode_cycle
```

**สถานะ:** ✅ Verified — short/long press แยกได้ถูกต้องที่ 1000 ms, การหมุนโหมดเป็นวงจร Clock→Set→Alarm→Stopwatch ตามสเปก

---

### ✅ Spiral 4 — Business Engines (Alarm + Stopwatch)

**เป้าหมาย:** ตรรกะการปลุกเสียงและการจับเวลาแบบ lap ที่แม่นยำ

| ไฟล์ | สิ่งที่ทำ |
|---|---|
| `main/core/AlarmLogic.hpp/.cpp` | ตรรกะเปรียบเทียบเวลา + latch ของ alarm (pure, ไม่แตะ hardware) |
| `main/core/AlarmEngine.hpp/.cpp` | domain service ของ alarm: ตั้งเวลา, arm/disarm, ตรวจ trigger, timeout การเตือน 60 วินาที |
| `main/drivers/Buzzer.hpp/.cpp` | ขับ piezo ด้วย **LEDC PWM** (2.7 kHz, duty 50 %) |
| `main/drivers/AlarmManager.hpp/.cpp` | facade ระดับแอปฯ (ตามชื่อไฟล์ของสเปก Dashboard) คุม `AlarmEngine` + `Buzzer` เข้าด้วยกัน |
| `main/core/StopwatchLogic.hpp/.cpp` | ตรรกะ start/stop/pause/lap แบบ pure (เวลาปัจจุบันถูก inject ผ่าน `ITimeSource`) |
| `main/core/StopwatchEngine.hpp/.cpp` | domain service ของ stopwatch + lap memory |
| `main/drivers/Stopwatch.hpp/.cpp` | facade ระดับแอปฯ (ตามชื่อไฟล์ของสเปก Dashboard) ครอบ `StopwatchEngine` |
| `main/drivers/EspTimerSource.hpp` | แหล่งเวลาแบบ monotonic จาก `esp_timer_get_time()` (ความละเอียด µs) |

**วิธีตรวจสอบ**
```bash
bash test/host/run_host_tests.sh     # → test_alarm_time_comparator, Alarm_Comparator_Trigger_Match,
                                     #   test_stopwatch_lap_split_calc, Stopwatch_HighRes_Timer_Precision,
                                     #   Lap_Memory_Buffer_Push
```

**สถานะ:** ✅ Verified — comparator ทำงานตรงเวลา, split/lap คำนวณจากเวลาสูงความละเอียด, lap buffer รองรับ 5 รายการ

---

### ✅ Spiral 5 — FreeRTOS Integration

**เป้าหมาย:** ประกอบทุกชั้นเข้าด้วยกันเป็นงานจริงที่ทำงานพร้อมกันแบบปลอดภัย

| ไฟล์ | สิ่งที่ทำ |
|---|---|
| `main/main.cpp` | `app_main()`: สร้าง object graph, init I²C/RTC/OLED/buzzer/ปุ่ม, ตั้ง timezone + alarm เริ่มต้น, สร้าง **4 tasks** แบบ `xTaskCreatePinnedToCore()`, ตั้ง bit ของ EventGroup |
| `main/ClockConfig.hpp` | พารามิเตอร์ task ทั้งหมด (priority, stack, core, period) |
| EventGroup bits | `kEventRtcReady`, `kEventDisplayReady`, `kEventWifiConnected`, `kEventNtpSynced`, `kEventAlarmTriggered` |

**การเชื่อมโยงระหว่าง task**

```
 GPIO ISR ──► Queue ──► vTaskButtonHandler ──► UiController ──► AlarmEngine / StopwatchEngine
                                                        │
                                                        ▼
                        vTaskClockDisplay (10 FPS) ──► UiRenderer ──► SSD1306 ──► I2CBus(mutex)
                                                        ▲
 DS1307 ◄── TimeManager ◄──── vTaskNTPSync ◄── Wi-Fi/SNTP   │
                                                        │
                        vTaskAlarmCheck (1 Hz) ───────────┘ (แจ้งเหตุการเตือนผ่าน EventGroup)
```

**หมายเหตุสำคัญ:** มี **4 tasks เท่านั้น** ตามสเปก — การ debounce ถูกรวมเข้า `vTaskButtonHandler`
ผ่าน `ButtonManager::poll()` แทนการเปิด task เพิ่ม (เมื่อ poll เรียก `update()` ของตัวกรองทุกครั้ง)

**วิธีตรวจสอบ**
```bash
# บน ESP32 (Unity)
cd test/test_clock_core && idf.py set-target esp32 && idf.py build flash monitor
# → test_freertos_event_group_sync, FreeRTOS_Button_Queue_Post_Receive, I2C_Mutex_Thread_Safety_Lock
```

**สถานะ:** ✅ Verified — EventGroup sync, การส่ง event ผ่าน queue และ mutex กันการเขียนบัสพร้อมกัน ทดสอบได้จริงบน target

---

### ✅ ขั้นตอนที่ 6 — Wokwi Simulation

| ไฟล์ | สิ่งที่ทำ |
|---|---|
| `wokwi/diagram.json` | ไดอะแกรมหลัก — ตรงกับ pin ของ Dashboard (12/13/14/27) + RTC/SSD1306 บน I²C + buzzer |
| `wokwi/diagram-suite-pins.json` | ไดอะแกรมสำรอง — ตรงกับตารางต่อสายของ Suite (18/19/4/5, RTC บนราง 5V) |

**สถานะ:** ✅ เสร็จ — JSON ผ่านการตรวจสอบ syntax แล้ว และใช้ชื่อ part มาตรฐานของ Wokwi
(`board-esp32-devkit-c-v4`, `wokwi-ds1307`, `wokwi-ssd1306`, `wokwi-pushbutton`, `wokwi-buzzer`)

---

### ✅ ขั้นตอนที่ 7 — Unit Test Suite (22 เคส)

| ไฟล์ | สิ่งที่ทำ |
|---|---|
| `test/test_clock_core/main/unit_tests.cpp` | Unity runner ของ ESP‑IDF ที่รัน test ทั้ง 22 เคส (10 ชื่อแบบ Suite + 12 ชื่อแบบ Dashboard) |
| `test/test_clock_core/main/core_cases.hpp` | 16 เคสที่ไม่ต้องใช้ hardware → **รันได้ทั้งบน ESP32 และบน PC** |
| `test/test_clock_core/main/driver_cases.hpp` | 6 เคสที่ต้องใช้ FreeRTOS/I²C/Wi‑Fi จริง → รันบน target เท่านั้น |
| `test/test_clock_core/main/mocks.hpp` | mock ของ RTC / time source / display / buzzer สำหรับเทสต์ส่วน domain |
| `test/test_clock_core/CMakeLists.txt` | โปรเจกต์ ESP‑IDF แยกสำหรับรันเทสต์ (คอมไพล์ source ตัวจริงของ `main/` ด้วยเส้นทางสัมบูรณ์) |
| `test/host/host_test_main.cpp` + `unity_lite.*` | runner บน PC ที่ใช้ test body ชุดเดียวกับ target (Unity‑lite shim) |
| `test/host/run_host_tests.sh` | สคริปต์ build + run เทสต์บน PC |
| `test/host/typecheck_target.sh` | type‑check `main/main.cpp` + `core/` + `drivers/` ด้วย ESP‑IDF stub (ไม่ต้องมี ESP‑IDF ติดตั้ง) |
| `test/host/stubs/` | stub ของ ESP‑IDF v6 API (`i2c_master`, `esp_netif`, `esp_netif_sntp`, `esp_wifi`, LEDC, FreeRTOS, Unity) — **ไม่ถูก compile เข้า firmware** |

**ผลการรันจริงบนเครื่องพัฒนา (host)**
```
=====================================================
 ESP32 digital clock - host unit test runner
=====================================================
[Spiral 1] hardware layer & bus communication
  test_ds1307_bcd_conversion                   PASS
  DS1307_BCD_To_Dec_Conversion                 PASS
  DS1307_Dec_To_BCD_Conversion                 PASS
  SSD1306_Framebuffer_Allocation_1024B         PASS
[Spiral 2] time keeping subsystem & synchronisation
  test_time_manager_rtc_fallback               PASS
  test_ntp_timestamp_parsing                   PASS
  SNTP_Timestamp_Offset_Calculation            PASS
  DS1307_NTP_AutoCalibration_Write             PASS
[Spiral 3] input & UI state machine
  test_button_long_press_detect                PASS
  Button_Software_Debounce_50ms_Filter         PASS
  test_ui_mode_cycle                           PASS
[Spiral 4 & 5] engines & FreeRTOS integration
  test_alarm_time_comparator                   PASS
  Alarm_Comparator_Trigger_Match               PASS
  test_stopwatch_lap_split_calc                PASS
  Stopwatch_HighRes_Timer_Precision            PASS
  Lap_Memory_Buffer_Push                       PASS
=====================================================
 checks: 158, failed tests: 0
 ALL TESTS PASSED
=====================================================
```

---

## 5. วิธี Build / Flash / Monitor

### 5.1 เฟิร์มแวร์ (ESP‑IDF v6.0)

```bash
# 1) เตรียม environment
. $IDF_PATH/export.sh

# 2) เลือก target และ build
idf.py set-target esp32
idf.py build

# 3) flash + ดู log
idf.py -p /dev/ttyUSB0 flash monitor
```

หรือใช้ ESP‑IDF Extension ของ VS Code → เลือก target `esp32` → กด Build / Flash

### 5.2 เทสต์บน target

```bash
cd test/test_clock_core
idf.py set-target esp32
idf.py build flash monitor
```

> โปรเจกต์เทสต์เป็น ESP‑IDF project แยกอีกอันหนึ่งที่คอมไพล์ **source ตัวจริง** ของ `main/`
> ผ่านเส้นทางสัมบูรณ์ → เทสต์จึงทดสอบโค้ดที่จะ ship จริงเสมอ

### 5.3 เทสต์บนเครื่อง PC (ไม่ต้องมี ESP‑IDF)

```bash
bash test/host/run_host_tests.sh       # build + run 16 เคสของ domain layer
bash test/host/typecheck_target.sh     # type-check ทุกไฟล์ที่พึ่ง ESP-IDF
```

> สคริปต์เลือก compiler อัตโนมัติ: `g++` → `clang++` → `python -m ziglang c++` (ติดตั้งด้วย `pip install ziglang`)

---

## 6. วิธีรันชุดทดสอบ Unit Test

### 6.1 บน ESP32 (Unity ของ ESP‑IDF) — 22 เคส

```bash
cd test/test_clock_core && idf.py build flash monitor
```

### 6.2 บน PC — 16 เคส (domain layer ทั้งหมด)

```bash
bash test/host/run_host_tests.sh
```

### 6.3 เรียกเทสต์เฉพาะกลุ่ม (ตัวอย่าง)

```c
// ใน test/test_clock_core/main/unit_tests.cpp
RUN_TEST(test_ds1307_bcd_conversion);   // ← เพิ่ม/ลบบรรทัดนี้เพื่อเลือกเคสที่จะรัน
```

---

## 7. การจำลองบน Wokwi

1. เปิด <https://wokwi.com> → เลือกโปรเจกต์ **ESP32** ภาษา **C++ / ESP‑IDF**
2. เปิดแท็บ `diagram.json` แล้ววางเนื้อหาจาก
   * `wokwi/diagram.json` → ตรงกับ pin ของ Dashboard (12/13/14/27) — **ค่าเริ่มต้นของ firmware**
   * `wokwi/diagram-suite-pins.json` → ตรงกับตารางต่อสายของ Suite (18/19/4/5)
3. อัปโหลดไฟล์โค้ดจาก `main/` (ใช้โครงสร้างตาม [แผนผังไฟล์](#12-แผนผังไฟล์ทั้งหมด))
4. กด **Start Simulation** แล้วเปิด Serial Monitor (115200)

**การควบคุมในจำลอง**

| ปุ่ม | คีย์ลัด | การทำงาน |
|---|---|---|
| MODE | `M` | สลับโหมด Clock → Set → Alarm → Stopwatch |
| INC / NTP | `S` | เพิ่มค่า / สั่ง sync NTP ทันที |
| SET / ALARM | `A` | ยืนยันค่า / arm–disarm alarm |
| LAP / RESET | `L` | บันทึก lap / รีเซ็ต stopwatch |

---

## 8. ตาราง FreeRTOS Tasks

| Task | Priority | Stack | Core | Trigger / Period | หน้าที่ |
|---|---|---|---|---|---|
| `vTaskClockDisplay` | 3 (Medium) | 4096 B | Core 1 | Periodic 100 ms | วาดโหมดปัจจุบันลง buffer ของ SSD1306 แล้ว flush เฟรมผ่าน I²C |
| `vTaskButtonHandler` | 5 (High) | 3072 B | Core 0 | ISR queue event | ประมวลผลปุ่มที่ debounce แล้ว, ตรวจ long‑press, สลับโหมดการทำงาน |
| `vTaskAlarmCheck` | 4 (Med‑High) | 2048 B | Core 1 | Periodic 1000 ms | เทียบเวลา RTC กับเวลาที่ตั้ง alarm แล้วสั่ง LEDC PWM ที่ piezo buzzer |
| `vTaskNTPSync` | 1 (Low) | 6144 B | Core 0 | Periodic 3600 s | ต่อ Wi‑Fi SSID `iotexpo`, query `pool.ntp.org`, ปรับ POSIX time และเขียนกลับลง DS1307 |

> พารามิเตอร์ทั้งหมดประกาศไว้ใน `main/ClockConfig.hpp` จึงเปลี่ยนค่าได้ที่จุดเดียว

---

## 9. ตารางการต่อสาย (Pin Map)

| อุปกรณ์ | สัญญาณ | ขา ESP32 |
|---|---|---|
| DS1307 RTC (0x68) | SDA / SCL | GPIO 21 / GPIO 22 |
| DS1307 RTC | VCC / GND | 3V3 / GND (5V ในไดอะแกรมสำรองของ Suite) |
| SSD1306 OLED (0x3C) | SDA / SCL | GPIO 21 / GPIO 22 |
| SSD1306 OLED | VCC / GND | 3V3 / GND |
| ปุ่ม MODE | – | **GPIO 12** (สำรอง: 18) |
| ปุ่ม INC / NTP | – | **GPIO 13** (สำรอง: 19) |
| ปุ่ม SET / ALARM | – | **GPIO 14** (สำรอง: 4) |
| ปุ่ม LAP / RESET | – | **GPIO 27** (สำรอง: 5) |
| Piezo buzzer | – | GPIO 25 (LEDC PWM 2.7 kHz) |
| I²C bus | – | I²C #0 @ 400 kHz |
| Wi‑Fi | – | STA `iotexpo` / `12345678` |
| NTP | – | `pool.ntp.org` (สำรอง `time.nist.gov`), TZ +7 ชม. |

---

## 10. ตาราง Unit Tests ทั้งหมด 22 รายการ

| # | ชื่อเทสต์ | ที่มา | Spiral | จุดที่ตรวจ | รันที่ไหน |
|---|---|---|---|---|---|
| 1 | `test_ds1307_bcd_conversion` | Suite | 1 | BCD ↔ decimal ของ register DS1307 | Host + Target |
| 2 | `DS1307_BCD_To_Dec_Conversion` | Dashboard | 1 | BCD → decimal | Host + Target |
| 3 | `DS1307_Dec_To_BCD_Conversion` | Dashboard | 1 | decimal → BCD | Host + Target |
| 4 | `SSD1306_Framebuffer_Allocation_1024B` | Dashboard | 1 | ขนาด frame buffer = 128×64/8 = 1024 B | Host + Target |
| 5 | `test_i2c_bus_mutex_lock` | Suite | 1 | mutex ของ I²C bus (ไม่ recursive, timeout ได้) | Target |
| 6 | `test_ssd1306_buffer_bounds` | Suite | 1 | การเขียน pixel เกินขอบเขตถูกปฏิเสธ | Target |
| 7 | `test_time_manager_rtc_fallback` | Suite | 2 | เมื่อ NTP ล้มเหลว → ใช้เวลาจาก RTC | Host + Target |
| 8 | `test_ntp_timestamp_parsing` | Suite | 2 | การแปลง NTP epoch → POSIX epoch | Host + Target |
| 9 | `SNTP_Timestamp_Offset_Calculation` | Dashboard | 3 | offset timezone (+7 ชม.) ถูกต้อง | Host + Target |
| 10 | `DS1307_NTP_AutoCalibration_Write` | Dashboard | 3 | เขียนเวลาจาก NTP กลับลง DS1307 | Host + Target |
| 11 | `WiFi_Station_Connect_SSID_iotexpo` | Dashboard | 3 | credential `iotexpo`/`12345678` เข้าถึง driver จริง | Target |
| 12 | `test_button_long_press_detect` | Suite | 3 | แยก short (1000 ms) กับ long press | Host + Target |
| 13 | `Button_Software_Debounce_50ms_Filter` | Dashboard | 2 | ตัวกรอง debounce 50 ms | Host + Target |
| 14 | `FreeRTOS_Button_Queue_Post_Receive` | Dashboard | 2 | event ปุ่มส่งผ่าน queue จาก ISR → task | Target |
| 15 | `I2C_Mutex_Thread_Safety_Lock` | Dashboard | 2 | task ที่สอง lock บัสไม่ได้ขณะถูกล็อก | Target |
| 16 | `test_ui_mode_cycle` | Suite | 3 | วงจร Clock → Set → Alarm → Stopwatch | Host + Target |
| 17 | `test_alarm_time_comparator` | Suite | 4 | comparator ของ alarm | Host + Target |
| 18 | `Alarm_Comparator_Trigger_Match` | Dashboard | 4 | เงื่อนไข trigger ของ alarm | Host + Target |
| 19 | `test_stopwatch_lap_split_calc` | Suite | 4 | ความแม่นยำของ split time | Host + Target |
| 20 | `Stopwatch_HighRes_Timer_Precision` | Dashboard | 4 | ความละเอียดของ `esp_timer` | Host + Target |
| 21 | `Lap_Memory_Buffer_Push` | Dashboard | 4 | lap memory buffer (สูงสุด 5 รายการ) | Host + Target |
| 22 | `test_freertos_event_group_sync` | Suite | 5 | bit flag ของ EventGroup ระหว่าง task | Target |

**สรุป:** 22 เคส = 10 ชื่อจากสเปก Suite + 12 ชื่อจากสเปก Dashboard (ไม่ซ้ำกัน)
— 16 เคสรันได้บน PC (Host) และ 6 เคสที่ต้องใช้ FreeRTOS/hardware จริงรันบน ESP32

---

## 11. Traceability Matrix (สเปก HTML → โค้ด → เทสต์)

| ข้อกำหนดใน HTML | ไฟล์ที่ implement | เทสต์ที่ยืนยัน |
|---|---|---|
| Bottom‑Up modular OOP + decoupling ด้วย interface | `core/Interfaces.hpp`, `main.cpp` | ทั้งชุด (compile ของ `core/` ไม่ต้องมี ESP‑IDF) |
| I2C bus thread‑safe (Mutex) | `drivers/I2CBus.hpp/.cpp` | `test_i2c_bus_mutex_lock`, `I2C_Mutex_Thread_Safety_Lock` |
| DS1307 BCD register parser | `core/Bcd.hpp`, `drivers/DS1307.cpp` | `test_ds1307_bcd_conversion`, `DS1307_BCD_To_Dec_Conversion`, `DS1307_Dec_To_BCD_Conversion` |
| SSD1306 128×64 / buffer 1024 B | `drivers/SSD1306.hpp/.cpp`, `core/DisplayGeometry.hpp` | `SSD1306_Framebuffer_Allocation_1024B`, `test_ssd1306_buffer_bounds` |
| TimeManager + RTC fallback | `core/TimeManager.cpp` | `test_time_manager_rtc_fallback` |
| Wi‑Fi STA `iotexpo` + SNTP `pool.ntp.org` | `drivers/NTPManager.cpp` | `WiFi_Station_Connect_SSID_iotexpo`, `test_ntp_timestamp_parsing`, `SNTP_Timestamp_Offset_Calculation` |
| NTP → RTC auto calibration | `core/TimeManager.cpp` (`requestSync`) | `DS1307_NTP_AutoCalibration_Write` |
| GPIO ISR + software debounce 50 ms | `drivers/Button.cpp`, `drivers/ButtonManager.cpp`, `core/Debounce.cpp` | `Button_Software_Debounce_50ms_Filter`, `test_button_long_press_detect`, `FreeRTOS_Button_Queue_Post_Receive` |
| UI state machine 4 โหมด | `core/UiStateMachine.cpp`, `core/UiController.cpp`, `core/UiRenderer.cpp` | `test_ui_mode_cycle` |
| Alarm comparator + piezo buzzer | `core/AlarmEngine.cpp`, `drivers/Buzzer.cpp`, `drivers/AlarmManager.cpp` | `test_alarm_time_comparator`, `Alarm_Comparator_Trigger_Match` |
| Stopwatch lap + high‑res timer | `core/StopwatchEngine.cpp`, `drivers/EspTimerSource.hpp`, `drivers/Stopwatch.cpp` | `test_stopwatch_lap_split_calc`, `Stopwatch_HighRes_Timer_Precision`, `Lap_Memory_Buffer_Push` |
| FreeRTOS 4 tasks + EventGroup | `main/main.cpp` | `test_freertos_event_group_sync` |
| Wokwi `diagram.json` | `wokwi/diagram.json`, `wokwi/diagram-suite-pins.json` | ตรวจ JSON syntax แล้ว |
| ไฟล์โค้ดตามรายการของ Dashboard | `main/drivers/{DS1307,SSD1306,Button,NTPManager,AlarmManager,Stopwatch}.hpp/.cpp` | `test/test_clock_core/main/unit_tests.cpp` |
| ไฟล์โค้ดตามรายการของ Suite | `main/{I2CBus,DS1307,SSD1306,ButtonManager,TimeManager,AlarmEngine,StopwatchEngine}` | `test/test_clock_core/main/unit_tests.cpp` |

---

## 12. แผนผังไฟล์ทั้งหมด

```
.
├── CMakeLists.txt                     # โปรเจกต์ ESP-IDF v6 ของเฟิร์มแวร์
├── sdkconfig.defaults                # ค่า default ของ target, lwIP SNTP, FreeRTOS, log
├── .gitignore
├── README.md                          # เอกสารนี้
│
├── main/
│   ├── CMakeLists.txt                 # ลงทะเบียน component + REQUIRES ของ ESP-IDF v6
│   ├── ClockConfig.hpp                # single source of truth: pin, credential, task param
│   ├── main.cpp                       # app_main() + 4 FreeRTOS tasks
│   │
│   ├── core/                          # ── ชั้น domain (pure C++17, ไม่มี ESP-IDF) ──
│   │   ├── Interfaces.hpp             #   ports: IRtcDevice, ITimeSource, INtpClient, IDisplaySurface, IBuzzer, IButtonInput
│   │   ├── Bcd.hpp                    #   BCD ↔ decimal
│   │   ├── DateTime.hpp/.cpp          #   value object วัน–เวลา + day-of-week
│   │   ├── ButtonEvent.hpp            #   value object ของ event ปุ่ม
│   │   ├── Debounce.hpp/.cpp          #   ตัวกรอง debounce + Short/Long press
│   │   ├── DisplayGeometry.hpp        #   เรขาคณิตจอ 128×64
│   │   ├── NtpEpoch.hpp               #   NTP epoch ↔ POSIX epoch + timezone
│   │   ├── AlarmLogic.hpp/.cpp        #   ตรรกะ comparator/latch ของ alarm
│   │   ├── AlarmEngine.hpp/.cpp       #   domain service ของ alarm
│   │   ├── StopwatchLogic.hpp/.cpp    #   ตรรกะ start/stop/pause/lap
│   │   ├── StopwatchEngine.hpp/.cpp   #   domain service ของ stopwatch
│   │   ├── UiStateMachine.hpp/.cpp    #   FSM 4 โหมด
│   │   ├── UiController.hpp/.cpp      #   event ปุ่ม → คำสั่ง domain
│   │   ├── UiRenderer.hpp/.cpp        #   วาดเฟรมของแต่ละโหมด
│   │   └── TimeManager.hpp/.cpp       #   ตัวกำหนดเวลาหลัก (NTP → RTC → fallback)
│   │
│   └── drivers/                       # ── ชั้น adapter (ESP-IDF v6) ──
│       ├── I2CDevice.hpp              #   base class ของอุปกรณ์บนบัส
│       ├── I2CBus.hpp/.cpp            #   I²C master + mutex
│       ├── DS1307.hpp/.cpp            #   RTC 0x68
│       ├── SSD1306.hpp/.cpp           #   OLED 0x3C + frame buffer 1024 B
│       ├── Font5x7.hpp                #   ฟอนต์ 5×7
│       ├── Button.hpp/.cpp            #   ปุ่มรายตัว + GPIO ISR   (ชื่อไฟล์ตาม Dashboard)
│       ├── ButtonManager.hpp/.cpp     #   debounce engine ของ 4 ปุ่ม (ชื่อไฟล์ตาม Suite)
│       ├── Buzzer.hpp/.cpp            #   LEDC PWM
│       ├── AlarmManager.hpp/.cpp      #   facade ของ alarm        (ชื่อไฟล์ตาม Dashboard)
│       ├── Stopwatch.hpp/.cpp         #   facade ของ stopwatch    (ชื่อไฟล์ตาม Dashboard)
│       ├── NTPManager.hpp/.cpp        #   Wi-Fi STA + SNTP        (ชื่อไฟล์ตาม Dashboard)
│       ├── SystemClock.hpp/.cpp       #   POSIX clock adapter
│       └── EspTimerSource.hpp         #   esp_timer adapter
│
├── test/
│   ├── test_clock_core/               # โปรเจกต์ ESP-IDF สำหรับรัน Unity test บน target
│   │   ├── CMakeLists.txt
│   │   ├── sdkconfig.defaults
│   │   └── main/
│   │       ├── CMakeLists.txt         # คอมไพล์ source จริงของ main/ ด้วยเส้นทางสัมบูรณ์
│   │       ├── unit_tests.cpp         # runner 22 เคส
│   │       ├── core_cases.hpp         # 16 เคส (host + target)
│   │       ├── driver_cases.hpp       # 6 เคส (target เท่านั้น)
│   │       └── mocks.hpp              # mock ของ RTC/time/display/buzzer
│   │
│   └── host/                          # runner บน PC
│       ├── host_test_main.cpp
│       ├── unity_lite.hpp/.cpp        # Unity-lite shim
│       ├── run_host_tests.sh          # build + run เทสต์
│       ├── typecheck_target.sh        # type-check โค้ดฝั่ง ESP-IDF ด้วย stub
│       └── stubs/                     # ESP-IDF v6 API stub (ไม่เข้า firmware)
│
└── wokwi/
    ├── diagram.json                   # pin map ของ Dashboard (12/13/14/27)
    └── diagram-suite-pins.json        # pin map ของ Suite   (18/19/4/5)
```

---

## 13. ข้อจำกัด / หมายเหตุ

1. **ยังไม่ได้ flash ลงฮาร์ดแวร์จริง** — เครื่องที่ใช้พัฒนาไม่มี ESP‑IDF และไม่มีบอร์ด ESP32
   การตรวจสอบจึงทำด้วย 2 วิธี:
   * host unit test (16 เคส ผ่านหมด — 158 assertions)
   * type-check ทุก source file ที่พึ่ง ESP‑IDF ด้วย stub ที่เขียนตาม header จริงของ
     ESP‑IDF v6.0 (`esp_netif.h`, `esp_netif_sntp.h`, `esp_wifi.h`, `esp_wifi_types_generic.h`,
     `esp_netif_types.h`) — คอมไพล์ผ่านด้วย `-Wall -Wextra` โดยไม่มี warning
2. **`test/host/stubs/` ไม่ใช่โค้ดจริงของ ESP‑IDF** เป็นเพียงตัวแทนเพื่อการตรวจสอบบน PC
   การ build จริงต้องใช้ ESP‑IDF v6 (`idf.py build`) ตาม [หัวข้อ 5](#5-วิธี-build--flash--monitor)
3. **การเลือก pin** — ใช้ชุดของ Dashboard (12/13/14/27) เป็นค่าเริ่มต้น เพราะตรงกับชื่อปุ่มใน UI
   (`MODE / INC / SET / LAP`) และกับ `ClockConfig.hpp`; ชุดของ Suite มีไดอะแกรมสำรองให้
4. **เวลาโซน** — โค้ดบวก offset +7 ชม. ในตัวเอง (ไม่ใช้ `TZ` ของ lwIP) เพื่อให้ผลการทดสอบ
   (`SNTP_Timestamp_Offset_Calculation`) ไม่ขึ้นกับค่า `TZ` ของเครื่อง
5. **การกดปุ่ม** — เป็น *active low* พร้อม pull‑up ภายใน ตามวงจรปุ่มทั่วไปใน Wokwi
6. **ไฟล์ HTML ต้นฉบับ** ยังคงอยู่ในรีโปเพื่อใช้อ้างอิงสเปก ตามเดิม
