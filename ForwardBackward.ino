/*
 * =============================================================================
 *  ForwardBackward.ino - เดินหน้า/ถอยหลัง
 * =============================================================================
 *  
 *  รูปแบบ: Forward(speed, kp, delayMs)
 *          Backward(speed, kp, delayMs)
 *  
 *  ตัวอย่าง:
 *    Forward(50, 1.5, 500);   // เดินหน้า speed=50, kp=1.5, 500ms
 *    Backward(40, 1.2, 300);  // ถอยหลัง speed=40, kp=1.2, 300ms
 * =============================================================================
 */

//-----------------------------------------------------------------------------
// เดินหน้า (speed, kp, delayMs)
//-----------------------------------------------------------------------------
void Forward(int speed, float kp, int delayMs) {
  // ตั้งค่า offset จาก speed
  setMotorOffset(speed);
  resetAngles();
  
  _integral = 0;
  _prevError = 0;
  unsigned long startTime = millis();

  while (millis() - startTime < delayMs) {
    float yaw = gyro('z');
    float err = -yaw;

    _integral += err;
    float deriv = err - _prevError;
    _prevError = err;

    float corr = (kp * err) + (GYRO_KI * _integral) + (GYRO_KD * deriv);

    int leftSpeed  = constrain(speed - corr - MOTOR_LEFT_OFFSET, -100, 100);
    int rightSpeed = constrain(speed + corr - MOTOR_RIGHT_OFFSET, -100, 100);
    Motor(leftSpeed, rightSpeed);

    delayMicroseconds(50);
  }
  MotorBrake();
}

//-----------------------------------------------------------------------------
// เดินหน้าไม่จำกัดเวลา (speed)
//-----------------------------------------------------------------------------
void Forward(int speed) {
  setMotorOffset(speed);
  int leftSpeed  = speed - MOTOR_LEFT_OFFSET;
  int rightSpeed = speed - MOTOR_RIGHT_OFFSET;
  Motor(leftSpeed, rightSpeed);
}

//-----------------------------------------------------------------------------
// เดินหน้าจนเจอเซ็นเซอร์ (speed, kp, sensor)
//-----------------------------------------------------------------------------
void ForwardUntil(int speed, float kp, String sensor) {
  setMotorOffset(speed);
  resetAngles();
  
  _integral = 0;
  _prevError = 0;

  while (true) {
    float yaw = gyro('z');
    float err = -yaw;

    _integral += err;
    float deriv = err - _prevError;
    _prevError = err;

    float corr = (kp * err) + (GYRO_KI * _integral) + (GYRO_KD * deriv);

    int leftSpeed  = constrain(speed - corr - MOTOR_LEFT_OFFSET, -100, 100);
    int rightSpeed = constrain(speed + corr - MOTOR_RIGHT_OFFSET, -100, 100);
    Motor(leftSpeed, rightSpeed);

    // ตรวจสอบเซ็นเซอร์
    bool stop = false;
    if (sensor == "a0" && read_sensorA(0) < md_sensorA(0)) stop = true;
    else if (sensor == "a7" && read_sensorA(7) < md_sensorA(7)) stop = true;
    else if (sensor == "a07" && read_sensorA(0) < md_sensorA(0) && read_sensorA(7) < md_sensorA(7)) stop = true;
    else if (sensor == "b0" && read_sensorB(0) < md_sensorB(0)) stop = true;
    else if (sensor == "b7" && read_sensorB(7) < md_sensorB(7)) stop = true;
    else if (sensor == "c0" && analogRead(CENTER_SENSOR_L) < md_sensorC(0)) stop = true;
    else if (sensor == "c1" && analogRead(CENTER_SENSOR_R) < md_sensorC(1)) stop = true;

    if (stop) break;

    delayMicroseconds(50);
  }
  MotorBrake();
}


//-----------------------------------------------------------------------------
// ถอยหลัง (speed, kp, delayMs)
//-----------------------------------------------------------------------------
void Backward(int speed, float kp, int delayMs) {
  setMotorOffset(speed);
  resetAngles();
  
  _integral = 0;
  _prevError = 0;
  unsigned long startTime = millis();

  while (millis() - startTime < delayMs) {
    float yaw = gyro('z');
    float err = -yaw;

    _integral += err;
    float deriv = err - _prevError;
    _prevError = err;

    float corr = (kp * err) + (GYRO_KI * _integral) + (GYRO_KD * deriv);

    int leftSpeed  = constrain(-(speed - corr - MOTOR_LEFT_OFFSET), -100, 100);
    int rightSpeed = constrain(-(speed + corr - MOTOR_RIGHT_OFFSET), -100, 100);
    Motor(leftSpeed, rightSpeed);

    delayMicroseconds(50);
  }
  MotorBrake();
}

//-----------------------------------------------------------------------------
// ถอยหลังไม่จำกัดเวลา (speed)
//-----------------------------------------------------------------------------
void Backward(int speed) {
  setMotorOffset(speed);
  int leftSpeed  = -(speed - MOTOR_LEFT_OFFSET);
  int rightSpeed = -(speed - MOTOR_RIGHT_OFFSET);
  Motor(leftSpeed, rightSpeed);
}

//-----------------------------------------------------------------------------
// ถอยหลังจนเจอเซ็นเซอร์ (speed, kp, sensor)
//-----------------------------------------------------------------------------
void BackwardUntil(int speed, float kp, String sensor) {
  setMotorOffset(speed);
  resetAngles();
  
  _integral = 0;
  _prevError = 0;

  while (true) {
    float yaw = gyro('z');
    float err = -yaw;

    _integral += err;
    float deriv = err - _prevError;
    _prevError = err;

    float corr = (kp * err) + (GYRO_KI * _integral) + (GYRO_KD * deriv);

    int leftSpeed  = constrain(-(speed - corr - MOTOR_LEFT_OFFSET), -100, 100);
    int rightSpeed = constrain(-(speed + corr - MOTOR_RIGHT_OFFSET), -100, 100);
    Motor(leftSpeed, rightSpeed);

    // ตรวจสอบเซ็นเซอร์
    bool stop = false;
    if (sensor == "b0" && read_sensorB(0) < md_sensorB(0)) stop = true;
    else if (sensor == "b7" && read_sensorB(7) < md_sensorB(7)) stop = true;
    else if (sensor == "b07" && read_sensorB(0) < md_sensorB(0) && read_sensorB(7) < md_sensorB(7)) stop = true;
    else if (sensor == "a0" && read_sensorA(0) < md_sensorA(0)) stop = true;
    else if (sensor == "a7" && read_sensorA(7) < md_sensorA(7)) stop = true;
    else if (sensor == "c0" && analogRead(CENTER_SENSOR_L) < md_sensorC(0)) stop = true;
    else if (sensor == "c1" && analogRead(CENTER_SENSOR_R) < md_sensorC(1)) stop = true;

    if (stop) break;

    delayMicroseconds(50);
  }
  MotorBrake();
}


//-----------------------------------------------------------------------------
// เดินหน้า delay แบบง่าย
//-----------------------------------------------------------------------------
void ForwardDelay(int delayMs) {
  Forward(BaseSpeed);
  delay(delayMs);
  MotorStop();
}

//-----------------------------------------------------------------------------
// ถอยหลัง delay แบบง่าย
//-----------------------------------------------------------------------------
void BackwardDelay(int delayMs) {
  Backward(BaseSpeed);
  delay(delayMs);
  MotorStop();
}
