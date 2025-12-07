/*
 * =============================================================================
 *  Turn.ino - ฟังก์ชันเลี้ยวซ้าย/ขวา
 * =============================================================================
 *  
 *  ฟังก์ชันหลัก:
 *    Left(speed, time)      - เลี้ยวซ้ายตามเวลา (ms)
 *    Right(speed, time)     - เลี้ยวขวาตามเวลา (ms)
 *    TurnLeft()             - เลี้ยวซ้ายจนเจอเส้น
 *    TurnRight()            - เลี้ยวขวาจนเจอเส้น
 *    TurnLeft(speed)        - เลี้ยวซ้ายจนเจอเส้น (กำหนดความเร็ว)
 *    TurnRight(speed)       - เลี้ยวขวาจนเจอเส้น (กำหนดความเร็ว)
 *    UTurnLeft()            - กลับตัวซ้าย 180°
 *    UTurnRight()           - กลับตัวขวา 180°
 *  
 *  ตัวอย่างการใช้งาน:
 *    Left(50, 200);         // เลี้ยวซ้าย speed=50, 200ms
 *    Right(50, 200);        // เลี้ยวขวา speed=50, 200ms
 *    TurnLeft();            // เลี้ยวซ้ายจนเจอเส้น
 *    TurnRight();           // เลี้ยวขวาจนเจอเส้น
 *    TurnLeft(60);          // เลี้ยวซ้าย speed=60 จนเจอเส้น
 *    UTurnLeft();           // กลับตัวซ้าย
 * =============================================================================
 */

// =============================================================================
//  ค่าตั้งต่างๆ สำหรับ Turn
// =============================================================================

int TurnSpeed = 50;           // ความเร็วเลี้ยวเริ่มต้น
int BrakeSpeed = 100;         // ความเร็วเบรค
int BrakeTime = 30;           // เวลาเบรค (ms)
int WheelDrive = 0;           // 0 = 2WD, 1 = 4WD

// Delay สำหรับเลี้ยว (ปรับตาม TurnSpeed)
int turn_delay_90 = 8000;     // delay = turn_delay_90 / TurnSpeed
int turn_delay_180 = 23000;   // delay = turn_delay_180 / TurnSpeed


// =============================================================================
//  ฟังก์ชันพื้นฐาน - เลี้ยวซ้าย (Spin Left)
// =============================================================================

void sl(int speed) {
  Motor(-speed, speed);
}

void SL(int speed) {
  // 4WD mode - ถ้าต้องการควบคุมแยก
  Motor(-speed, speed);
}


// =============================================================================
//  ฟังก์ชันพื้นฐาน - เลี้ยวขวา (Spin Right)
// =============================================================================

void sr(int speed) {
  Motor(speed, -speed);
}

void SR(int speed) {
  // 4WD mode
  Motor(speed, -speed);
}


// =============================================================================
//  ฟังก์ชันพื้นฐาน - เลี้ยวซ้าย (Turn Left - ล้อข้างหนึ่งหมุน)
// =============================================================================

void tl(int speed) {
  Motor(0, speed);
}

void TL(int speed) {
  Motor(0, speed);
}


// =============================================================================
//  ฟังก์ชันพื้นฐาน - เลี้ยวขวา (Turn Right - ล้อข้างหนึ่งหมุน)
// =============================================================================

void tr(int speed) {
  Motor(speed, 0);
}

void TR(int speed) {
  Motor(speed, 0);
}


// =============================================================================
//  Left - เลี้ยวซ้ายตามเวลา
// =============================================================================

void Left(int speed, int time) {
  Motor(0, 0);
  delay(5);
  
  if (WheelDrive == 0) {
    sl(speed);
  } else {
    SL(speed);
  }
  
  delay(time);
  Motor(0, 0);
}


// =============================================================================
//  Right - เลี้ยวขวาตามเวลา
// =============================================================================

void Right(int speed, int time) {
  Motor(0, 0);
  delay(5);
  
  if (WheelDrive == 0) {
    sr(speed);
  } else {
    SR(speed);
  }
  
  delay(time);
  Motor(0, 0);
}


// =============================================================================
//  TurnLeftDelay - เลี้ยวซ้ายตามเวลา (ไม่หยุด)
// =============================================================================

