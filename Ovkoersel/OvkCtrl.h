/*
 * Projekt: Overkørsel st. enkeltsporet strækning
 * Produkt: Overkørsel betjenings enheder
 * Version: 1.1
 * Type: Bibliotek
 * Programmeret af: Jan Birch
 * Opdateret: 17-05-2021
 * GNU General Public License version 3
 * This file is part of Overkørsel IO kerne.
 * 
 * "Overkørsel betjenings enheder" is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * "Overkørsel betjenings enheder" is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with "Overkørsel betjenings enheder".  If not, see <https://www.gnu.org/licenses/>.
 * 
 * Noter:
 * Se koncept og specifikation for en detaljeret beskrivelse af programmet, formål og anvendelse.
 * Version 1.1: nullptr brugt jf. standard for c++
 */

#include <Arduino.h>
#include "OvkHWDrivere.h"

#ifndef OvkCtrl_h
#define OvkCtrl_h

// Ansvar: Varetager magasinering for betjenings- og sensorenheder.
// En flipflop konfigureres til hardware driver normally open eller normally closed.
// Indgangen bliver trigget med flanke op eller flanke ned.
// Der er udgang for bistabil og for oneshot.
// FlankType: Flanketype
// flankType: flanketype variabel
// prevInput: Værdi indlæst fra sidste gang
// valueBistable: Udgangens bistabile værdi
// valueOneShot: Udgangens oneshot værdi
// update(...): Indlæser værdi 
// reset(...): Resætter flipflop
// bistable(...): Udlæser bistabil værdi
// oneshot(...): Udlæser oneshot værdi
class t_FlipFlop {
private:
  enum {DOWN, UP};
  unsigned int flankType : 1;
  unsigned int prevInput : 1;
  unsigned int valueBistable : 1;
  unsigned int valueOneShot : 1;
public:
  t_FlipFlop(byte ContacType);
  void update(bool currentInput);
  void reset(void) {valueBistable = valueOneShot = OFF;}
  bool bistable(void) const {return valueBistable;}
  bool oneshot(void) const {return valueOneShot;}
};

t_FlipFlop::t_FlipFlop(byte ContacType): valueBistable(OFF), valueOneShot(OFF) {
  flankType = (ContacType==NOPEN)? UP:DOWN;
  prevInput = (ContacType==NCLOSED) ? HIGH:LOW;
}

void t_FlipFlop::update(bool currentInput) {
  if (prevInput == currentInput) return;
  if ((flankType == UP) && (currentInput == HIGH)) valueBistable = !valueBistable;
  if ((flankType == DOWN) && (currentInput == LOW)) valueBistable = !valueBistable;
  if (valueBistable == ON) valueOneShot = ON;
  prevInput = currentInput;
}

//----------

// Ansvar: Varetager betjenings- og sensorenheder.
// Input fra driver bliver koblet til tilstandsmaskine og til magasin.
// p_driver: Pointer til input driver
// p_flipflop: Pointer til magasin (flipflop)
// flipflopType: Brug enten bistabil eller oneshot
// setDriver(...): Kobler til input driver
// setFlipFlop(...): Kobler til magasin (flipflop)
// doClockCyckle(...): Udfører polling og overfører værdi fra input til magasin
// status(...): Leverer kontroludgangens værdi
// reset(...): Styrer reset af magasin
class t_CrossingCtrl {
private:
  t_DigitalInDrv *p_driver;
  t_FlipFlop *p_flipflop;
  byte flipflopType;
public:
  t_CrossingCtrl(void): p_driver(nullptr), p_flipflop(nullptr) {}
  void setDriver(t_DigitalInDrv *a_driver) {p_driver = a_driver;}
  void setFlipFlop(t_FlipFlop *a_flipflop, byte a_flipflopType=BISTABLE) {p_flipflop=a_flipflop; flipflopType=a_flipflopType;}
  void doClockCycle(void);
  byte status(void) const;
  void reset(void) const {if (p_flipflop != nullptr) p_flipflop->reset();}  
};

void t_CrossingCtrl::doClockCycle(void) {
  if (p_driver != nullptr) p_driver->doClockCycle();
  if (p_flipflop != nullptr) p_flipflop->update(p_driver->read());
}

byte t_CrossingCtrl::status(void) const {
  byte outState = OFF;
  if (p_driver != nullptr) {
    if (p_flipflop == nullptr) outState = p_driver->read();
    else {
      if (flipflopType == BISTABLE) outState = p_flipflop->bistable();
      else outState = p_flipflop->oneshot();
    }   
  }
  return outState;
}

#endif
