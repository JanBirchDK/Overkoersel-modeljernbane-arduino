/*
 * Projekt: Overkørsel st. enkeltsporet strækning
 * Produkt: Overkørsel hardware drivere
 * Version: 1.0
 * Type: Bibliotek
 * Programmeret af: Jan Birch
 * Opdateret: 04-05-2021
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

#endif
