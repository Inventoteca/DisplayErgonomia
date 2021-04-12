Esta carpeta contiene descripciones de las librerías, 
comandos utilizados para instalarlas y capturas de pantalla.  

Se ha creado un repositorio independiente para sincronizar los programas desde Node-RED.
Contiene los flujos de Node-RED y programas escritos en python.  
https://github.com/Inventoteca/DisplayErgonomiaFlows  

## Instalación de herramientas

### Node-RED
Se instaló desde la lista de "programas recomendados" que viene con Raspberry OS.  

### Probar I2C
Para conocer las direcciones y probar comunicación con los módulos ADS1115 y TSL2561  
https://flows.nodered.org/node/node-red-contrib-i2c  
![i2c scan](/Capturas/i2c scan.png)

### ADS1115
Se probaron varias librerías  

La librería de meeki007 funcionó, si embargo, no se pueden hacer muchas lecturas por segundo. 
Cuando se hacen más de 2 lecturas por segundo Node-RED se reinicia.  
https://flows.nodered.org/node/node-red-contrib-anolog-to-digital-converter-raspberry-pi  

Se podría usar un programa escrito en pyhon y la librería de Adafruit. Así obtener más lecturas por segundo.  

La de NCD no funcionó. Se debe seleccionar un valor de una lista desplegable, pero la lista está vacía. ¿Qué falta?  
https://flows.nodered.org/node/ncd-red-ads1115  

La de cinqueon no tiene documentación. No se ha probado.  
https://flows.nodered.org/node/node-red-contrib-ads1x15  

La de redplc necesita un complemento llamado `module`, pero no hay instrucciones de uso.  
https://flows.nodered.org/node/node-red-contrib-redplc-rpi-ads1x15  
https://www.npmjs.com/package/node-red-contrib-redplc-module  

### Calidad del aire, ruido y luz UV
Los sensores utilizados para medir calidad del aire, ruido y luz UV se conectan al módulo ADC ADS1115.
Se deben calibrar para mostrar las lecturas en las unidades correspondientes.
Funcionan bien con baja velicidad de muestreo, excepto el sensor de sonido.
Es mejor si se toman muchas muestras por segundo para no dejar escapar picos en la señal.

¿Cómo solucionar el problema de baja velocidad de muestreo?  
Se probará la idea escrita anteriormente: usar un programa escrito en pyhon con un ciclo infinito.  
Se usará el nodo exec en spawn mode con un comando como este  
`python -u adc.py`  
El programa imprimirá líneas con lecturas de diferentes canales del módulo ADC.  

El parámetro `-u` evita que la salida de python se almacene en un buffer 
y de esa forma pueda llegar a la salida del nodo exec.  
https://discourse.nodered.org/t/issue-exec-node/1833  

También se puede instalar el nodo daemon para que el programa de python se 
ejecute  automaticamente al iniciar node-red y se reinicie si ocurre un error.
(marcar las opciones auto-start y relaunch) 
https://flows.nodered.org/node/node-red-node-daemon  

El programa de python puede tomar varias lecturas rápidas del sensor de sonido 
y solo una lectura del sensor de luz UV y del sensor de partículas en el aire cada cierto tiempo.
Con las muestras del sensor de sonido se pueden hacer operaciones para calcular los dB.  

### TSL2561
La librería "brads-i2c-nodes" tiene un nodo para el TSL2561 y funciona muy bien.  
https://flows.nodered.org/node/node-red-contrib-brads-i2c-nodes  
https://github.com/pedalPusher68/node-red-contrib-brads-i2c-nodes  

La librería de sonnens lanza un error al instalarse  
https://flows.nodered.org/node/node-red-contrib-tsl2561  

### DHT22
La librería "contrib-dht-sensor" no funcionó.  
https://flows.nodered.org/node/node-red-contrib-dht-sensor  
La instalación es un poco complicada porque depende de otras librerías  
http://www.airspayce.com/mikem/bcm2835/  
```
curl http://www.airspayce.com/mikem/bcm2835/bcm2835-1.68.tar.gz --output bcm2835-1.68.tar.gz
tar zxvf bcm2835-1.*.tar.gz
cd bcm2835-1.*
./configure
make
sudo make check
sudo make install
```
Al ejecutar el nodo, Node-RED se reinició.  

En este tema mencionan que algunos sensores no funcionan correctamente con la librería  
https://discourse.nodered.org/t/dht22-am2302-sensor-doesnt-work-in-nodered/27513  

Es más fácil usar el sensor con python.  
Este tutorial funcionó  
https://pimylifeup.com/raspberry-pi-humidity-sensor-dht22/  

Otros tutoriales con python (no se han leído)  
http://carlini.es/sensores-de-humedad-y-temperatura-dht11-y-dth22-en-la-raspberry-pi/  
https://www.instructables.com/Raspberry-Pi-Tutorial-How-to-Use-the-DHT-22/  
https://learn.adafruit.com/dht-humidity-sensing-on-raspberry-pi-with-gdocs-logging/python-setup  
