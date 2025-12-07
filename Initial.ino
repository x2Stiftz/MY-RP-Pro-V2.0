/*
 * =============================================================================
 *  Initial.ino - ตั้งค่าและจูนค่าต่างๆ ของหุ่นยนต์
 * =============================================================================
 *  
 *  ไฟล์นี้รวมการตั้งค่าทั้งหมด:
 *    1. PID Tuning ตาม Speed (หน้า/หลัง)
 *    2. Motor Offset Tuning
 *    3. Gyro Settings
 *    4. Turn Settings
 *    5. Servo Settings
 *    6. Sensor Settings
 *  
 *  วิธีใช้:
 *    - แก้ไขค่าในไฟล์นี้ตามที่ต้องการ
 *    - เรียก initTuning() ใน setup()
 * 
 * =============================================================================
 *                        📖 คู่มือการจูนค่า
 * =============================================================================
 * 
 *  🔧 PID คืออะไร?
 *  ---------------
 *  P (Proportional) = แก้ไขตามขนาด error (ยิ่ง error มาก ยิ่งแก้มาก)
 *  I (Integral)     = แก้ไข error สะสม (ถ้าเบี้ยวนานๆ จะค่อยๆ แก้)
 *  D (Derivative)   = แก้ไขตามความเร็วของ error (ป้องกันการแกว่ง)
 * 
 *  🎯 วิธีจูน PID:
 *  ---------------
 *  1. เริ่มจาก Kp ก่อน (ตั้ง Ki=0, Kd=0)
 *     - ถ้าหุ่นเบี้ยว = เพิ่ม Kp
 *     - ถ้าหุ่นแกว่งซ้าย-ขวา = ลด Kp
 *  
 *  2. เพิ่ม Kd
 *     - ถ้าหุ่นแกว่ง = เพิ่ม Kd
 *     - ถ้าหุ่นตอบสนองช้า = ลด Kd
 *  
 *  3. เพิ่ม Ki (ถ้าจำเป็น)
 *     - ถ้าหุ่นเบี้ยวเล็กน้อยตลอด = เพิ่ม Ki
 *     - ถ้าหุ่นแกว่งช้าๆ = ลด Ki
 * 
 *  ⚡ ความเร็วกับ PID:
 *  -------------------
 *  - Speed ต่ำ (20-40)  : ใช้ Kp ต่ำ, Kd ต่ำ
 *  - Speed กลาง (50-70) : ใช้ Kp กลาง, Kd กลาง
 *  - Speed สูง (80-100) : ใช้ Kp สูง, Kd สูง
 * 
 *  🔄 Motor Offset:
 *  ----------------
 *  ถ้าหุ่นเดินตรงแล้วเบี้ยวไปทางใดทางหนึ่ง:
 *  - เบี้ยวซ้าย = เพิ่ม offsetL (ลดความเร็วล้อซ้าย)
 *  - เบี้ยวขวา = เพิ่ม offsetR (ลดความเร็วล้อขวา)
 * 
 * =============================================================================
 */

// =============================================================================
//  1. PID TUNING ตาม SPEED - เซ็นเซอร์หน้า (Forward)
// =============================================================================
/*
 *  ตารางนี้กำหนดค่า Kp, Ki, Kd สำหรับแต่ละ Speed
 *  เมื่อเรียก Forward(speed, ...) จะดึงค่าจากตารางนี้อัตโนมัติ
 * 
 *  วิธีแก้ไข:
 *    1. หา speed ที่ต้องการจูน
 *    2. แก้ไขค่า kp, ki, kd
 *    3. ทดสอบ แล้ววนซ้ำ
 */

