/*
 * =============================================================================
 *  Motor.ino - ควบคุมมอเตอร์ + ฟังก์ชันจูน
 * =============================================================================
 *  
 *  วิธีจูน: แก้ไขในฟังก์ชัน getMotorTuning() ด้านล่าง
 *  
 *  ตัวอย่าง:
 *    if (speed == 50) {
 *      offsetL = 0;    // มอเตอร์ซ้ายลบ 0
 *      offsetR = 2;    // มอเตอร์ขวาลบ 2
 *    }
 * =============================================================================
 */

// =============================================================================
//  ฟังก์ชันจูนมอเตอร์ (แก้ไขตรงนี้!)
// =============================================================================

// getMotorTuning ถูกย้ายไปไว้ใน Initial.ino แล้ว
// ฟังก์ชันนี้ประกาศใน Initial.ino เพื่อให้จัดการ tuning ได้ง่าย


// =============================================================================
//  ฟังก์ชันควบคุมมอเตอร์
// =============================================================================

// ตั้งค่า offset จาก speed
void setMotorOffset(int speed) {
  getMotorTuning(speed, MOTOR_LEFT_OFFSET, MOTOR_RIGHT_OFFSET);
}

// ควบคุมมอเตอร์ (-100 ถึง 100)
void Motor(int pwmL, int pwmR) {
  analogWriteResolution(12);
  
  if (MOTOR_TYPE == "DC_Motors") {
    analogWriteFreq(1000);
  } else {
    analogWriteFreq(20000);
  }
  delayMicroseconds(50);

  int pwmValueL = map(abs(pwmL), 0, 100, 0, 4095);
  int pwmValueR = map(abs(pwmR), 0, 100, 0, 4095);

  // มอเตอร์ซ้าย
  if (pwmL > 0) {
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
  } else if (pwmL < 0) {
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);
  } else {
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, LOW);
    pwmValueL = 0;
  }

  // มอเตอร์ขวา
  if (pwmR > 0) {
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);
  } else if (pwmR < 0) {
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);
  } else {
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, LOW);
    pwmValueR = 0;
  }

  analogWrite(PWMA, pwmValueL);
  analogWrite(PWMB, pwmValueR);
}

// ควบคุมมอเตอร์พร้อม offset (ใช้ค่าที่ตั้งไว้)
void MotorWithOffset(int speedL, int speedR) {
  int actualL = speedL - MOTOR_LEFT_OFFSET;
  int actualR = speedR - MOTOR_RIGHT_OFFSET;
  Motor(actualL, actualR);
}

// ควบคุมมอเตอร์ด้วย speed เดียว (ใช้ offset อัตโนมัติ)
void MotorSpeed(int speed) {
  setMotorOffset(speed);
  int actualL = speed - MOTOR_LEFT_OFFSET;
  int actualR = speed - MOTOR_RIGHT_OFFSET;
  Motor(actualL, actualR);
}

// หยุดมอเตอร์
void MotorStop() {
  Motor(0, 0);
}

// เบรคมอเตอร์
void MotorBrake() {
  Motor(-BRAKE_POWER, -BRAKE_POWER);
  delay(BRAKE_TIME);
  Motor(0, 0);
}

// เบรคมอเตอร์แบบกำหนดเอง
void MotorBrake(int power, int timeMs) {
  Motor(-power, -power);
  delay(timeMs);
  Motor(0, 0);
}

// ตั้งค่าประเภทมอเตอร์
void setMotorType(String type) {
  MOTOR_TYPE = type;
}

// ตั้งค่าเบรค
void setBrake(int power, int timeMs) {
  BRAKE_POWER = power;
  BRAKE_TIME = timeMs;
}
