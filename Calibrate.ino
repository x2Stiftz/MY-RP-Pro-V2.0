/*
 * =============================================================================
 *  Calibrate.ino - วัดค่าเซ็นเซอร์เส้น (Calibration)
 * =============================================================================
 *  
 *  วิธีใช้:
 *    1. เรียก calibrate_manual()   - วัดแบบ Manual (กดปุ่มวัดขาว/ดำ)
 *    2. เรียก calibrate_auto()     - วัดแบบ Auto (หมุนตัวบนเส้น)
 *    3. เรียก save_calibration()   - บันทึกค่าลง EEPROM
 *    4. เรียก load_calibration()   - โหลดค่าจาก EEPROM (เรียกใน setup อัตโนมัติ)
 *  
 *  ค่าที่วัด:
 *    - sensorMin_A[0-7] : ค่าต่ำสุด (สีดำ) เซ็นเซอร์หน้า
 *    - sensorMax_A[0-7] : ค่าสูงสุด (สีขาว) เซ็นเซอร์หน้า
 *    - sensorMin_B[0-7] : ค่าต่ำสุด (สีดำ) เซ็นเซอร์หลัง
 *    - sensorMax_B[0-7] : ค่าสูงสุด (สีขาว) เซ็นเซอร์หลัง
 *    - sensorMin_C[0-1] : ค่าต่ำสุด (สีดำ) เซ็นเซอร์กลาง
 *    - sensorMax_C[0-1] : ค่าสูงสุด (สีขาว) เซ็นเซอร์กลาง
 * =============================================================================
 */

// =============================================================================
//  Calibration แบบ Manual (กดปุ่มวัดขาว/ดำ)
// =============================================================================

void calibrate_manual() {
  Serial.println("=== CALIBRATION MODE ===");
  Serial.println("วางหุ่นบนพื้นขาว แล้วกดปุ่ม 1");
  
  led_rgb('w');  // ไฟขาว = วัดสีขาว
  sw();
  
  // วัดค่าสีขาว (Max)
  Serial.println("กำลังวัดค่าสีขาว...");
  for (int i = 0; i < NUM_SENSORS; i++) {
    long sum_a = 0, sum_b = 0;
    for (int j = 0; j < 10; j++) {
      sum_a += read_sensorA(i);
      sum_b += read_sensorB(i);
      delay(10);
    }
    sensorMax_A[i] = sum_a / 10;
    sensorMax_B[i] = sum_b / 10;
  }
  
  // วัดเซ็นเซอร์กลาง
  long sum_c0 = 0, sum_c1 = 0;
  for (int j = 0; j < 10; j++) {
    sum_c0 += analogRead(CENTER_SENSOR_L);
    sum_c1 += analogRead(CENTER_SENSOR_R);
    delay(10);
  }
  sensorMax_C[0] = sum_c0 / 10;
  sensorMax_C[1] = sum_c1 / 10;
  
  beep(2000, 200);
  Serial.println("วัดค่าสีขาวเสร็จ!");
  print_calibration();
  
  // วัดค่าสีดำ
  Serial.println("\nวางหุ่นบนเส้นดำ แล้วกดปุ่ม 1");
  led_rgb('r');  // ไฟแดง = วัดสีดำ
  sw();
  
  Serial.println("กำลังวัดค่าสีดำ...");
  for (int i = 0; i < NUM_SENSORS; i++) {
    long sum_a = 0, sum_b = 0;
    for (int j = 0; j < 10; j++) {
      sum_a += read_sensorA(i);
      sum_b += read_sensorB(i);
      delay(10);
    }
    sensorMin_A[i] = sum_a / 10;
    sensorMin_B[i] = sum_b / 10;
  }
  
  // วัดเซ็นเซอร์กลาง
  sum_c0 = 0; sum_c1 = 0;
  for (int j = 0; j < 10; j++) {
    sum_c0 += analogRead(CENTER_SENSOR_L);
    sum_c1 += analogRead(CENTER_SENSOR_R);
    delay(10);
  }
  sensorMin_C[0] = sum_c0 / 10;
  sensorMin_C[1] = sum_c1 / 10;
  
  beep(2000, 200);
  Serial.println("วัดค่าสีดำเสร็จ!");
  print_calibration();
  
  // ถามว่าจะบันทึกไหม
  Serial.println("\nกดปุ่ม 1 เพื่อบันทึกลง EEPROM");
  led_rgb('g');
  sw();
  
  save_calibration();
  beep(3000, 500);
  Serial.println("บันทึกเสร็จสิ้น!");
  led_off();
}


