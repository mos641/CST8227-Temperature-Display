/************************************************************
  Practical Question
  Level Achieved: A+
  Status: Completed

  @author Mostapha A
*/

const int SER = 14; //data
const int LATCH = 15; //RCLK
const int CLOCK = 16; //SRCLK

const int THERMOMETER = 19;

const float MIN_TEMP = 15.0;
const float MAX_TEMP = 33.0;

const int TIMING = 800;
volatile unsigned long lastTime = 0;

const int INTERRUPT_TIMING = 4000;

IntervalTimer lowTempInterrupt;
IntervalTimer highTempInterrupt;

//  0  0  0  0  0  0  0  0
// 10  9  7  6  5  4  2  1
//  G  F  A  B  DP C  D  E

// F E D DP - 01001011 - 10110100 - 4+16+32+128 = 180
const int LOWT = 180;
// E F A B C - 01110101 - 10001010 - 2+8+128 = 138
const int NORMALT = 138;
// E F B C G DP - 11011101 - 00100010 - 2+32 = 34
const int HIGHT = 34;

void setup() {
  Serial.begin(9600);

  // set pin modes
  pinMode(LATCH, OUTPUT);
  pinMode(CLOCK, OUTPUT);
  pinMode(SER, OUTPUT);
  pinMode(THERMOMETER, INPUT);

  // set interval timers
  lowTempInterrupt.begin(injectTempLow, 4000000);
  lowTempInterrupt.priority(254);

  highTempInterrupt.begin(injectTempHigh, 4000000);
  highTempInterrupt.priority(255);
}

void loop() {
  // check and print temperature at set intervals
  if ((millis() - lastTime) >= TIMING) {
    readTemp(analogRead(THERMOMETER));
    lastTime = millis();
  }
}

// read temperature and print
void readTemp(int rawCode) {
  float celsius = 0;
  // convert raw reading to celsius
  if (rawCode <= 289) {
    celsius = 5 + (rawCode - 289) / 9.82;
  }
  if (rawCode > 289 && rawCode <= 342) {
    celsius = 10 + (rawCode - 342) / 10.60;
  }
  if (rawCode > 342 && rawCode <= 398) {
    celsius = 15 + (rawCode - 398) / 11.12;
  }
  if (rawCode > 398 && rawCode <= 455) {
    celsius = 20 + (rawCode - 455) / 11.36;
  }
  if (rawCode > 455 && rawCode <= 512) {
    celsius = 25 + (rawCode - 512) / 11.32;
  }
  if (rawCode > 512 && rawCode <= 566) {
    celsius = 30 + (rawCode - 566) / 11.00;
  }
  if (rawCode > 566 && rawCode <= 619) {
    celsius = 35 + (rawCode - 619) / 10.44;
  }
  if (rawCode > 619 && rawCode <= 667) {
    celsius = 40 + (rawCode - 667) / 9.73;
  }
  if (rawCode > 667) {
    celsius = 45 + (rawCode - 712) / 8.90;
  }

  // print temp information
  Serial.print("Temperature: ");
  Serial.print(rawCode);
  Serial.print(" (raw) ");
  Serial.print(celsius, 2);
  Serial.println(" Celsius");

  // check temp range and display accordingly
  if (celsius < MIN_TEMP) {
    printDisplay(LOWT);
  } else if (celsius > MAX_TEMP) {
    printDisplay(HIGHT);
  } else {
    printDisplay(NORMALT);
  }
}

// print to display
void printDisplay(int display) {
  // set latch low until changes are made then return to high
  digitalWrite(LATCH, LOW);
  shiftOut(SER, CLOCK, MSBFIRST, display);
  digitalWrite(LATCH, HIGH);
}

// inject low temperatures
void injectTempLow() {
  lowTempInterrupt.end();
  Serial.println("INJECT: LOW");
  injectTemp(-80);
}

// inject high temperatures
void injectTempHigh() {
  highTempInterrupt.end();
  Serial.println("INJECT: HIGH");
  injectTemp(800);
}

// inject a temperature for set amount of seconds
void injectTemp(int temp) {
  int startTime = millis();
  lastTime = millis();
  // inject a high temp for 4 seconds
  while (millis() - startTime <= INTERRUPT_TIMING) {
    // calculate and print information at interval
    if ((millis() - lastTime) >= TIMING) {
      readTemp(temp);
      lastTime = millis();
    }
  }
}
