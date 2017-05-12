/*
 * Lastenaufzug Steuerung mit einem ATtiny85 und einem 433Mhz Empfangsmodul.
 * 
 * Beschreibung:
 * Die Steuerung wird für einen kleinen Lastenaufzug am Balkon verwendet und muss mit einem 433MHz Sender gesteuert werden.
 * Den Sender selber haben wir zum schalten von 3 Terrassenlampen und somit haben wir noch 2 Kanäle (C,D) für unseren Lastenaufzug.
 * 
 * Da der Microcontroller keine hohe Lasten schalten kann, Steuere ich damit 2 Wechselrelais an.
 * Die Relais werden auf der 230VAC Seite so verdrahtet, dass auch bei einem Stecken von einem Relais immer nur eines aktiv sein kann.
 * Ob das jetzt das Relais für Rauf oder Runter ist kommt von der Sicherheit darauf an. (bei mir ist es Rauf da sonst der Motor keinen
 * Abschaltpunkt hat).
 * 
 * Tipp:
 * Wenn man mehr Geld Investieren möchte, wäre ein Sicherheitsrelais (Schütz) dafür besser vorgesehen. Da es aber Privat ist und der 
 * Lastenaufzug gerade mal ca. 10x im Jahr genutzt wird, ist das nicht so schlimm. (über Winter wird es vom Netz getrennt).
 * 
 * Damit auch der Lastenaufzug vor Ort Manuell bedient werden kann, sind auch noch 2 Taster (Rauf, Runter) am Kasten montiert.
 * 
 * Da es sehr viel mit 433Mhz Fernbedinung gibt, habe ich zuerst den Code von der FB Ausgelesen und es in die Konditionen eingebaut.
 * Zum Empfangen hat sich die Lib RemoteSwitch am besten bewährt.
 * 
 * Da die Relais eher Träge sind, wird zur Sicherheit zwischen dem Umschalten 80ms gewartet.
 * Es würde zwar durch die Verdrahtung nichts machen wenn beide Relais aktiv sind, aber man muss es ja nicht herausfordern.
 * 
 * 
 * ATtiny85:
 * ---------
 * VCC = 5V
 * GND = GND
 * PB1 = Transistor (PNP)
 * PB2 = DATA (433Mhz Empfänger)
 * PB3 = Relais 1 (AB)
 * PB4 = Relais 2 (AUF)
 * 
 * Info:
 * Falls ein anderer Arduino Controller verwendet wird, muss man die Pins anpassen.
 * 
 * MIT License:
 * Copyright (c) 2016 Günter Bailey
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated 
 * documentation files (the "Software"), to deal in the Software without restriction, 
 * including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, 
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 * MIT Lizenz in Deutsch:
 * Copyright (c) 2016 Günter Bailey
 * 
 * Hiermit wird unentgeltlich jeder Person, die eine Kopie der Software und der zugehörigen Dokumentationen (die "Software") erhält, 
 * die Erlaubnis erteilt, sie uneingeschränkt zu nutzen, inklusive und ohne Ausnahme mit dem Recht, sie zu verwenden, 
 * zu kopieren, zu verändern, zusammenzufügen, zu veröffentlichen, zu verbreiten, zu unterlizenzieren und/oder zu verkaufen, 
 * und Personen, denen diese Software überlassen wird, diese Rechte zu verschaffen, unter den folgenden Bedingungen:
 * 
 * Der obige Urheberrechtsvermerk und dieser Erlaubnisvermerk sind in allen Kopien oder Teilkopien der Software beizulegen.
 * DIE SOFTWARE WIRD OHNE JEDE AUSDRÜCKLICHE ODER IMPLIZIERTE GARANTIE BEREITGESTELLT, 
 * EINSCHLIESSLICH DER GARANTIE ZUR BENUTZUNG FÜR DEN VORGESEHENEN ODER EINEM BESTIMMTEN ZWECK SOWIE JEGLICHER RECHTSVERLETZUNG, 
 * JEDOCH NICHT DARAUF BESCHRÄNKT. IN KEINEM FALL SIND DIE AUTOREN ODER COPYRIGHTINHABER FÜR JEGLICHEN SCHADEN ODER 
 * SONSTIGE ANSPRÜCHE HAFTBAR ZU MACHEN, OB INFOLGE DER ERFÜLLUNG EINES VERTRAGES, 
 * EINES DELIKTES ODER ANDERS IM ZUSAMMENHANG MIT DER SOFTWARE ODER SONSTIGER VERWENDUNG DER SOFTWARE ENTSTANDEN.
 * 
 */
#include <RemoteReceiver.h>
char version = "1.2";
int RO = PB1; // LOW = Versorge Relais (VCC) mit Spannung HIGH = schalte Spannung ab (PNP Transistor)
int S1 = PB4; // Relais 1
int S2 = PB3; // Relais 2
/*
int RO = 8;
int S1 = 9;
int S2 = 10;
*/
// da bei ATtiny85 bereits alle Pins gebraucht werden, muessen wir uns mit einem Boolean Feld zufrieden geben.
// die Boolean Felder sind fuer uns sozusagen der Ersatz von 2 zusaetzlichen Input Pins
boolean S1State=false;
boolean S2State=false;
boolean ROState=false;
// wir haben auch keinen Sensor für das erkennen in welchem Stockwerk es ist, somit werden wir auch da mit 
// Boolean Felder Raten.
boolean floor_og=false;
boolean floor_eg=false;

