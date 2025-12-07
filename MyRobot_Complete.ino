/*
 * =============================================================================
 *  MyRobot Complete V2 - รวมโค้ดสำเร็จรูป
 * =============================================================================
 *  รองรับ: RP2350 Pro Board
 *  
 *  ไฟล์ในโปรเจค:
 *    - MyRobot_Complete.ino : ไฟล์หลัก + ตั้งค่า
 *    - Motor.ino            : ควบคุมมอเตอร์ + ฟังก์ชันจูน
 *    - Gyro.ino             : อ่านค่า Gyro (BMI160)
 *    - Sensor.ino           : อ่านค่าเซ็นเซอร์ ADC
 *    - ForwardBackward.ino  : เดินหน้า/ถอยหลัง (speed, kp, delay)
 *    - TracDegree.ino       : หมุนตัว (speed, degree, mode)
 *    - Trac.ino             : เดินตามเส้น TracJC
 *    - Servo.ino            : ควบคุม Servo แขนกล
 *    - Utility.ino          : ฟังก์ชันเสริม
 * =============================================================================
 */

#include <Wire.h>
#include <Servo.h>

// =============================================================================
//  ส่วนที่ 1: ตั้งค่า PID
// =============================================================================

// PID สำหรับ Gyro (หมุนตัว/เดินตรง)
float GYRO_KP = 1.5;
float GYRO_KI = 0.001;
float GYRO_KD = 0.5;

// PID สำหรับ TracLine (เดินตามเส้น)
float LINE_KP = 0.45;
float LINE_KI = 0.0001;
float LINE_KD = 0.025;


// =============================================================================
//  ส่วนที่ 2: ตั้งค่า Servo แขนกล
// =============================================================================

int servo_down   = 50;
int servoL_open  = 120;
int servoR_open  = 125;
int servo_trim34 = -5;
int servo_trim35 = 0;
int servo_trim36 = 0;


// =============================================================================
//  ส่วนที่ 3: ตั้งค่าเบรค
// =============================================================================

int BRAKE_TIME   = 10;
int BRAKE_POWER  = 20;


// =============================================================================
//  ส่วนที่ 4: ตั้งค่ามอเตอร์
// =============================================================================

String MOTOR_TYPE = "Coreless_Motors";  // "Coreless_Motors" หรือ "DC_Motors"


// =============================================================================
//  ส่วนที่ 5: Pin Definitions
// =============================================================================

// Motor Pins
#define PWMA 6
#define AIN1 22
#define AIN2 23
#define PWMB 3
#define BIN1 21
#define BIN2 20

// I2C
#define SDA1_PIN 26
#define SCL1_PIN 27

// RGB LED
int rgb[] = {24, 25, 28};

// Buzzer
#define BUZZER_PIN 32

// Buttons
#define BTN1 33
#define BTN2 19
#define BTN3 12

// Servo Pins
#define SERVO_39 39
#define SERVO_38 38
#define SERVO_37 37
#define SERVO_36 36
#define SERVO_35 35
#define SERVO_34 34

// ADC Pins (MCP3008)
#define ADC_CLK  14
#define ADC_MISO 15
#define ADC_MOSI 16
#define ADC_CS_A 13
#define ADC_CS_B 17

// Center Sensors
#define CENTER_SENSOR_L 46
#define CENTER_SENSOR_R 47

// EEPROM
#define EEPROM_ADDRESS 0x50


// =============================================================================
//  ส่วนที่ 6: Global Variables
// =============================================================================

// Gyro Variables
float _angleX = 0.0, _angleY = 0.0, _angleZ = 0.0;
float _gyroOffsetX = 0.0, _gyroOffsetY = 0.0, _gyroOffsetZ = 0.0;
float _accelX_prev = 0.0, _accelY_prev = 0.0, _accelZ_prev = 0.0;
unsigned long _gyroLastTime = 0;
const float GYRO_THRESHOLD = 0.3;
const float ALPHA = 0.95;
const float ACCEL_FILTER_ALPHA = 0.1;
uint8_t _gyroAddress = 0x69;

// PID Variables
float _prevError = 0.0;
float _integral = 0.0;

// Absolute Angle Reference (สำหรับ PID Mode 1)
float _absoluteAngle = 0.0;

// Motor Offset (จะถูกตั้งค่าจากฟังก์ชัน tuning)
int MOTOR_LEFT_OFFSET  = 0;
int MOTOR_RIGHT_OFFSET = 0;

// Sensor Variables
const int NUM_SENSORS = 8;
int sensorMin_A[NUM_SENSORS];
int sensorMax_A[NUM_SENSORS];
int sensorMin_B[NUM_SENSORS];
int sensorMax_B[NUM_SENSORS];
int sensorMin_C[2];
int sensorMax_C[2];
int _lastPosition = 2500;

// TracJC Variables
int BaseSpeed = 50;
int Speed = 50;
int SlowSpeed = 30;
int ACCSpeed = 70;
float Error = 0;
int ErrorClear = 0;
int Kt = 5;
unsigned long _timerStart = 0;
unsigned long _timer3Start = 0;

// Line Sensor State
int L3 = 1, R3 = 1;
int C = 1;
int BL1 = 1, BR1 = 1;

// Servo Objects
Servo servo_obj[6];
float s34_before_deg = 120;
float s35_before_deg = 120;
float s36_before_deg = 50;
int num_steps = 20;


// =============================================================================
//  ส่วนที่ 7: Setup & Loop
// =============================================================================

void setup() {
  Serial.begin(115200);
  setup_robot();
  
  // ตั้งค่าแขนกล
  arm_down_open();
  arm_l_r(120, 120, 50);
  
  // รอกดปุ่มเริ่ม
  sw();
  
  //==========================================================================
  //  ใส่คำสั่งตรงนี้!
  //==========================================================================
  
  /*
   * ===== รูปแบบคำสั่งใหม่ =====
   * 
   * เดินหน้า/ถอยหลัง:
   *   Forward(speed, kp, delayMs);      // เดินหน้า
   *   Backward(speed, kp, delayMs);     // ถอยหลัง
   *   
   *   ตัวอย่าง:
   *   Forward(50, 1.5, 500);   // เดินหน้า speed=50, kp=1.5, 500ms
   *   Backward(40, 1.2, 300);  // ถอยหลัง speed=40, kp=1.2, 300ms
   * 
   * หมุนตัว:
   *   SpinDegree(speed, degree, mode);
   *   
   *   Mode 0 = Relative (รีค่าหลังเลี้ยว)
   *   Mode 1 = Absolute (0=หน้า, 90=ขวา, 180=หลัง, 270=ซ้าย)
   *   
   *   ตัวอย่าง:
   *   SpinDegree(30, 90, 0);   // หมุนขวา 90° แบบ Relative
   *   SpinDegree(30, -90, 0);  // หมุนซ้าย 90° แบบ Relative
   *   
   *   SetHeading(0);            // ตั้งทิศหน้าเป็น 0
   *   TurnToHeading(30, 90);    // หมุนไปทิศขวา (Absolute)
   * 
   * เดินตามเส้น:
   *   TracJC();        // เดินจนเจอแยก
   *   TracJCSpeed();   // เดินเร็วจนเจอแยก
   *   TJCSS(3);        // เดินผ่าน 3 แยก
   * 
   * เดินตรงด้วย Gyro:
   *   TracDegreeSpeedTime(speed, degree, delayMs, mode);
   */
  
  
  //==========================================================================
}

void loop() {
  // ใส่โค้ดที่ต้องการรันซ้ำๆ
  
}
