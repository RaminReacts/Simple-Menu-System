#include <Arduino.h>

#include <LiquidCrystal.h>
#include <ESP32Time.h>

const int rs = 17, en = 16, d4 = 13, d5 = 12, d6 = 14, d7 = 27;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
ESP32Time rts(0);

const int butt1 = 26;
const int butt2 = 25;

int butt1CurrentStat;
int butt1LastStat = 0;

int butt2CurrentStat;
int butt2LastStat;

unsigned long optionEnterTime = 0;
unsigned long butt1PushMoment = 0;

bool optionModeActivated;
bool isOption1 = true;

char selector1 = '>';
char selector2 = ' ';

//------------SetTime option-----------

int hour;
int minute;
int second;
int day;
int month;
int year;

unsigned long selectorLastBlink;
bool isSelectorIsBlinking = false;
bool is24HourFormat = true;
const char *timeStampList[] = {"  :%02d:%02d", "%02d:  :%02d", "%02d:%02d:  "};
int blinkPointer = 0;

enum SystemState
{
  BASE,
  MENU,
  SET_TIME,
  SET_ALARM
};

SystemState currentState = BASE;

// put your setup code here, to run once:

void runMenu()
{

  char line1[17];
  char line2[17];

  int butt1CurrentStat = digitalRead(butt1);
  int butt2CurrentStat = digitalRead(butt2);

  if (butt1CurrentStat == 1 && butt1LastStat == 0)
  {
    if (isOption1)
    {
      selector1 = '>';
      selector2 = ' ';
      isOption1 = false;
    }
    else
    {
      selector1 = ' ';
      selector2 = '>';
      isOption1 = true;
    }
  }

  if (butt2CurrentStat == 1 && butt2LastStat == 0)
  {
    if (selector1 == '>')
    {
      currentState = SET_TIME;
      lcd.clear();
      return;
    }
    else if (selector2 == '>')
    {
      currentState = SET_ALARM;
      lcd.clear();
      return;
    }
  }

  snprintf(line1, sizeof(line1), "%C1. Set Time", selector1);
  snprintf(line2, sizeof(line2), "%C2. Set Alarm", selector2);

  lcd.setCursor(2, 0);
  lcd.print(line1);
  lcd.setCursor(2, 1);
  lcd.print(line2);
}

void showClock()
{
  lcd.setCursor(2, 0);
  lcd.print(rts.getTime());
  // lcd.setCursor(11, 0);
  // lcd.print(rts.getAmPm());
}

void setTime()
{
  char line1[17];
  char blinkLine[17];
  hour = rts.getHour(is24HourFormat);
  minute = rts.getMinute();
  second = rts.getSecond();

  int pos1 = 0;
  int pos2 = 0;

  const char *timeStamp;

  switch (blinkPointer)
  {
  case (0):
    timeStamp = timeStampList[0];
    pos1 = minute;
    pos2 = second;
    break;
  case (1):
    timeStamp = timeStampList[1];
    pos1 = hour;
    pos2 = second;
    break;
  case (2):
    timeStamp = timeStampList[2];
    pos1 = hour;
    pos2 = minute;
    break;
  }

  if (butt1CurrentStat == 1 && butt1LastStat == 0)
  {
    blinkPointer++;
    if (blinkPointer > 2)
    {
      blinkPointer = 0;
    }
  }

  if (butt2CurrentStat == 1 && butt2LastStat == 0)
  {
    switch (blinkPointer)
    {
    case 0:
      hour++;
      if (hour > 23)
        hour = 0;
      rts.setTime(second, minute, hour, day, month, year); 
      break;

    case 1:
      minute++;
      if (minute > 59)
        minute = 0;
      rts.setTime(second, minute, hour, day, month, year);
      break;

    case 2:
      second++;
      if (second > 59)
        minute = 0;
      rts.setTime(second, minute, hour, day, month, year);
      break;
    }
  }

  snprintf(line1, sizeof(line1), "%02d:%02d:%02d", hour, minute, second);
  snprintf(blinkLine, sizeof(blinkLine), timeStamp, pos1, pos2);

  if (millis() - selectorLastBlink > 200)
  {
    if (isSelectorIsBlinking)
    {
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print(blinkLine);
      isSelectorIsBlinking = false;
    }
    else
    {
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print(line1);
      isSelectorIsBlinking = true;
    }
    selectorLastBlink = millis();
  }

  
}

void setAlarm()
{
  lcd.setCursor(2, 0);
  lcd.print("Set Alarm");
}

void setup()
{
  pinMode(butt1, INPUT_PULLDOWN);
  pinMode(butt2, INPUT_PULLDOWN);

  rts.setTime(0, 0, 10, 7, 7, 2026);

  day = rts.getDay();
  month = rts.getMonth();
  year = rts.getYear();

  lcd.begin(16, 2);
  lcd.print("Good day Commander!");
  lcd.clear();

  Serial.begin(115200);
}

void loop()
{
  delay(10); // this speeds up the simulation
  butt1CurrentStat = digitalRead(butt1);
  butt2CurrentStat = digitalRead(butt2);

  switch (currentState)
  {
  case (BASE):
    showClock();
    break;
  case (MENU):
    runMenu();
    break;
  case (SET_TIME):
    setTime();
    break;
  case (SET_ALARM):
    setAlarm();
    break;
  }

  if (butt1CurrentStat == 1 && butt1LastStat == 0)
  {
    butt1PushMoment = millis();
  }

  if (butt1CurrentStat == 1 && butt1LastStat == 1 && millis() - butt1PushMoment > 2000 && !optionModeActivated)
  {
    lcd.clear();
    currentState = MENU;
    optionEnterTime = millis();
    optionModeActivated = true;
  }

  if (butt1CurrentStat == 1 || butt2CurrentStat == 1 && optionModeActivated)
  {
    optionEnterTime = millis();
  }

  if (millis() - optionEnterTime > 5000 && optionModeActivated)
  {
    lcd.clear();
    currentState = BASE;
    optionModeActivated = false;
  }

  butt1LastStat = butt1CurrentStat;
  butt2LastStat = butt2CurrentStat;
}
