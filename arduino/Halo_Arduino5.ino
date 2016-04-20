/*
This Script runthe the flow sensor, water temp sensor (i2c), voltage reading.
*/


//-------------------------------------------
//                Includes
//-------------------------------------------

#include <OneWire.h> // for water temp sensor
#include <avr/wdt.h> // for watchdog

//-------------------------------------------
//                Variables
//-------------------------------------------

char inData[20];                                // Allocate some space for the string
char inChar=-1;                                 // Where to store the character read
byte index = 0;                                 // Index into array; where to store the character

// flow sensor
volatile int NbTopsFan = 0;                       // measuring the rising edges of the signal
int Calc;
char data_buff[30];
int hallsensor = 2;                           // The pin location of the sensor

//Votl readings
// number of analog samples to take per reading
#define NUM_SAMPLES 5                    // Sample size
#define CAL_VOLT 5.08                      // calibrated reference voltage

unsigned char sample_count = 0;         // current sample number
float voltage1 = 0.0;                    // calculated voltage1
float voltage2 = 0.0;                    // calculated voltage2
int sum_votl1 = 0;                      // sum of samples taken voltage 1
int sum_votl2 = 0;                      // sum of samples taken voltage 2      
float cal_ref_div1 = 0.319;       // calibrated voltage divide1
float cal_ref_div2 = 0.321;       // calibrated voltage divide2
float volt_string1 = 0.0;
float volt_string2 = 0.0;
char volt_float1[10];                  // calculated voltage to string
char volt_float2[10];                  // calculated voltage to string

// water temp
OneWire  ds(10);                        // on pin 10 (a 4.7K resistor is necessary)                         
byte i;
byte present = 0;
byte type_s;
byte data[12];
byte addr[8];
float celsius, fahrenheit;
char temp_float[10];

//BMP
volatile int pulse = 0;
int BPM = 80;
const int HR_RX = 3;

// ATUO LED PWM
int ledPin = 9;              // LED connected to digital pin 9
int analogPin1 = 3;           // Photocell connected to analog pin 3 USE PULL DOWN RESISTOR
int analogPin2 = 4;           // Photocell connected to analog pin 4 USE PULL DOWN RESISTOR

int PWM_Val1 = 0;         // variable to store the read value
int PWM_Val2 = 0;         // variable to store the read value

int counter = 1;

//-------------------------------------------
//                Setup
//-------------------------------------------

void setup() {
  //wdt_disable();
  Serial.begin(9600);                       // This is the setup function where the serial port is initialised
  //Serial.flush();
  //Serial.println("Starting");
  //Serial.flush();
  
  //Flow sensor interrupt
  pinMode(hallsensor, INPUT);               // initializes digital pin 2 as an input flow sensor
  attachInterrupt(0, rpm, RISING);          // and the interrupt is attached

  //BPM interrupt
  pinMode (HR_RX, INPUT);  
  attachInterrupt(1, bpm, HIGH);
  
  pinMode(ledPin, OUTPUT);                 // sets the pin as output
  
  pinMode(13, OUTPUT);                      // LED for boot
  digitalWrite(13, HIGH);
  delay(1000);
  digitalWrite(13, LOW);
  //delay(500);
  //watchdogSetup();
}

//-------------------------------------------
//                Setup functions
//-------------------------------------------

void rpm ()                                   // This is the function that the interupt calls
{
    NbTopsFan++;                              // This function measures the rising and falling edge of the hall effect sensors signal
}

void bpm ()                                   // This is the function that the interupt calls
{
    pulse++;                                  // This function measures the HIGH on the BPM sensor signal
}

void watchdogSetup(void) {
  cli(); // disable all  interrupts
  wdt_reset(); // reset the WDT timer
  /* WDTCSR config:
  WDIE = 1 : Interupt Enable
  WDE = 1 : Reset Enabled
  WDP3 = 0 : Set the four prescaler bits for a 2 sec timeout (2000 msec)
  WDP2 = 1
  WDP1 = 1
  WDP0 = 1
  */
  // Watchdog config mode:
  WDTCSR |= (1<<WDCE) | (1<<WDE);
  // set watchdog
  WDTCSR = (1<<WDIE) | (1<<WDE) | (0<<WDP3) | (1<<WDP2) | (1<<WDP1) | (1<<WDP0);
  sei(); // Enable interrupts
}

//-------------------------------------------
//               Check Buffer
//-------------------------------------------

char Comp(char* This) {
    while (Serial.available() > 0)              // Don't read unless
                                                // there you know there is data
    {
        if(index < 19)                           // One less than the size of the array
        {
            inChar = Serial.read();                 // Read a character
            inData[index] = inChar;                 // Store it
            index++;                                 // Increment where to write next
            inData[index] = '\0';                   // Null terminate the string
        }
    }

    if (strcmp(inData,This)  == 0) {
        for (int i=0;i<19;i++) {
            inData[i]=0;
        }
        index=0;
        return(0);
    }
    else {
        return(1);
    }
}

//-------------------------------------------
//                Functions
//-------------------------------------------

