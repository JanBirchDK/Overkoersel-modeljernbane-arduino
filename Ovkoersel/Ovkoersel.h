/*
 * Projekt: Overkørsel st. enkeltsporet strækning
 * Produkt: Overkørsel kerne komponenter
 * Version: 1.0
 * Type: Bibliotek
 * Programmeret af: Jan Birch
 * Opdateret: 13-05-2021
 * GNU General Public License version 3
 * This file is part of Overkørsel kerne komponenter.
 * 
 * "Overkørsel kerne komponenter" is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * "Overkørsel kerne komponenter" is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with "Overkørsel kerne komponenter".  If not, see <https://www.gnu.org/licenses/>.
 * 
 * Noter:
 * Se koncept og specifikation for en detaljeret beskrivelse af programmet, formål og anvendelse.
 */

#include <Arduino.h>
#include "OvkTiming.h"
#include "OvkHWDrivere.h"
#include "OvkCtrl.h"
#include "OvkDevice.h"

#ifndef OvkCrossing_h
#define OvkCrossing_h

// Ansvar: Er grænseflade til tilstandsmaskine.
// clockwork: Bruges af tilstand, med tidsstyret overgang til næste tilstand.
// onEntry(...): I den konkrete tilstand indbygges opdatering af overkørslens enheder. Tilstand initialiseres.
// doCondition(...): Svarer på om betingelser for overgang til næste tilstand er opfyldt.
// onExit(...): I den konkrete tilstand indbygges opdatering af overkørslens enheder. Metoden kaldes ved afslutning af en tilstand.
class t_StateMachine {
protected:
  static t_ClockWork clockWork;
public:
  t_StateMachine(void) {}
  virtual void onEntry(void) {}
  virtual byte doCondition(byte currentStateNo) = 0;
  virtual void onExit(void) {}  
};

t_ClockWork t_StateMachine::clockWork;

//----------

// Typen af komponent kan udpeges for kontrol
enum {CTRLS, DEVICES, STATES, LASTYPE};  

// Ansvar: Opbevarer pointere til alle betjenings-, sensorenheder, ydre enheder og til alle tilstande.
// Sørger for at alle pointere er sat til ingen, så det senere kan tjekkes om pointeren er konfigureret.
// Strukturen leverer services, der kan bruges til at tjekke for at kun allokeret og konfigureret memory bliver brugt.
// isValidIndex(...): Er en service, som svarer på om den plads index peger på må bruges.
// hasConfig(...): Er en service, som svarer om der er konfigureret et objekt med den angive pointer. 
struct t_Collection {
private:
  byte maxNo[LASTYPE];
public:
  t_CrossingCtrl *ctrl[MaxNoCtrls];
  t_CrossingDevice *device[MaxNoDevices];
  t_StateMachine *state[MaxNoStates];
  void initialize(void) {
    maxNo[CTRLS] = MaxNoCtrls; maxNo[DEVICES] = MaxNoDevices; maxNo[STATES] = MaxNoStates;
    for (byte cnt=0; cnt < MaxNoCtrls; cnt++) ctrl[cnt] = nullptr;
    for (byte cnt=0; cnt < MaxNoDevices; cnt++) device[cnt] = nullptr;
    for (byte cnt=0; cnt < MaxNoStates; cnt++) state[cnt] = nullptr;  
  }
  bool isValidIndex(byte itemType, byte index) {
    return (index >= 0 && index < maxNo[itemType]);
  }
  bool hasConfig(byte itemType, byte index) {
    bool result = false;
    if (isValidIndex(itemType, index) == true) {
      if (itemType == CTRLS) result = (ctrl[index] != nullptr);
      if (itemType == DEVICES) result = (device[index] != nullptr);
      if (itemType == STATES) result = (state[index] != nullptr);    
    }
    return result;
  }
} collection;

//----------

// Ansvar: Sørger for at koble betjeningsenheder, sensorer og ydre enheder sammen. Sørger for al kommunikation mellem enheder.
// Designet gør det muligt at koble forskellige typer af komponenter sammen, så forskellige overkørsler kan modelleres.
// set...(...): Konfigurerer overkørslen.
// initState(...): Initialiserer den første tilstand, som overkørsel skal starte med.
// doClockCycle(...): Sørger for at alle overkørslens komponenter udfører polling.
// Desuden varetager metoden styring af overkørslens tilstand.
// status(...): Er en service til et tilstandsobjekt, som leverer en betjeningsenhed eller sensorenheds status.
// reset(...): Er en service til et tilstandsobjekt, som kan resette en betjeningsenhed eller sensorenhed.
// to(...): Er en service til et tilstandsobjekt, som kan sende en besked til en ydre enhed.
struct t_Crossing {
private:
  byte stateNo = 0;
  byte entryState = false;
public:
  void setCtrl(byte ctrlName, t_CrossingCtrl *ctrl) {
    if (collection.isValidIndex(CTRLS, ctrlName) == true) collection.ctrl[ctrlName] = ctrl;
  }

  void setDevice(byte deviceName, t_CrossingDevice *device) {
    if (collection.isValidIndex(DEVICES, deviceName) == true) collection.device[deviceName] = device;
  }

  void setState(byte stateName, t_StateMachine *state) {
    if (collection.isValidIndex(STATES, stateName) == true) collection.state[stateName] = state;
  }

  void initState(byte a_stateNo) {stateNo = a_stateNo; entryState = true;}

  void doClockCycle(void) {
    byte cnt;  // Loop tæller
    byte nextState;
    for (cnt=0; cnt < MaxNoCtrls; cnt++) {
      if (collection.hasConfig(CTRLS, cnt) == true) collection.ctrl[cnt]->doClockCycle();
    }
    if (collection.hasConfig(STATES, stateNo) == true) {
      if (entryState == true) {
        collection.state[stateNo]->onEntry();
        entryState = false;
      }
      nextState = collection.state[stateNo]->doCondition(stateNo);
      if (nextState != stateNo) {
        collection.state[stateNo]->onExit();
        stateNo = nextState;
        entryState = true;
      }
    }
    for (cnt=0; cnt < MaxNoDevices; cnt++) {
      if (collection.hasConfig(DEVICES, cnt) == true) collection.device[cnt]->doClockCycle();
    }
  }

  byte status(byte ctrlName) {
    byte result = OFF;
    if (collection.hasConfig(CTRLS, ctrlName) == true) result = collection.ctrl[ctrlName]->status();
    return result;
  }

  void reset(byte ctrlName) {
    if (collection.hasConfig(CTRLS, ctrlName) == true) collection.ctrl[ctrlName]->reset();  
  }
  
  void to(byte deviceName, byte deviceState) {
    if (collection.hasConfig(DEVICES, deviceName) == true) collection.device[deviceName]->to(deviceState);
  }
} crossing;

#endif
