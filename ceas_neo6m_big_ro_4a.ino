/* 
 * Arduino GPS clock with local time using NEO-6M module and 20x4 LCD.
 * This is a free software with NO WARRANTY.
 * https://simple-circuit.com/
 * https://simple-circuit.com/arduino-gps-clock-local-time-neo-6m/
 * http://www.instructables.com/id/Custom-Large-Font-For-16x2-LCDs/
 * small changes by Nicu FLORICA (niq_ro) from https://nicuflorica.blogspot.com/
 * ver. 2.0 - niq_ro added current time zones adjust (UTC-12.... UTC+12)
 * ver. 3.0 - niq_ro added 12/24 hours format selection
 * ver. 4.0 - niq_ro added 38 local time zones, from -12 to +14 with all real intermediate values
 * ver. 4.1 - use just first button for incresed local time zone, second button used for change 12/24-hur format
 * 
*/
 
#include <EEPROM.h>
#include <TinyGPS++.h>        // include TinyGPS++ library
#include <TimeLib.h>          // include Arduino time library
#include <SoftwareSerial.h>   // include software serial library
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// Set the LCD address to 0x27 for a 20 chars and 4 line display
LiquidCrystal_I2C lcd(0x27, 20, 4);
 
TinyGPSPlus gps;
 
#define S_RX    4   // define software serial RX pin
#define S_TX    3   // define software serial TX pin
 
SoftwareSerial SoftSerial(S_RX, S_TX);   // configure SoftSerial library
 
 
//#define time_offset   7200  // define a clock offset of 3600 seconds (1 hour) ==> UTC + 1
int timezone = 12; 
long time_offset = 0;
byte format12 = 0;

float diferenta[38] = {-12., -11.,-10.,-9.5,-9.,-8.,-7.,-6.,-5.,-4.,-3.5,-3.,-2.,-1.,0,
                      1.,2.,3.,3.5,4.,4.5,5.,5.5,5.75,6.,6.5,7.,8.,8.75,9.,9.5,10.,10.5,
                      11.,12.,12.75,13.,14};   // added manualy by niq_ro
                      

#define sw1 12  // pin for increase the time zone
#define sw2 11 // pin for decrease the time zone
#define adresa  100  // adress for store the
byte zero = 0;  // variable for control the initial read/write the eeprom memory

// variable definitions
//char Taim[]  = "TIME: 00:00:00";
//char Date[]  = "DATE: 00-00-2000";
char Taim[]  = "00:00:00";
char Date[]  = "00/00/2000";

byte last_second, Second, Minute, Hour, Day, Month;
int Year;

byte sat;
byte x = 0;
byte y = 0;

int orele, orez, oreu, minz, minu, secz, secu;
int orez0 = 11;
int oreu0 = 11;
int minz0 = 11;
int minu0 = 11;
int secz0 = 11;
int secu0 = 11;

float diferenta0, diferenta1;
int diferenta2;

// the 8 arrays that form each segment of the custom numbers
byte LT[8] = 
{
  B00111,
  B01111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};
byte UB[8] =
{
  B11111,
  B11111,
  B11111,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};
byte RT[8] =
{
  B11100,
  B11110,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};
