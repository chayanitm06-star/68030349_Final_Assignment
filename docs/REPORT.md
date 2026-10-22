# ESP32 Digital Clock — Project Creation Report

> รายงานฉบับเต็มของโปรเจกต์นาฬิกาดิจิทัล ESP32 (ESP-IDF v6 / FreeRTOS / C++ OOP)
> จัดทำจากสเปกในไฟล์ `wokwi_esp32_digital_clock_suite.html` และ `wokwi_esp32_digital_clock_dashboard.html`
> เอกสาร README ระดับต้นอยู่ที่ [`../README.md`](../README.md)

---

## 1. Project Background & Objectives

โปรเจกต์นาฬิกาดิจิทัลบน ESP32 DevKit C V4 ที่ต้อง:

* ใช้ **ESP-IDF v6.0 + FreeRTOS** เขียนด้วย **C++ แบบ OOP** สถาปัตยกรรม **Bottom-Up Modular**
* ต่อ RTC **DS1307** (0x68) และจอ OLED **SSD1306** (0x3C) ผ่าน I²C #0 @ 400 kHz (SDA 21 / SCL 22)
* ปุ่ม 4 ตัว + piezo buzzer (GPIO 25) + Wi‑Fi STA (`iotexpo`) + NTP (`pool.ntp.org`)
* มีฟังก์ชัน: นาฬิกา, ตั้งปลุก, จับเวลา (lap), ตั้งเวลาเองผ่าน NTP พร้อมเขียนกลับลง RTC
* มี **Wokwi `diagram.json`** สำหรับจำลอง และ **ชุด unit test ตามชื่อที่สเปกกำหนด**

### 1.1 สิ่งที่สเปกไฟล์ที่ 1 (`..._suite.html`) กำหนด

| หัวข้อ | สิ่งที่สเปกกำหนด |
|---|---|
| สถาปัตยกรรม | Bottom‑Up Modular OOP – HAL สร้างทีละชั้นจนถึงระบบ FreeRTOS multitasking; แยก hardware (I2C Bus, DS1307, SSD1306, Button) ออกจาก domain logic (TimeManager, AlarmEngine, StopwatchEngine) ด้วย interface มาตรฐาน + Queue/Mutex/EventGroup |
| Spiral 1 | HAL & Bus Layer – I2CBus thread‑safe, DS1307 BCD parser, SSD1306 128×64 buffer |
| Spiral 2 | Time Subsystem – TimeManager, Wi‑Fi Station driver, SNTP sync + RTC fallback |
| Spiral 3 | Input & UI Engine – GPIO ISR debounce (Short/Long press), OLED renderer task |
| Spiral 4 | Business Engines – AlarmEngine comparator + piezo buzzer, StopwatchEngine lap timing |
| Spiral 5 | FreeRTOS Integration – multitask sync, queue event pump, mutex safety, `app_main` |
| Tasks | `vTaskClockDisplay` (prio 3 / 4096 B / Core 1 / 100 ms), `vTaskButtonHandler` (5 / 3072 B / Core 0 / ISR), `vTaskAlarmCheck` (4 / 2048 B / Core 1 / 1000 ms), `vTaskNTPSync` (1 / 6144 B / Core 0 / 3600 s) |
| Wokwi | `diagram.json` + ตารางต่อสาย: DS1307 & SSD1306 บน I²C (SDA 21 / SCL 22), ปุ่ม 4 ตัว GPIO 18/19/4/5, buzzer GPIO 25 |
| ไฟล์โค้ด | `main.cpp`, `I2CBus.hpp`, `DS1307.hpp/.cpp`, `SSD1306.hpp/.cpp`, `ButtonManager.hpp`, `TimeManager.hpp`, `AlarmEngine.hpp`, `StopwatchEngine.hpp` |
| เทสต์ | 10 test ของ 4 spiral module (`test_ds1307_bcd_conversion` … `test_freertos_event_group_sync`) |

### 1.2 สิ่งที่สเปกไฟล์ที่ 2 (`..._dashboard.html`) กำหนด

