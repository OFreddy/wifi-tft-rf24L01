# PlatformIO

## Generic boards

Default boards definitionen sind in:
C:\Users\Oliver\.platformio\platforms\espressif8266\boards

Board definition in separates Verzeichnis. Referenzierung dann in Platforio.ini

'board      = esp8266_4M2M'

In der entdprechenden Board.json findet sich der Eintrag:

'"variant": "d1_mini"'
  


Die Variante referenziert dann die Einstellungen aus:
C:\Users\Oliver\.platformio\packages\framework-arduinoespressif8266\variants

z.B. pins_arduino.h