byte LL[8] =
{
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B01111,
  B00111
};
byte LB[8] =
{
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
  B11111
};
byte LR[8] =
{
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11110,
  B11100
};
byte UMB[8] =
{
  B11111,
  B11111,
  B11111,
  B00000,
  B00000,
  B00000,
  B11111,
  B11111
};
byte LMB[8] =
{
  B11111,
  B11111,
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
  B11111
};

 
void setup(void)
{
  SoftSerial.begin(9600);  // initialize software serial at 9600 baud
  Serial.begin(9600);
  Serial.println(" ");
zero = EEPROM.read(adresa - 1); // variable for write initial values in EEPROM
if (zero != 14)
{
EEPROM.update(adresa - 1, 14);  // zero
EEPROM.update(adresa, 12); // time zone (0...24 -> -12...+12) // https://en.wikipedia.org/wiki/Coordinated_Universal_Time
EEPROM.update(adresa + 1, 0);  // 0 => 24hours format, 1 => 12hours format
} 
  // set up the LCD's number of columns and rows
  lcd.begin(); // initialize the LCD
  // assignes each segment a write number
  lcd.createChar(8,LT);
  lcd.createChar(1,UB);
  lcd.createChar(2,RT);
  lcd.createChar(3,LL);
  lcd.createChar(4,LB);
  lcd.createChar(5,LR);
  lcd.createChar(6,UMB);
  lcd.createChar(7,LMB);
  lcd.clear(); // clear the screen
  lcd.backlight(); // Turn on the blacklight and print a message.

// read EEPROM memory;
timezone = EEPROM.read(adresa);  // timezone +12
//timezone = timezone -12; // convert from -12 to +12
//time_offset = (long)timezone*3600;  // convert in seconds
time_offset = (float)diferenta[timezone]*3600;  // convert in seconds
format12 = EEPROM.read(adresa+1);
 
  lcd.setCursor(5, 0); // move LCD cursor to column 1, row 0 [upper left position (0, 0)]
  lcd.print("GPS  CLOCK");
  lcd.setCursor(6, 1);
  lcd.print("ver. 4.1");
  lcd.setCursor(0, 2);
  lcd.print("  sketch by niq_ro  ");
  lcd.setCursor(0, 3);
  lcd.print("   (Nicu FLORICA)   "); 

pinMode(sw1, INPUT);
pinMode(sw2, INPUT);
digitalWrite(sw1, HIGH);
digitalWrite(sw2, HIGH);  
 
  delay(5000);
  lcd.clear();

//setTime(Hour, Minute, Second, Day, Month, Year);
setTime(0, 0, 0, 1, 1, 1973);  // niq_ro birthday
}

void custom0O()
{ // uses segments to build the number 0
  lcd.setCursor(x,y); 
  lcd.write(8);  
  lcd.write(1); 
  lcd.write(2);
  lcd.setCursor(x,y+1); 
  lcd.write(3);  
  lcd.write(4);  
  lcd.write(5);
}

void custom1()
{
  lcd.setCursor(x,y);
  lcd.write(1);
  lcd.write(2);
  lcd.setCursor(x+1,y+1);
  lcd.write(255);
}

void custom2()
{
  lcd.setCursor(x,y);
  lcd.write(1);
  lcd.write(6);
  lcd.write(2);
  lcd.setCursor(x,y+1);
  lcd.write(8);
  lcd.write(7);
  lcd.write(4);
}

void custom3()
{
  lcd.setCursor(x,y);
  lcd.write(6);
  lcd.write(6);
  lcd.write(2);
  lcd.setCursor(x,y+1);
  lcd.write(7);
  lcd.write(7);
  lcd.write(5); 
}

void custom4()
{
  lcd.setCursor(x,y);
  lcd.write(8);
  lcd.write(4);
  lcd.write(255);
  lcd.setCursor(x,y+1);
  lcd.write(1);
  lcd.write(1);
  lcd.write(255);
}

void custom5()
{
  lcd.setCursor(x,y);
  lcd.write(255);
  lcd.write(6);
  lcd.write(1);
  lcd.setCursor(x,y+1);
  lcd.write(7);
  lcd.write(7);
  lcd.write(5);
}

void custom6()
{
  lcd.setCursor(x,y);
  lcd.write(8);
  lcd.write(6);
  lcd.write(6);
  lcd.setCursor(x,y+1);
  lcd.write(3);
  lcd.write(7);
  lcd.write(5);
}

void custom7()
{
  lcd.setCursor(x,y);
  lcd.write(1);
  lcd.write(1);
  lcd.write(2);
  lcd.setCursor(x+1,y+1);
  lcd.write(8);
}

void custom8()
{
  lcd.setCursor(x,y);
  lcd.write(8);
  lcd.write(6);
  lcd.write(2);
  lcd.setCursor(x,y+1);
  lcd.write(3);
  lcd.write(7);
  lcd.write(5);
}

void custom9()
{
  lcd.setCursor(x,y);
  lcd.write(8);
  lcd.write(6);
  lcd.write(2);
  lcd.setCursor(x,y+1);
  lcd.write(4);
  lcd.write(7);
  lcd.write(5);
}

void customA()
{
  lcd.setCursor(x,y);
  lcd.write(8);
  lcd.write(6);
  lcd.write(2);
  lcd.setCursor(x,y+1);
  lcd.write(255);
  lcd.write(254);
  lcd.write(255);
}

void customB()
{
  lcd.setCursor(x,y);
  lcd.write(255);
  lcd.write(6);
  lcd.write(5);
  lcd.setCursor(x,y+1);
  lcd.write(255);
  lcd.write(7);
  lcd.write(2);
}

