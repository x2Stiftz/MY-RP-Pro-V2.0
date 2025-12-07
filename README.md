# 🤖 MyRobot_Complete - Arduino Robot Library

ไลบรารี่สำหรับหุ่นยนต์เดินตามเส้น พร้อมระบบ Gyro, PID Control และฟังก์ชันครบครัน

---

## 📋 สารบัญ

- [คุณสมบัติ](#-คุณสมบัติ)
- [ฮาร์ดแวร์ที่รองรับ](#-ฮาร์ดแวร์ที่รองรับ)
- [โครงสร้างไฟล์](#-โครงสร้างไฟล์)
- [การติดตั้ง](#-การติดตั้ง)
- [เริ่มต้นใช้งาน](#-เริ่มต้นใช้งาน)
- [คำสั่งทั้งหมด](#-คำสั่งทั้งหมด)
- [การจูนค่า](#-การจูนค่า)
- [ตัวอย่างโปรแกรม](#-ตัวอย่างโปรแกรม)

---

## ✨ คุณสมบัติ

- ✅ **เดินตามเส้น** - PID Control พร้อมเซ็นเซอร์หน้า/หลัง
- ✅ **Gyro Control** - เดินตรง, หมุนตัวตามองศา
- ✅ **Motor Tuning** - ระบบ offset อัตโนมัติตาม speed
- ✅ **PID Tuning** - ตารางค่า PID ตาม speed
- ✅ **Calibration** - ระบบวัดค่าเซ็นเซอร์อัตโนมัติ
- ✅ **แขนกล** - ควบคุม Servo 6 ตัว
- ✅ **EEPROM** - บันทึก/โหลดค่า Calibration

---

## 🔧 ฮาร์ดแวร์ที่รองรับ

| Component | รุ่น/สเปค |
|-----------|----------|
| **Board** | RP2040 / RP2350 Pro |
| **Gyro** | BMI160 (I2C 0x69) |
| **Motor Driver** | PWM 12-bit, 2 ช่อง |
| **Sensor** | MCP3008 ADC x2 (16 ช่อง) |
| **Servo** | 6 ตัว (pin 34-39) |
| **EEPROM** | CAT24C256 (I2C 0x50) |
| **Button** | 2 ปุ่ม |
| **LED RGB** | 1 ชุด |
| **Buzzer** | 1 ตัว |

---

## 📁 โครงสร้างไฟล์

```
MyRobot_Complete/
├── MyRobot_Complete.ino   # ไฟล์หลัก + Setup
├── Motor.ino              # ควบคุมมอเตอร์
├── Gyro.ino               # อ่านค่า Gyro BMI160
├── Sensor.ino             # อ่านค่าเซ็นเซอร์ MCP3008
├── ForwardBackward.ino    # เดินหน้า/ถอยหลัง (Gyro)
├── TracDegree.ino         # หมุนตัว + เดินด้วย Gyro
├── Trac.ino               # เดินตามเส้น TracJC
├── FLine.ino              # fline/bline Advanced
├── Turn.ino               # เลี้ยวซ้าย/ขวา
├── Servo.ino              # ควบคุม Servo แขนกล
├── Calibrate.ino          # Calibration เซ็นเซอร์
├── Initial.ino            # ตั้งค่า/จูนค่าทั้งหมด
├── Utility.ino            # LED, Buzzer, Button
├── CommandReference.ino   # รวมคำสั่งทั้งหมด
└── README.md              # ไฟล์นี้
```

---

## 📥 การติดตั้ง

1. **ดาวน์โหลด** ไฟล์ `MyRobot_Complete.zip`
2. **แตกไฟล์** ไปยังโฟลเดอร์ Arduino
3. **เปิด** `MyRobot_Complete.ino` ด้วย Arduino IDE
4. **เลือก Board** ที่ถูกต้อง (RP2040/RP2350)
5. **Upload** โค้ดลงบอร์ด

---

## 🚀 เริ่มต้นใช้งาน

### โครงสร้างโปรแกรมพื้นฐาน

```cpp
void setup() {
  Serial.begin(115200);
  setup_robot();        // ตั้งค่าฮาร์ดแวร์
  initTuning();         // โหลดค่า Tuning
  calibrateGyro();      // Calibrate Gyro
  arm_down_open();      // เตรียมแขน
  
  sw();                 // ⏸️ รอกดปุ่มเริ่ม
  
  // ===== โค้ดภารกิจ =====
  Forward(50, 1.5, 500);
  TurnLeft();
  TracJC();
  // ====================
  
  Finish();             // 🏁 จบการทำงาน
}

void loop() {
  // ว่างไว้
}
```

### ลำดับการทำงาน

```
┌─────────────────────────────────┐
│  1. setup_robot()               │  ← ตั้งค่าฮาร์ดแวร์
├─────────────────────────────────┤
│  2. initTuning()                │  ← โหลดค่า Tuning
├─────────────────────────────────┤
│  3. calibrateGyro()             │  ← Calibrate Gyro
├─────────────────────────────────┤
│  4. sw()                        │  ← ⏸️ รอกดปุ่ม (ไฟเขียว)
├─────────────────────────────────┤
│  5. โค้ดภารกิจ                  │  ← 🏃 ทำงาน (ไฟน้ำเงิน)
├─────────────────────────────────┤
│  6. Finish()                    │  ← 🏁 จบ (ไฟแดง + บี๊บ)
└─────────────────────────────────┘
```

---

## 📖 คำสั่งทั้งหมด

### 🏃 เดินหน้า/ถอยหลัง (Gyro)

```cpp
// เดินหน้าตามเวลา
Forward(speed, kp, delayMs);
Forward(50, 1.5, 500);            // เดินหน้า speed=50, 500ms
Forward(40, 1.2, 1000);           // เดินหน้า speed=40, 1000ms

// ถอยหลังตามเวลา
Backward(speed, kp, delayMs);
Backward(50, 1.5, 500);           // ถอยหลัง speed=50, 500ms

// เดินไม่จำกัดเวลา
Forward(50);                      // เดินหน้าไปเรื่อยๆ
Backward(40);                     // ถอยหลังไปเรื่อยๆ

// เดินจนเจอเซ็นเซอร์
ForwardUntil(50, 1.5, "a0");      // เดินจนเจอเซ็นเซอร์ a0
ForwardUntil(50, 1.5, "a7");      // เดินจนเจอเซ็นเซอร์ a7
ForwardUntil(50, 1.5, "a07");     // เดินจนเจอทั้ง a0 และ a7
BackwardUntil(40, 1.2, "b0");     // ถอยจนเจอเซ็นเซอร์ b0
BackwardUntil(40, 1.2, "b07");    // ถอยจนเจอทั้ง b0 และ b7
```

### 🔄 หมุนตัว (Gyro)

```cpp
// หมุนตัวตามองศา
SpinDegree(speed, degree, mode);
//   Mode 0 = Relative (หมุนจากตำแหน่งปัจจุบัน)
//   Mode 1 = Absolute (หมุนไปทิศที่กำหนด)

SpinDegree(30, 90, 0);            // หมุนขวา 90°
SpinDegree(30, -90, 0);           // หมุนซ้าย 90°
SpinDegree(30, 180, 0);           // หมุน 180° (กลับหลัง)
SpinDegree(30, 45, 0);            // หมุนขวา 45°

// ระบบทิศ Absolute
SetHeading(0);                    // ตั้งทิศหน้าเป็น 0°
TurnToHeading(30, 90);            // หมุนไปทิศขวา (90°)
TurnToHeading(30, 180);           // หมุนไปทิศหลัง (180°)
TurnToHeading(30, 270);           // หมุนไปทิศซ้าย (270°)

// ฟังก์ชันลัด
TurnToFront(30);                  // หมุนไปหน้า
TurnToBack(30);                   // หมุนไปหลัง
TurnToLeft(30);                   // หมุนไปซ้าย
TurnToRight(30);                  // หมุนไปขวา
```

### ↪️ เลี้ยวซ้าย/ขวา (เซ็นเซอร์)

```cpp
// เลี้ยวตามเวลา
Left(speed, time);                // เลี้ยวซ้ายตามเวลา (ms)
Right(speed, time);               // เลี้ยวขวาตามเวลา (ms)
Left(50, 200);                    // เลี้ยวซ้าย 200ms
Right(60, 300);                   // เลี้ยวขวา 300ms

// เลี้ยวจนเจอเส้น
TurnLeft();                       // เลี้ยวซ้ายจนเจอเส้น
TurnRight();                      // เลี้ยวขวาจนเจอเส้น
TurnLeft(60);                     // เลี้ยวซ้าย speed=60
TurnRight(50);                    // เลี้ยวขวา speed=50

// กลับตัว 180°
UTurnLeft();                      // กลับตัวซ้าย
UTurnRight();                     // กลับตัวขวา
UTurnLeft(60);                    // กลับตัวซ้าย speed=60

// เลี้ยวจนเจอเซ็นเซอร์ที่กำหนด
TurnLeftSensor(50, 3);            // เลี้ยวซ้ายจนเจอเซ็นเซอร์ 3
TurnRightSensor(50, 4);           // เลี้ยวขวาจนเจอเซ็นเซอร์ 4

// เลี้ยวจนเจอเส้น (ใช้เซ็นเซอร์หลัง)
TurnLeftBack(50);
TurnRightBack(50);

// โค้งอ้อมสิ่งกีดขวาง
CurveLeft(50, 100);
CurveRight(50, 100);
```

### 📍 เดินตามเส้น

```cpp
// เดินจนเจอแยก (เซ็นเซอร์หน้า)
TracJC();                         // เดินจนเจอแยก
TracJCSpeed();                    // เดินเร็วจนเจอแยก
TracJCSlow();                     // เดินช้าจนเจอแยก
TracFront();                      // เดินจนเจอแยก (ใช้ PID จากตาราง)
TracFront(50);                    // เดินจนเจอแยก speed=50

// เดินจนเจอแยก (เซ็นเซอร์หลัง)
TracBack();                       // ถอยจนเจอแยก
TracBack(40);                     // ถอยจนเจอแยก speed=40

// เดินผ่านหลายแยก
TJCSS(3);                         // เดินผ่าน 3 แยก
TJCSSL(2);                        // เดินผ่าน 2 แยก แล้วช้าลง
TracBackJC(3);                    // ถอยหลังผ่าน 3 แยก
```

### 🛤️ fline/bline (Advanced)

```cpp
fline(speed, kp, distance, stopMode, turnDir, power, sensor, brakeDelay);
bline(speed, kp, distance, stopMode, turnDir, power, sensor, brakeDelay);

/*
 * Parameters:
 *   speed     : ความเร็ว (ตัวเดียว)
 *   kp        : ค่า Kp (แนะนำ 0.45)
 *   distance  : ระยะทาง (cm) หรือ 0 หรือ "a0","a7"
 *   stopMode  : 'n'=ปกติ, 'f'=เจอแยกผ่าน, 'c'=เซ็นเซอร์กลาง
 *   turnDir   : 'p'=ผ่านไป, 's'=หยุด, 'l'=ซ้าย, 'r'=ขวา
 *   power     : กำลังมอเตอร์เลี้ยว (%)
 *   sensor    : เซ็นเซอร์เป้าหมาย "a3", "b4"
 *   brakeDelay: เวลาเบรค (ms)
 */

// ตัวอย่าง
fline(50, 0.45, 0, 'n', 'p', 100, "a3", 0);    // วิ่งจนเจอแยกผ่านไป
fline(50, 0.45, 20, 'n', 's', 100, "a3", 10);  // วิ่ง 20cm แล้วหยุด
fline(50, 0.45, 0, 'f', 'l', 80, "a3", 0);     // วิ่งเจอแยกเลี้ยวซ้าย
fline(50, 0.45, 0, 'f', 'r', 80, "a4", 0);     // วิ่งเจอแยกเลี้ยวขวา
fline(50, 0.45, "a0", 'n', 's', 100, "a3", 10); // วิ่งจนเจอ a0 หยุด

bline(40, 0.45, 0, 'n', 'p', 100, "b3", 0);    // ถอยจนเจอแยกผ่านไป
bline(40, 0.45, 0, 'n', 's', 100, "b3", 10);   // ถอยจนเจอแยกหยุด
```

### 🦾 แขนกล (Servo)

```cpp
// ควบคุม Servo
servo_write(servoNum, angle);     // ควบคุม servo (0-5)
servo_write(0, 90);               // servo 0 ไป 90°

// แขนกล
arm_down();                       // แขนลง
arm_up();                         // แขนขึ้น
arm_down_open();                  // แขนลง + กรงเล็บเปิด
arm_down_close();                 // แขนลง + กรงเล็บปิด
arm_up_open();                    // แขนขึ้น + กรงเล็บเปิด
arm_up_close();                   // แขนขึ้น + กรงเล็บปิด
arm_open();                       // กรงเล็บเปิด
arm_close();                      // กรงเล็บปิด
arm_l_r(120, 120, 50);            // กำหนดมุมกรงเล็บ
```

### ⏸️ ปุ่ม/Utility

```cpp
sw();                             // รอกดปุ่มเริ่ม
sw1();                            // รอกดปุ่ม 1
sw2();                            // รอกดปุ่ม 2
Finish();                         // จบการทำงาน

// LED
led_rgb('r');                     // ไฟแดง
led_rgb('g');                     // ไฟเขียว
led_rgb('b');                     // ไฟน้ำเงิน
led_rgb('y');                     // ไฟเหลือง
led_rgb('w');                     // ไฟขาว
led_off();                        // ปิดไฟ

// Buzzer
beep(2000, 100);                  // บี๊บ 2000Hz 100ms
beep(3000, 500);                  // บี๊บ 3000Hz 500ms
```

### 🔧 Calibration

```cpp
calibrate_auto();                 // Calibrate อัตโนมัติ (หมุนตัว)
calibrate_manual();               // Calibrate แบบ Manual
calibrate_front();                // Calibrate เซ็นเซอร์หน้า
calibrate_back();                 // Calibrate เซ็นเซอร์หลัง
calibrate_center();               // Calibrate เซ็นเซอร์กลาง

save_calibration();               // บันทึกลง EEPROM
load_calibration();               // โหลดจาก EEPROM
print_calibration();              // แสดงค่า
show_sensors_live();              // แสดงค่า Real-time
test_sensors();                   // ทดสอบเซ็นเซอร์

// ตั้งค่าเอง
set_sensor_A(0, 150, 850);        // เซ็นเซอร์หน้า 0
set_sensor_B(0, 160, 840);        // เซ็นเซอร์หลัง 0
set_sensor_C(0, 500, 3500);       // เซ็นเซอร์กลาง 0
```

### ⚙️ Tuning/Settings

```cpp
initTuning();                     // โหลดค่า Tuning
calibrateGyro();                  // Calibrate Gyro
printTuningValues();              // แสดงค่า Tuning ทั้งหมด

// ทดสอบ
testPID_Forward(50, 3000);        // ทดสอบ PID 3 วินาที
testMotorOffset(50, 2000);        // ทดสอบ offset 2 วินาที

// ตั้งค่า
setTurnSpeed(50);                 // ความเร็วเลี้ยว
setBrake(100, 30);                // เบรค
setTurnDelay90(8000);             // delay สำหรับ 90°
setWheelDrive(0);                 // 0=2WD, 1=4WD
```

---

## 🎯 การจูนค่า

### 1️⃣ PID คืออะไร?

| ค่า | ความหมาย | ถ้าเพิ่ม | ถ้าลด |
|-----|---------|---------|------|
| **Kp** | แก้ตามขนาด error | หุ่นตอบสนองไว | หุ่นเบี้ยว |
| **Ki** | แก้ error สะสม | แก้เบี้ยวช้าๆ | - |
| **Kd** | ป้องกันแกว่ง | หยุดนิ่งกว่า | แกว่งมาก |

### 2️⃣ วิธีจูน PID

1. เริ่มจาก **Kp** ก่อน (ตั้ง Ki=0, Kd=0)
   - ถ้าหุ่นเบี้ยว = เพิ่ม Kp
   - ถ้าหุ่นแกว่งซ้าย-ขวา = ลด Kp

2. เพิ่ม **Kd**
   - ถ้าหุ่นแกว่ง = เพิ่ม Kd
   - ถ้าหุ่นตอบสนองช้า = ลด Kd

3. เพิ่ม **Ki** (ถ้าจำเป็น)
   - ถ้าหุ่นเบี้ยวเล็กน้อยตลอด = เพิ่ม Ki

### 3️⃣ แก้ไขค่า PID ตาม Speed

แก้ไขใน `Initial.ino` → `getPID_Forward()`:

```cpp
void getPID_Forward(int speed, float &kp, float &ki, float &kd) {
  if (speed <= 50) {
    kp = 0.45;    // ← แก้ตรงนี้
    ki = 0.0001;
    kd = 0.025;
  }
  // ...
}
```

### 4️⃣ Motor Offset (ให้หุ่นเดินตรง)

แก้ไขใน `Initial.ino` → `getMotorTuning()`:

```cpp
void getMotorTuning(int speed, int &offsetL, int &offsetR) {
  if (speed <= 50) {
    offsetL = 0;   // ← ถ้าเบี้ยวซ้าย เพิ่มค่านี้
    offsetR = 2;   // ← ถ้าเบี้ยวขวา เพิ่มค่านี้
  }
}
```

### 5️⃣ Gyro Settings

แก้ไขใน `Initial.ino`:

```cpp
float GYRO_KP = 1.5;          // ความไวในการแก้ทิศ
float GYRO_KD = 0.5;          // ป้องกันการแกว่ง
float SPIN_TOLERANCE = 2.0;   // องศาที่ยอมรับได้ (±)
```

| ปัญหา | วิธีแก้ |
|-------|--------|
| หมุนเกิน | เพิ่ม SPIN_TOLERANCE หรือ GYRO_KD |
| หมุนไม่ถึง | ลด SPIN_TOLERANCE |
| แกว่งตอนหมุน | เพิ่ม GYRO_KD |

### 6️⃣ ขั้นตอนการจูน

```
1️⃣ จูน Motor Offset (ให้หุ่นเดินตรง)
   → testMotorOffset(50, 2000);
   → เบี้ยวซ้าย = เพิ่ม offsetL
   → เบี้ยวขวา = เพิ่ม offsetR

2️⃣ จูน PID (ให้หุ่นเกาะเส้น)
   → testPID_Forward(50, 3000);
   → แกว่ง = ลด Kp หรือ เพิ่ม Kd
   → เบี้ยว = เพิ่ม Kp

3️⃣ จูน Gyro (ให้หมุนตรง)
   → calibrateGyro();
   → SpinDegree(30, 90, 0);
   → หมุนเกิน = เพิ่ม SPIN_TOLERANCE

4️⃣ จูน Turn (ให้เลี้ยวเจอเส้น)
   → TurnLeft(); / TurnRight();
   → ไม่ถึง 90° = เพิ่ม TURN_DELAY_90
```

---

## 📝 ตัวอย่างโปรแกรม

### ตัวอย่าง 1: ภารกิจพื้นฐาน

```cpp
void setup() {
  Serial.begin(115200);
  setup_robot();
  initTuning();
  calibrateGyro();
  arm_down_open();
  
  sw();
  
  // เดินหน้า → เลี้ยวซ้าย → เดินจนเจอแยก
  Forward(50, 1.5, 500);
  TurnLeft();
  TracJC();
  
  // หยิบของ
  arm_close();
  delay(200);
  arm_up_close();
  
  // ถอยหลัง → กลับตัว
  Backward(40, 1.2, 300);
  UTurnLeft();
  
  // เดินกลับ → วางของ
  TracJC();
  arm_down_open();
  
  Finish();
}

void loop() {}
```

### ตัวอย่าง 2: ใช้ Gyro หมุนตัว

```cpp
void setup() {
  Serial.begin(115200);
  setup_robot();
  initTuning();
  calibrateGyro();
  
  SetHeading(0);  // ตั้งทิศหน้าเป็น 0°
  
  sw();
  
  Forward(50, 1.5, 1000);         // เดินหน้า
  SpinDegree(30, 90, 0);          // หมุนขวา 90°
  Forward(50, 1.5, 1000);         // เดินหน้า
  SpinDegree(30, 90, 0);          // หมุนขวา 90°
  Forward(50, 1.5, 1000);         // เดินหน้า
  TurnToFront(30);                // หมุนกลับทิศหน้า
  
  Finish();
}
```

### ตัวอย่าง 3: ใช้ fline Advanced

```cpp
void setup() {
  Serial.begin(115200);
  setup_robot();
  initTuning();
  
  sw();
  
  // เดินจนเจอแยก เลี้ยวซ้าย
  fline(50, 0.45, 0, 'f', 'l', 80, "a3", 0);
  
  // เดินจนเจอแยก เลี้ยวขวา
  fline(50, 0.45, 0, 'f', 'r', 80, "a4", 0);
  
  // เดินจนเจอแยก ผ่านไป
  fline(50, 0.45, 0, 'n', 'p', 100, "a3", 0);
  
  // เดิน 30cm แล้วหยุด
  fline(50, 0.45, 30, 'n', 's', 100, "a3", 10);
  
  Finish();
}
```

### ตัวอย่าง 4: Calibration

```cpp
void setup() {
  Serial.begin(115200);
  setup_robot();
  
  // เลือกวิธี Calibrate
  calibrate_auto();    // หมุนตัวอัตโนมัติ
  // หรือ
  // calibrate_manual();  // กดปุ่มวัดขาว/ดำ
  
  // แสดงค่า
  print_calibration();
  
  sw();
  
  // ทดสอบเซ็นเซอร์
  test_sensors();
}
```

---

## 🔍 Quick Reference

| หมวด | คำสั่ง |
|------|--------|
| **เดินหน้า** | `Forward(speed, kp, delayMs)` |
| **ถอยหลัง** | `Backward(speed, kp, delayMs)` |
| **หมุน Gyro** | `SpinDegree(speed, degree, mode)` |
| **เลี้ยว** | `TurnLeft()`, `TurnRight()` |
| **กลับตัว** | `UTurnLeft()`, `UTurnRight()` |
| **เดินตามเส้น** | `TracJC()`, `TracBack()` |
| **ผ่านหลายแยก** | `TJCSS(n)`, `TracBackJC(n)` |
| **แขนกล** | `arm_down_open()`, `arm_close()` |
| **รอปุ่ม** | `sw()` |
| **จบ** | `Finish()` |

---

## 📄 License

MIT License - ใช้งานได้อิสระ

---

## 👨‍💻 Author

Created with ❤️ for Robot Competition

Cr: [My Makers RP-PRO V.2.0](https://www.mymakers.online/myrpprov2.php)
Official: https://github.com/nui4328/My_RP_Bot/tree/main/libraries/MyRP_ProV2
---

*Last Updated: December 2025*