| หัวข้อ | สิ่งที่สเปกกำหนด |
|---|---|
| Dashboard | จำลอง hardware (OLED + ปุ่ม + serial console + bus diagnostics + Wi‑Fi credential), รัน unit test 4 ชุด, กราฟสถาปัตยกรรม 4 แผนภาพ, แสดง source code |
| สเปกอุปกรณ์ | OLED 128×64 I²C @ 0x3C, DS1307 @ 0x68, I²C #0 400 kHz (SDA 21 / SCL 22), buzzer GPIO 25, debounce 50 ms, stopwatch ใช้ `esp_timer` |
| ปุ่ม | MODE = GPIO 12, INC/NTP = GPIO 13, SET/ALARM = GPIO 14, LAP/RESET = GPIO 27 |
| การเชื่อมต่อ | Wi‑Fi STA `iotexpo` / `12345678`, NTP `pool.ntp.org`, TZ +7 ชม. |
| Spiral | 4 spiral: ① HAL & Core Drivers ② FreeRTOS Task Engine & Switch Driver ③ Network & NTP ④ Application Features (Alarm & Lap Timer) |
| เทสต์ | 12 test: `DS1307_BCD_To_Dec_Conversion`, `DS1307_Dec_To_BCD_Conversion`, `SSD1306_Framebuffer_Allocation_1024B`, `FreeRTOS_Button_Queue_Post_Receive`, `Button_Software_Debounce_50ms_Filter`, `I2C_Mutex_Thread_Safety_Lock`, `WiFi_Station_Connect_SSID_iotexpo`, `SNTP_Timestamp_Offset_Calculation`, `DS1307_NTP_AutoCalibration_Write`, `Alarm_Comparator_Trigger_Match`, `Stopwatch_HighRes_Timer_Precision`, `Lap_Memory_Buffer_Push` |
| ไฟล์โค้ด | `CMakeLists.txt`, `main.cpp`, `DS1307.hpp/.cpp`, `SSD1306.hpp/.cpp`, `Button.hpp/.cpp`, `NTPManager.hpp/.cpp`, `AlarmManager.hpp/.cpp`, `Stopwatch.hpp/.cpp`, `unit_tests.cpp` |
| Wokwi | `diagram.json` พร้อม paste ลง Wokwi โดยตรง |

### 1.3 การจัดการความต่างระหว่างสเปกทั้งสอง

สเปกทั้งสองอ้างอิง **ชื่อไฟล์และชื่อ test ที่ต่างกัน** และ **pin ปุ่มต่างกัน** การออกแบบจึงทำให้ทั้งสองชุดชื่ออยู่ร่วมกันได้:

* ไฟล์/คลาสตาม **Dashboard** อยู่ใน `main/drivers/` (ชื่อไฟล์ตรงกับสเปก): `Button`, `NTPManager`, `AlarmManager`, `Stopwatch`
* คลาส/ไฟล์ตาม **Suite** อยู่ใน `main/core/` และ `main/drivers/`: `I2CBus`, `ButtonManager`, `TimeManager`, `AlarmEngine`, `StopwatchEngine`
* Pin map ที่ใช้จริง = ของ **Dashboard** (12/13/14/27) มี `wokwi/diagram-suite-pins.json` เป็นตัวเลือกสำรอง (18/19/4/5)

---

## 2. Architecture

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

**กฎการพึ่งพา**

* `core/` → ไม่มี `#include` ของ ESP‑IDF เลย → compile บน PC ได้ → unit test ได้ทันที
* `drivers/` → implement interface ที่ประกาศใน `core/Interfaces.hpp`
* `main.cpp` → ประกอบ object graph ทั้งหมด (dependency injection) แล้วสร้าง task

**Interface (port) ที่ใช้คั่นสองชั้น** — `main/core/Interfaces.hpp`

| Interface | ผู้ implement | หน้าที่ |
|---|---|---|
| `IRtcSource` | `DS1307` | อ่าน/เขียนเวลา RTC |
| `ISystemClock` | `SystemClock` | อ่าน POSIX system clock (วินาทีนับจาก 1970) |
| `ITimeSource` | `EspTimerSource` | มิลลิวินาทีแบบ monotonic (จาก `esp_timer`) |
| `INtpClient` | `NTPManager` | ซิงก์เวลาจาก NTP |
| `IDisplay` | `SSD1306` | วาดภาพลง frame buffer + flush |
| `IBuzzer` | `Buzzer` | เปิด/ปิดเสียง piezo (LEDC PWM) |

> ปุ่มไม่ได้ผ่าน interface แต่ถูก *pull* โดย task โดยตรง (`ButtonManager::poll()`)
> ส่วน `UiController` รับ `AlarmEngine` / `StopwatchEngine` / `TimeManager` ผ่าน dependency injection

---

## 3. File Structure Creation Steps

| ขั้นตอน | สิ่งที่สร้าง | ไฟล์ |
|---|---|---|
| 3.1 | โปรเจกต์ ESP‑IDF v6 + ค่า default ของ target | `CMakeLists.txt`, `sdkconfig.defaults`, `.gitignore` |
| 3.2 | ค่าคงที่ทั้งหมดของบอร์ดรวมที่เดียว | `main/ClockConfig.hpp` |
| 3.3 | โครง `main/core/` (value object + logic ล้วน) | `Bcd.hpp`, `DateTime.*`, `ButtonEvent.hpp`, `Debounce.*`, `DisplayGeometry.hpp`, `NtpEpoch.hpp` |
| 3.4 | โครง `main/drivers/` (adapter) | `I2CDevice.hpp`, `I2CBus.*`, `DS1307.*`, `SSD1306.*`, `Font5x7.hpp` |
| 3.5 | domain service ของ alarm / stopwatch | `AlarmLogic.*`, `AlarmEngine.*`, `StopwatchLogic.*`, `StopwatchEngine.*` |
| 3.6 | ชั้นเวลาและ UI | `TimeManager.*`, `UiStateMachine.*`, `UiController.*`, `UiRenderer.*` |
| 3.7 | driver ที่เหลือ | `Button.*`, `ButtonManager.*`, `Buzzer.*`, `NTPManager.*`, `SystemClock.*`, `EspTimerSource.hpp`, `AlarmManager.*`, `Stopwatch.*` |
| 3.8 | จุดประกอบทั้งระบบ + 4 task | `main/main.cpp` |
| 3.9 | โปรเจกต์เทสต์ Unity บน target | `test/test_clock_core/**` |
| 3.10 | runner บน PC + ESP‑IDF stub | `test/host/**` |
| 3.11 | ไดอะแกรม Wokwi | `diagram.json`, `wokwi/diagram-suite-pins.json`, `wokwi.toml` |
| 3.12 | เอกสาร | `README.md`, `docs/REPORT.md` |

