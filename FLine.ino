/*
 * =============================================================================
 *  FLine.ino - เดินตามเส้นแบบ Advanced (fline / bline)
 * =============================================================================
 *  
 *  รูปแบบการใช้งาน:
 *    fline(speed, kp, distance, stopMode, turnDir, power, sensor, brakeDelay);
 *    bline(speed, kp, distance, stopMode, turnDir, power, sensor, brakeDelay);
 *  
 *  Parameters:
 *    speed     : ความเร็ว (ใช้ตัวเดียว จะดึง offset จาก getMotorTuning อัตโนมัติ)
 *    kp        : ค่า Kp สำหรับ PID (แนะนำ 0.45)
 *    distance  : ระยะทาง (cm) หรือ 0 = วิ่งจนเจอแยก
 *                หรือ String "a0","a7","a07" = วิ่งจนเจอเซ็นเซอร์
 *    stopMode  : 'n' = ปกติ, 'f' = เจอแยกแล้วผ่าน, 'c' = เจอเซ็นเซอร์กลาง
 *    turnDir   : 'p' = ผ่านไป, 's' = หยุด, 'l' = เลี้ยวซ้าย, 'r' = เลี้ยวขวา
 *    power     : กำลังมอเตอร์ตอนเลี้ยว (%)
 *    sensor    : เซ็นเซอร์เป้าหมาย "a3", "b4" etc.
 *    brakeDelay: เวลาเบรค (ms)
 * 
 *  ตัวอย่าง:
 *    fline(50, 0.45, 0, 'n', 'p', 100, "a3", 0);    // วิ่งจนเจอแยกแล้วผ่านไป
 *    fline(50, 0.45, 20, 'n', 's', 100, "a3", 10);  // วิ่ง 20cm แล้วหยุด
 *    fline(50, 0.45, 0, 'f', 'l', 80, "a3", 0);     // วิ่งเจอแยกแล้วเลี้ยวซ้าย
 * =============================================================================
 */

// =============================================================================
//  ตัวแปร Global สำหรับ FLine
// =============================================================================

// ความเร็วช้าสำหรับหาแยก
int slmotor = 25;
int srmotor = 25;

// ค่า PID
float kp_slow = 0.45;
float ki_slow = 0.0001;
float kd_f = 0.025;

// ค่ามอเตอร์สำหรับเลี้ยว (หลังเจอแยก)
int flml = -30, flmr = 30;   // เลี้ยวซ้ายหลังเดินหน้า
int frml = 30, frmr = -30;   // เลี้ยวขวาหลังเดินหน้า
int clml = -25, clmr = 25;   // เลี้ยวซ้ายปกติ
int crml = 25, crmr = -25;   // เลี้ยวขวาปกติ

// Delay สำหรับเลี้ยว
int delay_f = 10;
int break_ff = 15;
int break_fc = 10;
int break_bf = 20;
int break_bc = 15;

// ค่าสเกลระยะทาง
float speed_scale_fw = 0.08;
float speed_scale_bw = 0.09;

// ตัวแปร PID
float errors = 0;
float P = 0, I = 0, D = 0;
float previous_error = 0;
float PID_output = 0;

// Flag
bool _fw = true;
bool ch_p = false;
bool pid_error = false;
char led = 'b';


// =============================================================================
//  ฟังก์ชันคำนวณ Error
// =============================================================================

float error_A() {
  float pos = position_A() / 50.0;
  return 50.0 - pos;
}

float error_AA() {
  // Error แบบ weighted (สำหรับ kp สูง)
  long sum = 0;
  long weighted = 0;
  
  for (int i = 1; i <= 6; i++) {
    int val = map(read_sensorA(i), sensorMin_A[i], sensorMax_A[i], 0, 1000);
    val = constrain(val, 0, 1000);
    sum += val;
    weighted += val * (i * 1000);
  }
  
  if (sum == 0) return 0;
  return (weighted / sum - 3500) / 100.0;
}

float error_B() {
  float pos = position_B() / 50.0;
  return 50.0 - pos;
}

float error_BB() {
  long sum = 0;
  long weighted = 0;
  
  for (int i = 1; i <= 6; i++) {
    int val = map(read_sensorB(i), sensorMin_B[i], sensorMax_B[i], 0, 1000);
    val = constrain(val, 0, 1000);
    sum += val;
    weighted += val * (i * 1000);
  }
  
  if (sum == 0) return 0;
  return (weighted / sum - 3500) / 100.0;
}


// =============================================================================
//  ฟังก์ชันเลี้ยวหลังเจอแยก
// =============================================================================

