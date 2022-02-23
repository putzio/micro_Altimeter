# 1 "g:\\Studia\\PUT Rocket LAB\\altimetr\\Altimetr_SPI\\Altimetr_SPI.ino"
# 2 "g:\\Studia\\PUT Rocket LAB\\altimetr\\Altimetr_SPI\\Altimetr_SPI.ino" 2
# 3 "g:\\Studia\\PUT Rocket LAB\\altimetr\\Altimetr_SPI\\Altimetr_SPI.ino" 2
/*

1. UART connection

  -uC receve

  -uC send

2. SPI connection

  -Chip id

  -Read Calibration data

  -Read Temperature and preassure

  -Write Flash

  -Read Flash

3. Hight measurement

  -Get hight from preassure

  -Get the highest value

  -Write it to the Flash

4. Interrupt jumper

  -Interrupt if the pin is connected to GND

  -Get the highest value from the Flash

*/
# 21 "g:\\Studia\\PUT Rocket LAB\\altimetr\\Altimetr_SPI\\Altimetr_SPI.ino"
// pinout
// CS -> write LOW to choose the salve
# 35 "g:\\Studia\\PUT Rocket LAB\\altimetr\\Altimetr_SPI\\Altimetr_SPI.ino"
// Sensor's memory register addresses:
# 50 "g:\\Studia\\PUT Rocket LAB\\altimetr\\Altimetr_SPI\\Altimetr_SPI.ino"
struct BMP280_HandleTypedef
{
  uint16_t dig_T1;
  int16_t dig_T2;
  int16_t dig_T3;
  uint16_t dig_P1;
  int16_t dig_P2;
  int16_t dig_P3;
  int16_t dig_P4;
  int16_t dig_P5;
  int16_t dig_P6;
  int16_t dig_P7;
  int16_t dig_P8;
  int16_t dig_P9;
};

BMP280_HandleTypedef calibrationData;

void setup()
{
  // put your setup code here, to run once:
  for (int i = 0; i < 11; i++) // For all pins
    pinMode(i, 1 /* used for pinMode() */);
  pinMode(3 /* INT - PA7*/, 2 /* used for pinMode() */);
  Serial.begin(115200);
  Serial.println("START");
  // start the SPI library:
  SPI.begin();
  // // Configure SCP1000 for low noise configuration:
  // writeRegister(0x02, 0x2D);
  // writeRegister(0x01, 0x03);
  // writeRegister(0x03, 0x02);
  // give the sensor time to set up:
  BMP280_CONFIG();
  delay(100);
  calibrationData = calibration();
}
uint64_t t = 0;
void loop()
{
  if (millis() - t > 5000)
  {
    uint8_t values[6];
    int32_t t_fine = 0;
    ReadRegisterToUintArray(0xF7, 6, 7 /* PB0*/, values);
    Serial.print("Temperature:\t");
    Serial.println(ReadTemperature(values, &t_fine, calibrationData));
    Serial.print("t_fie:\t");
    Serial.println(t_fine);
    float pressure = ReadPreassure(values, t_fine, calibrationData);
    Serial.print("Pressure:\t");
    Serial.println(pressure);
    Serial.print("Hight:\t");
    Serial.println(ReadHight(pressure));
    t = millis();
  }
  if (Serial.available() && !digitalRead(3 /* INT - PA7*/))
  {
    uint8_t result = 0;
    ReadRegisterToUintArray(0xD0, 1, 7 /* PB0*/, &result);
    Serial.print("Chip ID:\t");
    Serial.print(0x58);
    Serial.print("\tResult:\t");
    Serial.println(result);
    delay(500);
  }

  delay(100);
}
// Read from or write to register from the SCP1000:
int32_t ReadRegister(uint8_t thisRegister, uint8_t bytesToRead, uint8_t chipSelectPin)
{
  int16_t inByte = 0; // incoming byte from the SPI
  int32_t result = 0; // result to return
  Serial.print("From register:\t");
  Serial.print(thisRegister, 16);
  Serial.print("\t");

  uint8_t readRegister = (thisRegister | 0x80);
  Serial.println(readRegister, 16);

  // take the chip select low to select the device:
  digitalWrite(chipSelectPin, 0 /* used for digitalWrite(), digitalRead(), openDrain() and attachInterrupt() */);
  // send the device the register you want to read:
  SPI.transfer(readRegister);
  // send a value of 0 to read the first byte returned:
  result = SPI.transfer(0x00);
  // decrement the number of bytes left to read:
  bytesToRead--;
  // if you still have another byte to read:
  while (bytesToRead > 0)
  {
    // shift the first byte left, then get the second byte:
    result = result << 8;
    inByte = SPI.transfer(0x00);
    // combine the byte you just got with the previous one:

    result = result | inByte;
    // decrement the number of bytes left to read:
    bytesToRead--;
  }

  // take the chip select high to de-select:
  digitalWrite(chipSelectPin, 1 /* used for digitalWrite(), digitalRead(). There is no option for HIGH level interrupt provided by the hardware */);
  // return the result:
  return (result);
}