void TurnLeftDelay(int speed, int time) {
  if (WheelDrive == 0) {
    tl(speed);
  } else {
    TL(speed);
  }
  delay(time);
}


// =============================================================================
//  TurnRightDelay - เลี้ยวขวาตามเวลา (ไม่หยุด)
// =============================================================================

void TurnRightDelay(int speed, int time) {
  if (WheelDrive == 0) {
    tr(speed);
  } else {
    TR(speed);
  }
  delay(time);
}


// =============================================================================
//  TurnLeft - เลี้ยวซ้ายจนเจอเส้น (ใช้ TurnSpeed)
// =============================================================================

void TurnLeft() {
  TurnLeft(TurnSpeed);
}


// =============================================================================
//  TurnLeft - เลี้ยวซ้ายจนเจอเส้น (กำหนดความเร็ว)
// =============================================================================

void TurnLeft(int speed) {
  Motor(0, 0);
  delay(5);
  
  // เริ่มหมุน
  if (WheelDrive == 0) {
    sl(speed);
  } else {
    SL(speed);
  }
  
  // Delay เริ่มต้น (ให้พ้นเส้นเดิมก่อน)
  delay(turn_delay_90 / speed);
  
  // ถ้าเร็วเกิน 90 ให้ลดความเร็ว
  if (speed > 90) {
    if (WheelDrive == 0) {
      sl(90);
    } else {
      SL(90);
    }
  }
  
  // รอจนเจอเส้น (เช็คตามความเร็ว)
  if (speed <= 40) {
    // ความเร็วต่ำ - เช็คเซ็นเซอร์กลาง
    while (read_sensorA(3) > md_sensorA(3) && read_sensorA(4) > md_sensorA(4)) {
      delayMicroseconds(50);
    }
  } else if (speed <= 50) {
    // เช็ค L1 (sensor 2)
    while (read_sensorA(2) > md_sensorA(2)) {
      delayMicroseconds(50);
    }
  } else if (speed <= 60) {
    // เช็ค L2 (sensor 1)
    while (read_sensorA(1) > md_sensorA(1)) {
      delayMicroseconds(50);
    }
  } else {
    // เช็ค L3 (sensor 0)
    while (read_sensorA(0) > md_sensorA(0)) {
      delayMicroseconds(50);
    }
  }
  
  // เบรค
  if (BrakeTime > 0 && speed > 70) {
    Right(BrakeSpeed, BrakeTime);
  } else {
    Motor(0, 0);
  }
}


// =============================================================================
//  TurnRight - เลี้ยวขวาจนเจอเส้น (ใช้ TurnSpeed)
// =============================================================================

void TurnRight() {
  TurnRight(TurnSpeed);
}


// =============================================================================
//  TurnRight - เลี้ยวขวาจนเจอเส้น (กำหนดความเร็ว)
// =============================================================================

void TurnRight(int speed) {
  Motor(0, 0);
  delay(5);
  
  // เริ่มหมุน
  if (WheelDrive == 0) {
    sr(speed);
  } else {
    SR(speed);
  }
  
  // Delay เริ่มต้น
  delay(turn_delay_90 / speed);
  
  // ถ้าเร็วเกิน 90 ให้ลดความเร็ว
  if (speed > 90) {
    if (WheelDrive == 0) {
      sr(90);
    } else {
      SR(90);
    }
  }
  
  // รอจนเจอเส้น
  if (speed <= 40) {
    while (read_sensorA(3) > md_sensorA(3) && read_sensorA(4) > md_sensorA(4)) {
      delayMicroseconds(50);
    }
  } else if (speed <= 50) {
    // เช็ค R1 (sensor 5)
    while (read_sensorA(5) > md_sensorA(5)) {
      delayMicroseconds(50);
    }
  } else if (speed <= 60) {
    // เช็ค R2 (sensor 6)
    while (read_sensorA(6) > md_sensorA(6)) {
      delayMicroseconds(50);
    }
  } else {
    // เช็ค R3 (sensor 7)
    while (read_sensorA(7) > md_sensorA(7)) {
      delayMicroseconds(50);
    }
  }
  
  // เบรค
  if (BrakeTime > 0 && speed > 70) {
    Left(BrakeSpeed, BrakeTime);
  } else {
    Motor(0, 0);
  }
}