void customC()
{
  lcd.setCursor(x,y);
  lcd.write(8);
  lcd.write(1);
  lcd.write(1);
  lcd.setCursor(x,y+1);
  lcd.write(3);
  lcd.write(4);
  lcd.write(4);
}

void customD()
{
  lcd.setCursor(x,y); 
  lcd.write(255);  
  lcd.write(1); 
  lcd.write(2);
  lcd.setCursor(x,y+1); 
  lcd.write(255);  
  lcd.write(4);  
  lcd.write(5);
}

void customE()
{
  lcd.setCursor(x,y);
  lcd.write(255);
  lcd.write(6);
  lcd.write(6);
  lcd.setCursor(x,y+1);
  lcd.write(255);
  lcd.write(7);
  lcd.write(7); 
}

void customF()
{
  lcd.setCursor(x,y);
  lcd.write(255);
  lcd.write(6);
  lcd.write(6);
  lcd.setCursor(x,y);
  lcd.write(255);
}

void customG()
{
  lcd.setCursor(x,y);
  lcd.write(8);
  lcd.write(1);
  lcd.write(1);
  lcd.setCursor(x,y+1);
  lcd.write(3);
  lcd.write(4);
  lcd.write(2);
}

void customH()
{
  lcd.setCursor(x,y);
  lcd.write(255);
  lcd.write(4);
  lcd.write(255);
  lcd.setCursor(x,y+1);
  lcd.write(255);
  lcd.write(254);
  lcd.write(255); 
}

void customI()
{
  lcd.setCursor(x,y);
  lcd.write(1);
  lcd.write(255);
  lcd.write(1);
  lcd.setCursor(x,y+1);
  lcd.write(4);
  lcd.write(255);
  lcd.write(4);
}

void customJ()
{
  lcd.setCursor(x+2,y);
  lcd.write(255);
  lcd.setCursor(x,y+1);
  lcd.write(4);
  lcd.write(4);
  lcd.write(5);
}

void customK()
{
  lcd.setCursor(x,y);
  lcd.write(255);
  lcd.write(4);
  lcd.write(5);
  lcd.setCursor(x,y+1);
  lcd.write(255);
  lcd.write(254);
  lcd.write(2); 
}

void customL()
{
  lcd.setCursor(x,y);
  lcd.write(255);
  lcd.setCursor(x,y+1);
  lcd.write(255);
  lcd.write(4);
  lcd.write(4);
}

void customM()
{
  lcd.setCursor(x,y);
  lcd.write(8);
  lcd.write(3);
  lcd.write(5);
  lcd.write(2);
  lcd.setCursor(x,y+1);
  lcd.write(255);
  lcd.write(254);
  lcd.write(254);
  lcd.write(255);
}

void customN()
{
  lcd.setCursor(x,y);
  lcd.write(8);
  lcd.write(2);
  lcd.write(254);
  lcd.write(255);
  lcd.setCursor(x,y+1);
  lcd.write(255);
  lcd.write(254);
  lcd.write(3);
  lcd.write(5);
}

void customP()
{
  lcd.setCursor(x,y);
  lcd.write(255);
  lcd.write(6);
  lcd.write(2);
  lcd.setCursor(x,y+1);
  lcd.write(255);
}

void customQ()
{
  lcd.setCursor(x,y);
  lcd.write(8);
  lcd.write(1);
  lcd.write(2);
  lcd.setCursor(x,y+1);
  lcd.write(3);
  lcd.write(4);
  lcd.write(255);
  lcd.write(4);
}

void customR()
{
  lcd.setCursor(x,y);
  lcd.write(255);
  lcd.write(6);
  lcd.write(2);
  lcd.setCursor(x,y+1);
  lcd.write(255);
  lcd.write(254);
  lcd.write(2); 
}

void customS()
{
  lcd.setCursor(x,y);
  lcd.write(8);
  lcd.write(6);
  lcd.write(6);
  lcd.setCursor(x,y+1);
  lcd.write(7);
  lcd.write(7);
  lcd.write(5);
}

void customT()
{
  lcd.setCursor(x,y);
  lcd.write(1);
  lcd.write(255);
  lcd.write(1);
  lcd.setCursor(x,y+1);
  lcd.write(254);
  lcd.write(255);
}

void customU()
{
  lcd.setCursor(x,y); 
  lcd.write(255);  
  lcd.write(254); 
  lcd.write(255);
  lcd.setCursor(x,y+1); 
  lcd.write(3);  
  lcd.write(4);  
  lcd.write(5);
}

