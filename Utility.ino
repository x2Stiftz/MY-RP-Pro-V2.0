/*
 * =============================================================================
 *  Utility.ino - ฟังก์ชันเสริม (LED, Buzzer, Button, EEPROM, Setup)
 * =============================================================================
 */

// =============================================================================
//  LED
// =============================================================================

void led_rgb(char color) {
  digitalWrite(rgb[0], (color == 'r' || color == 'w') ? HIGH : LOW);
  digitalWrite(rgb[1], (color == 'g' || color == 'w') ? HIGH : LOW);
  digitalWrite(rgb[2], (color == 'b' || color == 'w') ? HIGH : LOW);
}

void led_off() {
  digitalWrite(rgb[0], LOW);
  digitalWrite(rgb[1], LOW);
  digitalWrite(rgb[2], LOW);
}

void led_set(bool r, bool g, bool b) {
  digitalWrite(rgb[0], r);
  digitalWrite(rgb[1], g);
  digitalWrite(rgb[2], b);
}


// =============================================================================
//  Buzzer
// =============================================================================

void beep(int freq, int duration) {
  tone(BUZZER_PIN, freq, duration);
  delay(duration);
}

void beep() { beep(3000, 100); }

void beep_n(int n) {
  for (int i = 0; i < n; i++) {
    beep(3000, 100);
    delay(100);
  }
}


// =============================================================================
//  Button
// =============================================================================

void sw() {
  while (digitalRead(BTN1) == HIGH) delay(10);
  delay(200);
  tone(BUZZER_PIN, 3000, 200);
  delay(300);
}

void sw_led(char color) {
  led_rgb(color);
  sw();
  led_off();
}

bool btn1_pressed() { return digitalRead(BTN1) == LOW; }
bool btn2_pressed() { return digitalRead(BTN2) == LOW; }
bool btn3_pressed() { return digitalRead(BTN3) == LOW; }


// =============================================================================
//  EEPROM
// =============================================================================

void writeEEPROM(int deviceAddress, unsigned int eeAddress, byte *data, int dataLength) {
  Wire.beginTransmission(deviceAddress);
  Wire.write((int)(eeAddress >> 8));
  Wire.write((int)(eeAddress & 0xFF));
  for (int i = 0; i < dataLength; i++) Wire.write(data[i]);
  Wire.endTransmission();
  delay(5);
}

void readEEPROM(int deviceAddress, unsigned int eeAddress, byte *buffer, int dataLength) {
  Wire.beginTransmission(deviceAddress);
  Wire.write((int)(eeAddress >> 8));
  Wire.write((int)(eeAddress & 0xFF));
  Wire.endTransmission();
  Wire.requestFrom(deviceAddress, dataLength);
  for (int i = 0; i < dataLength; i++) {
    if (Wire.available()) buffer[i] = Wire.read();
  }
}

void get_EEP_Program() {
  byte buffer[4];
  
  for (int i = 0; i < NUM_SENSORS; i++) {
    readEEPROM(EEPROM_ADDRESS, i * 4, buffer, 4);
    sensorMin_A[i] = (buffer[0] << 8) | buffer[1];
    sensorMax_A[i] = (buffer[2] << 8) | buffer[3];
    if (sensorMin_A[i] == 0 && sensorMax_A[i] == 0) {
      sensorMin_A[i] = 100;
      sensorMax_A[i] = 900;
    }
  }
  
  for (int i = 0; i < NUM_SENSORS; i++) {
    readEEPROM(EEPROM_ADDRESS, 32 + i * 4, buffer, 4);
    sensorMin_B[i] = (buffer[0] << 8) | buffer[1];
    sensorMax_B[i] = (buffer[2] << 8) | buffer[3];
    if (sensorMin_B[i] == 0 && sensorMax_B[i] == 0) {
      sensorMin_B[i] = 100;
      sensorMax_B[i] = 900;
    }
  }
  
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
//  Setup
// =============================================================================

void setup_robot() {
  Wire.begin();
  Wire1.setSDA(SDA1_PIN);
  Wire1.setSCL(SCL1_PIN);
  analogReadResolution(12);
  
  if (!gyro_begin()) Serial.println("Failed to initialize GYRO!");
  resetAngles();
  Serial.println("GYRO initialized!");
  
  pinMode(rgb[0], OUTPUT);
  pinMode(rgb[1], OUTPUT);
  pinMode(rgb[2], OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BTN1, INPUT_PULLUP);
  pinMode(BTN2, INPUT_PULLUP);
  pinMode(BTN3, INPUT_PULLUP);
  
  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  
  pinMode(ADC_CLK, OUTPUT);
  pinMode(ADC_MISO, INPUT);
  pinMode(ADC_MOSI, OUTPUT);
  pinMode(ADC_CS_A, OUTPUT);
  pinMode(ADC_CS_B, OUTPUT);
  digitalWrite(ADC_CS_A, HIGH);
  digitalWrite(ADC_CS_B, HIGH);
  
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 3; j++) {
      digitalWrite(rgb[j], HIGH); delay(50);
      digitalWrite(rgb[j], LOW); delay(50);
    }
  }
  digitalWrite(rgb[2], HIGH);
  
  get_EEP_Program();
  Serial.println("Robot Ready!");
}


// =============================================================================
//  Debug
// =============================================================================

void debug_sensorA() {
  Serial.print("A: ");
  for (int i = 0; i < 8; i++) { Serial.print(read_sensorA(i)); Serial.print(" "); }
  Serial.println();
}

void debug_sensorB() {
  Serial.print("B: ");
  for (int i = 0; i < 8; i++) { Serial.print(read_sensorB(i)); Serial.print(" "); }
  Serial.println();
}

void debug_sensorC() {
  Serial.print("C: ");
  Serial.print(analogRead(CENTER_SENSOR_L)); Serial.print(" ");
  Serial.println(analogRead(CENTER_SENSOR_R));
}

void debug_gyro() {
  Serial.print("Gyro X:"); Serial.print(gyro('x'));
  Serial.print(" Y:"); Serial.print(gyro('y'));
  Serial.print(" Z:"); Serial.println(gyro('z'));
}

void debug_calibration() {
  Serial.println("=== Sensor A ===");
  for (int i = 0; i < 8; i++) {
    Serial.print(i); Serial.print(": ");
    Serial.print(sensorMin_A[i]); Serial.print("-");
    Serial.println(sensorMax_A[i]);
  }
}