void getPID_Forward(int speed, float &kp, float &ki, float &kd) {
  
  // ===== Speed 20-30 (ช้ามาก) =====
  if (speed <= 30) {
    kp = 0.30;
    ki = 0.0001;
    kd = 0.015;
  }
  
  // ===== Speed 31-40 (ช้า) =====
  else if (speed <= 40) {
    kp = 0.35;
    ki = 0.0001;
    kd = 0.020;
  }
  
  // ===== Speed 41-50 (ปานกลาง) =====
  else if (speed <= 50) {
    kp = 0.45;
    ki = 0.0001;
    kd = 0.025;
  }
  
  // ===== Speed 51-60 (ค่อนข้างเร็ว) =====
  else if (speed <= 60) {
    kp = 0.55;
    ki = 0.0001;
    kd = 0.030;
  }
  
  // ===== Speed 61-70 (เร็ว) =====
  else if (speed <= 70) {
    kp = 0.65;
    ki = 0.0001;
    kd = 0.035;
  }
  
  // ===== Speed 71-80 (เร็วมาก) =====
  else if (speed <= 80) {
    kp = 0.75;
    ki = 0.0001;
    kd = 0.040;
  }
  
  // ===== Speed 81-90 (เร็วสุด) =====
  else if (speed <= 90) {
    kp = 0.85;
    ki = 0.0001;
    kd = 0.045;
  }
  
  // ===== Speed 91-100 (Maximum) =====
  else {
    kp = 1.00;
    ki = 0.0001;
    kd = 0.050;
  }
}


// =============================================================================
//  2. PID TUNING ตาม SPEED - เซ็นเซอร์หลัง (Backward)
// =============================================================================
/*
 *  ค่า PID สำหรับถอยหลัง อาจต่างจากเดินหน้า
 *  เพราะเซ็นเซอร์หลังอยู่คนละตำแหน่ง
 */

void getPID_Backward(int speed, float &kp, float &ki, float &kd) {
  
  // ===== Speed 20-30 =====
  if (speed <= 30) {
    kp = 0.35;
    ki = 0.0001;
    kd = 0.018;
  }
  
  // ===== Speed 31-40 =====
  else if (speed <= 40) {
    kp = 0.40;
    ki = 0.0001;
    kd = 0.022;
  }
  
  // ===== Speed 41-50 =====
  else if (speed <= 50) {
    kp = 0.50;
    ki = 0.0001;
    kd = 0.028;
  }
  
  // ===== Speed 51-60 =====
  else if (speed <= 60) {
    kp = 0.60;
    ki = 0.0001;
    kd = 0.033;
  }
  
  // ===== Speed 61-70 =====
  else if (speed <= 70) {
    kp = 0.70;
    ki = 0.0001;
    kd = 0.038;
  }
  
  // ===== Speed 71-80 =====
  else if (speed <= 80) {
    kp = 0.80;
    ki = 0.0001;
    kd = 0.043;
  }
  
  // ===== Speed 81-100 =====
  else {
    kp = 0.90;
    ki = 0.0001;
    kd = 0.050;
  }
}


// =============================================================================
//  3. MOTOR OFFSET TUNING ตาม SPEED
// =============================================================================
/*
 *  ถ้าหุ่นเดินตรงแล้วเบี้ยว ให้ปรับค่านี้:
 *  - เบี้ยวซ้าย = เพิ่ม offsetL
 *  - เบี้ยวขวา = เพิ่ม offsetR
 * 
 *  ค่า offset จะถูกลบออกจาก speed ของล้อนั้น
 *  เช่น speed=50, offsetL=2 → ล้อซ้ายจะวิ่ง 48
 */

void getMotorTuning(int speed, int &offsetL, int &offsetR) {
  
  // ===== Speed 20-30 =====
  if (speed <= 30) {
    offsetL = 0;
    offsetR = 0;
  }
  
  // ===== Speed 31-40 =====
  else if (speed <= 40) {
    offsetL = 0;
    offsetR = 1;
  }
  
  // ===== Speed 41-50 =====
  else if (speed <= 50) {
    offsetL = 0;
    offsetR = 2;
  }
  
  // ===== Speed 51-60 =====
  else if (speed <= 60) {
    offsetL = 0;
    offsetR = 2;
  }
  
  // ===== Speed 61-70 =====
  else if (speed <= 70) {
    offsetL = 0;
    offsetR = 3;
  }
  
  // ===== Speed 71-80 =====
  else if (speed <= 80) {
    offsetL = 0;
    offsetR = 3;
  }
  
  // ===== Speed 81-90 =====
  else if (speed <= 90) {
    offsetL = 0;
    offsetR = 4;
  }
  
  // ===== Speed 91-100 =====
  else {
    offsetL = 0;
    offsetR = 5;
  }
}


