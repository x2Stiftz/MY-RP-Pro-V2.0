/*
 * =============================================================================
 *  Gyro.ino - อ่านค่า Gyroscope (BMI160)
 * =============================================================================
 */

void gyro_writeRegister(uint8_t reg, uint8_t value) {
  Wire1.beginTransmission(_gyroAddress);
  Wire1.write(reg);
  Wire1.write(value);
  Wire1.endTransmission();
}

bool gyro_begin() {
  Wire1.begin();
  Wire1.setClock(400000);

  // Check chip ID
  Wire1.beginTransmission(_gyroAddress);
  Wire1.write(0x00);
  Wire1.endTransmission();
  Wire1.requestFrom(_gyroAddress, (uint8_t)1);
  
  if (Wire1.available()) {
    uint8_t chipID = Wire1.read();
    if (chipID != 0xD1) return false;
  } else {
    return false;
  }

  // Reset sensor
  gyro_writeRegister(0x7E, 0xB6);
  delay(100);

  // Configure accelerometer (±2g, ODR 400Hz)
  gyro_writeRegister(0x40, 0x2A);
  gyro_writeRegister(0x41, 0x03);

  // Configure gyroscope (±2000 dps, ODR 400Hz)
  gyro_writeRegister(0x42, 0x2A);
  gyro_writeRegister(0x43, 0x00);

  // Start accelerometer and gyroscope
  gyro_writeRegister(0x7E, 0x11);
  delay(50);
  gyro_writeRegister(0x7E, 0x15);
  delay(50);

  // Calibrate
  const int samples = 200;
  float sumX = 0, sumY = 0, sumZ = 0;
  float sumAX = 0, sumAY = 0, sumAZ = 0;

  for (int i = 0; i < samples; i++) {
    Wire1.beginTransmission(_gyroAddress);
    Wire1.write(0x0C);
    Wire1.endTransmission();
    Wire1.requestFrom(_gyroAddress, (uint8_t)12);

    if (Wire1.available() == 12) {
      int16_t gx = Wire1.read() | (Wire1.read() << 8);
      int16_t gy = Wire1.read() | (Wire1.read() << 8);
      int16_t gz = Wire1.read() | (Wire1.read() << 8);
      int16_t ax = Wire1.read() | (Wire1.read() << 8);
      int16_t ay = Wire1.read() | (Wire1.read() << 8);
      int16_t az = Wire1.read() | (Wire1.read() << 8);

      sumX += gx / 16.4;
      sumY += gy / 16.4;
      sumZ += gz / 16.4;
      sumAX += ax / 16384.0;
      sumAY += ay / 16384.0;
      sumAZ += az / 16384.0;
    }
    delay(5);
  }

  _gyroOffsetX = sumX / samples;
  _gyroOffsetY = sumY / samples;
  _gyroOffsetZ = sumZ / samples;
  _accelX_prev = sumAX / samples;
  _accelY_prev = sumAY / samples;
  _accelZ_prev = sumAZ / samples;
  _gyroLastTime = micros();

  return true;
}

