/*
 * =============================================================================
 *  Sensor.ino - อ่านค่าเซ็นเซอร์ ADC (MCP3008)
 * =============================================================================
 */

// อ่านค่า ADC จาก MCP3008
uint16_t read_ADC(int cs_pin, int channel) {
  digitalWrite(cs_pin, LOW);
  
  uint8_t command = 0x18 | channel;
  uint16_t result = 0;
  
  for (int i = 4; i >= 0; i--) {
    digitalWrite(ADC_MOSI, (command >> i) & 1);
    digitalWrite(ADC_CLK, HIGH);
    delayMicroseconds(1);
    digitalWrite(ADC_CLK, LOW);
    delayMicroseconds(1);
  }
  
  for (int i = 0; i < 12; i++) {
    digitalWrite(ADC_CLK, HIGH);
    delayMicroseconds(1);
    result = (result << 1) | digitalRead(ADC_MISO);
    digitalWrite(ADC_CLK, LOW);
    delayMicroseconds(1);
  }
  
  digitalWrite(cs_pin, HIGH);
  return result & 0x3FF;
}

// อ่านเซ็นเซอร์หน้า (0-7)
uint16_t read_sensorA(int sensor) {
  return read_ADC(ADC_CS_A, sensor);
}

// อ่านเซ็นเซอร์หลัง (0-7)
uint16_t read_sensorB(int sensor) {
  return read_ADC(ADC_CS_B, sensor);
}

// ค่ากลางของเซ็นเซอร์
int md_sensorA(int sensor) {
  return (sensorMax_A[sensor] + sensorMin_A[sensor]) / 2;
}

int md_sensorB(int sensor) {
  return (sensorMax_B[sensor] + sensorMin_B[sensor]) / 2;
}

int md_sensorC(int sensor) {
  return (sensorMax_C[sensor] + sensorMin_C[sensor]) / 2;
}

// อ่านตำแหน่งเส้น
int position_A() {
  bool onLine = false;
  long avg = 0;
  long sum = 0;

  for (int i = 1; i <= 6; i++) {
    long value = map(read_sensorA(i), sensorMin_A[i], sensorMax_A[i], 1000, 0);
    value = constrain(value, 0, 1000);

    if (value > 200) onLine = true;
    if (value > 50) {
      avg += value * ((i - 1) * 1000);
      sum += value;
    }
  }

  if (!onLine) {
    if (_lastPosition < 2500) return 0;
    else return 5000;
  }

  _lastPosition = avg / sum;
  return _lastPosition;
}

int position_A_none() {
  bool onLine = false;
  long avg = 0;
  long sum = 0;

  for (int i = 1; i <= 6; i++) {
    long value = map(read_sensorA(i), sensorMin_A[i], sensorMax_A[i], 1000, 0);
    value = constrain(value, 0, 1000);

    if (value > 200) onLine = true;
    if (value > 50) {
      avg += value * ((i - 1) * 1000);
      sum += value;
    }
  }

  if (!onLine) return 2500;

  _lastPosition = avg / sum;
  return _lastPosition;
}

int position_B() {
  bool onLine = false;
  long avg = 0;
  long sum = 0;

  for (int i = 1; i <= 6; i++) {
    long value = map(read_sensorB(i), sensorMin_B[i], sensorMax_B[i], 1000, 0);
    value = constrain(value, 0, 1000);

    if (value > 200) onLine = true;
    if (value > 50) {
      avg += value * ((i - 1) * 1000);
      sum += value;
    }
  }

  if (!onLine) {
    if (_lastPosition < 2500) return 0;
    else return 5000;
  }

  _lastPosition = avg / sum;
  return _lastPosition;
}

// ตรวจหาแยก
void ConvertADC() {
  L3 = (read_sensorA(0) > md_sensorA(0)) ? 1 : 0;
  R3 = (read_sensorA(7) > md_sensorA(7)) ? 1 : 0;
  C  = (read_sensorA(3) > md_sensorA(3) || read_sensorA(4) > md_sensorA(4)) ? 1 : 0;
}

void ConvertADCBack() {
  BL1 = (read_sensorB(0) > md_sensorB(0)) ? 1 : 0;
  BR1 = (read_sensorB(7) > md_sensorB(7)) ? 1 : 0;
}

// ตรวจสอบเจอเส้น/แยก
bool isOnLine() {
  for (int i = 1; i <= 6; i++) {
    if (read_sensorA(i) < md_sensorA(i)) return true;
  }
  return false;
}

bool isJunctionLeft() {
  return (read_sensorA(0) < md_sensorA(0));
}

bool isJunctionRight() {
  return (read_sensorA(7) < md_sensorA(7));
}

bool isJunctionBoth() {
  return isJunctionLeft() && isJunctionRight();
}

// ตั้งค่า min/max
void setSensorA_MinMax(int sensor, int minVal, int maxVal) {
  if (sensor >= 0 && sensor < NUM_SENSORS) {
    sensorMin_A[sensor] = minVal;
    sensorMax_A[sensor] = maxVal;
  }
}

void setSensorB_MinMax(int sensor, int minVal, int maxVal) {
  if (sensor >= 0 && sensor < NUM_SENSORS) {
    sensorMin_B[sensor] = minVal;
    sensorMax_B[sensor] = maxVal;
  }
}

void setSensorC_MinMax(int sensor, int minVal, int maxVal) {
  if (sensor >= 0 && sensor < 2) {
    sensorMin_C[sensor] = minVal;
    sensorMax_C[sensor] = maxVal;
  }
}