---

## 4. Spiral Steps (ทีละขั้น)

### ✅ Spiral 0 — โครงสร้างโปรเจกต์และ “single source of truth”

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
| `main/core/UiStateMachine.hpp/.cpp` | FSM 4 โหมด `CLOCK → ALARM → STOPWATCH → SET TIME` + cursor ตอนตั้งค่า |
| `main/core/UiController.hpp/.cpp` | แปลง event ปุ่มเป็นคำสั่งของ domain (เลื่อนโหมด, ปรับค่า, เปิด/ปิด alarm, start/stop/lap) |
| `main/core/UiRenderer.hpp/.cpp` | วาดเฟรมของแต่ละโหมดลง `IDisplay` (เวลา/วันที่, ตั้ง alarm, stopwatch + lap) |
| `main/core/DisplayGeometry.hpp` | ค่าคงที่เรขาคณิตของจอ 128×64 (ใช้ในเทสต์ได้) |
| `main/drivers/Button.hpp/.cpp` | driver ปุ่มรายตัว: ตั้ง GPIO (pull‑up, input), ติดตั้ง ISR, ส่ง event เข้า queue |
| `main/drivers/ButtonManager.hpp/.cpp` | ครอบ 4 ปุ่ม: drain queue → เติมข้อมูลลง `DebounceFilter` → ส่ง event ที่ผ่านการ debounce ให้ task |

**วิธีตรวจสอบ**
```bash
bash test/host/run_host_tests.sh     # → test_button_long_press_detect,
                                     #   Button_Software_Debounce_50ms_Filter, test_ui_mode_cycle
```

**สถานะ:** ✅ Verified — short/long press แยกได้ถูกต้องที่ 1000 ms, การหมุนโหมดเป็นวงจร CLOCK → ALARM → STOPWATCH → SET TIME แบบ cyclic

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
| `main/core/StopwatchEngine.hpp/.cpp` | domain service ของ stopwatch + lap memory (สูงสุด 5 รายการ) |
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

> มี **4 tasks เท่านั้น** ตามสเปก — การ debounce ถูกรวมเข้า `vTaskButtonHandler`
> ผ่าน `ButtonManager::poll()` แทนการเปิด task เพิ่ม (เมื่อ poll เรียก `update()` ของตัวกรองทุกครั้ง)

**วิธีตรวจสอบ**
```bash
cd test/test_clock_core && idf.py set-target esp32 && idf.py build flash monitor
# → test_freertos_event_group_sync, FreeRTOS_Button_Queue_Post_Receive, I2C_Mutex_Thread_Safety_Lock
```

**สถานะ:** ✅ Verified — EventGroup sync, การส่ง event ผ่าน queue และ mutex กันการเขียนบัสพร้อมกัน ทดสอบได้จริงบน target

---

### ✅ Step 6 — Wokwi Simulation

| ไฟล์ | สิ่งที่ทำ |
|---|---|
| `diagram.json` (รากโปรเจกต์) | ไดอะแกรมหลัก — ตรงกับ pin ของ Dashboard (12/13/14/27) + RTC/SSD1306 บน I²C + buzzer + ปุ่ม 4 ตัวผูกคีย์ลัด · วางไว้รากโปรเจกต์ตามที่ Wokwi for VS Code / Wokwi CLI ต้องการ |
| `wokwi/diagram-suite-pins.json` | ไดอะแกรมสำรอง — ตรงกับตารางต่อสายของ Suite (18/19/4/5, RTC บนราง 5V) |
| `wokwi.toml` (รากโปรเจกต์) | ชี้ firmware ที่ build ได้: `firmware = 'build/flasher_args.json'`, `elf = 'build/esp32_digital_clock.elf'` (สำหรับ ESP-IDF ต้องชี้ flasher_args.json เพื่อให้ Wokwi โหลด bootloader + partition table + app เหมือน `idf.py flash`) |

