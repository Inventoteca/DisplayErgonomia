# DisplayErgonomia
Display que muestra diferentes valores del entorno. Usa NeoDígitos y varios sensores.  

+ Una Raspberry Pi Zero W controla los Neodígitos y recibe información de los sensores  
+ Almacena los valores que se podrían mostrar en forma de gráficas  
+ Los sensores de luz se instalan en la parte de arriba, cubiertos con una ventana de acrílico  
+ Caja para proteger raspberry y conexiones  
+ "Espiroqueta" para agrupar los cables  
+ Clema molex en los grupos de NeoDígitos para facilitar las conexiones  
+ NeoDígitos atornillados a base de MDF  
+ Raspberry con shield  

## Dígitos
Ocupa 17 NeoDígitos medianos que tienen 2 Neopixels por segmento más 2 Neopixels separados  
Son 16 Neopixels por dígito. En total son 272 Neopixels en el display  

La guía de Adafruit recomienda considerar que cada Neopixel consume 20 mA  
20 mA * 272 = 5.44 A
La Raspberry Zero W puede consumir 2 A  
5.44 A + 2 A = 7.44 A
Una fuente de 10 A es la elegida  

Cada Neopixel consume alrededor de 60 mA con brillo al máximo. 60 mA * 272 = 16.32 A  
Es muy raro que se lleguen a prender con el brillo máximo    
https://learn.adafruit.com/adafruit-neopixel-uberguide/individual-neopixels  

## Sensores

### ADC
Opciones de Convertidores Analógico a Digital  

MCP3008 - 8 channels of analog input with 10-bit precision  
ADS1x15 series - 4 channels with 12 to 16-bit precision and a programmable gain stage  
PCF8591 - 4 canales ADC, 1 canal DAC, resolución 8 bits, interfaz I2C  

https://learn.adafruit.com/raspberry-pi-analog-to-digital-converters  
https://articulo.mercadolibre.com.mx/MLM-544163593-modulo-analogico-digital-3-adc-1-dac-pcf8591-arduino-pic-dsp-_JM  

Se usa el **ADS1115** que tiene resolución de 16 bits.  

Datasheet  
https://www.ti.com/lit/ds/symlink/ads1113.pdf?ts=1616968419755  

Producto de Adafruit  
https://www.adafruit.com/product/1085  
https://github.com/adafruit/Adafruit_ADS1X15  

Tutorial para usar con Arduino  
https://www.luisllamas.es/entrada-analogica-adc-de-16-bits-con-arduino-y-ads1115/  

### Índice UV

Encontramos dos opciones para medir la radiación UV: Si1145 y ML8511.  
Se eligió el ML8511 porque la salida es voltaje analógico (no se requiere una librería específica para el sensor)
y tiene un fotodiodo sensible a la luz UV, en cambio el Si1145 no mide la luz UV directamente, la calcula
a partir de la luz visible e infrarroja.  

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

Si1145  
https://learn.adafruit.com/adafruit-si1145-breakout-board-uv-ir-visible-sensor  
https://github.com/adafruit/Adafruit-Si1145-Light-Sensor-PCB  
https://cdn-shop.adafruit.com/datasheets/Si1145-46-47.pdf  
The SI1145 (from SiLabs) has a calibrated UV sensing algorithm that can calculate UV Index.
It doesn't contain an actual UV sensing element, instead it approximates it based on visible & IR light from the sun.
Works over I2C so just about any microcontroller can use it.
Has individual visible and IR sensing elements, but the library doesn't calculate the exact values of IR and Visible light.
If you need precision Lux measurement check out the TSL2561.

### Sonido
Probaremos con el módulo KY-037 tomando muestras con el ADS1115.  

Tutorial para usar con Arduino  
https://create.arduino.cc/projecthub/electropeak/how-to-use-ky-037-sound-detection-sensor-with-arduino-a757a7  

