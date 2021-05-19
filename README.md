# Model til modeljernbaneoverkørsel - styring og overvågning
Til en modelbane bliver er bygget en model til styring og overvågning af en overkørsel:  
* For en station på en enkeltsporet strækning.  
* Som varslingsanlæg eller som halvbomsanlæg.

Model bliver bygget på breadboard og styret af en Arduino Uno. Modellen leverer.  
* Softwarepakke til Arduino.  
* Specifikation af tilslutning til hardware.  

Der er flere formål med dette udviklingsprojekt:  
* Der bliver i fællesskab med Stig og Henrik leveret en softwarepakke til styring af en overkørsel. Overkørsel bliver indbygget i Rambøll modeljernbane.  
* Jeg lærer at programmere Arduino, dens muligheder og dens begrænsninger.  
* Jeg videre udvikler min viden og kunnen i objektorienteret programudvikling.  

## I dette repository
Køreklar model til en Arduino ligger i de øvrige repositories.  

Dette repository indeholder software model, som er et bibliotek med software komponenter.  
Tilføjelse af bibliotek til Arduino IDE er beskrevet på arduino.cc.  

Til biblioteket ligger der en vejlening, i hvordan man bruger det. Desuden er der en vejledning til programmering af en konkret løsning til en overkørsel.  

## Versioner historik
* Version 1.0: Model med komponenter til styring og overvågning af overkørsel. Komponenter til vejbom venter på design og programmering.

# Think big build small
Udviklingen bliver udført trinvist fra det simple til den avancerede model.  
Model bliver oploaded i skyen til deling på Github.  
Model bliver leveret jævnligt og versionsstyret.  

# Model til styring og overvågning af overkørsel
Så vidt muligt er overkørslens funktion i overensstemmelse med SODB anlægsbestemmelser for overkørsler.  

## Afgrænsninger af model
Model har følgende afgrænsninger:
* Der er ikke intention om at udvikle en model der indeholder alle de funktioner SODB anlægsbestemmelser beskriver.
* Designet har som princip distribueret hardware og software. Der skal være 1 Arduino per overkørsel, som kun indeholder software til den ene overkørsel.
* Fejlmeldinger er udeladt. LED og servomotor leverer ikke de sensor signaler, der er behov for.
* Vejspoler er udeladt. Der er ikke nok I/O porte. Der er ikke intention om at indbygge disse i model.
Bliver det besluttet, at tilslutte overkørsel til et sikringsanlæg for modelbanens station, bør kommunikation foregår serielt, fordi det koster kun 2 digitale I/O, men kan overføre mange typer af meldinger.

## Fremtidige muligheder
Der er andre typer af overkørsler. Dem er der dog ikke aktuelt overvejelser om at udvikle moduler til.
* Overkørsel på fri bane. Enkeltstående med overkørselssignaler. En Arduino Uno kan sikkert bruges.
* Overkørsel kombineret på fri bane og station. Det kræver sandsynligvis en større Arduino.
* Koblede og tætstående overkørsler på fri bane. Det kræver sandsynligvis en større Arduino.

## Grænseflade til sikringsanlæg
Overkørslen bliver etableret som et modul, med en skarp grænseflade til sikringsanlægget.  
Overkørslen indeholder alene hardware og software som handler om overkørsel. Togvejslogik hører til i sikringsanlægget. På den måde kan der skabes en standard model til overkørsel, alternativet vil blive at hver overkørsel skal skræddersyes.  
Overkørslen magasinerer tænding, når tændsted bliver passeret af tog. Sikringsanlægget holder styr på betingelserne for tænding og sender en melding, når togvej er sat i korrekt køreretning.  
Tilsvarende holder sikringsanlægget styr på betingelserne for slukning af overkørsel og sender en melding, når togvej er opløst. Indstilling af togvej bliver kun fødet ind i overkørsel via 1 indgang.  
Overkørslen sender en melding om ”sikret” på 1 udgang. Herefter er det sikringsanlægget der bruger meldingen sammen med signallogik og sætter signal til indkørsel eller udkørsel.  
Skarp krydsning varetages af sikringsanlægget. Når den første togvej er opløst, venter sikringsanlægget i 30sekunder. Udløber tiden uden ny togvej, så sendes opløsning af togvej til overkørsel.

