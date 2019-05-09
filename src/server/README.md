# Server

## Collector

Before storing data in to database all data is collected and analysed here.

First, for each packet, rssi readings are gathered from alla anchors and this
allows to perform localization of the device that sent that packet.   
Second, since probe requests are sent in bursts (more than one packet within
a time interval of only a few milliseconds), for each devis are gathered all 
localizations done within a second and an average is calculated. This will be
the final position of the device that will be stored in the database.

Devices are flushed into database every 5 seconds (configurable) escluded the
last second in case any other packets must be still analysed with that 
timestamp.

All (unique) packets are saved into database, only averaged devices positions
are instead saved.

Back to [main doc](../../README.md).