// =============================================================================
//  UTurnLeft - กลับตัวซ้าย 180° (ใช้ TurnSpeed)
// =============================================================================

void UTurnLeft() {
  UTurnLeft(TurnSpeed);
}


// =============================================================================
//  UTurnLeft - กลับตัวซ้าย 180° (กำหนดความเร็ว)
// =============================================================================

void UTurnLeft(int speed) {
  Motor(0, 0);
  delay(5);
  
  // เริ่มหมุน
  if (WheelDrive == 0) {
    sl(speed);
  } else {
    SL(speed);
  }
  
  // Delay สำหรับ 180°
  delay(turn_delay_180 / speed);
  
  // ถ้าเร็วเกิน 70 ให้ลดความเร็ว
  if (speed > 70) {
    if (WheelDrive == 0) {
      sl(60);
    } else {
      SL(60);
    }
  }
  
  // รอจนเจอเส้น
  if (speed <= 40) {
    while (read_sensorA(3) > md_sensorA(3) && read_sensorA(4) > md_sensorA(4)) {
      delayMicroseconds(50);
    }
  } else if (speed <= 50) {
    while (read_sensorA(2) > md_sensorA(2)) {
      delayMicroseconds(50);
    }
  } else if (speed <= 60) {
    while (read_sensorA(1) > md_sensorA(1)) {
      delayMicroseconds(50);
    }
  } else {
    while (read_sensorA(0) > md_sensorA(0)) {
      delayMicroseconds(50);
    }
  }
  
  // เบรค
  if (BrakeTime > 0 && speed > 70) {
    Right(BrakeSpeed, BrakeTime);
  } else {
    Motor(0, 0);
  }
}


// =============================================================================
//  UTurnRight - กลับตัวขวา 180° (ใช้ TurnSpeed)
// =============================================================================

void UTurnRight() {
  UTurnRight(TurnSpeed);
}


// =============================================================================
//  UTurnRight - กลับตัวขวา 180° (กำหนดความเร็ว)
// =============================================================================

void UTurnRight(int speed) {
  Motor(0, 0);
  delay(5);
  
  // เริ่มหมุน
  if (WheelDrive == 0) {
    sr(speed);
  } else {
    SR(speed);
  }
  
  // Delay สำหรับ 180°
  delay(turn_delay_180 / speed);
  
  // ถ้าเร็วเกิน 70 ให้ลดความเร็ว
  if (speed > 70) {
    if (WheelDrive == 0) {
      sr(60);
    } else {
      SR(60);
    }
  }
  
  // รอจนเจอเส้น
  if (speed <= 40) {
    while (read_sensorA(3) > md_sensorA(3) && read_sensorA(4) > md_sensorA(4)) {
      delayMicroseconds(50);
    }
  } else if (speed <= 50) {
    while (read_sensorA(5) > md_sensorA(5)) {
      delayMicroseconds(50);
    }
  } else if (speed <= 60) {
    while (read_sensorA(6) > md_sensorA(6)) {
      delayMicroseconds(50);
    }
  } else {
    while (read_sensorA(7) > md_sensorA(7)) {
      delayMicroseconds(50);
    }
  }
  
  // เบรค
  if (BrakeTime > 0 && speed > 70) {
    Left(BrakeSpeed, BrakeTime);
  } else {
    Motor(0, 0);
  }
}


// =============================================================================
//  CurveLeft - โค้งซ้ายอ้อมสิ่งกีดขวาง
// =============================================================================

void CurveLeft(int speed, int time) {
  Motor(0, 0);
  delay(5);
  
  // เลี้ยวซ้ายเล็กน้อย
  Left(speed, 130);
  
  // วิ่งโค้ง (ล้อซ้ายเร็วกว่า)
  if (WheelDrive == 0) {
    Motor(speed, speed * 0.58);
  } else {
    Motor(speed, speed * 0.58);
  }
  delay(900);
  
  // รอจนเจอเส้น (เซ็นเซอร์ขวา)
  while (read_sensorA(5) > md_sensorA(5)) {
    delayMicroseconds(50);
  }
  while (read_sensorA(5) < md_sensorA(5)) {
    delayMicroseconds(50);
  }
  
  delay(time);
  TurnLeft();
}