// =============================================================================
//  4. GYRO SETTINGS
// =============================================================================
/*
 *  📐 Gyro ใช้สำหรับ:
 *  - เดินตรงโดยไม่ใช้เซ็นเซอร์เส้น
 *  - หมุนตัวตามองศา
 *  - รักษาทิศทาง
 * 
 *  🔧 วิธีจูน Gyro PID:
 *  -------------------
 *  GYRO_KP: ความไวในการแก้ทิศ
 *    - ถ้าหุ่นหมุนเบี้ยว = เพิ่ม GYRO_KP
 *    - ถ้าหุ่นแกว่งซ้าย-ขวา = ลด GYRO_KP
 * 
 *  GYRO_KD: ป้องกันการแกว่ง
 *    - ถ้าหุ่นแกว่งตอนหยุด = เพิ่ม GYRO_KD
 * 
 *  GYRO_KI: แก้ไข error สะสม
 *    - ปกติตั้งไว้น้อยมากหรือ 0
 * 
 *  🎯 SPIN Settings (หมุนตัว):
 *  ---------------------------
 *  SPIN_TOLERANCE: องศาที่ยอมรับได้ (±)
 *    - ค่าน้อย = แม่นยำแต่ช้า
 *    - ค่ามาก = เร็วแต่ไม่แม่น
 */

// ----- Gyro PID (ประกาศใน MyRobot_Complete.ino) -----
// ใช้ extern เพื่ออ้างถึงตัวแปรที่ประกาศไว้แล้ว
extern float GYRO_KP;
extern float GYRO_KI;
extern float GYRO_KD;

// ----- Spin Settings -----
float SPIN_TOLERANCE = 2.0;   // องศาที่ยอมรับได้ (±) (แนะนำ: 1.0 - 3.0)
int SPIN_MIN_SPEED = 15;      // ความเร็วต่ำสุดตอนหมุน (แนะนำ: 10 - 20)
int SPIN_BRAKE_TIME = 10;     // เวลาเบรคหลังหมุน (ms)

// ----- Gyro Offset -----
float GYRO_OFFSET_X = 0.0;    // ค่า offset แกน X (ปรับถ้า drift)
float GYRO_OFFSET_Y = 0.0;    // ค่า offset แกน Y
float GYRO_OFFSET_Z = 0.0;    // ค่า offset แกน Z

// ----- Heading -----
float currentHeading = 0.0;   // มุมปัจจุบัน
float targetHeading = 0.0;    // มุมเป้าหมาย


// =============================================================================
//  5. TURN SETTINGS
// =============================================================================
/*
 *  🔄 การเลี้ยว:
 *  -------------
 *  TurnSpeed: ความเร็วเลี้ยวเริ่มต้น
 *  BrakeSpeed: ความเร็วเบรค (หลังเลี้ยวเสร็จ)
 *  BrakeTime: เวลาเบรค (ms)
 * 
 *  📏 Turn Delay:
 *  --------------
 *  TURN_DELAY_90: delay สำหรับหมุน 90° = TURN_DELAY_90 / speed
 *  TURN_DELAY_180: delay สำหรับหมุน 180° = TURN_DELAY_180 / speed
 * 
 *  ถ้าหมุนไม่ถึง 90° = เพิ่มค่า TURN_DELAY_90
 *  ถ้าหมุนเกิน 90° = ลดค่า TURN_DELAY_90
 */

int TURN_SPEED = 50;          // ความเร็วเลี้ยวเริ่มต้น
int TURN_BRAKE_SPEED = 100;   // ความเร็วเบรค
int TURN_BRAKE_TIME = 30;     // เวลาเบรค (ms)

int TURN_DELAY_90 = 8000;     // delay = TURN_DELAY_90 / speed
int TURN_DELAY_180 = 23000;   // delay = TURN_DELAY_180 / speed


// =============================================================================
//  6. TRAC SETTINGS (เดินตามเส้น)
// =============================================================================
/*
 *  📍 TracJC Settings:
 *  -------------------
 *  TRAC_SPEED: ความเร็วเดินตามเส้น
 *  TRAC_SLOW_SPEED: ความเร็วตอนใกล้ถึงแยก
 *  TRAC_TUNE_DELAY: delay หลังเจอแยก (ปรับตำแหน่งหยุด)
 */