uint16_t ReadTwoRegistersUint(uint8_t thisRegister, uint8_t chipSelectPin, bool lastMSB)
{
  uint8_t inByte = 0; // incoming byte from the SPI
  uint16_t result = 0; // result to return
  Serial.print("From register:\t");
  Serial.print(thisRegister, 16);
  Serial.print("\t");

  uint8_t readRegister = (thisRegister | 0x80);
  Serial.println(readRegister, 16);

  // take the chip select low to select the device:
  digitalWrite(chipSelectPin, 0 /* used for digitalWrite(), digitalRead(), openDrain() and attachInterrupt() */);
  // send the device the register you want to read:
  SPI.transfer(readRegister);
  // send a value of 0 to read the first byte returned:
  result = SPI.transfer(0x00);
  inByte = SPI.transfer(0x00);
  // take the chip select high to de-select:
  digitalWrite(chipSelectPin, 1 /* used for digitalWrite(), digitalRead(). There is no option for HIGH level interrupt provided by the hardware */);
  if (lastMSB)
  {
    // combine the byte you just got with the previous one:
    result = result | (inByte << 8);
  }
  else
  {
    // shift the first byte left, then get the second byte:
    result = result << 8;
    // combine the byte you just got with the previous one:
    result = result | inByte;
  }
  // return the result:
  return (result);
}

int16_t ReadTwoRegistersInt(uint8_t thisRegister, uint8_t chipSelectPin, bool lastMSB)
{
  int8_t inByte = 0; // incoming byte from the SPI
  int16_t result = 0; // result to return
  Serial.print("From register:\t");
  Serial.print(thisRegister, 16);
  Serial.print("\t");

  uint8_t readRegister = (thisRegister | 0x80);
  Serial.println(readRegister, 16);

  // take the chip select low to select the device:
  digitalWrite(chipSelectPin, 0 /* used for digitalWrite(), digitalRead(), openDrain() and attachInterrupt() */);
  // send the device the register you want to read:
  SPI.transfer(readRegister);
  // send a value of 0 to read the first byte returned:
  result = SPI.transfer(0x00);
  inByte = SPI.transfer(0x00);
  // take the chip select high to de-select:
  digitalWrite(chipSelectPin, 1 /* used for digitalWrite(), digitalRead(). There is no option for HIGH level interrupt provided by the hardware */);
  if (lastMSB)
  {
    // combine the byte you just got with the previous one:
    result = result | (inByte << 8);
  }
  else
  {
    // shift the first byte left, then get the second byte:
    result = result << 8;
    // combine the byte you just got with the previous one:
    result = result | inByte;
  }
  // return the result:
  return (result);
}