// =============================================================================
//  Calibration แบบ Auto (หมุนตัวบนเส้น)
// =============================================================================

void calibrate_auto() {
  Serial.println("=== AUTO CALIBRATION ===");
  Serial.println("วางหุ่นบนเส้นดำ (ตรงกลาง) แล้วกดปุ่ม 1");
  
  led_rgb('b');
  sw();
  
  // ตั้งค่าเริ่มต้น
  for (int i = 0; i < NUM_SENSORS; i++) {
    sensorMin_A[i] = 9999;
    sensorMax_A[i] = 0;
    sensorMin_B[i] = 9999;
    sensorMax_B[i] = 0;
  }
  sensorMin_C[0] = sensorMin_C[1] = 9999;
  sensorMax_C[0] = sensorMax_C[1] = 0;
  
  Serial.println("กำลัง Calibrate... (หมุนตัว 3 วินาที)");
  
  unsigned long startTime = millis();
  bool dir = true;
  
  // หมุนตัว 3 วินาที
  while (millis() - startTime < 3000) {
    // หมุนซ้าย-ขวา สลับกัน
    if ((millis() - startTime) % 500 < 250) {
      Motor(-25, 25);
    } else {
      Motor(25, -25);
    }
    
    // อ่านค่าและบันทึก min/max
    for (int i = 0; i < NUM_SENSORS; i++) {
      int val_a = read_sensorA(i);
      int val_b = read_sensorB(i);
      
      if (val_a < sensorMin_A[i]) sensorMin_A[i] = val_a;
      if (val_a > sensorMax_A[i]) sensorMax_A[i] = val_a;
      if (val_b < sensorMin_B[i]) sensorMin_B[i] = val_b;
      if (val_b > sensorMax_B[i]) sensorMax_B[i] = val_b;
    }
    
    int val_c0 = analogRead(CENTER_SENSOR_L);
    int val_c1 = analogRead(CENTER_SENSOR_R);
    if (val_c0 < sensorMin_C[0]) sensorMin_C[0] = val_c0;
    if (val_c0 > sensorMax_C[0]) sensorMax_C[0] = val_c0;
    if (val_c1 < sensorMin_C[1]) sensorMin_C[1] = val_c1;
    if (val_c1 > sensorMax_C[1]) sensorMax_C[1] = val_c1;
    
    delay(5);
  }
  
  Motor(0, 0);
  beep(2000, 200);
  
  Serial.println("Calibrate เสร็จ!");
  print_calibration();
  
  // ถามว่าจะบันทึกไหม
  Serial.println("\nกดปุ่ม 1 เพื่อบันทึกลง EEPROM");
  led_rgb('g');
  sw();
  
  save_calibration();
  beep(3000, 500);
  Serial.println("บันทึกเสร็จสิ้น!");
  led_off();
}


// =============================================================================
//  Calibration เฉพาะเซ็นเซอร์หน้า
// =============================================================================

void calibrate_front() {
  Serial.println("=== CALIBRATE FRONT SENSORS ===");
  Serial.println("วางหุ่นบนเส้นดำ แล้วกดปุ่ม 1");
  
  led_rgb('b');
  sw();
  
  for (int i = 0; i < NUM_SENSORS; i++) {
    sensorMin_A[i] = 9999;
    sensorMax_A[i] = 0;
  }
  
  Serial.println("กำลัง Calibrate...");
  
  unsigned long startTime = millis();
  while (millis() - startTime < 2000) {
    Motor(-20, 20);
    for (int i = 0; i < NUM_SENSORS; i++) {
      int val = read_sensorA(i);
      if (val < sensorMin_A[i]) sensorMin_A[i] = val;
      if (val > sensorMax_A[i]) sensorMax_A[i] = val;
    }
    delay(5);
  }
  
  startTime = millis();
  while (millis() - startTime < 2000) {
    Motor(20, -20);
    for (int i = 0; i < NUM_SENSORS; i++) {
      int val = read_sensorA(i);
      if (val < sensorMin_A[i]) sensorMin_A[i] = val;
      if (val > sensorMax_A[i]) sensorMax_A[i] = val;
    }
    delay(5);
  }
  
  Motor(0, 0);
  beep(2000, 200);
  
  Serial.println("Calibrate เซ็นเซอร์หน้าเสร็จ!");
  Serial.println("Sensor A (Front):");
  for (int i = 0; i < NUM_SENSORS; i++) {
    Serial.print("  A"); Serial.print(i);
    Serial.print(": "); Serial.print(sensorMin_A[i]);
    Serial.print(" - "); Serial.println(sensorMax_A[i]);
  }
  
  led_off();
}