void customV()
{
  lcd.setCursor(x,y); 
  lcd.write(3);  
  lcd.write(254);
  lcd.write(254); 
  lcd.write(5);
  lcd.setCursor(x+1,y+1); 
  lcd.write(2);  
  lcd.write(8);
}

void customW()
{
  lcd.setCursor(x,y);
  lcd.write(255);
  lcd.write(254);
  lcd.write(254);
  lcd.write(255);
  lcd.setCursor(x,y+1);
  lcd.write(3);
  lcd.write(8);
  lcd.write(2);
  lcd.write(5);
}

void customX()
{
  lcd.setCursor(x,y);
  lcd.write(3);
  lcd.write(4);
  lcd.write(5);
  lcd.setCursor(x,y+1);
  lcd.write(8);
  lcd.write(254);
  lcd.write(2); 
}

void customY()
{
  lcd.setCursor(x,y);
  lcd.write(3);
  lcd.write(4);
  lcd.write(5);
  lcd.setCursor(x+1,y+1);
  lcd.write(255);
}

void customZ()
{
  lcd.setCursor(x,y);
  lcd.write(1);
  lcd.write(6);
  lcd.write(5);
  lcd.setCursor(x,y+1);
  lcd.write(8);
  lcd.write(7);
  lcd.write(4);
}

void customqm()
{
  lcd.setCursor(x,y);
  lcd.write(1);
  lcd.write(6);
  lcd.write(2);
  lcd.setCursor(x,y+1);
  lcd.write(254);
  lcd.write(7);
}

void customsm()
{
  lcd.setCursor(x,y);
  lcd.write(255);
  lcd.setCursor(x,y+1);
  lcd.write(7);
}

void customsplus()  // made by niq_ro
{
  lcd.setCursor(x,y);
  lcd.write(4);
  lcd.write(255);
  lcd.write(4);
  lcd.setCursor(x,y+1);
  lcd.write(1);
  lcd.write(255);
  lcd.write(1);
}

void customminus() // made by niq_ro
{
  lcd.setCursor(x,y);
  lcd.write(4);
  lcd.write(4);
  lcd.write(4);
  lcd.setCursor(x,y+1);
  lcd.write(1);
  lcd.write(1);
  lcd.write(1);
}

void customgrad()
{
  lcd.setCursor(x,y);
  lcd.write(1);
  lcd.write(8);
  lcd.write(1);
  lcd.write(1);
  lcd.setCursor(x,y+1);
  lcd.write(254);
  lcd.write(3);
  lcd.write(4);
  lcd.write(4);
}

void spatiu()
{
  lcd.setCursor(x,y);
  lcd.print("    ");
  lcd.setCursor(x,y+1);
  lcd.print("    ");
}

void custompuncte()
{
 // lcd.setCursor(x,y);
 // lcd.write(1);
 // lcd.write(6);
 // lcd.write(2);
  lcd.setCursor(x,y+1);
  lcd.write(254);
  lcd.write(7);
}