int TRAC_SPEED = 50;          // ความเร็วปกติ
int TRAC_SLOW_SPEED = 25;     // ความเร็วช้า
int TRAC_TUNE_DELAY = 50;     // delay หลังเจอแยก (ms)

// PID สำหรับ Trac
float TRAC_KP = 0.45;
float TRAC_KI = 0.0001;
float TRAC_KD = 0.025;


// =============================================================================
//  7. SERVO SETTINGS
// =============================================================================
/*
 *  🦾 Servo Trim:
 *  --------------
 *  ถ้า Servo ไม่ตรง ให้ปรับค่า trim
 *  ค่า + = หมุนทวนเข็ม
 *  ค่า - = หมุนตามเข็ม
 */

int SERVO_TRIM[6] = {0, 0, 0, 0, 0, 0};  // trim สำหรับ servo 0-5

// ตำแหน่งแขน
int ARM_DOWN = 50;            // แขนลง
int ARM_cm = 150;             // แขนขึ้น
int GRIPPER_OPEN_L = 120;     // กรงเล็บซ้ายเปิด
int GRIPPER_OPEN_R = 125;     // กรงเล็บขวาเปิด
int GRIPPER_CLOSE_L = 60;     // กรงเล็บซ้ายปิด
int GRIPPER_CLOSE_R = 65;     // กรงเล็บขวาปิด


// =============================================================================
//  8. BRAKE SETTINGS
// =============================================================================
/*
 *  🛑 การเบรค:
 *  -----------
 *  BRAKE_TIME: เวลาเบรค (ms)
 *  BRAKE_POWER: กำลังเบรค (% ของ speed)
 * 
 *  ถ้าหุ่นหยุดไม่ทัน = เพิ่ม BRAKE_TIME หรือ BRAKE_POWER
 *  ถ้าหุ่นกระตุก = ลด BRAKE_POWER
 */

// BRAKE_TIME และ BRAKE_POWER ประกาศใน MyRobot_Complete.ino
extern int BRAKE_TIME;
extern int BRAKE_POWER;


// =============================================================================
//  9. SENSOR THRESHOLD SETTINGS
// =============================================================================
/*
 *  🎚️ Threshold:
 *  -------------
 *  ค่า threshold สำหรับแยกสีขาว/ดำ
 *  ปกติใช้ค่ากลาง (min + max) / 2
 *  
 *  THRESHOLD_OFFSET: ปรับ threshold เพิ่ม/ลด
 *  ค่า + = ต้องดำกว่าปกติถึงจะนับว่าเจอเส้น
 *  ค่า - = ขาวกว่าปกติก็นับว่าเจอเส้น
 */

int THRESHOLD_OFFSET_A = 0;   // offset สำหรับเซ็นเซอร์หน้า
int THRESHOLD_OFFSET_B = 0;   // offset สำหรับเซ็นเซอร์หลัง
int THRESHOLD_OFFSET_C = 0;   // offset สำหรับเซ็นเซอร์กลาง


// =============================================================================
//  TracBack - เดินตามเส้นถอยหลัง
// =============================================================================
/*
 *  เหมือน TracJC แต่ใช้เซ็นเซอร์หลัง
 */

void TracBack() {
  float kp, ki, kd;
  getPID_Backward(TRAC_SPEED, kp, ki, kd);
  
  float _I = 0;
  float _prevError = 0;
  
  while (1) {
    // คำนวณ error จากเซ็นเซอร์หลัง
    float error = error_B();
    
    // PID
    float P = error;
    _I += error * 0.00005;
    _I = constrain(_I, -1000, 1000);
    float D = error - _prevError;
    _prevError = error;
    
    float output = (kp * P) + (ki * _I) + (kd * D);
    
    // ควบคุมมอเตอร์ (ถอยหลัง)
    Motor(-(TRAC_SPEED + output), -(TRAC_SPEED - output));
    
    // เช็คเจอแยก
    if ((read_sensorB(0) < md_sensorB(0) && read_sensorB(1) < md_sensorB(1)) ||
        (read_sensorB(7) < md_sensorB(7) && read_sensorB(6) < md_sensorB(6))) {
      break;
    }
    
    delayMicroseconds(50);
  }
  
  Motor(0, 0);
  delay(TRAC_TUNE_DELAY);
}