// =============================================================================
//  CurveRight - โค้งขวาอ้อมสิ่งกีดขวาง
// =============================================================================

void CurveRight(int speed, int time) {
  Motor(0, 0);
  delay(5);
  
  // เลี้ยวขวาเล็กน้อย
  Right(speed, 130);
  
  // วิ่งโค้ง (ล้อขวาเร็วกว่า)
  if (WheelDrive == 0) {
    Motor(speed * 0.58, speed);
  } else {
    Motor(speed * 0.58, speed);
  }
  delay(900);
  
  // รอจนเจอเส้น (เซ็นเซอร์ซ้าย)
  while (read_sensorA(2) > md_sensorA(2)) {
    delayMicroseconds(50);
  }
  while (read_sensorA(2) < md_sensorA(2)) {
    delayMicroseconds(50);
  }
  
  delay(time);
  TurnRight();
}


// =============================================================================
//  TurnLeftSensor - เลี้ยวซ้ายจนเจอเซ็นเซอร์ที่กำหนด
// =============================================================================

void TurnLeftSensor(int speed, int sensor) {
  Motor(0, 0);
  delay(5);
  
  sl(speed);
  delay(50);  // ให้พ้นเส้นเดิม
  
  // รอจนเจอเส้น
  while (read_sensorA(sensor) > md_sensorA(sensor)) {
    delayMicroseconds(50);
  }
  
  Motor(0, 0);
}


// =============================================================================
//  TurnRightSensor - เลี้ยวขวาจนเจอเซ็นเซอร์ที่กำหนด
// =============================================================================

void TurnRightSensor(int speed, int sensor) {
  Motor(0, 0);
  delay(5);
  
  sr(speed);
  delay(50);
  
  while (read_sensorA(sensor) > md_sensorA(sensor)) {
    delayMicroseconds(50);
  }
  
  Motor(0, 0);
}


// =============================================================================
//  TurnLeftBack - เลี้ยวซ้ายจนเจอเส้น (ใช้เซ็นเซอร์หลัง)
// =============================================================================

void TurnLeftBack(int speed) {
  Motor(0, 0);
  delay(5);
  
  sl(speed);
  delay(turn_delay_90 / speed);
  
  if (speed > 90) sl(90);
  
  // รอจนเซ็นเซอร์หลังเจอเส้น
  if (speed <= 50) {
    while (read_sensorB(3) > md_sensorB(3) && read_sensorB(4) > md_sensorB(4)) {
      delayMicroseconds(50);
    }
  } else {
    while (read_sensorB(2) > md_sensorB(2)) {
      delayMicroseconds(50);
    }
  }
  
  if (BrakeTime > 0 && speed > 70) {
    Right(BrakeSpeed, BrakeTime);
  } else {
    Motor(0, 0);
  }
}


// =============================================================================
//  TurnRightBack - เลี้ยวขวาจนเจอเส้น (ใช้เซ็นเซอร์หลัง)
// =============================================================================

void TurnRightBack(int speed) {
  Motor(0, 0);
  delay(5);
  
  sr(speed);
  delay(turn_delay_90 / speed);
  
  if (speed > 90) sr(90);
  
  // รอจนเซ็นเซอร์หลังเจอเส้น
  if (speed <= 50) {
    while (read_sensorB(3) > md_sensorB(3) && read_sensorB(4) > md_sensorB(4)) {
      delayMicroseconds(50);
    }
  } else {
    while (read_sensorB(5) > md_sensorB(5)) {
      delayMicroseconds(50);
    }
  }
  
  if (BrakeTime > 0 && speed > 70) {
    Left(BrakeSpeed, BrakeTime);
  } else {
    Motor(0, 0);
  }
}


// =============================================================================
//  ฟังก์ชันตั้งค่า
// =============================================================================

void setTurnSpeed(int speed) {
  TurnSpeed = speed;
}

void setTurnBrake(int speed, int time) {
  BrakeSpeed = speed;
  BrakeTime = time;
}

void setTurnDelay90(int delayVal) {
  turn_delay_90 = delayVal;
}

void setTurnDelay180(int delayVal) {
  turn_delay_180 = delayVal;
}

void setWheelDrive(int mode) {
  WheelDrive = mode;  // 0 = 2WD, 1 = 4WD
}