void ReadRegisterToIntArray(uint8_t thisRegister, uint8_t bytesToRead, uint8_t chipSelectPin, int8_t *array)
{
  Serial.print("From register:\t");
  Serial.print(thisRegister, 16);
  Serial.print("\t");
  uint8_t readRegister = (thisRegister | 0x80);
  Serial.println(readRegister, 16);

  // take the chip select low to select the device:
  digitalWrite(chipSelectPin, 0 /* used for digitalWrite(), digitalRead(), openDrain() and attachInterrupt() */);
  // send the device the register you want to read:
  SPI.transfer(readRegister);
  for (int i = 0; i < bytesToRead; i++)
  {
    // send a value of 0 to read the first byte returned:
    array[i] = SPI.transfer(0x00);
  }
  // take the chip select high to de-select:
  digitalWrite(chipSelectPin, 1 /* used for digitalWrite(), digitalRead(). There is no option for HIGH level interrupt provided by the hardware */);
}
void ReadRegisterToUintArray(uint8_t thisRegister, uint8_t bytesToRead, uint8_t chipSelectPin, uint8_t *array)
{
  Serial.print("From register:\t");
  Serial.print(thisRegister, 16);
  Serial.print("\t");
  uint8_t readRegister = (thisRegister | 0x80);
  Serial.println(readRegister, 16);

  // take the chip select low to select the device:
  digitalWrite(chipSelectPin, 0 /* used for digitalWrite(), digitalRead(), openDrain() and attachInterrupt() */);
  // send the device the register you want to read:
  SPI.transfer(readRegister);
  for (int i = 0; i < bytesToRead; i++)
  {
    // send a value of 0 to read the first byte returned:
    array[i] = SPI.transfer(0x00);
  }
  // take the chip select high to de-select:
  digitalWrite(chipSelectPin, 1 /* used for digitalWrite(), digitalRead(). There is no option for HIGH level interrupt provided by the hardware */);
}
// Sends a write command to SCP1000

void writeRegister(uint8_t thisRegister, uint8_t thisValue, uint8_t chipSelectPin)
{
  // MSB = 0 -> WRITE
  uint8_t writeRegister = (thisRegister & 0x7F);

  // take the chip select low to select the device:
  digitalWrite(chipSelectPin, 0 /* used for digitalWrite(), digitalRead(), openDrain() and attachInterrupt() */);

  SPI.transfer(writeRegister); // Send register location
  SPI.transfer(thisValue); // Send value to record into register

  // take the chip select high to de-select:
  digitalWrite(chipSelectPin, 1 /* used for digitalWrite(), digitalRead(). There is no option for HIGH level interrupt provided by the hardware */);
}
struct BMP280_HandleTypedef calibration()
{
  int32_t data[12];
  Serial.println("CALIBRATION:");
  for (int i = 0; i < 12; i++)
  {
    if (i == 0 || i == 3)
      data[i] = ReadTwoRegistersUint(0x88 + i * 2, 7 /* PB0*/, 1);
    else
      data[i] = ReadTwoRegistersInt(0x88 + i * 2, 7 /* PB0*/, 1);
    Serial.println(data[i]);
  }
  BMP280_HandleTypedef datastruct;
  datastruct.dig_T1 = (uint16_t)data[0];
  datastruct.dig_T2 = (int16_t)data[1];
  datastruct.dig_T3 = (int16_t)data[2];
  datastruct.dig_P1 = (uint16_t)data[3];
  datastruct.dig_P2 = (int16_t)data[4];
  datastruct.dig_P3 = (int16_t)data[5];
  datastruct.dig_P4 = (int16_t)data[6];
  datastruct.dig_P5 = (int16_t)data[7];
  datastruct.dig_P6 = (int16_t)data[8];
  datastruct.dig_P7 = (int16_t)data[9];
  datastruct.dig_P8 = (int16_t)data[10];
  datastruct.dig_P9 = (int16_t)data[11];
  return datastruct;
}