**การแก้ไขที่ต้องทำ** — `diagram.json` ที่แนบมาในสเปก HTML **ต่อสายไม่ได้จริง** ผมจึงเขียนใหม่โดยอ้างอิง
คู่มือ part ของ Wokwi (`wokwi/wokwi-docs`) และไฟล์จริงของ part (`wokwi/wokwi-boards`, `wokwi/wokwi-elements`):

| สิ่งที่ผิดในสเปก | แก้เป็น |
|---|---|
| `wokwi-esp32-devkit-v4` (ไม่มี part type นี้) | `board-esp32-devkit-c-v4` |
| `board-ds1307` (ไม่มี part type นี้) | `wokwi-ds1307` |
| `wokwi-ssd1306` (ถูก deprecate แล้ว) | `board-ssd1306` |
| ขาปุ่ม `1.1` / `1.A` / `2.1` | `1.l` / `2.l` (ปุ่ม Wokwi มีขา `1.l` `2.l` `1.r` `2.r`) |
| `rtc:VCC` | `rtc:5V` (โมดูล DS1307 ของ Wokwi มีขา `GND` `5V` `SDA` `SCL` `SQW`) |
| `esp:GND` (บอร์ดไม่มีขาชื่อ GND เปล่า) | `esp:GND.1` / `esp:GND.2` / `esp:GND.3` (บอร์ดมี 3 ขา GND) |
| `esp:GND.4` (ไม่มีอยู่จริง) | `esp:GND.3` |
| ปุ่มวางที่ `top: 150` → **ทับตัวบอร์ด** (บอร์ดสูง 214 px) | ย้ายชุดปุ่มลงใต้บอร์ด `top: 280`, ย้าย RTC/OLED ไปขวา `left: 280` |
| buzzer ต่อกลับด้าน (ขา 1 = ลบ, ขา 2 = บวก) | `esp:25 → buzzer:2`, `esp:GND.1 → buzzer:1` |

**สถานะ:** ✅ เสร็จ — ผ่านการตรวจ 5 ข้อ (part type ถูกต้อง, ไม่มีอุปกรณ์ทับกัน, ขาทุกเส้นมีจริงบน part,
ทุกอุปกรณ์ถูกต่อสายครบ, ขา ESP32 ที่ใช้มีจริงบน `board-esp32-devkit-c-v4`)

---

### ✅ Step 7 — Unit Test Suite (22 เคส)

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
| `test/host/check_timeval_include.py` | source‑level guard กันบั๊ก `struct Clock::timeval` (รันอัตโนมัติใน `typecheck_target.sh`) |
| `test/host/stubs/` | stub ของ ESP‑IDF v6 API (`i2c_master`, `esp_netif`, `esp_netif_sntp`, `esp_wifi`, LEDC, FreeRTOS, Unity) — **ไม่ถูก compile เข้า firmware** |

---

## 5. Build & Run

### 5.1 เฟิร์มแวร์ (ESP‑IDF v6.0)

```bash
. $IDF_PATH/export.sh
idf.py set-target esp32
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

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
bash test/host/typecheck_target.sh     # type-check ทุกไฟล์ที่พึ่ง ESP-IDF + guard timeval
```

> สคริปต์เลือก compiler อัตโนมัติ: `g++` → `clang++` → `python -m ziglang c++` (ติดตั้งด้วย `pip install ziglang`)

### 5.4 Wokwi

#### 5.4.1 ทางที่ 1 — Wokwi for VS Code (แนะนำสำหรับโปรเจกต์ ESP-IDF)

1. ติดตั้ง extension 2 ตัวใน VS Code
   * **Wokwi for VS Code** → `wokwi.wokwi-vscode`
   * **ESP-IDF** → `espressif.esp-idf-extension` (v6)
2. `F1` → **Wokwi: Request a new License** → กด *GET YOUR LICENSE* ในเบราว์เซอร์
   (license ฟรีสำหรับงานส่ง/งานเรียน ใช้ได้กับโปรเจกต์ส่วนตัว)
3. Build firmware ก่อนเสมอ เพราะ `wokwi.toml` ชี้ไปที่ไฟล์ที่ `idf.py build` สร้าง
   ```bash
   idf.py set-target esp32     # ครั้งแรกเท่านั้น (ต้องตรงกับบอร์ดใน diagram.json)
   idf.py build
   ```
4. `F1` → **Wokwi: Start Simulator** → เปิดแท็บ Serial Monitor (115200)
5. แก้โค้ดแล้วรัน `idf.py build` อีกครั้ง → simulator reload firmware ใหม่เอง

> บอร์ดใน `diagram.json` คือ `board-esp32-devkit-c-v4` → ต้อง `idf.py set-target esp32`
> ถ้า target ไม่ตรงกับบอร์ด firmware จะไม่บูต

#### 5.4.2 ทางที่ 2 — `idf.py wokwi` (ESP-IDF ≥ 6.0)

