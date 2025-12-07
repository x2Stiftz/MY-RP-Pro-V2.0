/*
 * =============================================================================
 *  Servo.ino - ควบคุม Servo แขนกล
 * =============================================================================
 */

void servo(int servoNum, int angle) {
  int idx = servoNum - 34;
  if (idx < 0 || idx > 5) return;

  int pin;
  int minPulse = 400, maxPulse = 2600;
  
  switch (servoNum) {
    case 34: pin = SERVO_34; angle += servo_trim34; break;
    case 35: pin = SERVO_35; angle = (180 - angle) - servo_trim35; break;
    case 36: pin = SERVO_36; angle += servo_trim36; minPulse = 500; maxPulse = 2500; break;
    case 37: pin = SERVO_37; minPulse = 500; maxPulse = 2500; break;
    case 38: pin = SERVO_38; minPulse = 500; maxPulse = 2500; break;
    case 39: pin = SERVO_39; minPulse = 500; maxPulse = 2500; break;
    default: return;
  }

  servo_obj[idx].attach(pin, minPulse, maxPulse);
  servo_obj[idx].write(constrain(angle, 0, 180));
}

void s34_trim(int trim) { servo_trim34 = trim; }
void s35_trim(int trim) { servo_trim35 = trim; }
void s36_trim(int trim) { servo_trim36 = trim; }

void setServoTrim(int s34, int s35, int s36) {
  servo_trim34 = s34;
  servo_trim35 = s35;
  servo_trim36 = s36;
}

void setArmConfig(int down, int leftOpen, int rightOpen) {
  servo_down = down;
  servoL_open = leftOpen;
  servoR_open = rightOpen;
}


// =============================================================================
//  เคลื่อนที่นุ่มนวล
// =============================================================================

void arm_left_right(float sl, float sr, int sp) {
  float step34 = (sl - s34_before_deg) / num_steps;
  float step35 = (sr - s35_before_deg) / num_steps;

  for (int i = 0; i < num_steps; i++) {
    float pos34 = s34_before_deg + (i * step34);
    float pos35 = s35_before_deg + (i * step35);
    servo(34, pos34);
    servo(35, pos35);
    delay(sp);
  }
  servo(34, sl);
  servo(35, sr);
  s34_before_deg = sl;
  s35_before_deg = sr;
}

void arm_l_r(int l, int r, int sp) { arm_left_right(l, r, sp); }

void arm_up_down(float sl, int sp) {
  float step36 = (sl - s36_before_deg) / num_steps;

  for (int i = 0; i < num_steps; i++) {
    float pos36 = s36_before_deg + (i * step36);
    servo(36, pos36);
    delay(sp);
  }
  servo(36, sl);
  s36_before_deg = sl;
}

void setArmSteps(int steps) { num_steps = steps; }


// =============================================================================
//  ท่าสำเร็จรูป
// =============================================================================

void arm_ready() {
  servo(36, servo_down);
  servo(34, servoL_open - 30);
  servo(35, servoR_open - 30);
}

void arm_behind() {
  servo(36, servo_down);
  servo(34, servoL_open + 55);
  servo(35, servoR_open + 55);
}

void arm_open_down() {
  servo(34, servoL_open);
  servo(35, servoR_open);
  delay(300);
  servo(36, servo_down);
}

void arm_down_open() {
  servo(36, servo_down);
  delay(300);
  servo(34, servoL_open);
  servo(35, servoR_open);
}

void arm_open_up() {
  servo(34, servoL_open);
  servo(35, servoR_open);
  delay(300);
  servo(36, servo_down + 95);
}

void arm_up_open() {
  servo(36, servo_down + 95);
  delay(300);
  servo(34, servoL_open);
  servo(35, servoR_open);
}

void arm_down_close() {
  servo(36, servo_down);
  delay(100);
  servo(34, servoL_open - 91);
  servo(35, servoR_open - 91);
}

void arm_up_close() {
  servo(36, servo_down + 95);
  delay(100);
  servo(34, servoL_open - 91);
  servo(35, servoR_open - 91);
}

void arm_big_box() {
  servo(36, servo_down);
  servo(34, servoL_open - 30);
  servo(35, servoR_open - 30);
}

void arm_close() {
  servo(34, servoL_open - 91);
  servo(35, servoR_open - 91);
}

void arm_open() {
  servo(34, servoL_open);
  servo(35, servoR_open);
}

void arm_up() { servo(36, servo_down + 95); }
void arm_down() { servo(36, servo_down); }