Otras opciones para medir sonido se muestran abajo. No se han probado.  

Hat con micrófono  
https://medium.com/homeday/building-a-noise-level-dashboard-for-your-office-with-a-raspberry-pi-71360ee1ff46  
https://thepihut.com/products/respeaker-4-mic-array-for-raspberry-pi  

¿Usar la entrada de micrófono o un micrófono USB?  
https://www.raspberrypi.org/forums/viewtopic.php?t=258965  

Diseño de una herramienta de medición de ruidos (con módulo micrófono DfRobot 0034)  
http://www.scielo.org.co/pdf/pml/v12n1/1909-0455-pml-12-01-00081.pdf  

### Luz (emitancia luminosa)  
Se mide en lux  
https://es.wikipedia.org/wiki/Emitancia_luminosa  

El sensor elegido es el **TSL2561**. Es fácil de usar con las librerías.  
El valor en el display se podría mostrar en kilolux, de ese modo se puede mostrar todo el rango
de medición del sensor 0.001 hasta 40.00 kilolux.  

Tutoriales (Arduino)  
https://learn.adafruit.com/tsl2561/overview  
https://hetpro-store.com/TUTORIALES/sensor-de-luz-tsl2560/  

Datasheet  
https://ams.com/TSL2561#tab/documents  

### Calidad del aire
El sensor MQ-135 se utiliza en equipos de control de calidad del aire para edificios y oficinas,
son adecuados para la detección de NH3, NOx, alcohol, benceno, humo, CO2, etc.  
Se debe calibrar.  

Tutoriales para empezar con Arduino  
https://naylampmechatronics.com/blog/42_tutorial-sensores-de-gas-mq2-mq3-mq7-y-mq135.html  
https://www.luisllamas.es/arduino-detector-gas-mq/  

Datasheet  
http://www.china-total.com/Product/meter/gas-sensor/MQ135.pdf  
http://www.china-total.com/Product/meter/gas-sensor/Gas-sensor.htm  

Librería para sensores MQ  
Podemos basarnos en las funciones de regresión  
https://github.com/miguel5612/MQSensorsLib  


## Raspberry
Los sensores serán leidos por la Raspberry y sus valores serán mostrados por los NeoDígitos, el programa funciona con Node-RED, con 
los módulos necesarios y librerias contenidas en una imagen de Docker. El control de los Dígitos está realizado por un nodo desarrollado 
por nosotros. Otro contenedor de Docker administrará el WiFi, para ser el medio de configuración del panel, através de un Dashboard.

Wifi  
Es necesario configurar la Raspberry como AP + Client al mismo tiempo, se realizó con las instrucciones siguientes, basadas en este tutorial.  
https://pifi.imti.co/  

- Con la Raspberry corriendo ya algún S.O. y conectada a Internet por cable, con SSH activado, 
se procede a desconecatar el wifi del sistema y se le pasa el control a un contenedor de Docker.
- Se ha probado con RPi Zero W, RPi 3. Se debe terminar la configuración de país del wifi antes de proceder a quitarlo del sistema.

## Diagramas
Dibujos de conexiones se encuentran en la carpeta `Diagramas`.

## Diseños
La carpeta `Diseños` contiene 

+ El modelo 3D del display
+ Piezas en STL para imprimir
+ Los dibujos para cortar las piezas de MDF
+ PCBs en formato Eagle

## Documentos
En la carpeta `Documentos` está la hoja de cotización y otra información privada.

## Fotos
En la carpeta `Fotos` hay imágenes del proceso de ensamblaje.

## Programa
La carpeta `Programa` contiene 

+ Descripciones de las librerías
+ Comandos utilizados para instalar las librerías
+ Capturas de pantalla

Se ha creado un repositorio independiente para sincronizar los programas desde Node-RED.
Contiene los flujos de Node-RED y programas escritos en python.  
https://github.com/Inventoteca/DisplayErgonomiaFlows  