// =============================================================================
//  Calibration เฉพาะเซ็นเซอร์หลัง
// =============================================================================

void calibrate_back() {
  Serial.println("=== CALIBRATE BACK SENSORS ===");
  Serial.println("วางหุ่นบนเส้นดำ แล้วกดปุ่ม 1");
  
  led_rgb('b');
  sw();
  
  for (int i = 0; i < NUM_SENSORS; i++) {
    sensorMin_B[i] = 9999;
    sensorMax_B[i] = 0;
  }
  
  Serial.println("กำลัง Calibrate...");
  
  unsigned long startTime = millis();
  while (millis() - startTime < 2000) {
    Motor(-20, 20);
    for (int i = 0; i < NUM_SENSORS; i++) {
      int val = read_sensorB(i);
      if (val < sensorMin_B[i]) sensorMin_B[i] = val;
      if (val > sensorMax_B[i]) sensorMax_B[i] = val;
    }
    delay(5);
  }
  
  startTime = millis();
  while (millis() - startTime < 2000) {
    Motor(20, -20);
    for (int i = 0; i < NUM_SENSORS; i++) {
      int val = read_sensorB(i);
      if (val < sensorMin_B[i]) sensorMin_B[i] = val;
      if (val > sensorMax_B[i]) sensorMax_B[i] = val;
    }
    delay(5);
  }
  
  Motor(0, 0);
  beep(2000, 200);
  
  Serial.println("Calibrate เซ็นเซอร์หลังเสร็จ!");
  Serial.println("Sensor B (Back):");
  for (int i = 0; i < NUM_SENSORS; i++) {
    Serial.print("  B"); Serial.print(i);
    Serial.print(": "); Serial.print(sensorMin_B[i]);
    Serial.print(" - "); Serial.println(sensorMax_B[i]);
  }
  
  led_off();
}


// =============================================================================
//  Calibration เซ็นเซอร์กลาง
// =============================================================================

void calibrate_center() {
  Serial.println("=== CALIBRATE CENTER SENSORS ===");
  
  // วัดสีขาว
  Serial.println("วางเซ็นเซอร์กลางบนพื้นขาว แล้วกดปุ่ม 1");
  led_rgb('w');
  sw();
  
  long sum_c0 = 0, sum_c1 = 0;
  for (int j = 0; j < 20; j++) {
    sum_c0 += analogRead(CENTER_SENSOR_L);
    sum_c1 += analogRead(CENTER_SENSOR_R);
    delay(10);
  }
  sensorMax_C[0] = sum_c0 / 20;
  sensorMax_C[1] = sum_c1 / 20;
  
  beep(2000, 100);
  Serial.print("ขาว: C0="); Serial.print(sensorMax_C[0]);
  Serial.print(" C1="); Serial.println(sensorMax_C[1]);
  
  // วัดสีดำ
  Serial.println("\nวางเซ็นเซอร์กลางบนเส้นดำ แล้วกดปุ่ม 1");
  led_rgb('r');
  sw();
  
  sum_c0 = 0; sum_c1 = 0;
  for (int j = 0; j < 20; j++) {
    sum_c0 += analogRead(CENTER_SENSOR_L);
    sum_c1 += analogRead(CENTER_SENSOR_R);
    delay(10);
  }
  sensorMin_C[0] = sum_c0 / 20;
  sensorMin_C[1] = sum_c1 / 20;
  
  beep(2000, 100);
  Serial.print("ดำ: C0="); Serial.print(sensorMin_C[0]);
  Serial.print(" C1="); Serial.println(sensorMin_C[1]);
  
  Serial.println("\nCalibrate เซ็นเซอร์กลางเสร็จ!");
  led_off();
}