void turn_speed_fl() {
  Motor(-5, -5);
  delay(5);
  Motor(0, 0);
}

void turn_speed_fr() {
  Motor(-5, -5);
  delay(5);
  Motor(0, 0);
}


// =============================================================================
//  ฟังก์ชัน LED Toggle
// =============================================================================

void toggle_led() {
  if (led == 'b') {
    digitalWrite(rgb[0], 1); digitalWrite(rgb[1], 0); digitalWrite(rgb[2], 0);
    led = 'g';
  } else if (led == 'g') {
    digitalWrite(rgb[0], 0); digitalWrite(rgb[1], 1); digitalWrite(rgb[2], 0);
    led = 'r';
  } else if (led == 'r') {
    digitalWrite(rgb[0], 0); digitalWrite(rgb[1], 0); digitalWrite(rgb[2], 1);
    led = 'w';
  } else {
    digitalWrite(rgb[0], 1); digitalWrite(rgb[1], 1); digitalWrite(rgb[2], 1);
    led = 'b';
  }
}


// =============================================================================
//  fline - เดินหน้าตามเส้น (speed ตัวเดียว)
// =============================================================================

void fline(int speed, float kp, float distance, char stopMode, char turnDir, int power, String sensor, int brakeDelay) {
  _fw = true;
  toggle_led();
  
  // ดึง offset จาก speed
  setMotorOffset(speed);
  int spl = speed - MOTOR_LEFT_OFFSET;
  int spr = speed - MOTOR_RIGHT_OFFSET;
  
  // แปลง sensor string
  char sensors[4];
  sensor.toCharArray(sensors, sizeof(sensors));
  int sensor_f = atoi(&sensors[1]);
  
  // ตัวแปรระยะทาง
  float traveled = 0;
  unsigned long lastTime = millis();
  
  // Reset PID ถ้า kp = 0
  if (kp == 0) {
    I = kp_slow = ki_slow = 0;
  }
  
  // ===== วิ่งหลัก =====
  while (1) {
    // คำนวณ error
    if (kp <= 0.65) {
      if (read_sensorA(2) > md_sensorA(2) && read_sensorA(3) > md_sensorA(3) && 
          read_sensorA(4) > md_sensorA(4) && read_sensorA(5) > md_sensorA(5)) {
        errors = 0;
      } else if (read_sensorA(5) < md_sensorA(5) && read_sensorA(6) < md_sensorA(6) && read_sensorA(7) < md_sensorA(7)) {
        errors = 10;
      } else if (read_sensorA(2) < md_sensorA(2) && read_sensorA(1) < md_sensorA(1) && read_sensorA(0) < md_sensorA(0)) {
        errors = -10;
      } else {
        errors = error_A();
      }
    } else {
      errors = error_AA();
    }
    
    // PID
    P = errors;
    I += errors * 0.00005;
    I = constrain(I, -1000, 1000);
    D = errors - previous_error;
    previous_error = errors;
    PID_output = (kp * P) + (0.000001 * I) + (kd_f * D);
    
    Motor(spl - PID_output, spr + PID_output);
    delayMicroseconds(50);
    
    // เช็คระยะทาง
    if (distance > 0) {
      unsigned long now = millis();
      float dt = (now - lastTime) / 500.0;
      lastTime = now;
      traveled += speed * speed_scale_fw * dt;
      
      if (traveled >= distance) {
        // ค่อยๆ ลดความเร็ว
        for (int i = speed; i > slmotor; i--) {
          errors = error_A();
          P = errors;
          D = errors - previous_error;
          previous_error = errors;
          PID_output = (kp / 2) * P + (kd_f * D);
          Motor(i - PID_output, i + PID_output);
          delayMicroseconds(50);
        }
        Motor(0, 0);
        break;
      }
    } else {
      // เช็คเจอแยก
      if (kp >= 0.65) {
        if ((read_sensorA(0) < md_sensorA(0) && read_sensorA(1) < md_sensorA(1) && read_sensorA(2) < md_sensorA(2)) ||
            (read_sensorA(7) < md_sensorA(7) && read_sensorA(6) < md_sensorA(6) && read_sensorA(5) < md_sensorA(5))) {
          break;
        }
      } else {
        if ((read_sensorA(0) < md_sensorA(0) && read_sensorA(1) < md_sensorA(1)) ||
            (read_sensorA(7) < md_sensorA(7) && read_sensorA(6) < md_sensorA(6))) {
          break;
        }
      }
    }
  }
  
  ch_p = false;
  
  // ===== จัดการหลังเจอแยก =====
  if (stopMode == 'n') {
    if (turnDir == 'p') {
      ch_p = true;
      // วิ่งช้าจนเจอแยกอีกครั้ง
      while (1) {
        errors = error_A();
        P = errors;
        I += errors * 0.00005;
        D = errors - previous_error;
        previous_error = errors;
        PID_output = (kp_slow * P) + (0.000001 * I) + (0.025 * D);
        Motor(slmotor - PID_output, srmotor + PID_output);
        delayMicroseconds(50);
        
        if ((read_sensorA(0) < md_sensorA(0) && read_sensorA(1) < md_sensorA(1)) ||
            (read_sensorA(7) < md_sensorA(7) && read_sensorA(6) < md_sensorA(6))) {
          break;
        }
      }
    } else if (turnDir == 's') {
      if (brakeDelay > 0) {
        Motor(-spl, -spr);
        delay(brakeDelay);
        Motor(-1, -1);
        delay(10);
      } else {
        Motor(0, 0);
      }
      return;
    }
  } else if (stopMode == 'f') {
    // ลดความเร็วก่อนหาแยก
    for (int i = speed; i > slmotor; i--) {
      errors = error_A();
      P = errors;
      D = errors - previous_error;
      previous_error = errors;
      PID_output = (kp_slow * P) + (0.025 * D);
      Motor(i - PID_output, i + PID_output);
      delayMicroseconds(50);
      
      if ((read_sensorA(0) < md_sensorA(0) && read_sensorA(1) < md_sensorA(1)) ||
          (read_sensorA(7) < md_sensorA(7) && read_sensorA(6) < md_sensorA(6))) {
        break;
      }
    }
    
    // วิ่งช้าจนเจอแยก
    while (1) {
      errors = error_A();
      P = errors;
      D = errors - previous_error;
      previous_error = errors;
      PID_output = (kp_slow * P) + (0.025 * D);
      Motor(slmotor - PID_output, slmotor + PID_output);
      delayMicroseconds(50);
      
      if ((read_sensorA(0) < md_sensorA(0) && read_sensorA(1) < md_sensorA(1)) ||
          (read_sensorA(7) < md_sensorA(7) && read_sensorA(6) < md_sensorA(6))) {
        break;
      }
    }
    
    if (turnDir == 'p') {
      ch_p = true;
      while (1) {
        Motor(spl, spr);
        delayMicroseconds(50);
        if (read_sensorA(0) > md_sensorA(0) && read_sensorA(7) > md_sensorA(7)) {
          break;
        }
      }
      delay(10);
      if (brakeDelay > 0) {
        Motor(-spl, -spr);
        delay(brakeDelay);
        Motor(-1, -1);
        delay(10);
      } else {
        Motor(0, 0);
      }
    } else if (turnDir == 's') {
      Motor(-spl, -spr);
      delay(brakeDelay);
      Motor(0, 0);
    }
  } else if (stopMode == 'c') {
    // เจอเซ็นเซอร์กลาง
    if (turnDir == 'p') {
      ch_p = true;
      while (1) {
        errors = error_A();
        P = errors;
        D = errors - previous_error;
        previous_error = errors;
        PID_output = (kp / 2.5 * P) + (0.0125 * D);
        Motor(spl - PID_output, spr + PID_output);
        delayMicroseconds(50);
        
        if (analogRead(CENTER_SENSOR_L) < (sensorMin_C[0] + md_sensorC(0)) / 2 ||
            analogRead(CENTER_SENSOR_R) < (sensorMin_C[1] + md_sensorC(1)) / 2) {
          break;
        }
      }
      if (brakeDelay > 0) {
        Motor(-spl, -spr);
        delay(brakeDelay);
        Motor(-1, -1);
        delay(10);
      } else {
        Motor(0, 0);
      }
    } else {
      while (1) {
        errors = error_A();
        P = errors;
        D = errors - previous_error;
        previous_error = errors;
        PID_output = (kp * P) + (0.0125 * D);
        Motor(slmotor - PID_output, slmotor + PID_output);
        delayMicroseconds(50);
        
        if (analogRead(CENTER_SENSOR_L) < (sensorMin_C[0] + md_sensorC(0)) / 2 ||
            analogRead(CENTER_SENSOR_R) < (sensorMin_C[1] + md_sensorC(1)) / 2) {
          break;
        }
      }
      if (turnDir == 's') {
        Motor(-spl, -spr);
        delay(brakeDelay);
        Motor(0, 0);
      }
    }
  }
  
  // ===== เลี้ยว =====
  if (turnDir == 'l') {
    if (stopMode == 'f' || (stopMode == 'n' && speed > 0 && distance == 0)) {
      while (1) {
        Motor(slmotor, srmotor);
        if (read_sensorA(0) > md_sensorA(0) && read_sensorA(7) > md_sensorA(7)) {
          delay(delay_f);
          break;
        }
      }
      Motor(-slmotor, -srmotor);
      delay(break_ff);
      
      for (int i = 0; i <= sensor_f; i++) {
        do {
          Motor((flml * power) / 100, (flmr * power) / 100);
          delayMicroseconds(50);
        } while (read_sensorA(i) > md_sensorA(i) - 50);
      }
    } else {
      Motor(stopMode == 'n' ? 0 : -slmotor, stopMode == 'n' ? 0 : -srmotor);
      delay(stopMode == 'n' ? 2 : break_fc);
      
      for (int i = 0; i <= sensor_f; i++) {
        do {
          Motor((clml * power) / 100, (clmr * power) / 100);
          delayMicroseconds(50);
        } while ((sensor[0] == 'a' ? read_sensorA(i) : read_sensorB(i)) > 
                 (sensor[0] == 'a' ? md_sensorA(i) : md_sensorB(i)));
      }
    }
    
    if (brakeDelay == 0) {
      turn_speed_fl();
    } else {
      Motor(-((clml * power) / 100), -((clmr * power) / 100));
      delay(brakeDelay);
      Motor(-1, -1);
      delay(10);
    }
  } else if (turnDir == 'r') {
    if (stopMode == 'f' || (stopMode == 'n' && speed > 0 && distance == 0)) {
      while (1) {
        Motor(slmotor, srmotor);
        delayMicroseconds(50);
        if (read_sensorA(0) >= md_sensorA(0) && read_sensorA(7) >= md_sensorA(7)) {
          break;
        }
      }
      delay(delay_f);
      Motor(-slmotor, -srmotor);
      delay(break_ff);
      
      for (int i = 7; i >= sensor_f; i--) {
        do {
          Motor((frml * power) / 100, (frmr * power) / 100);
          delayMicroseconds(50);
        } while (read_sensorA(i) > md_sensorA(i) - 50);
      }
    } else {
      Motor(stopMode == 'n' ? 0 : -slmotor, stopMode == 'n' ? 0 : -srmotor);
      delay(stopMode == 'n' ? 2 : break_fc);
      
      for (int i = 7; i >= sensor_f; i--) {
        do {
          Motor((crml * power) / 100, (crmr * power) / 100);
          delayMicroseconds(50);
        } while ((sensor[0] == 'a' ? read_sensorA(i) : read_sensorB(i)) > 
                 (sensor[0] == 'a' ? md_sensorA(i) : md_sensorB(i)));
      }
    }
    
    if (brakeDelay == 0) {
      turn_speed_fr();
    } else {
      Motor(-((crml * power) / 100), -((crmr * power) / 100));
      delay(brakeDelay);
      Motor(-1, -1);
      delay(10);
    }
  }
  
  delay(5);
}


