
// Original idea & coding by Yves Caze, Savoie Plongee, modified and supplemented by GoDive BRB, 2021
// Modified by Dominik Wiedmer 2021
// Version 1.1 04.10.2021
// Version 1.2 07.10.2023 Change of library names, Calibration goal less strict
// Modified by Johannes Six 2024
// Version 1.3 27.08.2024 Add Function to show "Air" when o2 is lower 22%
//                        All outputs in english
//                        Reset cursor in loop to fit my display better
//                        Added Correction Value for o2 measurements


#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <RunningAverage.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_SH110X.h> // Library for 1.3" AliExpress OLED display
#include <SPI.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1   //   QT-PY / XIAO
#define i2c_Address 0x3c //initialize with the I2C addr 0x3C Typically eBay OLED's
#define SH110X_NO_SPLASH
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


Adafruit_ADS1115 ads;    

float Vcalib = 0;        
float voltage = 0;       
float bruecke = 0;       

float gain = 0.03125;     
float calibMD62 = 600.00;   
float brueckeCalib = 0;   
float TempKomp = 0 ;       
unsigned long time;       

RunningAverage RA0(10);       
RunningAverage RA1(10);

int16_t adc0;
int16_t adc1;


void cal(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(8,3);
  display.print("Calibration");
  display.setCursor(8,13);
  display.print("O2 Sensor");
  display.setCursor(8,23);
  display.print("(Air 20.9% O2 )");
  display.display();
  int i = 0;
  float Vavg = 0;
  
  for(i = 1; i <10 or (abs (voltage - (Vavg / (i-1)))) > 0.005; i++) // chnged 0.001 to 0.005 for less strict goal
    {
      adc0 = ads.readADC_Differential_0_1();
      RA0.addValue(adc0);
      voltage = abs(RA0.getAverage()*gain); 
      Vavg = Vavg + voltage;
      delay(200);
    }
     
  
   display.clearDisplay();
   display.setCursor(8,23);
   display.print("Calibration OK");
 
   Vavg = Vavg / (i - 1);
   Vcalib = Vavg;

   display.setCursor(8,33);
   display.print("V Cal. = ");
   display.print(Vcalib,2);
   display.print(" mV");
   display.display();   
   delay(2000);
  
  }


void setup(void) {
  Serial.begin(9600);
  Wire.begin();
  Wire.setClock(400000L);
  Serial.print(F("Test")); // Setup and COM check
  pinMode(1, INPUT_PULLUP); // D1 as input with internal pull up resistor, so high when button is not pressed.

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.

  display.begin(i2c_Address, true); // Address 0x3C default
 //display.setContrast (0); // dim display
 
  display.display();
  //delay(2000);

  // Clear the buffer.
  display.clearDisplay();


  uint16_t time = millis();
  time = millis() - time;
  delay(1000);

 display.setTextSize(2);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(20, 20);
  display.print("START");
  display.display();
  
  delay(2000);
  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(20, 20);
  display.print("Trimix");
  display.setCursor(20, 40);
  display.print("Analyser");
  display.display();
  
  delay(2000);

  
  ads.setGain(GAIN_FOUR); // 4x gain 1 bit = 0.03125mV
  ads.begin();

 
 
  adc0 = ads.readADC_Differential_0_1();
  RA0.addValue(adc0);
  voltage = abs(RA0.getAverage()*gain);
  
  adc1 = ads.readADC_Differential_2_3();
  RA1.addValue(adc1);
  bruecke = RA1.getAverage()*gain;
 
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(8,3);
  display.print("He Bridge");
  display.setCursor(8,13);
  display.print("V Cell = ");
  display.print(voltage,2);
  display.print("  mV");
  display.setCursor(8,23);
  display.print("V Bridge = ");
  display.print(bruecke,2);
  display.print(" mV");
  display.display();
  delay(5000);

cal();
 

   display.clearDisplay();
   display.setCursor(8,3);
   display.print("Pre-heating");
   display.setCursor(8,23);
   display.print("Helium sensor...");
   display.display();
   delay(500);

      while(bruecke > 10){
         adc1 = ads.readADC_Differential_2_3();
         RA1.addValue(adc1);
         bruecke = RA1.getAverage()*gain;
         display.clearDisplay();
         display.setCursor(8,3);
         display.print("Pre-heating");
         display.setCursor(8,23);
         display.print("Helium sensor...");
         display.setCursor(8,43);    
         display.print("V Bridge= ");
         display.print(bruecke,0);
         display.print(" mV  ");
         display.display(); 
         delay(50);
         }
         
   display.clearDisplay();
   display.setCursor(8,13);
   display.print(" Helium sensor OK");
   display.display();           
   delay(2000);

   display.clearDisplay();
   display.setCursor(8,13);
   display.print(" Analyser ready "); 
   display.display();         
   delay(2000);
}