//----------
// Water Temp
//----------  
void water_temp () 
{
  /*
  if ( !ds.search(addr)) {
    //Serial.println("No more addresses.");
    //Serial.println();
    ds.reset_search();
    delay(500);
    return;
  }
  
  //Serial.print("ROM =");
  for( i = 0; i < 8; i++) {
    //Serial.write(' ');
    //Serial.print(addr[i], HEX);
  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
      //Serial.println("CRC is not valid!");
      return;
  }
  //Serial.println();
 
  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:
      //Serial.println("  Chip = DS18S20");  // or old DS1820
      type_s = 1;
      break;
    case 0x28:
      //Serial.println("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
      //Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
      //Serial.println("Device is not a DS18x20 family device.");
      return;
  } 

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  
  delay(250);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
 present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  //Serial.print("  Data = ");
  //Serial.print(present, HEX);
  //Serial.print(" ");
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
    //Serial.print(data[i], HEX);
    //Serial.print(" ");
  }
  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;
  
  dtostrf(celsius, 5, 2, temp_float);
  
 
  */ 
  //temp setup due to sensors death!***************************************************************************************
  celsius = 12;
  dtostrf(celsius, 4, 2, temp_float);

  
}
//----------

//----------
//Flow sensor
//----------
void flow_speed () 
{
  
  //sei();                                    // Enables interrupts
  //delay (250);                             // Wait 1 second
  //cli();                                    // Disable interrupts
 
  Calc = ((NbTopsFan) * 60 / 4.5);            // (Pulse frequency x 60) / 4.5Q, = flow rate in L/hour
  NbTopsFan = 0;                            // Set NbTops to 0 ready for calculations
  
}
//----------

//----------
//Read Voltage
//----------
void read_volts () 
{
  // take a number of analog samples and add them up
   while (sample_count < NUM_SAMPLES) {
       sum_votl1 += analogRead(A0);
       sum_votl2 += analogRead(A1);
       sample_count++;
       delay(10);
   }
  
   voltage1 = (((float)sum_votl1 / (float)NUM_SAMPLES) * (CAL_VOLT / 1023.0));
   volt_string1 = ((voltage1) / cal_ref_div1);
   
   voltage2 = (((float)sum_votl2 / (float)NUM_SAMPLES) * (CAL_VOLT / 1023.0));
   volt_string2 = ((voltage2) / cal_ref_div2);
   
   sample_count = 0;
   sum_votl1 = 0;
   sum_votl2 = 0;
      
   dtostrf(volt_string1, 4, 2, volt_float1);
   dtostrf(volt_string2, 4, 2, volt_float2);
}
//----------

//----------
//BMP Stuff Here
//----------
void get_bpm () 
{
  BPM = ((pulse) * (1.5));              // calculation for the BPM
  pulse = 0;                            // Set pulse to 0 ready for calculations
  
  //BPM = 87;
  //pulse = 0;  
}
//----------

//----------
//Atuo LED on PWM
//----------
void pwm_led () 
{
  // analogRead values go from 0 to 1023, analogWrite values from 0 to 255
  // used linear form to map analog read to PWM value
  PWM_Val1 = ((-0.31875 * (analogRead(analogPin1))) + 318.8);
  PWM_Val2 = ((-0.31875 * (analogRead(analogPin2))) + 318.8);
  
  // Catch out of bound cases
  if (PWM_Val1 > 255){
    PWM_Val1=255;
  }
  else if (PWM_Val1 <0){
    PWM_Val1 = 0;
  }
  
  if (PWM_Val2 > 255){
    PWM_Val2=255;
  }
  else if (PWM_Val2 <0){
    PWM_Val2 = 0;
  }
  
  //Set the LED brightness based on 2 value ave
  analogWrite(ledPin, ((PWM_Val1 + PWM_Val2)/2) );  // analogRead values go from 0 to 1023, analogWrite values from 0 to 255
    
}
//----------

//-------------------------------------------
//                Main loop
//-------------------------------------------

void loop()
{
  pwm_led ();
  
  if ((counter % 199) == 0){
    flow_speed ();
    
  }

  else if ((counter % 3999) == 0){
    get_bpm ();
  }
  
    
  //----------
  //Send String + Bblink LED
  //----------
  else if ((counter % 200) == 0) {
    water_temp ();
    read_volts ();
        
    //----------
    //Build String
    //----------
  
    sprintf(data_buff, "%d %s %s %s %d\n", Calc, temp_float, volt_float1, volt_float2, BPM);
    //----------
    
    Serial.flush();
    Serial.write(data_buff);
    Serial.flush();
    digitalWrite(13, HIGH);
   
    /*
    if (Comp("A")==0) {
        digitalWrite(13, HIGH);
        delay(1000);
        digitalWrite(13, LOW);
        delay(500);
    }
    */
    
  }
  
  else if ((counter % 201) == 0) {
    digitalWrite(13, LOW);
  }
  
  //counter reset after 10 secs
  else if(counter == 4000){
   counter = 1; 
  }
  
  counter ++;
  delay(5);
  
  //----------
  
  //wdt_reset(); // kicks the dog
}

//-------------------------------------------
//                ISR
//-------------------------------------------

ISR(WTD_vect) { // watchdog interrupt functions if needed

}