// เดินตามเส้นถอยหลังด้วย speed ที่กำหนด
void TracBack(int speed) {
  float kp, ki, kd;
  getPID_Backward(speed, kp, ki, kd);
  
  float _I = 0;
  float _prevError = 0;
  
  while (1) {
    float error = error_B();
    
    float P = error;
    _I += error * 0.00005;
    _I = constrain(_I, -1000, 1000);
    float D = error - _prevError;
    _prevError = error;
    
    float output = (kp * P) + (ki * _I) + (kd * D);
    
    Motor(-(speed + output), -(speed - output));
    
    if ((read_sensorB(0) < md_sensorB(0) && read_sensorB(1) < md_sensorB(1)) ||
        (read_sensorB(7) < md_sensorB(7) && read_sensorB(6) < md_sensorB(6))) {
      break;
    }
    
    delayMicroseconds(50);
  }
  
  Motor(0, 0);
  delay(TRAC_TUNE_DELAY);
}


// เดินตามเส้นถอยหลัง ผ่านหลายแยก
void TracBackJC(int junctions) {
  for (int i = 0; i < junctions; i++) {
    TracBack();
    if (i < junctions - 1) {
      // ผ่านแยกไปก่อน
      Motor(-TRAC_SLOW_SPEED, -TRAC_SLOW_SPEED);
      delay(100);
    }
  }
}


// =============================================================================
//  TracFront - เดินตามเส้นเดินหน้า (ใช้ PID จากตาราง)
// =============================================================================

void TracFront() {
  float kp, ki, kd;
  getPID_Forward(TRAC_SPEED, kp, ki, kd);
  
  float _I = 0;
  float _prevError = 0;
  
  while (1) {
    float error = error_A();
    
    float P = error;
    _I += error * 0.00005;
    _I = constrain(_I, -1000, 1000);
    float D = error - _prevError;
    _prevError = error;
    
    float output = (kp * P) + (ki * _I) + (kd * D);
    
    Motor(TRAC_SPEED - output, TRAC_SPEED + output);
    
    if ((read_sensorA(0) < md_sensorA(0) && read_sensorA(1) < md_sensorA(1)) ||
        (read_sensorA(7) < md_sensorA(7) && read_sensorA(6) < md_sensorA(6))) {
      break;
    }
    
    delayMicroseconds(50);
  }
  
  Motor(0, 0);
  delay(TRAC_TUNE_DELAY);
}


void TracFront(int speed) {
  float kp, ki, kd;
  getPID_Forward(speed, kp, ki, kd);
  
  float _I = 0;
  float _prevError = 0;
  
  while (1) {
    float error = error_A();
    
    float P = error;
    _I += error * 0.00005;
    _I = constrain(_I, -1000, 1000);
    float D = error - _prevError;
    _prevError = error;
    
    float output = (kp * P) + (ki * _I) + (kd * D);
    
    Motor(speed - output, speed + output);
    
    if ((read_sensorA(0) < md_sensorA(0) && read_sensorA(1) < md_sensorA(1)) ||
        (read_sensorA(7) < md_sensorA(7) && read_sensorA(6) < md_sensorA(6))) {
      break;
    }
    
    delayMicroseconds(50);
  }
  
  Motor(0, 0);
  delay(TRAC_TUNE_DELAY);
}


// =============================================================================
//  Gyro Functions
// =============================================================================

// Calibrate Gyro (หาค่า offset)
// หมายเหตุ: ฟังก์ชัน gyro_begin() ใน Gyro.ino จะ calibrate อัตโนมัติอยู่แล้ว
// ฟังก์ชันนี้ใช้สำหรับ re-calibrate ถ้าต้องการ
void calibrateGyro() {
  Serial.println("Calibrating Gyro... Keep robot still!");
  led_rgb('y');

  // รีเซ็ตมุมทั้งหมด
  resetAngles();

  // อ่านค่าหลายครั้งเพื่อให้ gyro เสถียร
  float roll, pitch, yaw;
  for (int i = 0; i < 100; i++) {
    gyro_readAngles(roll, pitch, yaw);
    delay(10);
  }

  // รีเซ็ตอีกครั้งหลังอ่านค่าเสถียรแล้ว
  resetAngles();

  Serial.println("Gyro Calibration Complete!");
  Serial.print("Current angles - Roll: "); Serial.print(roll);
  Serial.print(" Pitch: "); Serial.print(pitch);
  Serial.print(" Yaw: "); Serial.println(yaw);

  beep(2000, 200);
  led_off();
}


