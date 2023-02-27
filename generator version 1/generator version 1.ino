#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SimpleKalmanFilter.h>
// #include <ezButton.h>
unsigned long prevTime, curTime;
int y;
float m;
int monitor;
int an_input = A0;
int input = 8;
bool skip = false;
int buttonPin2 = 4;
#define MCP4725_ADDR 0x60

int ADXL345 = 0x53; // The ADXL345 sensor I2C address
float X_out, Y_out, Z_out;  // Outputs
SimpleKalmanFilter simpleKalmanFilter(2, 2, 0.01);
//const long SERIAL_REFRESH_TIME = 100;
//long refresh_time;
const long SAMPLE_RATE = 5000; // 5 seconds
const long OUTPUT_RATE = 500; // 0.5 seconds
const long SAMPLE_COUNT = 5;
float avg_list[SAMPLE_COUNT];
int idx = 0;
unsigned long sample_time;
unsigned long output_time;
float total = 0.0;
int count = 0;
float avg_value;


// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);
// ezButton button(13);
// unsigned long count = 0;
int buttonPin = 2;    // Pin where the button is connected
int buttoncount = 0;        // Counter variable

void setup() {

  Wire.begin();

  lcd.begin();
  lcd.setBacklight((uint8_t)1); // First row

  // button.setDebounceTime(50); // set debounce time to 50 milliseconds
  // button.setCountMode(COUNT_FALLING);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
  Serial.begin(9600);
  pinMode(input, INPUT);
  prevTime = micros();

}

void loop() {
  // button.loop();
  // if (count == 5) {
  //   button.resetCount();
  //   count = 0;
  // }
  // count = button.getCount();
  // Serial.println(count);  // print count to Serial Monitor
  

  //   lcd.clear();
  //   lcd.setCursor(0, 0); // start to print at the first row
  //   lcd.print("Mode: ");
  //   lcd.print(count);

  //   lastCount != count;
  // }
  Wire.beginTransmission(ADXL345);
  Wire.write(0x32); // Start with register 0x32 (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(ADXL345, 6, true); // Read 6 registers total, each axis value is stored in 2 registers
  X_out = ( Wire.read()| Wire.read() << 8); // X-axis value
  X_out = X_out/256*10; //For a range of +-2g, we need to divide the raw values by 256, according to the datasheet
  Y_out = ( Wire.read()| Wire.read() << 8); // Y-axis value
  Y_out = Y_out/256*10;
  Z_out = ( Wire.read()| Wire.read() << 8); // Z-axis value
  Z_out = Z_out/256*10;
  float avg = abs(X_out)+abs(Y_out)+abs(Z_out)-10;
  float estimated_value = simpleKalmanFilter.updateEstimate(abs(avg));
  // if (millis() > refresh_time) {
  //   Serial.print(estimated_value,4);
  //   Serial.println();
  //   refresh_time = millis() + SERIAL_REFRESH_TIME;
  // }
  if (millis() > sample_time + SAMPLE_RATE) {
    total += estimated_value;
    count++;
    sample_time = millis();
    if (count > SAMPLE_COUNT) {
      total -= avg_list[idx];
      avg_list[idx++] = estimated_value;
      idx = idx % SAMPLE_COUNT;
    }  
  }
  if (millis() > output_time + OUTPUT_RATE) {
    avg_value = total/count;
    //Serial.print("High:10, Low:0, Signal:");
    //Serial.print(avg_value, 4);
    //Serial.println();
    output_time = millis();
  }


  if (digitalRead(buttonPin) == LOW) {
    buttoncount++;
    delay(200); // Debounce delay to prevent multiple counts on a single button press
  }
  if (buttoncount == 5) {
    buttoncount = 0;
  }
  //Serial.println(buttoncount);


  // Wire.beginTransmission(MCP4725_ADDR);
  // Wire.write(64); // cmd to update the DAC
  //Serial.println(y);
  if(buttoncount == 0){
    lcd.setCursor(0, 0);
    lcd.print("Welcome to   ");
    lcd.setCursor(0,1); // Second row
    lcd.print("Heart Synth     ");
    Wire.beginTransmission(MCP4725_ADDR);
    Wire.write(64); // cmd to update the DAC
    y = 0;
    Wire.write(y >> 4); // High byte
    Wire.write(y & 0x0F); // Low byte
    Wire.endTransmission();
    
  }
  else if(buttoncount == 1){
    lcd.setCursor(0, 0);
    lcd.print("Mode:         ");
    lcd.setCursor(0,1); // Second row
    lcd.print("Normal          "); 
    Wire.beginTransmission(MCP4725_ADDR);
    Wire.write(64); // cmd to update the DAC
    normal(60);
    Wire.write(y >> 4); // High byte
    Wire.write(y & 0x0F); // Low byte
    Wire.endTransmission();
  }
  else if(buttoncount == 2){
    lcd.setCursor(0, 0);
    lcd.print("Mode:         ");
    lcd.setCursor(0,1); // Second row
    lcd.print("Arrhythmia          ");
    Wire.beginTransmission(MCP4725_ADDR);
    Wire.write(64); // cmd to update the DAC
    arrythmia(60, 10);
    Wire.write(y >> 4); // High byte
    Wire.write(y & 0x0F); // Low byte
    Wire.endTransmission();
  } 
  else if(buttoncount == 3){
    lcd.setCursor(0, 0);
    lcd.print("Mode:         ");
    lcd.setCursor(0,1); // Second row
    lcd.print("Tachycardia          "); 
    Wire.beginTransmission(MCP4725_ADDR);
    Wire.write(64); // cmd to update the DAC
    tachycardia(60);
    Wire.write(y >> 4); // High byte
    Wire.write(y & 0x0F); // Low byte
    Wire.endTransmission();
  } 
  else if(buttoncount == 4){
    lcd.setCursor(0, 0);
    lcd.print("Mode:         ");
    lcd.setCursor(0,1); // Second row
    lcd.print("Bradycardia          ");
    Wire.beginTransmission(MCP4725_ADDR);
    Wire.write(64); // cmd to update the DAC
    bradycardia(60);
    Wire.write(y >> 4); // High byte
    Wire.write(y & 0x0F); // Low byte
    Wire.endTransmission();
  }  
  //y = (int)(4095/6*abs(5*sin(2*PI*micros()/1000000)+0.7*cos(3*2*PI*micros()/1000000)));
  // Wire.write(y >> 4); // High byte
  // Wire.write(y & 0x0F); // Low byte
  // Wire.endTransmission();
  monitor = analogRead(an_input);
  Serial.print("High:1200, Low:0, Signal:");
  Serial.println(monitor);
  
}