// =============================================================================
//  fline - เวอร์ชัน String distance (เช็คเซ็นเซอร์)
// =============================================================================

void fline(int speed, float kp, String distSensor, char stopMode, char turnDir, int power, String sensor, int brakeDelay) {
  _fw = true;
  toggle_led();
  
  setMotorOffset(speed);
  int spl = speed - MOTOR_LEFT_OFFSET;
  int spr = speed - MOTOR_RIGHT_OFFSET;
  
  char sensors[4];
  sensor.toCharArray(sensors, sizeof(sensors));
  int sensor_f = atoi(&sensors[1]);
  
  // ===== วิ่งจนเจอเซ็นเซอร์ที่กำหนด =====
  while (1) {
    if (kp <= 0.65) {
      if (read_sensorA(2) > md_sensorA(2) && read_sensorA(3) > md_sensorA(3) && 
          read_sensorA(4) > md_sensorA(4) && read_sensorA(5) > md_sensorA(5)) {
        errors = 0;
      } else if (read_sensorA(5) < md_sensorA(5) && read_sensorA(6) < md_sensorA(6) && read_sensorA(7) < md_sensorA(7)) {
        errors = 10;
      } else if (read_sensorA(2) < md_sensorA(2) && read_sensorA(1) < md_sensorA(1) && read_sensorA(0) < md_sensorA(0)) {
        errors = -10;
      } else {
        errors = error_A();
      }
    } else {
      errors = error_AA();
    }
    
    P = errors;
    I += errors * 0.00005;
    D = errors - previous_error;
    previous_error = errors;
    PID_output = (kp * P) + (0.000001 * I) + (kd_f * D);
    
    Motor(spl - PID_output, spr + PID_output);
    
    // เช็คเซ็นเซอร์ตาม distSensor
    if (distSensor == "a0") {
      if (read_sensorA(0) < md_sensorA(0)) break;
    } else if (distSensor == "a1") {
      if (read_sensorA(1) < md_sensorA(1)) break;
    } else if (distSensor == "a6") {
      if (read_sensorA(6) < md_sensorA(6)) break;
    } else if (distSensor == "a7") {
      if (read_sensorA(7) < md_sensorA(7)) break;
    } else if (distSensor == "a07" || distSensor == "a70") {
      if (read_sensorA(7) < md_sensorA(7) && read_sensorA(0) < md_sensorA(0)) break;
    }
    
    delayMicroseconds(50);
  }
  
  // เรียก fline ปกติต่อเพื่อจัดการหลังเจอแยก
  ch_p = false;
  
  if (stopMode == 'n' && turnDir == 's') {
    if (brakeDelay > 0) {
      Motor(-spl, -spr);
      delay(brakeDelay);
      Motor(-1, -1);
      delay(10);
    } else {
      Motor(0, 0);
    }
    return;
  }
  
  // จัดการเลี้ยว/หยุด เหมือน fline ปกติ
  if (turnDir == 'l') {
    Motor(0, 0);
    delay(2);
    for (int i = 0; i <= sensor_f; i++) {
      do {
        Motor((clml * power) / 100, (clmr * power) / 100);
        delayMicroseconds(50);
      } while (read_sensorA(i) > md_sensorA(i));
    }
    if (brakeDelay == 0) {
      turn_speed_fl();
    } else {
      Motor(-((clml * power) / 100), -((clmr * power) / 100));
      delay(brakeDelay);
      Motor(-1, -1);
      delay(10);
    }
  } else if (turnDir == 'r') {
    Motor(0, 0);
    delay(2);
    for (int i = 7; i >= sensor_f; i--) {
      do {
        Motor((crml * power) / 100, (crmr * power) / 100);
        delayMicroseconds(50);
      } while (read_sensorA(i) > md_sensorA(i));
    }
    if (brakeDelay == 0) {
      turn_speed_fr();
    } else {
      Motor(-((crml * power) / 100), -((crmr * power) / 100));
      delay(brakeDelay);
      Motor(-1, -1);
      delay(10);
    }
  } else if (turnDir == 'p') {
    ch_p = true;
    while (1) {
      errors = error_A();
      P = errors;
      D = errors - previous_error;
      previous_error = errors;
      PID_output = (kp_slow * P) + (0.025 * D);
      Motor(slmotor - PID_output, srmotor + PID_output);
      delayMicroseconds(50);
      
      if ((read_sensorA(0) < md_sensorA(0) && read_sensorA(1) < md_sensorA(1)) ||
          (read_sensorA(7) < md_sensorA(7) && read_sensorA(6) < md_sensorA(6))) {
        break;
      }
    }
  }
  
  delay(5);
}


