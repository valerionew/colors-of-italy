# DPCM-map
inspired by http://www.lucadentella.it/en/2021/01/09/impariamo-insieme-mappa-dinamica-con-arduino/


![dpcm](proof-of-concept/proof-of-concept-v.jpg)

# Roadmap
## Hardware
### Minimum viable product
- basato su esp32 (modulo wroom o modulo coi pin da amzon, sarebbe ideale predisporre il pcb per entrambi)
- controlla i led multiplexando attraverso dei 595 (se si limita ogni led, tutte le regioni dello stesso colore possono essere accese insieme)
- usb C o micro per l'alimentazione (potrebbero esserci i footprint per entrambe e due variazioni del frame con il buco in due posizioni leggermente diverse per poter scegliere al momento dell'assemblaggio)
- Controllo della luminosità rispetto alla luminosità ambientale (di notte non possiamo avere un faro negli occhi)

### Feature addizionali
- footprint del bme280 just because (target hardware V1)
- possibilità di aggiungere uno shcermo oled in basso a sinistra (target hardware V2)
- touch sensors in cima (vedi software) (target hardware V1)
- batteria?? almeno il footprint per portabatteria 18650 e caricabatteria da usb (target hardware V2)
- led bianchi sul resto della mappa (quanti ne servono per illuminazione omogenea?)

## Software
### Minimum viable product
- alla prima accensione apre un access point senza password e ti chiede le credenziali di login al tuo wifi. dopo di ché si resetta e si connette alla tua rete, salvando le credenziali. 
- ogni 10 minuti si collega a un indirizzo web e parsa i colori delle regioni
- Controllo della luminosità rispetto alla luminosità ambientale (di notte non possiamo avere un faro negli occhi)
### Feature addizionali
- pulsante touch per spegnere la luce , posto sopra (esp32 integra gpio per fare touch funxion)  (target software V2 su hardware V1)
- pulsanti touch -/0/+ per diminuire e aumentare la luminosità manualmente (dovrebbe rimappare il suo comportamento rispetto all'ambiente)  (target hardware V2)
- Altri dati: vaccini, contagi... Da concepire l'interfaccia (target software V3)