// =============================================================================
//  บันทึกค่า Calibration ลง EEPROM
// =============================================================================

void save_calibration() {
  byte buffer[4];
  
  Serial.println("กำลังบันทึกลง EEPROM...");
  
  // บันทึก Sensor A (address 0-31)
  for (int i = 0; i < NUM_SENSORS; i++) {
    buffer[0] = (sensorMin_A[i] >> 8) & 0xFF;
    buffer[1] = sensorMin_A[i] & 0xFF;
    buffer[2] = (sensorMax_A[i] >> 8) & 0xFF;
    buffer[3] = sensorMax_A[i] & 0xFF;
    writeEEPROM(EEPROM_ADDRESS, i * 4, buffer, 4);
    delay(10);
  }
  
  // บันทึก Sensor B (address 32-63)
  for (int i = 0; i < NUM_SENSORS; i++) {
    buffer[0] = (sensorMin_B[i] >> 8) & 0xFF;
    buffer[1] = sensorMin_B[i] & 0xFF;
    buffer[2] = (sensorMax_B[i] >> 8) & 0xFF;
    buffer[3] = sensorMax_B[i] & 0xFF;
    writeEEPROM(EEPROM_ADDRESS, 32 + i * 4, buffer, 4);
    delay(10);
  }
  
  // บันทึก Sensor C (address 64-71)
  for (int i = 0; i < 2; i++) {
    buffer[0] = (sensorMin_C[i] >> 8) & 0xFF;
    buffer[1] = sensorMin_C[i] & 0xFF;
    buffer[2] = (sensorMax_C[i] >> 8) & 0xFF;
    buffer[3] = sensorMax_C[i] & 0xFF;
    writeEEPROM(EEPROM_ADDRESS, 64 + i * 4, buffer, 4);
    delay(10);
  }
  
  Serial.println("บันทึกเสร็จสิ้น!");
}


// =============================================================================
//  โหลดค่า Calibration จาก EEPROM
// =============================================================================

void load_calibration() {
  byte buffer[4];
  
  // อ่าน Sensor A
  for (int i = 0; i < NUM_SENSORS; i++) {
    readEEPROM(EEPROM_ADDRESS, i * 4, buffer, 4);
    sensorMin_A[i] = (buffer[0] << 8) | buffer[1];
    sensorMax_A[i] = (buffer[2] << 8) | buffer[3];
    
    // ตรวจสอบค่าที่ไม่ถูกต้อง
    if (sensorMin_A[i] == 0 && sensorMax_A[i] == 0) {
      sensorMin_A[i] = 100;
      sensorMax_A[i] = 900;
    }
    if (sensorMin_A[i] > sensorMax_A[i]) {
      int temp = sensorMin_A[i];
      sensorMin_A[i] = sensorMax_A[i];
      sensorMax_A[i] = temp;
    }
  }
  
  // อ่าน Sensor B
  for (int i = 0; i < NUM_SENSORS; i++) {
    readEEPROM(EEPROM_ADDRESS, 32 + i * 4, buffer, 4);
    sensorMin_B[i] = (buffer[0] << 8) | buffer[1];
    sensorMax_B[i] = (buffer[2] << 8) | buffer[3];
    
    if (sensorMin_B[i] == 0 && sensorMax_B[i] == 0) {
      sensorMin_B[i] = 100;
      sensorMax_B[i] = 900;
    }
    if (sensorMin_B[i] > sensorMax_B[i]) {
      int temp = sensorMin_B[i];
      sensorMin_B[i] = sensorMax_B[i];
      sensorMax_B[i] = temp;
    }
  }
  
  // อ่าน Sensor C
  for (int i = 0; i < 2; i++) {
    readEEPROM(EEPROM_ADDRESS, 64 + i * 4, buffer, 4);
    sensorMin_C[i] = (buffer[0] << 8) | buffer[1];
    sensorMax_C[i] = (buffer[2] << 8) | buffer[3];
    
    if (sensorMin_C[i] == 0 && sensorMax_C[i] == 0) {
      sensorMin_C[i] = 500;
      sensorMax_C[i] = 3500;
    }
  }
}


