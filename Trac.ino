/*
 * =============================================================================
 *  Trac.ino - เดินตามเส้น TracJC
 * =============================================================================
 *  
 *  ฟังก์ชันหลัก:
 *    TracJC()        - เดินจนเจอแยก
 *    TracJCSpeed()   - เดินเร็วจนเจอแยก
 *    TJCSS(n)        - เดินผ่าน n แยก
 * =============================================================================
 */

// Timer Functions
void StartTimer() { _timerStart = millis(); }
unsigned long ReadTimer() { return millis() - _timerStart; }
void StartTimer3() { _timer3Start = millis(); }
unsigned long ReadTimer3() { return millis() - _timer3Start; }

// Initialize Speed
void InitialSpeed() {
  setMotorOffset(BaseSpeed);
}

// คำนวณ Error
float CalError_Line() {
  float pos = position_A() / 50.0;
  float setpoint = 50.0;
  return setpoint - pos;
}

void CalError() {
  ConvertADC();
  
  if (L3 == 0 && R3 == 0) {
    Error = 99;
  } else if (L3 == 0 && R3 == 1) {
    Error = 101;
  } else if (L3 == 1 && R3 == 0) {
    Error = 102;
  } else {
    Error = CalError_Line();
  }
}

// TracPID
void TracPID() {
  float P = Error;
  float D = Error - _prevError;
  _prevError = Error;
  _integral += Error;
  
  float output = (LINE_KP * P) + (LINE_KI * _integral) + (LINE_KD * D);
  
  int leftSpeed  = BaseSpeed - output - MOTOR_LEFT_OFFSET;
  int rightSpeed = BaseSpeed + output - MOTOR_RIGHT_OFFSET;
  
  Motor(constrain(leftSpeed, -100, 100), constrain(rightSpeed, -100, 100));
}

void TracPIDBack() {
  float P = Error;
  float D = Error - _prevError;
  _prevError = Error;
  _integral += Error;
  
  float output = (LINE_KP * P) + (LINE_KI * _integral) + (LINE_KD * D);
  
  int leftSpeed  = -(BaseSpeed + output - MOTOR_LEFT_OFFSET);
  int rightSpeed = -(BaseSpeed - output - MOTOR_RIGHT_OFFSET);
  
  Motor(constrain(leftSpeed, -100, 100), constrain(rightSpeed, -100, 100));
}

// เดินตามเส้นเป็นเวลา
void TracSpeedTime(int TracSpeed, int TracTime) {
  int TempSpeed = BaseSpeed;
  BaseSpeed = TracSpeed;
  InitialSpeed();
  
  StartTimer();
  CalError();
  
  while (ReadTimer() < TracTime) {
    if (Error >= 99) Error = 0;
    TracPID();
    delay(Kt);
    CalError();
  }
  
  ErrorClear = 1;
  BaseSpeed = TempSpeed;
  InitialSpeed();
}

// เดินจนเจอแยก
void TracJCStop() {
  InitialSpeed();
  TracSpeedTime(BaseSpeed, 5000 / BaseSpeed);
  CalError();
  
  while (Error < 99) {
    TracPID();
    StartTimer();
    CalError();
    while ((ReadTimer() < Kt) && (Error < 99)) {
      CalError();
    }
  }
  ErrorClear = 0;
}

void TuneJC() {
  Forward(BaseSpeed);
  if (BaseSpeed <= 50) delay(4000 / BaseSpeed);
  else if (BaseSpeed <= 60) delay(3000 / BaseSpeed);
  else if (BaseSpeed <= 70) delay(1500 / BaseSpeed);
  else if (BaseSpeed <= 80) delay(500 / BaseSpeed);
  else if (BaseSpeed <= 90) delay(100 / BaseSpeed);
  MotorStop();
}

void TracJC() {
  TracJCStop();
  TuneJC();
}

void TracJCStraight() {
  TracJC();
  ForwardDelay(5000 / BaseSpeed);
}

void TracJCSlowStop() {
  BaseSpeed = SlowSpeed;
  TracJCStop();
  BaseSpeed = Speed;
  InitialSpeed();
}

void TracJCSlow() {
  TracJCSlowStop();
  Forward(BaseSpeed);
  while (Error >= 100) {
    CalError();
  }
  MotorStop();
}

void TracJCSpeedStop() {
  BaseSpeed = ACCSpeed;
  TracJCStop();
  BaseSpeed = Speed;
  InitialSpeed();
}

void TracJCSpeed() {
  int TempSpeed = BaseSpeed;
  BaseSpeed = ACCSpeed;
  TracJCStop();
  Forward(BaseSpeed);
  delay(5000 / BaseSpeed);
  BaseSpeed = TempSpeed;
  InitialSpeed();
}

