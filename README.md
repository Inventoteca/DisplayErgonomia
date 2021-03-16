# DisplayErgonomia
Display que muestra diferentes valores del entorno. Usa NeoDígitos y varios sensores.

+ Una Raspberry Pi Zero W controla los Neodígitos y recibe información de los sensores.  
+ Almacena los valores para mostrar en forma de gráficas.  
+ Los sensores de luz se instalan en la parte de arriba, cubiertos con una ventana de acrílico.  
+ Caja impermeable para proteger raspberry y conexiones
+ "espiroqueta" para agrupar los cables
+ Clema molex en el primer NeoDígito para facilitar la conexión  
+ NeoDígitos atornillados a dos capas de MDF
+ Raspberry con shield

## Dígitos
Ocupa 17 NeoDígitos medianos que tienen 2 Neopixels por segmento más 2 Neopixels separados.  
Son 16 Neopixels por dígito. En total son 272 Neopixels en el display.  

La guía de Adafruit recomienda considerar que cada Neopixel consume 20 mA.
20 mA * 272 = 5.44 A
La Raspberry Zero W puede consumir 2 A  
Una fuente de 10 A es la elegida.  

Cada Neopixel consume alrededor de 60 mA con brillo al máximo. 60 mA * 272 = 16.32 A  
Es muy raro que se lleguen a prender con el brillo máximo    
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

ADS1115  
https://grabcad.com/library/16bit-i2c-adc-pga-ads1115-1  

TSL2561  
https://grabcad.com/library/tsl2561-breakout-board-1  

Sensores MQ  
https://grabcad.com/library/mq-2-gas-sensor-1  
https://grabcad.com/library/mq3-sensor-1  
https://grabcad.com/library/mq135-air-sensor-1  
https://grabcad.com/library/mq2-gas-sensor-1  

DHT22  
https://grabcad.com/library/dht22-am2302-1  

Sensor de sonido (micrófono)  
https://grabcad.com/library/microphone-sound-detection-sensor-module-1  

Cuenta temporal de GrabCAD  
gibap25040@grokleft.com  
asterisco  

----





## Raspberry
Los sensores serán leidos por la Raspberry y sus valores serán mostrados por los NeoDígitos, el programa funciona con Node-RED, con los módulos necesarios y librerias contenidas en una imagen de Docker. El control de los Dígitos está realizado por un nodo desarrollado por nosotros. Otro contenedor de Docker administrará el WiFi, para ser el medio de configuración del panel, através de un Dashboard.

Wifi  
Es necesario configurar la Raspberry como AP + Client al mismo tiempo, se realizó con las instrucciones siguientes, basadas en este tutorial.
https://pifi.imti.co/

- Con la Raspberry corriendo ya algún S.O. y conectada a Internet por cable, con SSH activado, se procede a desconecatar el wifi del sistema y se le pasa el control a un contenedor de Docker.
- Se ha probado con RPi Zero W, RPi 3. Se debe terminar la configuración de país del wifi antes de proceder a quitarlo del sistema.
- 
----