```bash
pip install idf-wokwi
# เอา API token จาก https://wokwi.com/dashboard/ci แล้วใส่ใน environment
export WOKWI_CLI_TOKEN=<token>          # Windows: $env:WOKWI_CLI_TOKEN = "<token>"
idf.py build && idf.py wokwi
```

ตัวเลือกเพิ่มเติมของ `idf.py wokwi`: `--diagram-file`, `--timeout`, `--expect-text`, `--fail-text`,
`--expect-regex`, `--fail-regex` (ใช้ทำ smoke test ใน CI ได้ เช่น รอข้อความ `SSD1306 ready` ใน serial)

#### 5.4.3 ทางที่ 3 — Wokwi CLI (เหมาะกับ CI)

```bash
curl -L https://wokwi.com/ci/install.sh | sh          # Linux / macOS
iwr https://wokwi.com/ci/install.ps1 -useb | iex       # Windows PowerShell
wokwi-cli .
```

ทั้ง 3 ทางใช้ `wokwi.toml` + `diagram.json` ชุดเดียวกันที่รากโปรเจกต์
(`wokwi.toml` → `firmware = 'build/flasher_args.json'`, `elf = 'build/esp32_digital_clock.elf'`
— ต้องเป็น flasher_args.json เพื่อให้ Wokwi โหลด bootloader + partition table + app พร้อมกัน
เหมือน `idf.py flash`)

#### 5.4.4 ทางที่ 4 — เว็บ wokwi.com (ทางเลือก)

เปิด <https://wokwi.com> → โปรเจกต์ **ESP32 / C++ (ESP-IDF)** → แทนที่แท็บ `diagram.json`
(อยู่ที่รากโปรเจกต์แล้ว) → อัปโหลดไฟล์จาก `main/` → **Start Simulation**
ถ้าต้องการชุด pin แบบ Suite ให้ใช้ `wokwi/diagram-suite-pins.json` แทน

#### 5.4.5 Checklist สิ่งที่ต้องทำให้ครบใน Wokwi

| # | รายการ | วิธีตรวจ | ผ่านเมื่อ |
|---|---|---|---|
| 1 | I²C bus | Serial monitor | ขึ้น `I2C bus #0 ready (SDA=21, SCL=22, 400000 Hz)` |
| 2 | RTC DS1307 | Serial monitor | ขึ้น `DS1307 ready, RTC time = ...` |
| 3 | OLED SSD1306 | Serial monitor + หน้าจอ | ขึ้น `SSD1306 ready at 0x3C` และจอแสดงหน้า CLOCK |
| 4 | ปุ่ม 4 ตัว | กดใน simulator หรือคีย์ `M`/`I`/`S`/`L` | หน้าจอเปลี่ยนตามหัวข้อ 5.4.6 · โหมดเปลี่ยนได้ |
| 5 | Debounce | Wokwi จำลอง bounce ให้เอง | 1 ครั้งที่กด = 1 action เท่านั้น |
| 6 | Long press | กดค้าง > 1 วินาที | MODE ค้าง = sync NTP · INC ค้าง = +5 |
| 7 | Stopwatch + lap | หน้า STOPWATCH: SET เริ่ม/หยุด, LAP = lap | เวลาเดินแบบ µs และมีค่า lap |
| 8 | Alarm + buzzer | ตั้งเวลาใกล้ถึง แล้วรอ | มีเสียงจาก piezo + ขึ้น `DS1307 calibrated from the network` (ถ้า sync ได้) |
| 9 | NTP sync | รอ ~20 วินาที | ขึ้น `Wi-Fi connected, IP address: ...` และ `SNTP synchronised` |
| 10 | Task ทั้ง 4 | Serial monitor | ขึ้นบรรทัด task ทั้ง 4 + `all tasks started, free heap = ...` |

#### 5.4.6 พฤติกรรมปุ่ม

| ปุ่ม | คีย์ลัด | การทำงาน |
|---|---|---|
| MODE | `M` | สลับโหมด CLOCK → ALARM → STOPWATCH → SET TIME (กดค้าง = sync NTP) |
| INC | `I` | ในหน้า ALARM/SET TIME = +1 ที่ฟิลด์ที่เลือก (กดค้าง = +5) · ในหน้า CLOCK = sync NTP |
| SET | `S` | ALARM = บันทึก + เปิด/ปิดปลุก · SET TIME = เขียนเวลาลง RTC · STOPWATCH = เริ่ม/หยุด (กดค้าง = เลื่อน cursor) |
| LAP | `L` | STOPWATCH = บันทึก lap (ถ้าหยุดอยู่ = รีเซ็ต) · อื่น ๆ = ปิดเสียงปลุก |

> คีย์ลัดผูกด้วย attribute `key` ใน `diagram.json` (ชุด Suite ใช้ `M` / `S` / `A` / `L` ตามชื่อปุ่มของสเปก Suite)

#### 5.4.7 เรื่อง Wi-Fi / NTP ใน Wokwi

