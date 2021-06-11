/*
 * Projekt: Overkørsel st. enkeltsporet strækning
 * Produkt: Overkørsel eksterne enheder
 * Version: 1.2
 * Type: Bibliotek
 * Programmeret af: Jan Birch
 * Opdateret: 11-06-2021
 * GNU General Public License version 3
 * This file is part of Overkørsel IO kerne.
 * 
 * "Overkørsel eksterne enheder" is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * "Overkørsel eksterne enheder" is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with "Overkørsel eksterne enheder".  If not, see <https://www.gnu.org/licenses/>.
 * 
 * Noter:
 * Se koncept og specifikation for en detaljeret beskrivelse af programmet, formål og anvendelse.
 * Version 1.1: nullptr brugt jf. standard for c++
 * Version 1.2: Tilføjet ydre enhed for vejbom
*/

#include <Arduino.h>
#include "OvkTiming.h"
#include "OvkHWDrivere.h"

#ifndef OvkDevice_h
#define OvkDevice_h

// Ansvar: Varetager fælles funktion for overkørslens ydre enheder.
// p_driver: Pointer til hardwaredriver
// state: Den ydre enheds tilstand
// setDriver(...): Kobler hardware driver til ydre enhed
// doClockCycle(...): Udfører klokcyklus for ydre enheder, som ikke har den metode
// to(...): Opdaterer den ydre enheds status
class t_CrossingDevice {
protected:
  t_DigitalOutDrv *p_driver;
  byte state;
public:
  t_CrossingDevice(byte a_state=BLOCK) : p_driver(nullptr), state(a_state) {}
  void setDriver(t_DigitalOutDrv *a_driver);
  virtual void doClockCycle(void) {}
  virtual void to(byte a_state)=0;
};

void t_CrossingDevice::setDriver(t_DigitalOutDrv *a_driver) {
  p_driver = a_driver;
  to(state);
}

//----------

// Ansvar: Varetager banesignal, der kan have udgang til gul og hvid lampe.
// p_whiteDrv: Pointer til hardware driver hvid lampe
// blinker: Abbonerer på blinker
// blinkHigh: Toggler blink
// to(...): Opdaterer den ydre enheds status
class t_RailSignal: public t_CrossingDevice {
private:
  t_DigitalOutDrv *p_whiteDrv;
  bool (*blinker)();
  bool blinkHigh;
public:
  t_RailSignal(byte a_state=BLOCK) : t_CrossingDevice(a_state), p_whiteDrv(nullptr), blinker(blinkerTriggered), blinkHigh(false) {}
  void setWhiteLamp(t_DigitalOutDrv *a_whiteDrv);
  void doClockCycle(void);
  void to(byte a_state);
};

void t_RailSignal::setWhiteLamp(t_DigitalOutDrv *a_whiteDrv) {
  p_whiteDrv = a_whiteDrv;
  to(state);
}

void t_RailSignal::doClockCycle(void) {
  if (p_whiteDrv == nullptr) return;
  if (state == PASS) {
    if (blinker() == true) {
      p_whiteDrv->write(blinkHigh);
      blinkHigh = !blinkHigh;
    }
  }
}

void t_RailSignal::to(byte a_state) {
  if (p_driver == nullptr) return;
  state = a_state;
  if (state == BLOCK) {
    p_driver->write(HIGH);
    if (p_whiteDrv != nullptr) p_whiteDrv->write(LOW);
  }
  if (state == PASS) p_driver->write(LOW);
}

//----------

// Ansvar: Varetager vejsignal
// blinker: Abbonerer på blinker
// blinkHigh: Toggler blink
// to(...): Opdaterer den ydre enheds status
class t_RoadSignal: public t_CrossingDevice {
private:
  bool (*blinker)();
  bool blinkHigh;
public:
  t_RoadSignal(byte a_state=BLOCK) : t_CrossingDevice(a_state), blinker(blinkerTriggered), blinkHigh(false) {}
  void doClockCycle(void);
  void to(byte a_state);
};

void t_RoadSignal::doClockCycle(void) {
  if (p_driver == nullptr) return;
  if (state == BLOCK) {
    if (blinker() == true) {
      p_driver->write(blinkHigh);
      blinkHigh = !blinkHigh;
    }
  }
}
  
void t_RoadSignal::to(byte a_state) {
  if (p_driver == nullptr) return;
  state = a_state;
  if (state == PASS) p_driver->write(LOW);
}

//----------

#ifdef BrugVejbom

// Ansvar: Varetager vejsignal
// doClockCycle: Klokcyklus overføres til driver
// to(...): Opdaterer den ydre enheds status
class t_Barrier: public t_CrossingDevice {
public:
  t_Barrier(byte a_state=BLOCK) : t_CrossingDevice(a_state) {}
  void doClockCycle(void) {if (p_driver == nullptr) return; p_driver->doClockCycle();}
  void to(byte a_state);
};

void t_Barrier::to(byte a_state) {
  if (p_driver == nullptr) return;
  state = a_state;
  if (state == BLOCK) p_driver->write(LOW);
  else p_driver->write(HIGH);
}

#endif
#endif
