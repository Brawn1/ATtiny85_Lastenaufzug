# RF AtTiny85 gesteuerter Lastenaufzug

**Anleitung in Arbeit**

Da wir gerne Grillen aber die Sachen vom 1.OG jedesmal hinunter und 
wieder rauftragen müssen, haben wir uns entschlossen 
eine Seilwinde die noch in der Garage lag unter dem Balkon im 2.OG zu 
montieren.

Die Montage war einfach, aber leider stellte sich heraus das die 
Seilwinde sehr Langsam ist, und man die nur mit einem 2m Kabel vom 1.OG 
bedienen konnte.

Also habe ich mir überlegt dies per Microcontroller über Funk zu 
Steuern.

Da der Microcontroller keine hohe Lasten schalten kann, Steuere ich 
damit 2 Wechselrelais an.
Die Relais werden auf der 230VAC Seite so verdrahtet, dass auch bei 
einem Stecken von einem Relais immer nur eines aktiv sein kann.
Ob das jetzt das Relais für Rauf oder Runter ist kommt von der 
Sicherheit darauf an (bei mir ist es Rauf da sonst der Motor keinen
Abschaltpunkt hat).

**Tipp:**
Wenn man mehr Geld Investieren möchte, wäre ein Sicherheitsrelais 
(Schütz) dafür besser vorgesehen. Da es aber Privat ist und der 
Lastenaufzug gerade mal ca. 10x im Jahr genutzt wird, ist das nicht so 
schlimm. (über Winter wird es vom Netz getrennt).

Damit auch der Lastenaufzug vor Ort Manuell bedient werden kann, sind 
auch noch 2 Taster (Rauf, Runter) am Kasten montiert.

Da es sehr viel mit 433Mhz Fernbedinung gibt, habe ich zuerst den Code 
von der FB Ausgelesen und es in die Konditionen im Loop eingebaut.
Zum Empfangen hat sich die Lib RC-Switch am besten bewährt.

Da die Relais eher Träge sind, wird zur Sicherheit zwischen dem 
Umschalten 80ms gewartet.
Es würde zwar durch die Verdrahtung nichts machen wenn beide Relais 
aktiv sind, aber man muss es ja nicht fordern.


## Benötigte Teile

* 1x 433Mhz Empfänger (E-Bay um ca. 2,50 € (Sender & Empfänger)
* 1x AtTiny85 (für die Steuerung)
* 1x Transistor PNP
* 1x 800 Ohm Widerstand
* 1x 2-Fach Relais (Bei E-Bay um ca. 5 €) *Achtung: ich habe aus 
versehen die Relais mit Optokopler gekauft, somit muss ich zum 
Abschalten der Relais die Ports auf HIGH setzen.*


## Verdrahtung

Die Verdrahtung ist dank dem AtTiny85 sehr einfach.

![Verdrahtung AtTiny85](Doku/Schaltplan.jpg)
Wie man sieht verwende ich einen Widerstand von ca. 800 Ohm für den 
Transistor, somit habe ich für die Relais genau 5,5VDC.

So als Startpunkt sollte man einen 1KOhm Widerstand verwenden und es mit 
einem Multimeter nachmessen.

---

**Die Pins vom AtTiny85 werden wie dargestellt verwendet:**

---

* *PB1* -> aktivierung Spannungsversorgung vom Relais über PNP Transistor
* *PB2* -> *DATA* RF-Empfänger
* *PB3* -> *IN1* Relais 1
* *PB4* -> *IN2* Relais 2
* *VIN* -> max. 5,5 VDC
* *GND* -> GND

**433 Mhz Empfänger Aufgelötet**
![433Mhz Empfänger Aufgelötet](Doku/DSC_1584.JPG)

**2-Fach Relais**

**Info:** Falls eines gekauft werden muss, würde ich diese mit den 
Optokoplern nicht mehr für so ein Projekt empfehlen. 
Da nur mit *HIGH* das Relais abgeschaltet wird.
![2-Fach Relais](Doku/DSC_1585.JPG)

**Schaltung Verdrahtet und an das Relais angeschlossen**
![Schaltung Verdrahtet](Doku/DSC_1586.JPG)


## Erklärung Sketch


```Arduino
int RO = 1; // LOW = Versorge Relais (VCC) mit Spannung HIGH = schalte Spannung ab (PNP Transistor)
int S1 = 4; // Relais 1
int S2 = 3; // Relais 2
```

Da bei ATtiny85 bereits alle Pins gebraucht werden, muessen wir uns mit 
einem Boolean Feld zufrieden geben.
Die Boolean Felder sind fuer uns sozusagen der Ersatz von 2 
zusaetzlichen Input Pins.
```Arduino
boolean S1_State = false;
boolean S2_State = false;
boolean RO_State = false;
```

Schalte die Relais Versorgungsspannung nach 1min. ab.
```Arduino
unsigned long relaistime = 60000L;
```

Wie lange das Relais den Status ON hat bis es wieder abgeschalten wird. 
Somit kann es einen bestimmten Weg zuruecklegen.
Unser Lastenaufzug schaft 6m pro Minute somit habe ich es auf 1m 
selbstfahren beschraenkt 
```Arduino
unsigned long worktime = 5000;
```

Zur Sicherheit die Pins sofort auf HIGH setzen da das Relais mit LOW 
aktiv schaltet (Sozusagen schalten wir das Relais gleich ab).
```Arduino
void setup() {
  ...
  digitalWrite(S1, HIGH);
  digitalWrite(S2, HIGH);
  digitalWrite(RO, HIGH);
  ...
  mySwitch.enableReceive(0); // 0 => Pin 2
}
```

Aktiviere den Empfang auf den Interrupt *Pin0* => ist bei Arduino Pin2.
```Arduino
void setup() {
  ...
  ...
  mySwitch.enableReceive(0); // 0 => Pin 2
}
```

## AtTinyX5 Pins und Datenblatt

Eine kurze Darstellung der Pins vom AtTinyX5 Serie

![Darstellung der Pins](Doku/attiny85_pinout.jpg)

Das Offizielle Datenblatt von Atmel

[PDF Datasheet AtTiny25 - 85](Doku/Atmel-2586-AVR-8-bit-Microcontroller-ATtiny25-ATtiny45-ATtiny85_Datasheet.pdf)

## Quellen

* [RC-Switch](https://github.com/sui77/rc-switch)
* [Atmel AtTiny85](http://www.atmel.com/devices/attiny85.aspx)