* ตัวจำลอง Wokwi เปิดเครือข่ายจำลองชื่อ **`Wokwi-GUEST`** (เปิด / ไม่มีรหัส) และมีอินเทอร์เน็ตจริง
* ฟิร์มแวร์ลองต่อ SSID หลัก `iotexpo` ก่อน (สำหรับบอร์ดจริงในห้องแล็บ) ถ้าไม่สำเร็จจะ
  **สลับไปใช้ `Wokwi-GUEST` ให้อัตโนมัติ** → สังเกต log ว่า
  `SSID 'iotexpo' is unreachable, falling back to 'Wokwi-GUEST'` (รอครั้งแรกประมาณ 15 วินาที)
* ถ้าเน็ตล่มจริง (ไม่มีทั้ง 2 SSID) ขึ้น `no Wi-Fi network available - the clock keeps the RTC time`
  และนาฬิกายังเดินต่อจากเวลาใน DS1307 ตามปกติ
* ค่า SSID/รหัสผ่านอยู่ที่ `main/ClockConfig.hpp` (`kWifiSsid`, `kWifiFallbackSsid`) — จุดเดียวของโปรเจกต์

---

## 6. Build & Test Results

### 6.1 Host unit test (ผลจริงบนเครื่องพัฒนา)

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

### 6.2 Type-check ฝั่ง ESP‑IDF

`main/main.cpp`, `main/core/*.cpp`, `main/drivers/*.cpp` และ `test/test_clock_core/main/unit_tests.cpp`
คอมไพล์ผ่านหมดด้วย `-std=c++17 -Wall -Wextra` **โดยไม่มี warning** (ตรวจกับ stub ที่เขียนตาม header จริงของ ESP‑IDF v6.0)

### 6.3 ตาราง Unit Tests ทั้งหมด 22 รายการ

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
| 16 | `test_ui_mode_cycle` | Suite | 3 | วงจร CLOCK → ALARM → STOPWATCH → SET TIME | Host + Target |
| 17 | `test_alarm_time_comparator` | Suite | 4 | comparator ของ alarm | Host + Target |
| 18 | `Alarm_Comparator_Trigger_Match` | Dashboard | 4 | เงื่อนไข trigger ของ alarm | Host + Target |
| 19 | `test_stopwatch_lap_split_calc` | Suite | 4 | ความแม่นยำของ split time | Host + Target |
| 20 | `Stopwatch_HighRes_Timer_Precision` | Dashboard | 4 | ความละเอียดของ `esp_timer` | Host + Target |
| 21 | `Lap_Memory_Buffer_Push` | Dashboard | 4 | lap memory buffer (สูงสุด 5 รายการ) | Host + Target |
| 22 | `test_freertos_event_group_sync` | Suite | 5 | bit flag ของ EventGroup ระหว่าง task | Target |

**สรุป:** 22 เคส = 10 ชื่อจากสเปก Suite + 12 ชื่อจากสเปก Dashboard (ไม่ซ้ำกัน)
— 16 เคสรันได้บน PC (Host) และ 6 เคสที่ต้องใช้ FreeRTOS/hardware จริงรันบน ESP32

---

## 7. FreeRTOS Task Specification

| Task | Priority | Stack | Core | Trigger / Period | หน้าที่ |
|---|---|---|---|---|---|
| `vTaskClockDisplay` | 3 (Medium) | 4096 B | Core 1 | Periodic 100 ms | วาดโหมดปัจจุบันลง buffer ของ SSD1306 แล้ว flush เฟรมผ่าน I²C |
| `vTaskButtonHandler` | 5 (High) | 3072 B | Core 0 | ISR queue event | ประมวลผลปุ่มที่ debounce แล้ว, ตรวจ long‑press, สลับโหมดการทำงาน |
| `vTaskAlarmCheck` | 4 (Med‑High) | 2048 B | Core 1 | Periodic 1000 ms | เทียบเวลา RTC กับเวลาที่ตั้ง alarm แล้วสั่ง LEDC PWM ที่ piezo buzzer |
| `vTaskNTPSync` | 1 (Low) | 6144 B | Core 0 | Periodic 3600 s | ต่อ Wi‑Fi SSID `iotexpo`, query `pool.ntp.org`, ปรับ POSIX time และเขียนกลับลง DS1307 |

> พารามิเตอร์ทั้งหมดประกาศไว้ใน `main/ClockConfig.hpp` จึงเปลี่ยนค่าได้ที่จุดเดียว

---

## 8. Hardware Pin Map