void arrythmia(int bpm, int block_rate){
  curTime = micros();
  int r = random(0, 100);
  float val = 60000000/bpm;
  if (digitalRead(buttonPin2) != LOW){
    if (curTime - prevTime >= val/2) {
      m = random(50.0, 80.0)/100.0;
      if (r < block_rate) skip = true;
      else skip = false;
      prevTime = micros();
    } else if (!skip) {
      y = (int) (4095/6.5*abs(5*sin(2*PI*(curTime - prevTime)/val)+cos(3*2*PI*(curTime - prevTime)/val)+.5*cos(3*2*PI*(curTime - prevTime)/val)));
      if (y >600){
        y = simpleKalmanFilter.updateEstimate(y*m + random(-200, 0));
      }else y = (int) 0;
    } else y = (int) 0;
  } else y = (int)0;
}

void normal(int bpm){
  curTime = micros();
  int r = random(0, 100);
  float val = 60000000/bpm;
  if (digitalRead(buttonPin2) != LOW){
    if (curTime - prevTime >= val/2) {
      m = random(50.0, 80.0)/100.0;
      if (r < 0) skip = true;
      else skip = false;
      prevTime = micros();
    } else if (!skip) {
      y = (int) (4095/6.5*abs(5*sin(2*PI*(curTime - prevTime)/val)+cos(3*2*PI*(curTime - prevTime)/val)+.5*cos(3*2*PI*(curTime - prevTime)/val)));
      if (y >600){
        y = simpleKalmanFilter.updateEstimate(y*m + random(-200, 0));
      }else y = (int) 0;
    } else y = (int) 0;
  } else y = (int)0;
}

void tachycardia(int bpm){
  curTime = micros();
  int r = random(0, 100);
  float val = 60000000/bpm*.7;
  if (digitalRead(buttonPin2) != LOW){
    if (curTime - prevTime >= val/2) {
      m = random(50.0, 80.0)/100.0;
      if (r < 0) skip = true;
      else skip = false;
      prevTime = micros();
    } else if (!skip) {
      y = (int) (4095/6.5*abs(5*sin(2*PI*(curTime - prevTime)/val)+cos(3*2*PI*(curTime - prevTime)/val)+.5*cos(3*2*PI*(curTime - prevTime)/val)));
      if (y >600){
        y = simpleKalmanFilter.updateEstimate(y*m + random(-200, 0));
      }else y = (int) 0;
    } else y = (int) 0;
  } else y = (int)0;
}

void bradycardia(int bpm){
  curTime = micros();
  int r = random(0, 100);
  float val = 60000000/bpm*2;
  if (digitalRead(buttonPin2) != LOW){
    if (curTime - prevTime >= val/2) {
      m = random(50.0, 80.0)/100.0;
      if (r < 0) skip = true;
      else skip = false;
      prevTime = micros();
    } else if (!skip) {
      y = (int) (4095/6.5*abs(5*sin(2*PI*(curTime - prevTime)/val)+cos(3*2*PI*(curTime - prevTime)/val)+.5*cos(3*2*PI*(curTime - prevTime)/val)));
      if (y >600){
        y = simpleKalmanFilter.updateEstimate(y*m + random(-200, 0));
      }else y = (int) 0;
    } else y = (int) 0;
  } else y = (int)0;
}
 