void loop() {
  // put your main code here, to run repeatedly:

  int16_t adc0;
  int16_t adc1;
  adc0 = ads.readADC_Differential_0_1();
  adc1 = ads.readADC_Differential_2_3();
  
  time = millis();      //  MS

  RA0.addValue(adc0);
  voltage = abs(RA0.getAverage()*gain);

  RA1.addValue(adc1);
  bruecke = RA1.getAverage()*gain;

  float nitrox = 0;
  float helium = 0;
  float o2_correction = 0.02;
  //Correct the o2 measurement by 1percent (if gas is below 20.9% correction value will be added; if gas is above 20.9% correction value will be subtracted)
  
    nitrox = voltage * (20.9 / Vcalib) * (1 + (o2_correction * (20.9 - (voltage * (20.9 / Vcalib))) / 20.9));

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0,3);
   // if (voltage > 1) {
       display.print("O2:");  
       display.print(nitrox,1);  
       display.print(" %");
       display.display(); 
 /*     }
      else {
        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(20,10);       
        display.print("O2 Sensor");
        display.setCursor(20,20);    
        display.print("abgelaufen ");
        display.display();
        delay(2000);
        
        }
        */
    display.setCursor(0,13);
    display.setTextSize(1);
    display.print("O2mV ");
    display.print(voltage,2);
    display.setCursor(66,13);       
    display.print("HEmV ");
    display.print(bruecke,0);
    
    display.display(); 


    bruecke = bruecke - brueckeCalib;  
    
    if (time < 480000) {  TempKomp = 0 ; }   
    if (time < 360000) {  TempKomp = 2 ; }
    if (time < 300000) {  TempKomp = 3 ; }
    if (time < 270000) {  TempKomp = 4 ; }
    if (time < 240000) {  TempKomp = 5 ; }
    if (time < 210000) {  TempKomp = 6 ; }
    if (time < 180000) {  TempKomp = 7 ; }
    if (time < 165000) {  TempKomp = 8 ; }
    if (time < 150000) {  TempKomp = 9 ; }
    if (time < 120000) {  TempKomp = 10 ; }
    if (time < 105000) {  TempKomp = 11 ; }
    if (time < 90000) {  TempKomp = 12 ; }
    if (time < 80000) {  TempKomp = 13 ; }
    if (time < 70000) {  TempKomp = 14 ; }
    if (time < 60000) {  TempKomp = 15 ; }
    if (time < 50000) {  TempKomp = 16 ; }
    if (time < 40000) {  TempKomp = 17 ; }
    if (time < 30000) {  TempKomp = 18 ; }
    
    bruecke = bruecke - TempKomp;          
   
    display.setCursor(66,3);
    display.setTextSize(1); 
    display.print("HE:");
    helium = 100 * bruecke / calibMD62;
    if (helium > 50) {
      helium = helium * (1 + (helium - 50) * 0.4 / 100);
      }
    if (helium > 2) {
      display.print(helium,1); 
      display.print(" %    ");
      display.display(); 
      }
    else {
      helium = 0;
      display.print("  0");
      display.print(" %");
      display.display();  
      }
      
  
    if (helium > 0) {
      display.setCursor(10,28);
      display.setTextSize(2);
      display.print("Trimix ");
      display.setCursor(10,48);
      display.print(nitrox,0);
      display.print(" / ");
      display.print(helium,0);
      display.print(" ");
      display.setCursor(10,53);
      display.display();
      }
    if (nitrox > 22) {
      display.setCursor(10,28);
      display.setTextSize(2);
      display.print("Nitrox ");
      display.setCursor(10,48);
      display.print(nitrox,0);
      display.display();
    }
    else {
      display.setCursor(10,28);
      display.setTextSize(2);
      display.print("Air ");
      display.setCursor(10,48);
      display.print(nitrox,0);
      display.display();
      }
       
// manual calibration when pressing button at D1

if(digitalRead(1) == LOW)
{
cal();
}

delay(2000);
}
