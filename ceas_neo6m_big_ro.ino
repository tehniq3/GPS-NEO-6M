/* 
 * Arduino GPS clock with local time using NEO-6M module and 20x4 LCD.
 * This is a free software with NO WARRANTY.
 * https://simple-circuit.com/
 * https://simple-circuit.com/arduino-gps-clock-local-time-neo-6m/
 * http://www.instructables.com/id/Custom-Large-Font-For-16x2-LCDs/
 * small changes by Nicu FLORICA (niq_ro) from https://nicuflorica.blogspot.com/
*/
 
 
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
 
 
#define time_offset   7200  // define a clock offset of 3600 seconds (1 hour) ==> UTC + 1
 
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

int orez, oreu, minz, minu, secz, secu;
int orez0 = 11;
int oreu0 = 11;
int minz0 = 11;
int minu0 = 11;
int secz0 = 11;
int secu0 = 11;
byte contor = 0;

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
 
  lcd.setCursor(0, 0); // move LCD cursor to column 1, row 0 [upper left position (0, 0)]
  lcd.print("  GPS CLOCK (UTC+2) ");
  lcd.setCursor(0, 2);
  lcd.print("  sketch by niq_ro  ");
  lcd.setCursor(0, 3);
  lcd.print("   (Nicu FLORICA)   "); 
  delay(5000);
  lcd.clear();
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

void customplus()  // made by niq_ro
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
  while (SoftSerial.available() > 0)
  {
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


      // get received sattelites from GPS module
      if (gps.satellites.isValid())
      {
       sat =  gps.satellites.value();
      }

      if(last_second != gps.time.second())  // if time has changed
      {
        last_second = gps.time.second();
        contor++;
 
        // set current UTC time
        setTime(Hour, Minute, Second, Day, Month, Year);
        // add the offset to get local time
        adjustTime(time_offset);


orez = hour() / 10;
oreu = hour() % 10;
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
      x = 1;
      y = 0;
      if (orez == 0) spatiu();
      else
      afisare(orez);
    }
    if (oreu != oreu0)
    {
      x = 5;
      y = 0;
      spatiu();
      afisare(oreu);
    }

    x = 8;
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
      x = 11;
      y = 0;
      spatiu();
      afisare(minz);
    }
    if (minu != minu0)
    {
      x = 15;
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

 
 lcd.setCursor(15,3); // move cursor to column 10 row 3
 if (sat < 10)  lcd.print(" ");
 lcd.print(sat);
 lcd.print("sat");

 //contor++;
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