// Reset Heading
void resetHeading() {
  currentHeading = 0;
  targetHeading = 0;
}


// Set Heading
void setHeading(float heading) {
  currentHeading = heading;
}


// Get Current Heading
float getHeading() {
  return currentHeading;
}


// =============================================================================
//  ฟังก์ชัน Init - เรียกใน setup()
// =============================================================================

void initTuning() {
  // ตั้งค่า Turn (ใช้ฟังก์ชันตั้งค่าแทน)
  setTurnSpeed(TURN_SPEED);
  setTurnBrake(TURN_BRAKE_SPEED, TURN_BRAKE_TIME);
  setTurnDelay90(TURN_DELAY_90);
  setTurnDelay180(TURN_DELAY_180);

  // ตั้งค่า Trac - ใช้ตัวแปรที่มีอยู่แล้ว
  LINE_KP = TRAC_KP;
  LINE_KI = TRAC_KI;
  LINE_KD = TRAC_KD;
  SlowSpeed = TRAC_SLOW_SPEED;

  Serial.println("=== Tuning Initialized ===");
}


// =============================================================================
//  Debug Functions - แสดงค่า Tuning
// =============================================================================

void printTuningValues() {
  Serial.println("\n========================================");
  Serial.println("         CURRENT TUNING VALUES");
  Serial.println("========================================");
  
  Serial.println("\n----- PID Forward (by Speed) -----");
  for (int s = 30; s <= 100; s += 10) {
    float kp, ki, kd;
    getPID_Forward(s, kp, ki, kd);
    Serial.print("Speed "); Serial.print(s);
    Serial.print(": Kp="); Serial.print(kp, 2);
    Serial.print(" Ki="); Serial.print(ki, 4);
    Serial.print(" Kd="); Serial.println(kd, 3);
  }
  
  Serial.println("\n----- PID Backward (by Speed) -----");
  for (int s = 30; s <= 100; s += 10) {
    float kp, ki, kd;
    getPID_Backward(s, kp, ki, kd);
    Serial.print("Speed "); Serial.print(s);
    Serial.print(": Kp="); Serial.print(kp, 2);
    Serial.print(" Ki="); Serial.print(ki, 4);
    Serial.print(" Kd="); Serial.println(kd, 3);
  }
  
  Serial.println("\n----- Motor Offset (by Speed) -----");
  for (int s = 30; s <= 100; s += 10) {
    int offL, offR;
    getMotorTuning(s, offL, offR);
    Serial.print("Speed "); Serial.print(s);
    Serial.print(": OffsetL="); Serial.print(offL);
    Serial.print(" OffsetR="); Serial.println(offR);
  }
  
  Serial.println("\n----- Gyro Settings -----");
  Serial.print("GYRO_KP="); Serial.println(GYRO_KP);
  Serial.print("GYRO_KI="); Serial.println(GYRO_KI);
  Serial.print("GYRO_KD="); Serial.println(GYRO_KD);
  Serial.print("SPIN_TOLERANCE="); Serial.println(SPIN_TOLERANCE);
  
  Serial.println("\n----- Turn Settings -----");
  Serial.print("TURN_SPEED="); Serial.println(TURN_SPEED);
  Serial.print("TURN_BRAKE_SPEED="); Serial.println(TURN_BRAKE_SPEED);
  Serial.print("TURN_BRAKE_TIME="); Serial.println(TURN_BRAKE_TIME);
  
  Serial.println("\n----- Trac Settings -----");
  Serial.print("TRAC_SPEED="); Serial.println(TRAC_SPEED);
  Serial.print("TRAC_SLOW_SPEED="); Serial.println(TRAC_SLOW_SPEED);
  Serial.print("TRAC_KP="); Serial.println(TRAC_KP);
  
  Serial.println("========================================\n");
}


