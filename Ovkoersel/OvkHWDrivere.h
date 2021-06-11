/*
 * Projekt: Overkørsel st. enkeltsporet strækning
 * Produkt: Overkørsel hardware drivere
 * Version: 1.1
 * Type: Bibliotek
 * Programmeret af: Jan Birch
 * Opdateret: 31-05-2021
 * GNU General Public License version 3
 * This file is part of Overkørsel IO kerne.
 * 
 * "Overkørsel hardware drivere" is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * "Overkørsel hardware drivere" is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with "Overkørsel hardware drivere".  If not, see <https://www.gnu.org/licenses/>.
 * 
 * Noter:
 * Se koncept og specifikation for en detaljeret beskrivelse af programmet, formål og anvendelse.
 * Version 1.1: Tilføjet driver til servomotor
 */

#include <Arduino.h>
#include "OvkTiming.h"

#ifndef OvkHWDrivere_h
#define OvkHWDrivere_h

// Ansvar: Er grænseflade til input hardware drivere.
// value: Input værdi
// read(...): Udlæser knappens værdi
// doClockCycle(...): Gennemløber en klokcyklus
class t_DigitalInDrv {
protected:
  bool value;
public:
  t_DigitalInDrv(void) {}
  virtual void doClockCycle(void)=0;
  bool read(void) const {return value;}   
};

//----------

// Ansvar: Denne klasse varetager al funktion af en trykknap. Software er et spejl af hardwarefunktion.
// Indlæsning fra parallel hardware port. Filtrering af kontaktprel. Grænseflade til software.
// Seqs: En knap løber igennem 2 trin, når der trykkes på den
// Udløbstid for timer til kontaktprel bliver sat til 30msek
// pin: Arduino portnr
// bounceTimer: Timer til kontaktprel
// value: Knappen er høj eller lav
// seq: Knappens trin
// doClockCycle(...): Gennemløb på tid
class t_PushButton: public t_DigitalInDrv {
private:
  enum {STABLE, BOUNCE};
  enum {BOUNCTIME = 30};
  byte pin;
  t_ClockWork bounceWait;
  byte seq;
public:
  t_PushButton(byte a_pin, byte a_contact);
  void doClockCycle(void);
};

t_PushButton::t_PushButton(byte a_pin, byte a_contact) : t_DigitalInDrv(), pin(a_pin), bounceWait(BOUNCTIME), seq(STABLE) {
  if (a_contact == NCLOSED) pinMode(pin, INPUT_PULLUP);
  else pinMode(pin, INPUT);
  value = digitalRead(pin);
}

void t_PushButton::doClockCycle(void){
  switch (seq) {
    case STABLE:
      if (value != digitalRead(pin)) {
        bounceWait.setDuration(BOUNCTIME);
        seq = BOUNCE;
      }
    break;
    case BOUNCE:
     if (bounceWait.triggered() == true) {
        value = digitalRead(pin);
        seq = STABLE;
      }
    break;
  }
}

//----------

// Ansvar: Er grænseflade til output hardware drivere.
// value: Output værdi
// write(...): Indlæser værdi. Sørger for kun at opdatere arduino port ved behov
// sendOut(...): Sender værdi til aktuel driver
class t_DigitalOutDrv {
protected:
  bool value;
  virtual void sendOut(void)=0;
public:
  t_DigitalOutDrv(bool a_value = LOW) : value(a_value) {}
  virtual void doClockCycle(void) {};
  void write(bool a_value);
};

void t_DigitalOutDrv::write(bool a_value) {
  if (a_value != value) {
    value = a_value;
    sendOut();
  }
}

//----------

// Ansvar: Denne klasse varetager al funktion af simpel tændt eller slukket udgang. Software er et spejl af hardwarefunktion.
// Udlæsning til parallel hardware port. Grænseflade til software.
// pin: Arduino portnr
// sendOut(...): Sender værdi til port
class t_SimpleOnOff: public t_DigitalOutDrv {
private:
  byte pin;
  void sendOut(void) {digitalWrite(pin, value);}
public:
  t_SimpleOnOff(byte a_pin, bool a_value);
};

t_SimpleOnOff::t_SimpleOnOff(byte a_pin, bool a_value = LOW) : t_DigitalOutDrv(a_value), pin(a_pin) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, a_value);
}

//----------

#ifdef BrugVejbom
#include <Servo.h>

// Opsætning af maks specifikationer til servomotor
const struct {
  int PulseWidthMin = 544;
  int PulseWidthMax = 2400;
  int AngleMin = 0;
  int AngleMax = 180;
  int AnglePmsek = 20;
  int AngleDiff = 90;
} PWMLimits;

