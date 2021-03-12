# DisplayErgonomia
Display que muestra diferentes valores del entorno. Usa NeoDígitos y varios sensores.

## Dígitos
Ocupa 17 NeoDígitos medianos que tienen 2 Neopixels 5050 por segmento más 2 Neopixels separados.  
Son 16 Neopixels por dígito. En total son 272 Neopixels en el display.  

Cada Neopixel consume alrededor de 60 mA con brillo al máximo. 60 mA * 272 = 16.32 A  
Es muy raro que se lleguen a prender con el brillo máximo  
La Raspberry Zero W consume 2.5 A  
Una fuente de 20 A puede alimentar todo sin problemas.  
Una fuente de 10 A no debe usarse con todos los pixels en brillo máximo. Esta es la fuente elegida.  
https://learn.adafruit.com/adafruit-neopixel-uberguide/powering-neopixels  
https://learn.adafruit.com/adafruit-neopixel-uberguide/individual-neopixels  

## Sensores

### Índice UV

Si1145  
https://learn.adafruit.com/adafruit-si1145-breakout-board-uv-ir-visible-sensor  
https://github.com/adafruit/Adafruit-Si1145-Light-Sensor-PCB  
https://cdn-shop.adafruit.com/datasheets/Si1145-46-47.pdf  
The SI1145 (from SiLabs) has a calibrated UV sensing algorithm that can calculate UV Index.
It doesn't contain an actual UV sensing element, instead it approximates it based on visible & IR light from the sun.
Works over I2C so just about any microcontroller can use it.
Has individual visible and IR sensing elements, but the library doesn't calculate the exact values of IR and Visible light.
If you need precision Lux measurement check out the TSL2561.

ML8511  
https://cdn.sparkfun.com/datasheets/Sensors/LightImaging/ML8511_3-8-13.pdf  
The ML8511 is a UV sensor, which is suitable for acquiring UV intensity indoors or outdoors. The ML8511 is
equipped with an internal amplifier, which converts photo-current to voltage depending on the UV intensity.
This unique feature offers an easy interface to external circuits such as ADC.  
Photodiode sensitive to UV-A and UV-B  
Analog voltage output  
Operating Voltage 3.3 V  
Output Voltage 1.0 - 3.0 V (depende de temperatura)  
Se debe conectar ADC a la salida  
280 a 390nm  

### Sonido
Hat con micrófono  
https://medium.com/homeday/building-a-noise-level-dashboard-for-your-office-with-a-raspberry-pi-71360ee1ff46  
https://thepihut.com/products/respeaker-4-mic-array-for-raspberry-pi  

¿Usar la entrada de micrófono o un micrófono USB?  
https://www.raspberrypi.org/forums/viewtopic.php?t=258965  

Módulo micrófono DfRobot 0034  
http://www.scielo.org.co/pdf/pml/v12n1/1909-0455-pml-12-01-00081.pdf  

Probaremos con el módulo KY-037  

### ADC
Opciones de Convertidores Analógico a Digital  

MCP3008 - 8 channels of analog input with 10-bit precision  
ADS1x15 series - 4 channels with 12 to 16-bit precision and a programmable gain stage  
PCF8591 - 4 canales ADC, 1 canal DAC, resolución 8 bits, interfaz I2C  

https://learn.adafruit.com/raspberry-pi-analog-to-digital-converters  
https://articulo.mercadolibre.com.mx/MLM-544163593-modulo-analogico-digital-3-adc-1-dac-pcf8591-arduino-pic-dsp-_JM  

### Luz (emitancia luminosa)  
Se mide en lux  
https://es.wikipedia.org/wiki/Emitancia_luminosa  


----

Modelos 3D descargados  

Caja 25 x 15 x 10 cm  
https://grabcad.com/library/lemotech-ip67-junction-box-250-mm-x-150-mm-x-100-mm-1  

Otras cajas (no descargadas)  
https://grabcad.com/library/ip65-ip67-electrical-enclosures-2/details?folder_id=4320709  

Power supply 12V 20A by T-Maker  
https://grabcad.com/library/power-supply-12v-20a-by-t-maker-1/details?folder_id=9293058  
Editada para acercarse a las medidas de la fuente comprada  



Cuenta temporal de GrabCAD  
gibap25040@grokleft.com  
asterisco  

----