// =============================================================================
//  bline - ถอยหลังตามเส้น (speed ตัวเดียว)
// =============================================================================

void bline(int speed, float kp, float distance, char stopMode, char turnDir, int power, String sensor, int brakeDelay) {
  _fw = false;
  toggle_led();
  
  setMotorOffset(speed);
  int spl = speed - MOTOR_LEFT_OFFSET;
  int spr = speed - MOTOR_RIGHT_OFFSET;
  
  char sensors[4];
  sensor.toCharArray(sensors, sizeof(sensors));
  int sensor_f = atoi(&sensors[1]);
  
  float traveled = 0;
  unsigned long lastTime = millis();
  
  if (kp == 0) {
    I = kp_slow = ki_slow = 0;
  }
  
  // ===== วิ่งหลัก (ถอยหลัง) =====
  while (1) {
    if (kp <= 0.65) {
      if (read_sensorB(2) > md_sensorB(2) && read_sensorB(3) > md_sensorB(3) && 
          read_sensorB(4) > md_sensorB(4) && read_sensorB(5) > md_sensorB(5)) {
        errors = 0;
      } else if (read_sensorB(5) < md_sensorB(5) && read_sensorB(6) < md_sensorB(6) && read_sensorB(7) < md_sensorB(7)) {
        errors = 10;
      } else if (read_sensorB(2) < md_sensorB(2) && read_sensorB(1) < md_sensorB(1) && read_sensorB(0) < md_sensorB(0)) {
        errors = -10;
      } else {
        errors = error_B();
      }
    } else {
      errors = error_BB();
    }
    
    P = errors;
    I += errors * 0.00005;
    I = constrain(I, -1000, 1000);
    D = errors - previous_error;
    previous_error = errors;
    PID_output = (kp * P) + (0.00005 * I) + (kd_f * D);
    
    Motor(-(spl + PID_output), -(spr - PID_output));
    delayMicroseconds(100);
    
    if (distance > 0) {
      unsigned long now = millis();
      float dt = (now - lastTime) / 1000.0;
      lastTime = now;
      traveled += speed * (speed_scale_bw + 0.95) * dt;
      
      if (traveled >= distance) {
        Motor(0, 0);
        break;
      }
    } else {
      if (kp >= 0.65) {
        if ((read_sensorB(0) < md_sensorB(0) && read_sensorB(1) < md_sensorB(1) && read_sensorB(2) < md_sensorB(2)) ||
            (read_sensorB(7) < md_sensorB(7) && read_sensorB(6) < md_sensorB(6) && read_sensorB(5) < md_sensorB(5))) {
          break;
        }
      } else {
        if ((read_sensorB(0) < md_sensorB(0) && read_sensorB(1) < md_sensorB(1)) ||
            (read_sensorB(7) < md_sensorB(7) && read_sensorB(6) < md_sensorB(6))) {
          break;
        }
      }
    }
  }
  
  ch_p = false;
  
  // ===== จัดการหลังเจอแยก =====
  if (stopMode == 'n') {
    if (turnDir == 'p') {
      ch_p = true;
      while (1) {
        errors = error_B();
        P = errors;
        I += errors * 0.00005;
        I = constrain(I, -1000, 1000);
        D = errors - previous_error;
        previous_error = errors;
        PID_output = (kp_slow * P) + (0.000001 * I) + (0.025 * D);
        Motor(-(slmotor + PID_output), -(srmotor - PID_output));
        delayMicroseconds(50);
        
        if ((read_sensorB(0) < md_sensorB(0) && read_sensorB(1) < md_sensorB(1)) ||
            (read_sensorB(7) < md_sensorB(7) && read_sensorB(6) < md_sensorB(6))) {
          break;
        }
      }
    } else if (turnDir == 's') {
      if (brakeDelay > 0) {
        Motor(spl, spr);
        delay(brakeDelay);
        Motor(-1, -1);
        delay(10);
      }
      return;
    }
  } else if (stopMode == 'f') {
    while (1) {
      errors = error_B();
      P = errors;
      D = errors - previous_error;
      previous_error = errors;
      PID_output = (kp_slow * P) + (0.025 * D);
      Motor(-(slmotor + PID_output), -(srmotor - PID_output));
      delayMicroseconds(50);
      
      if ((read_sensorB(0) < md_sensorB(0) && read_sensorB(1) < md_sensorB(1)) ||
          (read_sensorB(7) < md_sensorB(7) && read_sensorB(6) < md_sensorB(6))) {
        break;
      }
    }
    
    if (turnDir == 'p') {
      ch_p = true;
      while (1) {
        Motor(-spl, -spr);
        delayMicroseconds(50);
        if (read_sensorB(0) > md_sensorB(0) && read_sensorB(7) > md_sensorB(7)) {
          break;
        }
      }
      delay(10);
    } else if (turnDir == 's') {
      Motor(spl, spr);
      delay(brakeDelay);
      Motor(0, 0);
    }
  } else if (stopMode == 'c') {
    if (turnDir == 'p') {
      ch_p = true;
      while (1) {
        errors = error_B();
        P = errors;
        D = errors - previous_error;
        previous_error = errors;
        PID_output = (kp_slow * P) + (0.025 * D);
        Motor(-(spl + PID_output), -(spr - PID_output));
        delayMicroseconds(50);
        
        if (analogRead(CENTER_SENSOR_L) < (sensorMin_C[0] + md_sensorC(0)) / 2 ||
            analogRead(CENTER_SENSOR_R) < (sensorMin_C[1] + md_sensorC(1)) / 2) {
          break;
        }
      }
      if (brakeDelay > 0) {
        Motor(spl, spr);
        delay(brakeDelay);
        Motor(1, 1);
        delay(10);
      } else {
        Motor(0, 0);
      }
    } else {
      while (1) {
        errors = error_B();
        P = errors;
        D = errors - previous_error;
        previous_error = errors;
        PID_output = (kp / 2.5 * P) + (0.0125 * D);
        Motor(-(slmotor + PID_output), -(slmotor - PID_output));
        delayMicroseconds(50);
        
        if (analogRead(CENTER_SENSOR_L) < (sensorMin_C[0] + md_sensorC(0)) / 2 ||
            analogRead(CENTER_SENSOR_R) < (sensorMin_C[1] + md_sensorC(1)) / 2) {
          break;
        }
      }
      if (turnDir == 's') {
        Motor(spl, spr);
        delay(brakeDelay);
        Motor(0, 0);
      }
    }
  }
  
  // ===== เลี้ยว =====
  if (turnDir == 'l') {
    if (stopMode == 'f' || (stopMode == 'n' && speed > 0 && distance == 0)) {
      while (1) {
        Motor(-slmotor, -srmotor);
        if (read_sensorB(0) > md_sensorB(0) && read_sensorB(7) > md_sensorB(7)) {
          delay(delay_f);
          break;
        }
      }
      Motor(slmotor + 10, srmotor + 10);
      delay(break_bf);
      
      for (int i = 0; i <= sensor_f; i++) {
        do {
          Motor(-((flmr * power) / 100), -((flml * power) / 100));
          delayMicroseconds(50);
        } while (read_sensorB(i) > md_sensorB(i) - 50);
      }
    } else {
      Motor(stopMode == 'n' ? 0 : slmotor, stopMode == 'n' ? 0 : srmotor);
      delay(stopMode == 'n' ? 2 : break_bc);
      
      for (int i = 0; i <= sensor_f; i++) {
        do {
          Motor((clml * power) / 100, (clmr * power) / 100);
          delayMicroseconds(50);
        } while ((sensor[0] == 'a' ? read_sensorA(i) : read_sensorB(i)) > 
                 (sensor[0] == 'a' ? md_sensorA(i) : md_sensorB(i)));
      }
    }
    
    if (brakeDelay == 0) {
      turn_speed_fl();
    } else {
      Motor(-((clml * power) / 100), -((clmr * power) / 100));
      delay(brakeDelay);
      Motor(-1, -1);
      delay(10);
    }
  } else if (turnDir == 'r') {
    if (stopMode == 'f') {
      while (1) {
        Motor(-slmotor, -srmotor);
        delayMicroseconds(50);
        if (read_sensorB(0) >= md_sensorB(0) && read_sensorB(7) >= md_sensorB(7)) {
          break;
        }
      }
      delay(delay_f);
      Motor(slmotor, srmotor);
      delay(break_bf);
      
      for (int i = 7; i >= sensor_f; i--) {
        do {
          Motor(-((frmr * power) / 100), -((frml * power) / 100));
          delayMicroseconds(50);
        } while (read_sensorB(i) > md_sensorB(i) - 50);
      }
    } else {
      Motor(stopMode == 'n' ? 0 : slmotor, stopMode == 'n' ? 0 : srmotor);
      delay(stopMode == 'n' ? 2 : break_bc);
      
      for (int i = 7; i >= sensor_f; i--) {
        do {
          Motor((crml * power) / 100, (crmr * power) / 100);
          delayMicroseconds(50);
        } while ((sensor[0] == 'a' ? read_sensorA(i) : read_sensorB(i)) > 
                 (sensor[0] == 'a' ? md_sensorA(i) : md_sensorB(i)));
      }
    }
    
    if (brakeDelay == 0) {
      turn_speed_fr();
    } else {
      Motor(-((crml * power) / 100), -((crmr * power) / 100));
      delay(brakeDelay);
      Motor(-1, -1);
      delay(10);
    }
  }
  
  delay(5);
}