// =============================================================================
//  แสดงค่า Calibration
// =============================================================================

void print_calibration() {
  Serial.println("\n=== CALIBRATION VALUES ===");
  
  Serial.println("Sensor A (Front):");
  Serial.println("  [Sensor] [Min(ดำ)] - [Max(ขาว)] : [Mid]");
  for (int i = 0; i < NUM_SENSORS; i++) {
    Serial.print("     A"); Serial.print(i);
    Serial.print("      "); Serial.print(sensorMin_A[i]);
    Serial.print("    -    "); Serial.print(sensorMax_A[i]);
    Serial.print("    : "); Serial.println(md_sensorA(i));
  }
  
  Serial.println("\nSensor B (Back):");
  for (int i = 0; i < NUM_SENSORS; i++) {
    Serial.print("     B"); Serial.print(i);
    Serial.print("      "); Serial.print(sensorMin_B[i]);
    Serial.print("    -    "); Serial.print(sensorMax_B[i]);
    Serial.print("    : "); Serial.println(md_sensorB(i));
  }
  
  Serial.println("\nSensor C (Center):");
  Serial.print("     C0      "); Serial.print(sensorMin_C[0]);
  Serial.print("    -    "); Serial.print(sensorMax_C[0]);
  Serial.print("    : "); Serial.println(md_sensorC(0));
  Serial.print("     C1      "); Serial.print(sensorMin_C[1]);
  Serial.print("    -    "); Serial.print(sensorMax_C[1]);
  Serial.print("    : "); Serial.println(md_sensorC(1));
  
  Serial.println("===========================\n");
}


// =============================================================================
//  แสดงค่าเซ็นเซอร์ Real-time
// =============================================================================

void show_sensors_live() {
  Serial.println("=== LIVE SENSOR VALUES ===");
  Serial.println("กดปุ่ม 1 เพื่อหยุด");
  
  while (digitalRead(BTN1) == HIGH) {
    Serial.print("A: ");
    for (int i = 0; i < NUM_SENSORS; i++) {
      int val = read_sensorA(i);
      int mid = md_sensorA(i);
      Serial.print(val);
      Serial.print(val < mid ? "*" : " ");
      Serial.print(" ");
    }
    
    Serial.print("| B: ");
    for (int i = 0; i < NUM_SENSORS; i++) {
      int val = read_sensorB(i);
      int mid = md_sensorB(i);
      Serial.print(val);
      Serial.print(val < mid ? "*" : " ");
      Serial.print(" ");
    }
    
    Serial.print("| C: ");
    Serial.print(analogRead(CENTER_SENSOR_L));
    Serial.print(" ");
    Serial.println(analogRead(CENTER_SENSOR_R));
    
    delay(100);
  }
  
  delay(300);
  Serial.println("หยุดแสดงค่า");
}


// =============================================================================
//  แสดงค่าเซ็นเซอร์แบบกราฟ
// =============================================================================

void show_sensors_graph() {
  Serial.println("=== SENSOR GRAPH ===");
  Serial.println("กดปุ่ม 1 เพื่อหยุด");
  
  while (digitalRead(BTN1) == HIGH) {
    Serial.print("A: ");
    for (int i = 0; i < NUM_SENSORS; i++) {
      int val = read_sensorA(i);
      int mapped = map(val, sensorMin_A[i], sensorMax_A[i], 0, 10);
      mapped = constrain(mapped, 0, 10);
      
      for (int j = 0; j < mapped; j++) Serial.print("#");
      for (int j = mapped; j < 10; j++) Serial.print(".");
      Serial.print("|");
    }
    Serial.println();
    delay(100);
  }
  
  delay(300);
}


// =============================================================================
//  ตั้งค่าเซ็นเซอร์แบบ Manual (ใส่ค่าเอง)
// =============================================================================