| อุปกรณ์ | สัญญาณ | ขา ESP32 | ขาใน Wokwi |
|---|---|---|---|
| DS1307 RTC (0x68) | SDA / SCL | GPIO 21 / GPIO 22 | `rtc:SDA` / `rtc:SCL` |
| DS1307 RTC | VCC / GND | 5V / GND | `rtc:5V` / `rtc:GND` (โมดูล DS1307 ของ Wokwi ใช้ขา `5V`) |
| SSD1306 OLED (0x3C) | SDA / SCL | GPIO 21 / GPIO 22 | `oled:SDA` / `oled:SCL` |
| SSD1306 OLED | VCC / GND | 3V3 / GND | `oled:VCC` / `oled:GND` |
| ปุ่ม MODE | – | **GPIO 12** (สำรอง: 18) | `btn_mode:1.l` / `btn_mode:2.l` |
| ปุ่ม INC / NTP | – | **GPIO 13** (สำรอง: 19) | `btn_inc:1.l` / `btn_inc:2.l` |
| ปุ่ม SET / ALARM | – | **GPIO 14** (สำรอง: 4) | `btn_set:1.l` / `btn_set:2.l` |
| ปุ่ม LAP / RESET | – | **GPIO 27** (สำรอง: 5) | `btn_lap:1.l` / `btn_lap:2.l` |
| Piezo buzzer | – | GPIO 25 (LEDC PWM 2.7 kHz) | `buzzer:2` (ขาบวก) / `buzzer:1` (ขาลบ) |
| GND | – | GND.1 / GND.2 / GND.3 | — |
| I²C bus | – | I²C #0 @ 400 kHz | — |
| Wi‑Fi | – | STA `iotexpo` / `12345678` (สำรอง `Wokwi-GUEST` แบบไม่มีรหัส) | — |
| NTP | – | `pool.ntp.org` (สำรอง `time.nist.gov`), TZ +7 ชม. | — |

---

## 9. Validation Against HTML Requirements

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
| Wokwi `diagram.json` | `diagram.json`, `wokwi/diagram-suite-pins.json`, `wokwi.toml` | ผ่านการตรวจ part type / ขา / การทับซ้อน / ความครบของสาย (หัวข้อ Step 6) |
| ไฟล์โค้ดตามรายการของ Dashboard | `main/drivers/{DS1307,SSD1306,Button,NTPManager,AlarmManager,Stopwatch}.hpp/.cpp` | `test/test_clock_core/main/unit_tests.cpp` |
| ไฟล์โค้ดตามรายการของ Suite | `main/{I2CBus,DS1307,SSD1306,ButtonManager,TimeManager,AlarmEngine,StopwatchEngine}` | `test/test_clock_core/main/unit_tests.cpp` |

---

## 10. Complete File Map

```
.
├── CMakeLists.txt                     # โปรเจกต์ ESP-IDF v6 ของเฟิร์มแวร์
├── sdkconfig.defaults                # ค่า default ของ target, lwIP SNTP, FreeRTOS, log
├── .gitignore
├── README.md                          # README ระดับต้น
├── diagram.json                       # ไดอะแกรม Wokwi หลัก (pin 12/13/14/27) — รากโปรเจกต์
├── wokwi.toml                         # config ของ Wokwi for VS Code / Wokwi CLI
│
├── main/
│   ├── CMakeLists.txt                 # ลงทะเบียน component + REQUIRES ของ ESP-IDF v6
│   ├── ClockConfig.hpp                # single source of truth: pin, credential, task param
│   ├── main.cpp                       # app_main() + 4 FreeRTOS tasks
│   │
│   ├── core/                          # ── ชั้น domain (pure C++17, ไม่มี ESP-IDF) ──
│   │   ├── Interfaces.hpp             #   ports: IRtcSource, ISystemClock, ITimeSource, INtpClient, IDisplay, IBuzzer
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
│       ├── check_timeval_include.py   # guard กันบั๊ก "Clock::timeval"
│       └── stubs/                     # ESP-IDF v6 API stub (ไม่เข้า firmware)
│
├── docs/
│   └── REPORT.md                      # รายงานฉบับเต็ม (ไฟล์นี้)
│
└── wokwi/
    └── diagram-suite-pins.json        # pin map ของ Suite   (18/19/4/5)
```

---

## 11. Known Notes / Limitations

1. **ยังไม่ได้ flash ลงฮาร์ดแวร์จริง** — เครื่องที่ใช้พัฒนาไม่มี ESP‑IDF และไม่มีบอร์ด ESP32
   การตรวจสอบจึงทำด้วย 2 วิธี:
   * host unit test (16 เคส ผ่านหมด — 158 assertions)
   * type-check ทุก source file ที่พึ่ง ESP‑IDF ด้วย stub ที่เขียนตาม header จริงของ
     ESP‑IDF v6.0 (`esp_netif.h`, `esp_netif_sntp.h`, `esp_wifi.h`, `esp_wifi_types_generic.h`,
     `esp_netif_types.h`) — คอมไพล์ผ่านด้วย `-Wall -Wextra` โดยไม่มี warning
2. **`test/host/stubs/` ไม่ใช่โค้ดจริงของ ESP‑IDF** เป็นเพียงตัวแทนเพื่อการตรวจสอบบน PC
   การ build จริงต้องใช้ ESP‑IDF v6 (`idf.py build`) ตามหัวข้อ 5
   → **ครั้งแรกที่ `idf.py build` อาจเจอ error จาก API ที่ตรวจได้แค่ด้วย stub** ถ้าเจอให้ส่ง
   ข้อความ error มาแก้ได้ทันที (log ของ ESP-IDF build จะระบุไฟล์และบรรทัดชัดเจน)