void BMP280_CONFIG()
{
  /*

  Mode:           Normal -> 0xF4 [1:0] -> 11

  Oversampling:   Standard Resolution

  osrs_p:         x4 -> 0xF4 [4:2] -> 011

  osrs_t:         x1 -> 0xF4 [7:5] -> 001



  ODR[Hz]:        7.3

  RMS Noise[cm]:  6.4



  3 wire SPI:     false -> 0xF5[0] -> 0

  IIR filter:     4 -> 0xF5[4:2] -> 010?

  standby [ms]:   125 -> 0xF5 [7:5] -> 010

  */
# 329 "g:\\Studia\\PUT Rocket LAB\\altimetr\\Altimetr_SPI\\Altimetr_SPI.ino"
  // Serial.println("CONFIG()");
  uint8_t F5_Register, F4_Register;
  F5_Register = (2 << 5); // standby time 125 ms
  F5_Register |= (2 << 2); // Filter = 4

  F4_Register = (1 << 5); // oversampling: t->x2(010)
  F4_Register |= (3 << 2); // oversampling: =->x16(101)
  F4_Register |= (3); // Normal mode
  writeRegister(0xF4, F4_Register, 7 /* PB0*/);
  writeRegister(0xF5, F5_Register, 7 /* PB0*/);
  //
  // write8(0xF5,F5_Register);
  // write8(0xF4,F4_Register);
  // uint8_t configValues[2];
  // configValues[0] = F4_Register;
  // configValues[1] = F5_Register;
}

//---------Calculationg Temperature and preassure------------------------

float ReadTemperature(volatile uint8_t *values, int32_t *t_fine, struct BMP280_HandleTypedef _bmp280_calib)
{
  int32_t adc_T = values[3];
  adc_T <<= 16;
  adc_T |= ((values[4]) << 8);
  adc_T |= ((values[5]));
  adc_T >>= 4;

  int32_t var1, var2;
  var1 = ((((adc_T >> 3) - ((int32_t)_bmp280_calib.dig_T1 << 1))) *
          ((int32_t)_bmp280_calib.dig_T2)) >>
         11;

  var2 = (((((adc_T >> 4) - ((int32_t)_bmp280_calib.dig_T1)) *
            ((adc_T >> 4) - ((int32_t)_bmp280_calib.dig_T1))) >>
           12) *
          ((int32_t)_bmp280_calib.dig_T3)) >>
         14;
  int32_t sum = var1 + var2;
  *t_fine = sum;
  int32_t T = (sum * 5 + 128) >> 8;
  return (float)T / 100;
}

float ReadPreassure(volatile uint8_t values[6], int32_t t_fine, BMP280_HandleTypedef _bmp280_calib)
{
  int64_t p, var1, var2;
  // pressure
  int32_t adc_P = values[0];
  adc_P <<= 16;
  adc_P |= (values[1] << 8);
  adc_P |= values[2];
  adc_P >>= 4;

  var1 = ((int64_t)t_fine) - 128000;
  var2 = var1 * var1 * (int64_t)_bmp280_calib.dig_P6;
  var2 = var2 + ((var1 * (int64_t)_bmp280_calib.dig_P5) << 17);
  var2 = var2 + (((int64_t)_bmp280_calib.dig_P4) << 35);
  var1 = ((var1 * var1 * (int64_t)_bmp280_calib.dig_P3) >> 8) +
         ((var1 * (int64_t)_bmp280_calib.dig_P2) << 12);
  var1 =
      (((((int64_t)1) << 47) + var1)) * ((int64_t)_bmp280_calib.dig_P1) >> 33;

  if (var1 == 0)
  {
    return 0; // avoid exception caused by division by zero
  }
  p = 1048576 - adc_P;
  p = (((p << 31) - var2) * 3125) / var1;
  var1 = (((int64_t)_bmp280_calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
  var2 = (((int64_t)_bmp280_calib.dig_P8) * p) >> 19;

  p = ((p + var1 + var2) >> 8) + (((int64_t)_bmp280_calib.dig_P7) << 4);
  return ((uint32_t)p / 256.0);
}

float ReadHight(float preassure)
{
  preassure /= (float)100.0;

  float altitude = 44330.0 * (1.0 - pow(preassure / 1020.0, 0.1903));

  return altitude;
}