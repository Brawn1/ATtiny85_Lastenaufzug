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
 * Da es sehr viel mit 433Mhz Fernbedinung gibt, habe ich zuerst den Code von der FB Ausgelesen und es in die Konditionen im Loop
 * eingebaut.
 * Zum Empfangen hat sich die Lib RC-Switch am besten bewährt.
 * 
 * Da die Relais eher Träge sind, wird zur Sicherheit zwischen dem Umschalten 150ms gewartet.
 * Es würde zwar durch die Verdrahtung nichts machen wenn beide Relais aktiv sind, aber man muss es ja nicht fordern.
 * 
 * 
 * ATtiny85:
 * ---------
 * VCC = 5V
 * GND = GND
 * PB1 = Transistor (PNP)
 * PB2 = DATA (433Mhz Empfänger)
 * PB3 = Relais 1
 * PB4 = Relais 2
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
#include <RCSwitch.h>
RCSwitch mySwitch = RCSwitch();
int RO = 1; // LOW = Versorge Relais (VCC) mit Spannung HIGH = schalte Spannung ab (PNP Transistor)
int S1 = 4; // Relais 1
int S2 = 3; // Relais 2
// da bei ATtiny85 bereits alle Pins gebraucht werden, muessen wir uns mit einem Boolean Feld zufrieden geben.
// die Boolean Felder sind fuer uns sozusagen der Ersatz von 2 zusaetzlichen Input Pins
boolean S1_State = false;
boolean S2_State = false;
boolean RO_State = false;
char mode[4]; // ein Reservierter Speicher fuer Modus ON / OFF
unsigned long relaistime = 60000L; // Schalte die Relais Versorgungsspannung nach 1min. ab.
// Wie lange das Relais den Status ON hat bis es wieder abgeschalten wird. Somit kann es
// einen bestimmten Weg zuruecklegen.
// unser Lastenaufzug schaft 6m pro Minute somit habe ich es auf 1m selbstfahren beschraenkt
unsigned long worktime = 5000;

void setup() {
  pinMode(RO, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  // zur Sicherheit die Pins sofort auf HIGH setzen da das Relais mit LOW aktiv schaltet
  digitalWrite(S1, HIGH);
  digitalWrite(S2, HIGH);
  digitalWrite(RO, HIGH); // Pin auf HIGH Setzen
  // Setze Status auf false
  S1_State = false;
  S2_State = false;
  RO_State = false; // setze auf true da es auf HIGH gesetzt wurde
  mySwitch.enableReceive(0); // 0 => Pin 2
}

void SWRelais(char mode){
  /*
   * Versorgt das Relais mit Spannung ueber einen PNP Transistor
   */
  if (mode == "ON") {
    if (RO_State == false) {
      // Schalte Versorgungspsannung am Relais EIN
      digitalWrite(RO, LOW);
      RO_State = true;
    } else if (RO_State == true) {
      // do nothing
    }
  } else {
    // Sicherheitshalber bei Aufruf das Relais ausschalten
    digitalWrite(RO, HIGH);
    RO_State = false;
  }
}

void SecRelais1(char mode){
  /*
   * Secure Schalt Logic fuer Relais 1
   */
  if (mode == "ON") {
    if (S1_State == false && S2_State == false) {
      S1_State = true;
      digitalWrite(S1, LOW);
    } else if (S1_State == false && S2_State == true) {
      digitalWrite(S2, HIGH);
      S2_State = false;
      delay(80);
      S1_State = true;
      digitalWrite(S1, LOW);
    } else if (S1_State == true && S2_State == false) {
      // do nothing
    }
  } else {
    digitalWrite(S1, HIGH);
    S1_State = false;
  }
}

void SecRelais2(char mode){
  /*
   * Secure Schalt Logic fuer Relais 2
   */
  if (mode == "ON") {
    if (S2_State == false && S1_State == false) {
      S2_State = true;
      digitalWrite(S2, LOW);
    } else if (S2_State == false && S1_State == true) {
      digitalWrite(S1, HIGH);
      S1_State = false;
      delay(80);
      S2_State = true;
      digitalWrite(S2, LOW);
    } else if (S2_State == true && S1_State == false) {
      // do nothing
    }
  } else {
    digitalWrite(S2, HIGH);
    S2_State = false;
  }
}

unsigned long task2 = 0;
void loop() {
  unsigned long currmillis = millis();
  
  // Betrieb ueber eine 433Mhz Fernbedienung mit den Kanaelen C & D
  if (mySwitch.available()) {
    int value = mySwitch.getReceivedValue();
    if ( value == 0 ) {
      //do nothing
    } else {
      unsigned long rec_value = mySwitch.getReceivedValue();
      if (rec_value == 5592145 || rec_value == 5592151 || rec_value == 5570560) {
        task2 = millis();
        SWRelais("ON");
        SecRelais1("ON");
      
      } else if (rec_value == 5592337) {
        task2 = millis();
        SWRelais("ON");
        SecRelais2("ON");
      
      } else if (rec_value == 5592148 || rec_value == 5592064 || rec_value == 5592340 || rec_value == 5592407 || rec_value == 5570560) {
        SecRelais2("OFF");
        SecRelais1("OFF");
      }
    }
    mySwitch.resetAvailable();
  }
  
  // Wenn die Zeit (worktime) kleiner als die Vergangene Zeit ist, schalte ab.
  if ((unsigned long)(currmillis - task2) >= worktime) {
    if ( S1_State == true || S2_State == true ) {
      SecRelais1("OFF");
      SecRelais2("OFF");
    }
  }

  // schalte das Relais nach eingestellter Zeit aus
  if ((unsigned long)(currmillis - task2) >= relaistime) {
    if (RO_State == true) {
      SWRelais("OFF");
    }
  }
}