// subrutina de afisare a numerelor
void afisare(int numar)
{
  switch (numar)
  {
    case 0:
    custom0O();
    break;

    case 1:
    custom1();
    break;

    case 2:
    custom2();
    break;

    case 3:
    custom3();
    break;

    case 4:
    custom4();
    break;

    case 5:
    custom5();
    break;

    case 6:
    custom6();
    break;

    case 7:
    custom7();
    break;

    case 8:
    custom8();
    break;

    case 9:
    custom9();
    break;
  }
}



 
void loop()
{

//if ((digitalRead(sw1) == LOW) and (digitalRead(sw2) == HIGH))
if (digitalRead(sw1) == LOW)
{
  // Serial.println("+");
  timezone = timezone + 1;
  delay(250);
  if (timezone > 37) timezone = 0;
 // time_offset = (long)timezone*3600;  // convert in seconds
 time_offset = (float)diferenta[timezone]*3600.;
  EEPROM.update(adresa, timezone); // store in eeprom 
  lcd.clear();
  orez0 = orez + 33;
  oreu0 = oreu + 33;
  minz0 = minz + 33;
  minu0 = minu + 33;
}
/*
if ((digitalRead(sw2) == LOW) and (digitalRead(sw1) == HIGH))
{
 // Serial.println("-");
  timezone = timezone - 1;
  delay(250);
  if (timezone < 0) timezone = 37;
//  time_offset = (long)timezone*3600;  // convert in seconds
time_offset = (float)diferenta[timezone]*3600.;
  EEPROM.update(adresa, timezone); // store in eeprom
  lcd.clear();
  orez0 = orez + 33;
  oreu0 = oreu + 33;
  minz0 = minz + 33;
  minu0 = minu + 33;
}
*/
diferenta0 = diferenta[timezone];
if (diferenta0 < 0)
   diferenta1 = - diferenta0;
 else
   diferenta1 = diferenta0;
diferenta2 = 100 * diferenta1;
lcd.setCursor(11,3); // move cursor to column 14 row 3
if (diferenta1 < 10)  lcd.print(" ");
  lcd.print("UTC");     
if (diferenta0 > 0) lcd.print("+");
if (diferenta0 < 0) lcd.print("-");
if (diferenta0 != 0)
{
lcd.print(diferenta2 / 100);  
if (diferenta2 % 100 == 0) lcd.print("   ");
if (diferenta2 % 100 == 50) lcd.print(":30");
if (diferenta2 % 100 == 75) lcd.print(":45");
if (diferenta2 % 100 == 25) lcd.print(":15");
}
else lcd.print("     ");

//if ((digitalRead(sw2) == LOW) and (digitalRead(sw1) == LOW))
if (digitalRead(sw2) == LOW)
{
 // Serial.println("-/+");
  format12 = format12 + 1;
  format12 = format12 % 2;
 // delay(250);
  EEPROM.update(adresa + 1, format12); // store in eeprom
  lcd.clear();
  orez0 = orez + 33;
  oreu0 = oreu + 33;
  minz0 = minz + 33;
  minu0 = minu + 33;
if (format12 == 0)
{
lcd.setCursor(0,0); // move cursor to column 0 row 0
lcd.print("2");
lcd.setCursor(0,1); // move cursor to column 0 row 1
lcd.print("4");
}
else
{
lcd.setCursor(0,0); // move cursor to column 0 row 0
lcd.print("1");
lcd.setCursor(0,1); // move cursor to column 0 row 1
lcd.print("2");  
}
delay(500);
lcd.setCursor(0,0); // move cursor to column 0 row 0
lcd.print(" ");
lcd.setCursor(0,1); // move cursor to column 0 row 1
lcd.print(" ");  
}

   
  while (SoftSerial.available() > 0)
  {

//if ((digitalRead(sw1) == LOW) and (digitalRead(sw2) == HIGH))
if (digitalRead(sw1) == LOW)
{
  timezone = timezone + 1;
  delay(250);
  if (timezone > 37) timezone = 0;
//  time_offset = (long)timezone*3600;  // convert in seconds
time_offset = (float)diferenta[timezone]*3600.;
  EEPROM.update(adresa, timezone); // store in eeprom 
  lcd.clear();
  orez0 = orez + 33;
  oreu0 = oreu + 33;
  minz0 = minz + 33;
  minu0 = minu + 33;
}
/*
if ((digitalRead(sw2) == LOW) and (digitalRead(sw1) == HIGH))
{
  timezone = timezone - 1;
  delay(250);
  if (timezone < 0) timezone = 37;
//  time_offset = (long)timezone*3600;  // convert in seconds
time_offset = (float)diferenta[timezone]*3600.;
  EEPROM.update(adresa, timezone); // store in eeprom
  lcd.clear();
  orez0 = orez + 33;
  oreu0 = oreu + 33;
  minz0 = minz + 33;
  minu0 = minu + 33;
}
*/

//if ((digitalRead(sw2) == LOW) and (digitalRead(sw1) == LOW))
if (digitalRead(sw2) == LOW)
{
 // Serial.println("-/+");
  format12 = format12 + 1;
  format12 = format12 % 2;
  delay(250);
  EEPROM.update(adresa + 1, format12); // store in eeprom
  lcd.clear();
  orez0 = orez + 33;
  oreu0 = oreu + 33;
  minz0 = minz + 33;
  minu0 = minu + 33;
}
    
    if (gps.encode(SoftSerial.read()))
    {
      // get time from GPS module
      if (gps.time.isValid())
      {
        Minute = gps.time.minute();
        Second = gps.time.second();
        Hour   = gps.time.hour();
      }
 
      // get date from GPS module
      if (gps.date.isValid())
      {
        Day   = gps.date.day();
        Month = gps.date.month();
        Year  = gps.date.year();
      }

      if(last_second != gps.time.second())  // if time has changed
      {
        last_second = gps.time.second();
 
        // set current UTC time
        setTime(Hour, Minute, Second, Day, Month, Year);
        // add the offset to get local time
        Serial.println(time_offset);
        adjustTime(time_offset);

if (format12 == 0)
{
  orele = hour();
  lcd.setCursor(0,0); // move cursor to column 0 row 0  
  lcd.print(" ");
  lcd.setCursor(0,1); // move cursor to column 0 row 1
  lcd.print(" ");
}
else
{
  orele = hour()%12;
  if (orele == 0) orele = 12;
  lcd.setCursor(0,0); // move cursor to column 0 row 0
  if (hour()/12 == 0) 
  {   
   lcd.print("A");
  }
  else
  {
   lcd.print("P");
  }
  lcd.setCursor(0,1); // move cursor to column 0 row 1
  lcd.print("M");
if (orele == 12)
{
if ((hour()/12 == 0) and (minute() == 0))
{
  lcd.setCursor(0,2); // move cursor to column 0 row 2
  lcd.print("midnight");
}
if ((hour()/12 == 1) and (minute() == 0))
{
  lcd.setCursor(0,2); // move cursor to column 0 row 2
  lcd.print("noon    ");
}
}
else
{
  lcd.setCursor(0,2); // move cursor to column 0 row 2
  lcd.print("        ");  
}
}

orez = orele / 10;
oreu = orele % 10;
minz = minute() / 10;
minu = minute() % 10;
secz = second() / 10;
secu = second() % 10;

        // update date array
        Date[8] = (year()  / 10) % 10 + '0';
        Date[9] =  year()  % 10 + '0';
        Date[3]  =  month() / 10 + '0';
        Date[4] =  month() % 10 + '0';
        Date[0]  =  day()   / 10 + '0';
        Date[1]  =  day()   % 10 + '0';
 
        // print date & day
        lcd.setCursor(0, 3);     // move cursor to column 11 row 1
        
        if ((millis()/1000)%12 < 3)
        print_wday1(weekday());   // print day of the week
        else
        if (((millis()/1000)%9 >= 6) and ((millis()/1000)%12 <= 8))
        print_wday2(weekday());
        else
        lcd.print(Date);         // print date (DD-MM-YY)


    if (orez != orez0)
    {
      x = 2;
      y = 0;
      if (orez == 0) spatiu();
      else
      afisare(orez);
    }
    if (oreu != oreu0)
    {
      x = 6;
      y = 0;
      spatiu();
      afisare(oreu);
    }

    x = 9;
    y = 0;

    if (millis()/1000%2 == 0)
    custompuncte();
    else
    {
      lcd.setCursor(x+1,y);
      lcd.print(" ");
      lcd.setCursor(x+1,y+1);
      lcd.print(" ");
    }

    
    if (minz != minz0)
    {
      x = 12;
      y = 0;
      spatiu();
      afisare(minz);
    }
    if (minu != minu0)
    {
      x = 16;
      y = 0;
      spatiu();
      afisare(minu);
    }

orez0 = orez;
oreu0 = oreu;
minz0 = minz;
minu0 = minu;
secz0 = secz;
secu0 = secu;

lcd.setCursor(14,3); // move cursor to column 14 row 3
if (abs(timezone) < 10)  lcd.print(" ");
lcd.print("UTC");
if (timezone > 0) lcd.print("+");
if (timezone < 0) lcd.print("-");
if (timezone != 0)
lcd.print(abs(timezone));
else lcd.print("  ");
      }
    } 
  }

}  // end main loop
 
// function for displaying day of the week
void print_wday1(byte wday)
{
  switch(wday)
  {
    case 1:  lcd.print(" DUMINICA ");   break;
    case 2:  lcd.print(" LUNI     ");   break;
    case 3:  lcd.print(" MARTI    ");   break;
    case 4:  lcd.print(" MIERCURI ");   break;
    case 5:  lcd.print(" JOI      ");   break;
    case 6:  lcd.print(" VINERI   ");   break;
    default: lcd.print(" SAMBATA  ");
  }
}

void print_wday2(byte wday)
{
  switch(wday)
  {
    case 1:  lcd.print(" SUNDAY   ");   break;
    case 2:  lcd.print(" MONDAY   ");   break;
    case 3:  lcd.print(" TUESDAY  ");   break;
    case 4:  lcd.print("WEDNESDAY ");   break;
    case 5:  lcd.print(" THURSDAY ");   break;
    case 6:  lcd.print(" FRIDAY   ");   break;
    default: lcd.print(" SATURDAY ");
  }
}
// end of code.