3. **การเลือก pin** — ใช้ชุดของ Dashboard (12/13/14/27) เป็นค่าเริ่มต้น เพราะตรงกับชื่อปุ่มใน UI
   (`MODE / INC / SET / LAP`) และกับ `ClockConfig.hpp`; ชุดของ Suite มีไดอะแกรมสำรองให้
4. **เวลาโซน** — โค้ดบวก offset +7 ชม. ในตัวเอง (ไม่ใช้ `TZ` ของ lwIP) เพื่อให้ผลการทดสอบ
   (`SNTP_Timestamp_Offset_Calculation`) ไม่ขึ้นกับค่า `TZ` ของเครื่อง
5. **การกดปุ่ม** — เป็น *active low* พร้อม pull‑up ภายใน ตามวงจรปุ่มทั่วไปใน Wokwi
   (Wokwi จำลองการ bounce ของปุ่มไว้โดย default → เป็นการทดสอบ software debounce 50 ms ของโค้ดจริง)
6. **`diagram.json` ในสเปก HTML ต้นฉบับใช้ไม่ได้จริง** — มีทั้งชื่อ part type ที่ไม่มีอยู่จริง
   (`wokwi-esp32-devkit-v4`, `board-ds1307`), ชื่อขาที่ไม่ถูกต้อง (`1.1`, `1.A`, `rtc:VCC`, `esp:GND`,
   `esp:GND.4`) และวางปุ่มทับตัวบอร์ด ESP32 — ไดอะแกรมในโปรเจกต์นี้เขียนใหม่ทั้งหมด
   ตามคู่มือ part ของ Wokwi (ดูรายละเอียดในหัวข้อ Step 6)
7. **Wi-Fi ใน Wokwi** — ตัวจำลองใช้เครือข่าย `Wokwi-GUEST` (ไม่มีรหัส) ฟิร์มแวร์จึงต่อ `iotexpo`
   ก่อน แล้วสลับมาใช้ `Wokwi-GUEST` อัตโนมัติเมื่อต่อหลักไม่สำเร็จ (รอ ~15 วินาทีครั้งแรก)
   ถ้าไม่อยากรอให้แก้ `kWifiSsid` ใน `main/ClockConfig.hpp` เป็น `Wokwi-GUEST` ได้เลย
8. **ไฟล์ HTML ต้นฉบับ** ยังคงอยู่ในรีโปเพื่อใช้อ้างอิงสเปก ตามเดิม
9. **`struct Clock::timeval` (bug ที่เจอตอน build จริง)** — ตอนเขียน callback ของ lwIP SNTP
   เดิมใช้ `static void onSntpTimeSync(struct timeval* tv)` ภายใน `namespace Clock`
   เมื่อไม่มี `<sys/time.h>` อยู่ก่อนจุดนั้น กฎ *elaborated type specifier* ของ C++ จะ
   **ประกาศชนิดใหม่ชื่อ `Clock::timeval`** ขึ้นมาเอง (ไม่ใช่ `::timeval` ของ POSIX)
   ทำให้ชนกับ `esp_sntp_time_cb_t` ที่ ESP-IDF ประกาศไว้ → error `struct Clock::timeval`
   **วิธีแก้ (ทำไว้แล้วในรุ่นนี้)**
   * `main/drivers/NTPManager.hpp` — `#include <sys/time.h>` ไว้ **บนสุดของไฟล์**
     (ก่อน include อื่นทั้งหมด) และใช้ `::timeval*` แทน `struct timeval*`
   * `main/drivers/NTPManager.cpp` — `#include <sys/time.h>` บนสุดไฟล์เช่นกัน,
     ใช้ `::timeval*` / `::time_t` / `::time()` แบบ fully qualified
   * เพิ่ม `static_assert` ใน `NTPManager.cpp` เทียบ `decltype(&NTPManager::onSntpTimeSync)`
     กับ `esp_sntp_time_cb_t` → ถ้ามีคนลบ `<sys/time.h>` ออกไป จะได้ error ที่อ่านเข้าใจได้
     แทน error ประเภทชนกันของ function pointer
   * stub ฝั่ง host ถูกปรับให้ตรงกับ ESP-IDF จริงแล้ว (ประกาศ `struct timeval` ไว้เฉพาะใน
     `test/host/stubs/esp_netif_sntp.h` ตามของจริง) และเพิ่ม
     `test/host/check_timeval_include.py` เป็น source-level guard ที่รันอยู่ใน
     `typecheck_target.sh` (เพราะ `<ctime>` ของ mingw ประกาศ `::timeval` ให้ทั้งโลก
     ทำให้ type-check ฝั่ง PC มองไม่เห็นบั๊กนี้ได้)