void gyro_readAngles(float &roll, float &pitch, float &yaw) {
  Wire1.beginTransmission(_gyroAddress);
  Wire1.write(0x0C);
  Wire1.endTransmission();
  Wire1.requestFrom(_gyroAddress, (uint8_t)12);

  int16_t gx, gy, gz, ax, ay, az;
  if (Wire1.available() == 12) {
    gx = Wire1.read() | (Wire1.read() << 8);
    gy = Wire1.read() | (Wire1.read() << 8);
    gz = Wire1.read() | (Wire1.read() << 8);
    ax = Wire1.read() | (Wire1.read() << 8);
    ay = Wire1.read() | (Wire1.read() << 8);
    az = Wire1.read() | (Wire1.read() << 8);
  } else {
    return;
  }

  float accelX = ax / 16384.0;
  float accelY = ay / 16384.0;
  float accelZ = az / 16384.0;

  accelX = ACCEL_FILTER_ALPHA * accelX + (1.0 - ACCEL_FILTER_ALPHA) * _accelX_prev;
  accelY = ACCEL_FILTER_ALPHA * accelY + (1.0 - ACCEL_FILTER_ALPHA) * _accelY_prev;
  accelZ = ACCEL_FILTER_ALPHA * accelZ + (1.0 - ACCEL_FILTER_ALPHA) * _accelZ_prev;
  _accelX_prev = accelX;
  _accelY_prev = accelY;
  _accelZ_prev = accelZ;

  float gyroX = (gx / 16.4) - _gyroOffsetX;
  float gyroY = (gy / 16.4) - _gyroOffsetY;
  float gyroZ = (gz / 16.4) - _gyroOffsetZ;

  // Dynamic offset update
  const float GYRO_CALIBRATION_ALPHA = 0.01;
  const float GYRO_CAL_THRESHOLD = 0.5;
  
  if (abs(gx / 16.4) < GYRO_CAL_THRESHOLD) {
    _gyroOffsetX = GYRO_CALIBRATION_ALPHA * (gx / 16.4) + (1.0 - GYRO_CALIBRATION_ALPHA) * _gyroOffsetX;
  }
  if (abs(gy / 16.4) < GYRO_CAL_THRESHOLD) {
    _gyroOffsetY = GYRO_CALIBRATION_ALPHA * (gy / 16.4) + (1.0 - GYRO_CALIBRATION_ALPHA) * _gyroOffsetY;
  }
  if (abs(gz / 16.4) < GYRO_CAL_THRESHOLD) {
    _gyroOffsetZ = GYRO_CALIBRATION_ALPHA * (gz / 16.4) + (1.0 - GYRO_CALIBRATION_ALPHA) * _gyroOffsetZ;
  }

  // Deadzone
  if (abs(gyroX) < GYRO_THRESHOLD) gyroX = 0.0;
  if (abs(gyroY) < GYRO_THRESHOLD) gyroY = 0.0;
  if (abs(gyroZ) < GYRO_THRESHOLD) gyroZ = 0.0;

  float accelRoll = atan2(accelY, accelZ) * 180.0 / PI;
  float accelPitch = atan2(-accelX, sqrt(accelY * accelY + accelZ * accelZ)) * 180.0 / PI;

  unsigned long currentTime = micros();
  float deltaTime = (currentTime - _gyroLastTime) / 1000000.0;
  if (deltaTime > 0.1) deltaTime = 0.1;
  _gyroLastTime = currentTime;

  float gyroAngleX = _angleX + gyroX * deltaTime;
  float gyroAngleY = _angleY + gyroY * deltaTime;
  float gyroAngleZ = _angleZ + gyroZ * deltaTime;

  _angleX = ALPHA * gyroAngleX + (1.0 - ALPHA) * accelRoll;
  _angleY = ALPHA * gyroAngleY + (1.0 - ALPHA) * accelPitch;
  _angleZ = gyroAngleZ;

  // Wrap yaw to [-180, 180]
  if (_angleZ > 180) _angleZ -= 360;
  else if (_angleZ < -180) _angleZ += 360;

  roll = _angleX;
  pitch = _angleY;
  yaw = _angleZ;
}

// อ่านค่า Gyro แกนที่ต้องการ
float gyro(char axis) {
  float roll, pitch, yaw;
  gyro_readAngles(roll, pitch, yaw);
  switch (axis) {
    case 'x': case 'X': return roll;
    case 'y': case 'Y': return pitch;
    case 'z': case 'Z': return yaw;
    default: return 0.0;
  }
}

// รีเซ็ต Yaw เป็น 0
void resetYaw() {
  _angleZ = 0.0;
}

// รีเซ็ตทุกมุมเป็น 0
void resetAngles() {
  _angleX = 0.0;
  _angleY = 0.0;
  _angleZ = 0.0;
}

// ตั้งค่า PID สำหรับ Gyro
void setGyroPID(float kp, float ki, float kd) {
  GYRO_KP = kp;
  GYRO_KI = ki;
  GYRO_KD = kd;
}