// =============================================================================
//  Test Functions - ทดสอบการจูน
// =============================================================================

void testPID_Forward(int speed, int duration) {
  Serial.print("Testing Forward PID at speed ");
  Serial.println(speed);
  
  float kp, ki, kd;
  getPID_Forward(speed, kp, ki, kd);
  Serial.print("Using: Kp="); Serial.print(kp);
  Serial.print(" Ki="); Serial.print(ki);
  Serial.print(" Kd="); Serial.println(kd);
  
  float _I = 0;
  float _prevError = 0;
  unsigned long startTime = millis();
  
  while (millis() - startTime < duration) {
    float error = error_A();
    
    float P = error;
    _I += error * 0.00005;
    _I = constrain(_I, -1000, 1000);
    float D = error - _prevError;
    _prevError = error;
    
    float output = (kp * P) + (ki * _I) + (kd * D);
    
    Motor(speed - output, speed + output);
    
    // Print debug
    Serial.print("Error: "); Serial.print(error);
    Serial.print(" Output: "); Serial.println(output);
    
    delay(50);
  }
  
  Motor(0, 0);
  Serial.println("Test complete!");
}


void testMotorOffset(int speed, int duration) {
  Serial.print("Testing Motor Offset at speed ");
  Serial.println(speed);
  
  int offL, offR;
  getMotorTuning(speed, offL, offR);
  Serial.print("Using: OffsetL="); Serial.print(offL);
  Serial.print(" OffsetR="); Serial.println(offR);
  
  int actualL = speed - offL;
  int actualR = speed - offR;
  Serial.print("Actual: L="); Serial.print(actualL);
  Serial.print(" R="); Serial.println(actualR);
  
  Motor(actualL, actualR);
  delay(duration);
  Motor(0, 0);
  
  Serial.println("Test complete! Did it go straight?");
}


/*
 * =============================================================================
 *                     📚 สรุปวิธีการจูนค่า
 * =============================================================================
 * 
 *  1️⃣ จูน Motor Offset (ให้หุ่นเดินตรง)
 *  ------------------------------------
 *  - เรียก testMotorOffset(50, 2000);
 *  - ดูว่าหุ่นเบี้ยวซ้ายหรือขวา
 *  - แก้ไขค่าใน getMotorTuning()
 *  - ทดสอบซ้ำจนเดินตรง
 * 
 *  2️⃣ จูน PID (ให้หุ่นเกาะเส้น)
 *  -----------------------------
 *  - เรียก testPID_Forward(50, 3000);
 *  - ดู Serial Monitor
 *  - ถ้า error แกว่งมาก = ลด Kp หรือ เพิ่ม Kd
 *  - ถ้าหุ่นเบี้ยว = เพิ่ม Kp
 *  - แก้ไขค่าใน getPID_Forward()
 * 
 *  3️⃣ จูน Gyro (ให้หมุนตรง)
 *  -------------------------
 *  - เรียก calibrateGyro(); ตอนเริ่ม
 *  - ทดสอบหมุน 90° ด้วย SpinDegree(30, 90, 0);
 *  - ถ้าหมุนเกิน = เพิ่ม SPIN_TOLERANCE หรือ GYRO_KD
 *  - ถ้าหมุนไม่ถึง = ลด SPIN_TOLERANCE หรือ GYRO_KD
 * 
 *  4️⃣ จูน Turn (ให้เลี้ยวเจอเส้น)
 *  -------------------------------
 *  - ทดสอบ TurnLeft(); และ TurnRight();
 *  - ถ้าหมุนไม่ถึง 90° = เพิ่ม TURN_DELAY_90
 *  - ถ้าหมุนเกิน = ลด TURN_DELAY_90 หรือเพิ่ม TURN_BRAKE_TIME
 * 
 *  5️⃣ บันทึกค่า
 *  ------------
 *  - หลังจูนเสร็จ ค่าจะอยู่ในไฟล์นี้
 *  - เรียก printTuningValues(); เพื่อดูค่าทั้งหมด
 * 
 * =============================================================================
 */