## Overkørsel
Overkørsel kan bestå af:
* 0, 1 eller 2 uordenssignaler.
* 0, 1 eller 2 overkørselssignaler.
* Vejlys.
* Vejklokker.
* 0, 1 eller 2 sæt vejbomme.
* Sportavle:
  * Knap for manuel tænd eller sluk.
  * Knap for simulering af tændsted.
  * Knap for simulering af togvej.
  * LED for togvej sat (tændt ved togvej).

## Tilstandsmaskine
En overkørsel gennemløber en række tilstande, fra tænding, til vej er spærret, til tog må passere, til jernbane er spærret og vej er genåbnet. Hele det forløb styres af en tilstandsmaskine.  

Tilstandsmaskine giver mulighed for at magasinere sensorsignaler og knaptryk til senere udførsel af den tilstand, der opdaterer overkørslen.  

Der er mulighed for at tilpasse en tilstand, så den fortsætter videre til 2 eller flere tilstande. Det afhænger af betingelserne for transition.  
 
## Overkørslens ydre enheder
Ydre enheder kan have blinkende lys og vejklokke kan ringe pulserende. Blink er tændt i 1 sekund og slukket i 1 sekund.  

### Uordenssignal
Et uordenssignal har 2 brandgule lanterner. Signalet giver 2 signalaspekter:
* Overkørsel er spærret for tog. Gule lanterner lyser.
* Overkørsel må passeres af tog. Gule lanterner er slukket.  
Et uordenssignal kan have 1 hvid lanterne. Den blinker hvidt lys, når overkørsel må passeres af tog.


### Overkørselssignal
Et overkørselssignal har 1 brandgul lanterne. Signalet giver 2 signalaspekter:
* Overkørsel er spærret for tog. Gul lanterne lyser.
* Overkørsel må passeres af tog. Gul lanterne er slukket.  
Et overkørselssignal kan have 1 hvid lanterne. Den blinker hvidt lys, når overkørsel må passeres af tog.

### Vejlys
Vejlys har 1 rød lanterne. Signalet giver 2 signalaspekter:
* Overkørsel er spærret for vejtrafik. Rød lanterne blinker.
* Overkørsel må passeres af vejtrafik. Rød lanterne er slukket.

### Vejklokke
Vejklokke giver 2 signalaspekter:
* Overkørsel er spærret for vejtrafik. Klokken ringer pulserende.
* Overkørsel må passeres af vejtrafik. Klokken er slukket.

### Vejbom
Vejbom giver 2 signalaspekter:
* Overkørsel er spærret for vejtrafik. Bommen er nede.
* Overkørsel må passeres af vejtrafik. Bommen er oppe.

### Lampe for togvej
En lampe for togvej hører i virkeligheden ikke til styring af overkørsel, men leveres med model, så man ved simulering kan se om der er stillet togvej.

## Overkørslens betjening og sensorer
Når overkørsel er i gang, kan den være i en tilstand, hvor der ikke er mulighed for at modtage kommando. For eksempel slukning, når bomme er på vej ned. Der skal være mulighed for at magasinere til senere. Der skal være mulighed for at resette magasin.

### Knap for manuel tænd eller sluk
Er overkørslen slukket, tænder knappen for overkørslen. Er overkørslen tændt, slukker knappen for overkørslen; men kun hvis overkørslen må slukkes. Normalt skal knappen ikke være permanent tændt og normalt skal man ikke huske at slukke, så normalt skal knappen tilbagestilles og være klar
til ny tænding. Er overkørsel i rette tilstand, kan en ny tænding magasineres til senere.

### Tændsted
Tændsted kan give 1 melding: Tog passeret i 1 retning.
Efter en passage, skal sensorenheden beholde sin melding, indtil overkørslens styring har brugt den. 

### Togvejsmelding
Enheden til togvejsmelding styres udelukkende af sikringsanlægget og giver melding til overkørslens styring. Muligvis vælges at give melding om togvejsspærring.

## Opbygning af hardware med Arduino
En Arduino’s ind- og udgange konfigureres til den konkrete overkørsel.

Prototype opstilles på breadboard.
Arduino får 1 udgang til styring. Er der behov for at styre flere vejlys, klokker og bomme, skal disse styres af effektelektronik.