// Ansvar: Denne klasse varetager al funktion til styring af en servomotor.
// Udlæsning til pulsbreddemoduleret hardware port. Grænseflade til software.
// Seqs: Et bomdrev løber igennem 3 trin, når det går op eller ned
// seq: Bomdrevets trin
// servoPort: Portobjekt
// minPulseWidth: Konfigureret minimum pulsbredde
// maxPulseWidth: Konfigureret maksimum pulsbredde
// timeAngle: Timer til bombevægelse
// anglePmsek: Antal msek per grad bombevægelse
// upAngle: Vinkel når bomdrev er i oppe
// downAngle: Vinkel når bomdrev er i nede
// currentAngle: Vinkel på et tidspunkt
// startMotor(...): Sætter PWM variable indenfor grænser og kobler motor til port
// startMotor variant til konfiguration af alle motorparametre
// doClockCycle(...): Gennemløb på tid
// sendOut(...): Sender værdi til port
// setAngleAdjust(...): Sætter justeringsvinkel og tjekker om max grænser overholdes. Sætter arm i startposition
// setBarrierTime(...): Sætter tid for bevægelse fra yderstilling til yderstilling
// setPWtime(...): Sætter grænser for pulsbredde
class t_ServoMotor: public t_DigitalOutDrv {
private:
  enum {STABLE, GOUP, GODOWN};
  byte seq;
  Servo servoPort;
  int minPulseWidth;
  int maxPulseWidth;
  t_ClockWork timeAngle;
  int anglePmsek;
  int upAngle;
  int downAngle;
  int currentAngle;
  void sendOut(void);
  bool setAngleAdjust(int a_upAngle, int a_angleDiff);
  bool setBarrierTime(unsigned long barrierTime);
  bool setPWtime(int minPWt,int maxPWt);
public:  
  t_ServoMotor(bool a_value=LOW): t_DigitalOutDrv(a_value), seq(STABLE) {}
  void startMotor(byte pin, int angleAdjust, unsigned long barrierTime) {
    startMotor(pin, angleAdjust, PWMLimits.AngleDiff, barrierTime, PWMLimits.PulseWidthMin, PWMLimits.PulseWidthMax);}
  void startMotor(byte pin, int angleAdjust, int angleDiff, unsigned long barrierTime, int minPWt, int maxPWt);
  void doClockCycle(void);
};

void t_ServoMotor::sendOut(void) {
  int currentPW;
  if (servoPort.attached() == true) {
    currentPW = map(currentAngle, PWMLimits.AngleMin, PWMLimits.AngleMax, minPulseWidth, maxPulseWidth);
    servoPort.writeMicroseconds(currentPW);
  }
}

bool t_ServoMotor::setAngleAdjust(int a_upAngle, int a_angleDiff) {
  bool isValid = false;
  int angleDiff, maxUpAngle;
  angleDiff = constrain(a_angleDiff, PWMLimits.AngleMin, PWMLimits.AngleMax);
  isValid = (angleDiff == a_angleDiff);
  maxUpAngle = PWMLimits.AngleMax-angleDiff;
  upAngle = constrain(a_upAngle, PWMLimits.AngleMin, maxUpAngle);
  isValid = isValid && (upAngle == a_upAngle);
  downAngle = upAngle+angleDiff;
  currentAngle = (value == HIGH)?upAngle:downAngle;
  return isValid;
}

bool t_ServoMotor::setBarrierTime(unsigned long barrierTime) {
  bool isValid = false;
  anglePmsek = (downAngle > upAngle)?barrierTime/(downAngle-upAngle):PWMLimits.AnglePmsek;
  isValid = (anglePmsek >= PWMLimits.AnglePmsek);
  timeAngle.setDuration(anglePmsek);
  return isValid;
}

bool t_ServoMotor::setPWtime(int minPWt, int maxPWt) {
  bool isValid = false;
  minPulseWidth = constrain(minPWt, PWMLimits.PulseWidthMin, PWMLimits.PulseWidthMax);
  isValid = (minPulseWidth == minPWt);
  maxPulseWidth = constrain(maxPWt, PWMLimits.PulseWidthMin, PWMLimits.PulseWidthMax);
  isValid = isValid && (maxPulseWidth == maxPWt);
  isValid = isValid && (minPulseWidth < maxPulseWidth);
  return isValid;
}

void t_ServoMotor::startMotor(byte pin, int angleAdjust, int angleDiff, unsigned long barrierTime, int minPWt, int maxPWt) {
  bool allowStart = setAngleAdjust(angleAdjust, angleDiff);
  allowStart = allowStart && setBarrierTime(barrierTime);
  allowStart = allowStart && setPWtime(minPWt, maxPWt);
  if (allowStart == true) {
    servoPort.attach(pin);
    sendOut();  
  }
}

void t_ServoMotor::doClockCycle(void){
  switch (seq) {
    case STABLE:
      if ((value == HIGH) && (currentAngle == downAngle)) seq = GOUP;
      if ((value == LOW) && (currentAngle == upAngle)) seq = GODOWN;
    break;
    case GOUP:
      if (currentAngle > upAngle) {
        if (timeAngle.triggered() == true) {
          currentAngle--;
          sendOut();
        }       
      }
      else seq = STABLE;      
    break;
    case GODOWN:
      if (currentAngle < downAngle) {
        if (timeAngle.triggered() == true) {
          currentAngle++;
          sendOut();
        }
      }
      else seq = STABLE;
    break;
  }
}
  
#endif
#endif