char mode[4]; // ein Reservierter Speicher fuer Modus ON / OFF
unsigned long relaistime = 60000L; // Schalte die Relais Versorgungsspannung nach 1min. ab.
// Wie lange das Relais den Status ON hat bis es wieder abgeschalten wird. Somit kann es
// einen bestimmten Weg zuruecklegen.
// unser Lastenaufzug schaft 6m pro Minute somit habe ich es auf 2m selbstfahren beschraenkt
unsigned long worktime = 6000L;
// vom 1.OG runter in das EG braucht es ca. 17 Sekunden.
unsigned long worktimeto_eg = 17000L;

unsigned long task2 = 0;

void setup() {
  pinMode(RO, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  // zur Sicherheit die Pins sofort auf HIGH setzen da das Relais mit LOW aktiv schaltet
  digitalWrite(S1, HIGH);
  digitalWrite(S2, HIGH);
  digitalWrite(RO, HIGH); // Pin auf HIGH Setzen
  
  RemoteReceiver::init(0, 0, 3, checkCode, 0); //RemoteReceiver::init(type, interrupt-pin, min-Repeats, callback-function, PulseWidth);

  //only for tests
  //Serial.begin(19200);
  //Serial.println("Start programm");
}

void SWRelais(char mode){
  /*
   * Powerswitch for relais modul
   */
  if (mode == "ON") {
    if (ROState == false) {
      // Schalte Versorgungspsannung am Relais EIN
      digitalWrite(RO, LOW);
      ROState=true;
    } else if (ROState == true) {
      // do nothing
    }
  } else {
    // Sicherheitshalber bei Aufruf das Relais ausschalten
    digitalWrite(RO, HIGH);
    ROState=false;
  }
  //Serial.println(ROState);
}

void SecRelais1(char mode){
  /*
   * Relais 1 with state check (DOWN)
   */
  if (mode == "ON") {
    if (S1State == false && S2State == false) {
      S1State=true;
      //liftdown=true;
      //liftup=false;
      digitalWrite(S1, LOW);
    } else if (S1State == false && S2State == true) {
      digitalWrite(S2, HIGH);
      S2State=false;
      //liftup=false;
      delay(80);
      S1State=true;
      //liftdown=true;
      digitalWrite(S1, LOW);
    } else if (S1State == true && S2State == false) {
      // do nothing
    }
  } else {
    digitalWrite(S1, HIGH);
    S1State=false;
  }
  //Serial.println(S1State);
}

void SecRelais2(char mode){
  /*
   * Relais 2 with state check (UP)
   */
  if (mode == "ON") {
    if (S2State == false && S1State == false) {
      S2State=true;
      digitalWrite(S2, LOW);
    } else if (S2State == false && S1State == true) {
      digitalWrite(S1, HIGH);
      S1State=false;
      delay(80);
      S2State=true;
      digitalWrite(S2, LOW);
    } else if (S2State == true && S1State == false) {
      // do nothing
    }
  } else {
    digitalWrite(S2, HIGH);
    S2State=false;
  }
  //Serial.println(S2State);
}

//Callback function is called only when a valid code is received.
char checkCode(unsigned long receivedCode, unsigned int period){
  //Note: interrupts are disabled. You can re-enable them if needed.

  //print received code
  //Serial.print("Code = ");
  //Serial.println(receivedCode);

  if (receivedCode == 531272) {
    task2 = millis();
    //Serial.println("Switch Relais 1 ON");
    if (floor_og == true && floor_eg == true) {
      // do nothing
    } else {
      SWRelais("ON");
      SecRelais1("ON");
    }
  
  } else if (receivedCode == 531380) {
    //Serial.println("Switch Relais 2 ON");
    task2 = millis();
    SWRelais("ON");
    SecRelais2("ON");
  
  } else if ( receivedCode == 531276 || receivedCode == 531436 || receivedCode == 531384) {
    //Serial.println("Switch Relais 1 and 2 OFF");
    SecRelais2("OFF");
    SecRelais1("OFF");
  }
}


void loop() {
  unsigned long currmillis = millis();

  // weg von der Winde zum 1.OG
  if (floor_eg == false && floor_og == false && S2State == true) {
    if ((unsigned long)(currmillis - task2) >= worktime) {
      if ( S1State == true || S2State == true ) {
        //Serial.println("Switch Relais after worktime OFF");
        SecRelais1("OFF");
        SecRelais2("OFF");
        floor_og=true;
      }
    }
    // weg vom 1.OG nach EG
  } else if (floor_eg == false && floor_og == true && S2State == true) {
    if ((unsigned long)(currmillis - task2) >= worktimeto_eg) {
      if ( S1State == true || S2State == true ) {
        //Serial.println("Switch Relais after worktime OFF");
        SecRelais1("OFF");
        SecRelais2("OFF");
        floor_eg=true;
      }
    }
    // zueruck auf das 1.OG
  } else if (floor_eg == true && floor_og == true && S1State == true) {
    if ((unsigned long)(currmillis - task2) >= worktimeto_eg) {
      if ( S1State == true || S2State == true ) {
        //Serial.println("Switch Relais after worktime OFF");
        SecRelais1("OFF");
        SecRelais2("OFF");
        floor_eg=false;
      }
    }
    // zurueck auf start
  } else if (floor_eg == false && floor_og == true && S1State == true) {
    if ((unsigned long)(currmillis - task2) >= worktime) {
      if ( S1State == true || S2State == true ) {
        //Serial.println("Switch Relais after worktime OFF");
        SecRelais1("OFF");
        SecRelais2("OFF");
        floor_og=false;
      }
    }
  } 
  
  // schalte das Relais nach eingestellter Zeit aus
  if ((unsigned long)(currmillis - task2) >= relaistime) {
    if (ROState == true) {
      //Serial.println("switch Relais OFF");
      SWRelais("OFF");
    }
  }
}