void TracJCSpeedLine(int line) {
  int TempSpeed = BaseSpeed;
  BaseSpeed = ACCSpeed;
  for (int i = 0; i < line; i++) {
    TracJCStop();
    Forward(BaseSpeed);
    delay(5000 / BaseSpeed);
  }
  BaseSpeed = TempSpeed;
  InitialSpeed();
}

void TracJCSpeedTime(int MotorSpeed, int TimeDelay) {
  BaseSpeed = MotorSpeed;
  TracJCStop();
  Forward(BaseSpeed);
  delay(TimeDelay);
  BaseSpeed = Speed;
  InitialSpeed();
}


// =============================================================================
//  TJCSS Functions
// =============================================================================

void TJCSS(int line) {
  for (int i = 0; i < line; i++) {
    TracJCSpeed();
  }
  TracJC();
  MotorStop();
}

void TJCSS() { TJCSS(1); }

void TJCSSL(int line) {
  for (int i = 0; i < line; i++) {
    TracJCSpeed();
  }
  TracJCSlow();
  MotorStop();
}

void TJCSSL() { TJCSSL(1); }

void TJCSSL2(int line) {
  for (int i = 0; i < line; i++) {
    TracJC();
  }
  TracJCSlow();
  MotorStop();
}

void TJCSSL2() { TJCSSL2(1); }

void TJCSSA(int line, int Tracs) {
  for (int i = 0; i < line; i++) {
    TracJCSpeed();
  }
  for (int a = 0; a < Tracs; a++) {
    TracJC();
  }
  MotorStop();
}

void TJCSSB(int line, int Tracc) {
  for (int i = 0; i < line; i++) {
    TracJCSpeed();
  }
  for (int a = 0; a < Tracc; a++) {
    TracJCSlow();
  }
  MotorStop();
}


// =============================================================================
//  TracJC แบบพิเศษ
// =============================================================================

void TracJCStopLorR() {
  InitialSpeed();
  TracSpeedTime(BaseSpeed, 10000 / BaseSpeed);
  CalError();
  
  while (L3 == 1 && R3 == 1) {
    TracPID();
    StartTimer();
    CalError();
    while ((ReadTimer() < Kt) && (Error < 100)) {
      CalError();
    }
    if (Error >= 99) Error = 0;
  }
  ErrorClear = 0;
}

void TracJCLorR() { TracJCStopLorR(); TuneJC(); }

void TracJCSpeedLorR() {
  BaseSpeed = ACCSpeed;
  TracJCStopLorR();
  Forward(BaseSpeed);
  while (Error >= 100) { CalError(); }
  delay(3000 / BaseSpeed);
  BaseSpeed = Speed;
  ErrorClear = 0;
  InitialSpeed();
}

void TracJCStopL() {
  InitialSpeed();
  TracSpeedTime(BaseSpeed, 10000 / BaseSpeed);
  CalError();
  
  while (L3 == 1) {
    TracPID();
    StartTimer();
    CalError();
    while ((ReadTimer() < Kt) && (Error < 100)) { CalError(); }
    if (Error >= 99) Error = 0;
  }
  ErrorClear = 0;
}

void TracJCL() { TracJCStopL(); TuneJC(); }

void TracJCStopR() {
  InitialSpeed();
  TracSpeedTime(BaseSpeed, 10000 / BaseSpeed);
  CalError();
  
  while (R3 == 1) {
    TracPID();
    StartTimer();
    CalError();
    while ((ReadTimer() < Kt) && (Error < 100)) { CalError(); }
    if (Error >= 99) Error = 0;
  }
  ErrorClear = 0;
}

void TracJCR() { TracJCStopR(); TuneJC(); }

void TracJCStopLeft() {
  InitialSpeed();
  CalError();
  
  while (Error != 101) {
    if (Error > 99) Error = 0;
    TracPID();
    StartTimer();
    while ((ReadTimer() < Kt) && (Error != 101)) { CalError(); }
  }
  ErrorClear = 0;
}

void TracJCLeft() { TracJCStopLeft(); TuneJC(); }

void TracJCStopRight() {
  InitialSpeed();
  CalError();
  
  while (Error != 102) {
    if (Error > 99) Error = 0;
    TracPID();
    StartTimer();
    while ((ReadTimer() < Kt) && (Error != 102)) { CalError(); }
  }
  ErrorClear = 0;
}

void TracJCRight() { TracJCStopRight(); TuneJC(); }


// =============================================================================
//  ตั้งค่า
// =============================================================================

void setSpeed(int normalSpeed, int slowSpeed, int accSpeed) {
  Speed = normalSpeed;
  BaseSpeed = normalSpeed;
  SlowSpeed = slowSpeed;
  ACCSpeed = accSpeed;
}

void setLinePID(float kp, float ki, float kd) {
  LINE_KP = kp;
  LINE_KI = ki;
  LINE_KD = kd;
}

void setKt(int kt) { Kt = kt; }