// =============================================================================
//  bline - เวอร์ชัน String distance
// =============================================================================

void bline(int speed, float kp, String distSensor, char stopMode, char turnDir, int power, String sensor, int brakeDelay) {
  _fw = false;
  toggle_led();
  
  setMotorOffset(speed);
  int spl = speed - MOTOR_LEFT_OFFSET;
  int spr = speed - MOTOR_RIGHT_OFFSET;
  
  char sensors[4];
  sensor.toCharArray(sensors, sizeof(sensors));
  int sensor_f = atoi(&sensors[1]);
  
  // ===== วิ่งจนเจอเซ็นเซอร์ =====
  while (1) {
    if (kp <= 0.65) {
      if (read_sensorB(2) > md_sensorB(2) && read_sensorB(3) > md_sensorB(3) && 
          read_sensorB(4) > md_sensorB(4) && read_sensorB(5) > md_sensorB(5)) {
        errors = 0;
      } else if (read_sensorB(5) < md_sensorB(5) && read_sensorB(6) < md_sensorB(6) && read_sensorB(7) < md_sensorB(7)) {
        errors = 10;
      } else if (read_sensorB(2) < md_sensorB(2) && read_sensorB(1) < md_sensorB(1) && read_sensorB(0) < md_sensorB(0)) {
        errors = -10;
      } else {
        errors = error_B();
      }
    } else {
      errors = error_BB();
    }
    
    P = errors;
    I += errors * 0.00005;
    I = constrain(I, -1000, 1000);
    D = errors - previous_error;
    previous_error = errors;
    PID_output = (kp * P) + (0.00005 * I) + (kd_f * D);
    
    Motor(-(spl + PID_output), -(spr - PID_output));
    
    // เช็คเซ็นเซอร์
    if (distSensor == "b0") {
      if (read_sensorB(0) < md_sensorB(0)) break;
    } else if (distSensor == "b1") {
      if (read_sensorB(1) < md_sensorB(1)) break;
    } else if (distSensor == "b6") {
      if (read_sensorB(6) < md_sensorB(6)) break;
    } else if (distSensor == "b7") {
      if (read_sensorB(7) < md_sensorB(7)) break;
    } else if (distSensor == "b07" || distSensor == "b70") {
      if (read_sensorB(7) < md_sensorB(7) && read_sensorB(0) < md_sensorB(0)) break;
    }
    
    delayMicroseconds(100);
  }
  
  delay(10);
  ch_p = false;
  
  if (stopMode == 'n' && turnDir == 's') {
    if (brakeDelay > 0) {
      Motor(spl, spr);
      delay(brakeDelay);
      Motor(-1, -1);
      delay(10);
    }
    return;
  }
  
  // จัดการเลี้ยว
  if (turnDir == 'l') {
    Motor(0, 0);
    delay(2);
    for (int i = 0; i <= sensor_f; i++) {
      do {
        Motor((clml * power) / 100, (clmr * power) / 100);
        delayMicroseconds(50);
      } while (read_sensorB(i) > md_sensorB(i));
    }
    if (brakeDelay == 0) {
      turn_speed_fl();
    } else {
      Motor(-((clml * power) / 100), -((clmr * power) / 100));
      delay(brakeDelay);
      Motor(-1, -1);
      delay(10);
    }
  } else if (turnDir == 'r') {
    Motor(0, 0);
    delay(2);
    for (int i = 7; i >= sensor_f; i--) {
      do {
        Motor((crml * power) / 100, (crmr * power) / 100);
        delayMicroseconds(50);
      } while (read_sensorB(i) > md_sensorB(i));
    }
    if (brakeDelay == 0) {
      turn_speed_fr();
    } else {
      Motor(-((crml * power) / 100), -((crmr * power) / 100));
      delay(brakeDelay);
      Motor(-1, -1);
      delay(10);
    }
  } else if (turnDir == 'p') {
    ch_p = true;
    while (1) {
      errors = error_B();
      P = errors;
      D = errors - previous_error;
      previous_error = errors;
      PID_output = (kp_slow * P) + (0.025 * D);
      Motor(-(slmotor + PID_output), -(srmotor - PID_output));
      delayMicroseconds(50);
      
      if ((read_sensorB(0) < md_sensorB(0) && read_sensorB(1) < md_sensorB(1)) ||
          (read_sensorB(7) < md_sensorB(7) && read_sensorB(6) < md_sensorB(6))) {
        break;
      }
    }
  }
  
  delay(5);
}


// =============================================================================
//  ฟังก์ชันตั้งค่า FLine
// =============================================================================

void setFLineSpeed(int slow_l, int slow_r) {
  slmotor = slow_l;
  srmotor = slow_r;
}

void setFLinePID(float kp, float ki, float kd) {
  kp_slow = kp;
  ki_slow = ki;
  kd_f = kd;
}

void setTurnMotor(int fl_l, int fl_r, int fr_l, int fr_r) {
  flml = fl_l; flmr = fl_r;
  frml = fr_l; frmr = fr_r;
}

void setTurnMotorSlow(int cl_l, int cl_r, int cr_l, int cr_r) {
  clml = cl_l; clmr = cl_r;
  crml = cr_l; crmr = cr_r;
}

void setTurnDelay(int d_f, int b_ff, int b_fc, int b_bf, int b_bc) {
  delay_f = d_f;
  break_ff = b_ff;
  break_fc = b_fc;
  break_bf = b_bf;
  break_bc = b_bc;
}

void setSpeedScale(float fw, float bw) {
  speed_scale_fw = fw;
  speed_scale_bw = bw;
}