void set_sensor_values(int sensor, int minA, int maxA, int minB, int maxB) {
  if (sensor >= 0 && sensor < NUM_SENSORS) {
    sensorMin_A[sensor] = minA;
    sensorMax_A[sensor] = maxA;
    sensorMin_B[sensor] = minB;
    sensorMax_B[sensor] = maxB;
  }
}

void set_sensor_A(int sensor, int minVal, int maxVal) {
  if (sensor >= 0 && sensor < NUM_SENSORS) {
    sensorMin_A[sensor] = minVal;
    sensorMax_A[sensor] = maxVal;
  }
}

void set_sensor_B(int sensor, int minVal, int maxVal) {
  if (sensor >= 0 && sensor < NUM_SENSORS) {
    sensorMin_B[sensor] = minVal;
    sensorMax_B[sensor] = maxVal;
  }
}

void set_sensor_C(int sensor, int minVal, int maxVal) {
  if (sensor >= 0 && sensor < 2) {
    sensorMin_C[sensor] = minVal;
    sensorMax_C[sensor] = maxVal;
  }
}


// =============================================================================
//  Quick Test - ทดสอบเซ็นเซอร์
// =============================================================================

void test_sensors() {
  Serial.println("=== SENSOR TEST ===");
  Serial.println("เลื่อนหุ่นไปบนเส้นดำ/พื้นขาว เพื่อทดสอบ");
  Serial.println("กดปุ่ม 1 เพื่อหยุด\n");
  
  while (digitalRead(BTN1) == HIGH) {
    // แสดงสถานะเซ็นเซอร์หน้า
    Serial.print("Front: ");
    for (int i = 0; i < NUM_SENSORS; i++) {
      if (read_sensorA(i) < md_sensorA(i)) {
        Serial.print("■ ");  // เจอสีดำ
      } else {
        Serial.print("□ ");  // เจอสีขาว
      }
    }
    
    // แสดงสถานะเซ็นเซอร์หลัง
    Serial.print(" | Back: ");
    for (int i = 0; i < NUM_SENSORS; i++) {
      if (read_sensorB(i) < md_sensorB(i)) {
        Serial.print("■ ");
      } else {
        Serial.print("□ ");
      }
    }
    
    // แสดงสถานะเซ็นเซอร์กลาง
    Serial.print(" | Center: ");
    if (analogRead(CENTER_SENSOR_L) < md_sensorC(0)) Serial.print("■ "); else Serial.print("□ ");
    if (analogRead(CENTER_SENSOR_R) < md_sensorC(1)) Serial.print("■"); else Serial.print("□");
    
    Serial.println();
    delay(150);
  }
  
  delay(300);
  Serial.println("หยุดทดสอบ");
}


// =============================================================================
//  เมนู Calibration หลัก
// =============================================================================

void calibration_menu() {
  Serial.println("\n========================================");
  Serial.println("       CALIBRATION MENU");
  Serial.println("========================================");
  Serial.println("  1. Calibrate Auto (หมุนตัว)");
  Serial.println("  2. Calibrate Manual (กดปุ่ม)");
  Serial.println("  3. Calibrate Front Only");
  Serial.println("  4. Calibrate Back Only");
  Serial.println("  5. Calibrate Center Only");
  Serial.println("  6. Show Current Values");
  Serial.println("  7. Show Live Sensors");
  Serial.println("  8. Test Sensors");
  Serial.println("  9. Save to EEPROM");
  Serial.println("  0. Exit");
  Serial.println("========================================");
  Serial.println("เลือกเมนู (พิมพ์ตัวเลข):");
  
  while (!Serial.available()) {
    delay(10);
  }
  
  char choice = Serial.read();
  while (Serial.available()) Serial.read();  // Clear buffer
  
  switch (choice) {
    case '1': calibrate_auto(); break;
    case '2': calibrate_manual(); break;
    case '3': calibrate_front(); break;
    case '4': calibrate_back(); break;
    case '5': calibrate_center(); break;
    case '6': print_calibration(); break;
    case '7': show_sensors_live(); break;
    case '8': test_sensors(); break;
    case '9': save_calibration(); beep(3000, 200); break;
    case '0': Serial.println("ออกจากเมนู"); return;
    default: Serial.println("ไม่ถูกต้อง"); break;
  }
  
  calibration_menu();  // แสดงเมนูอีกครั้ง
}
