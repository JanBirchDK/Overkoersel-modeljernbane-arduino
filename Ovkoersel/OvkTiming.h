/*
 * Projekt: Overkørsel st. enkeltsporet strækning
 * Produkt: Timing bibliotek til overkørsel
 * Version: 1.0
 * Type: Bibliotek
 * Programmeret af: Jan Birch
 * Opdateret: 26-04-2021
 * GNU General Public License version 3
 * This file is part of "Timing bibliotek til overkørsel".
 * 
 * "Timing bibliotek til overkørsel" is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * "Timing bibliotek til overkørsel" is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with "Timing bibliotek til overkørsel".  If not, see <https://www.gnu.org/licenses/>.
 * 
 * Noter:
 * Se koncept og specifikation for en detaljeret beskrivelse af programmet, formål og anvendelse.
 */

#include <Arduino.h>

#ifndef OvkTiming_h
#define OvkTiming_h

// Ansvar: Klokken varetager taktslaget til polling.
// Det er en ventefunktion som sørger for synkronisering med arduino klokken
// og kompenserer for den tid det tager at gennemløbe programmet.
// ClockCycle: Sat til msek
// pendulum(...): Leverer takslaget
namespace Clock {
  const byte ClockCycle=5;
  void pendulum(void) {
    static unsigned long cycleStart=0;
    unsigned long w_millis;     // Tiden skrider hvis millis læser flere gange
    do w_millis = millis();
    while (w_millis < (cycleStart+ClockCycle));
    cycleStart = (w_millis/ClockCycle)*ClockCycle;  // Omregner til eksakt multiplum clockcykles
  }
}

//----------

// Ansvar: Urværk leverer en tidsperiode.
// Det er et tælleværk styret af polling
// Varighed duration i msek omregnes til antal cyklus
// triggered(...): Leverer sand når tiden er udløbet
class t_ClockWork {
private:
  unsigned long noCycles;
  unsigned long cycle;
public:
  t_ClockWork(void);
  t_ClockWork(unsigned long a_duration);
  void setDuration(unsigned long a_duration, bool inSeconds);
  bool triggered(void);    
};

t_ClockWork::t_ClockWork(): noCycles(1), cycle(1){};

t_ClockWork::t_ClockWork(unsigned long a_duration) {
  setDuration(a_duration, false);
}

void t_ClockWork::setDuration(unsigned long a_duration, bool inSeconds = false) {
  a_duration=a_duration*((inSeconds==true)?1000:1);
  noCycles=cycle=a_duration/Clock::ClockCycle;
}

bool t_ClockWork::triggered(void) {
  cycle--;
  if (cycle == 0) cycle = noCycles;
  return (cycle == noCycles);
}

//----------

// Ansvar: Blinker leverer tidsperiode til blink.
// Period: Sat til msek
// triggered(...): Leverer sand når tiden er udløbet
namespace Blinker {
  const unsigned int Period=1000;
  bool triggered;
  t_ClockWork ClockWork(Period);
  void doClockCycle(void) {triggered = ClockWork.triggered();}
  bool toSubscriber(void) {return triggered;}
}

// Pointer til alle abbonenter på blinker
bool (*blinkerTriggered)()=Blinker::toSubscriber;  

#endif
