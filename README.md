# Carrell-Ino

### Descrizione

##### Autore

Filippo Maraffio

##### Descrizione

Il progetto prevede la realizzazione di un carrello elettrico automatico, in grado di spostare piccoli oggetti lungo un percorso prefissato.

Il carrell-ino partirà automaticamente quando rileverà la presenza di un oggetto nel suo cassone, e si muoverà seguendo una linea marcata precedentemente sul pavimento con del nastro isolante scuro.

Raggiunta la sua destinazione, il carrell-ino si fermerà, in attesa di essere scaricato.

Una volta che il suo cassone sarà libero, il carrell-ino tornerà al punto di partenza, pronto per essere caricato nuovamente.

##### Hardware utilizzato (provvisorio)

- Board: Arduino Uno

- 4x modulo di rilevamento linea KY-033 (con sensore a infrarossi TCRT5000)

- 1x sensore a ultrasuoni HC-SR04

- 1x cella di carico 0-5kg

- 1x moudlo ADC HX711 (per amplificare e convertire il segnale della cella di carico)

- 1x Motor driver L298N

- 2x motori CC da 6V

- 5x batterie AA da 7.5V

- 1x pulsante

- 1x diodo led

- 1x resistore da 1000 Ohm

##### Librerie utilizzate

- [GitHub - bogde/HX711: An Arduino library to interface the Avia Semiconductor HX711 24-Bit Analog-to-Digital Converter (ADC) for Weight Scales.](https://github.com/bogde/HX711)
