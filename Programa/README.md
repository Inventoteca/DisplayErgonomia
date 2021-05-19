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

Se necesita especificar la ruta del archivo py. Si la ruta cambia se debe actualizar el nodo.
Una forma de hacerlo dinámicamente es con la siguiente librería, pero por el momento no parece necesario.  
https://flows.nodered.org/node/node-red-contrib-projectdir  
https://discourse.nodered.org/t/project-directory-path/14424/8  

Para controlar usando python se siguieron estas instrucciones  
https://learn.adafruit.com/adafruit-4-channel-adc-breakouts/python-circuitpython  

También se debe instalar Adafruit-Blinka  
```
pip3 install Adafruit-Blinka
sudo pip3 install adafruit-circuitpython-ads1x15
```
La librería puede funcionar en single mode o en continuous mode.  
La documentación no tiene mucha información extra  
https://circuitpython.readthedocs.io/projects/ads1x15/en/latest/  

Observación importante  
Se conectaron los dispositivos I2C del display,
el de luz visible y el ADC alimentado, ambos alimentados con 5V.
En esa forma el ADC no se detecta. Solo se detecta si el otro
módulo está desconectado.
El sensor de luz visible tiene un regulador a 3.3 V integrado en la placa.
El otro sensor no tiene un regulador y funciona desde 2 hasta 5 V.

Los 2 módulos funcionan de esta forma
ADS115 alimentado con 3.3 V (usando salida de las rasp)
TSL alimentado con 5 V (tiene regulador de 3.3 V integrado)

El voltaje aplicado en cualquier canal ADC no debe ser mayor al voltaje
de alimentación.
Esto trae un problema. Los sensores de sonido y de calidad del aire
pueden dar voltajes de hasta 4 V. El de calidad de aire solo da ese 
valor si los niveles de contaminación son peligrosos, es poco probable
que suceda.  
El de sonido solo da picos en ese nivel, pero podría dañar el módulo.

### TSL2561
La librería "brads-i2c-nodes" funcionó en las primeras pruebas, pero dejó de funcionar al actualizar node-red.  
https://flows.nodered.org/node/node-red-contrib-brads-i2c-nodes  
https://github.com/pedalPusher68/node-red-contrib-brads-i2c-nodes  

Se dejó de leer con node-red y todos los sensores pasaron a ser leídos con scripts de python.  
https://learn.adafruit.com/tsl2561/python-circuitpython  

La librería de sonnens lanza un error al instalarse  
https://flows.nodered.org/node/node-red-contrib-tsl2561  

### DHT22
La librería "contrib-dht-sensor" no funcionó.  
https://flows.nodered.org/node/node-red-contrib-dht-sensor  
La instalación es un poco complicada porque depende de otras librerías  
http://www.airspayce.com/mikem/bcm2835/  
Al ejecutar el nodo, Node-RED se reinició.  
Abajo se muestran los comandos utilizados  
```
curl http://www.airspayce.com/mikem/bcm2835/bcm2835-1.68.tar.gz --output bcm2835-1.68.tar.gz
tar zxvf bcm2835-1.*.tar.gz
cd bcm2835-1.*
./configure
make
sudo make check
sudo make install
```  

En este tema mencionan que algunos sensores no funcionan correctamente con la librería  
https://discourse.nodered.org/t/dht22-am2302-sensor-doesnt-work-in-nodered/27513  

Es más fácil usar el sensor con python.  
Este tutorial funcionó  
https://pimylifeup.com/raspberry-pi-humidity-sensor-dht22/  
Instalar librería  
```
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install python3-dev python3-pip
sudo python3 -m pip install --upgrade pip setuptools wheel
sudo pip3 install Adafruit_DHT
```

Otros tutoriales con python (son menos recientes)  
http://carlini.es/sensores-de-humedad-y-temperatura-dht11-y-dth22-en-la-raspberry-pi/  
https://www.instructables.com/Raspberry-Pi-Tutorial-How-to-Use-the-DHT-22/  
https://learn.adafruit.com/dht-humidity-sensing-on-raspberry-pi-with-gdocs-logging/python-setup  

### Neopixel
Se instala la librería de Node-RED  
https://flows.nodered.org/node/node-red-node-pi-neopixel  

Primer paso instalar pre-requisitos  
```curl -sS get.pimoroni.com/unicornhat | bash```  

Instalar la librería desde "manage pallete" o con este comando  
```npm install node-red-node-pi-neopixel```

Nota: solo usar un nodo para neopixel o pasan cosas raras.  

## Calibración de sensores

### Luz UV
En la oscuridad da un voltaje de 0.99 V (se midió con multímetro y ADC)  
El espectro más dañino para los humanos es de 295 a 325 nm
Máxima sensibilidad a radiación de 365 nm, pero tiene buena sensibilidad al rango más dañino.  
Se puede calcular la intensidad en mW/cm<sup>2</sup>  
Si baja la temperatura el voltaje de salida es un poco mayor. El cambio se puede despreciar.  
El datasheet indica salida de 2.2V cuando recibe luz UV con intensidad de 10 mW/cm<sup>2</sup>  
https://cdn.sparkfun.com/datasheets/Sensors/LightImaging/ML8511_3-8-13.pdf  

Aquí se menciona que es proporcional a la intensidad  
https://en.wikipedia.org/wiki/Ultraviolet_index  

Guí para el ML8511. Se explica como calcular la intensidad  
https://learn.sparkfun.com/tutorials/ml8511-uv-sensor-hookup-guide/all  
Primero obtener voltaje, luego mapear al rango de intensidad que aparece en el datasheet.  

Se obtuvo una lectura máxima de 5.5 mW/cm<sup>2</sup> con el sol en punto más alto (sombras casi verticales).
Estas páginas mostraban que el índica máximo en el día sería 13  
https://www.tutiempo.net/puebla-pue.html?datos=detallados  
https://www.accuweather.com/es/mx/puebla/245020/hourly-weather-forecast/245020  
https://www.weatheronline.mx/Mexico/Puebla/IndiceUV.htm  
En ese momento había nubes pequeñas, en algunos momentos tapaban el sol.  

Según la definición de Wikipedia el índice UV de 10 corresponde a 250 mW/m<sup>2</sup> = 0.025 mW/cm<sup>2</sup>.
Pero no se acerca al valor leído con el sensor.  
Si la intensidad era de 5.5 y el índice era cercano a 13, una forma 
rápida de calcular es multiplicar por 2 y redondear al entero más cercano.  

Este documento explica cómo aproximar el índice comparando con lecturas 
de un dispositivo profesional. Se puede usar la misma fórmula  
https://cdn.sparkfun.com/assets/learn_tutorials/2/0/6/ML8511_UV.pdf  

Otro medidor ¿cómo hace los cálculos?  
https://www.instructables.com/UltraV-a-Portable-UV-index-Meter/  

Forma compleja de calcular el índice (EPA)  
https://www.epa.gov/sunsafety/calculating-uv-index-0  
https://www.epa.gov/sites/production/files/documents/uviguide.pdf  

### Ventana transparente
Los sensores de luz UV y luz visibles están protegidos con una caja y una ventana transparente que deje pasar
mucha luz. Se probaron varios materiales para la ventana

+ Acrílico de 3 mm - Absorbe mucha radiación, la lectura de luxes se reducía 100 unidades.
+ Vidirio - Efecto similar al acrílico
+ FEP de la impresora Anycubic Photon - Casi no atenúa la lectura, pero es muy flexible
+ PLA blanco delgado - Atenúa mucho
+ Caja de plastico transparente (¿qué material es?) - Casi no atenúa, efecto similar al FEP

Parece que la atenuación es poca si el material es delgado. Se podría usar 
acetato o mica de las papelerías, pegado a un marco hecho con impresión 3D.  

### Partículas en el aire
El sensor funciona mejor alimentado con 5 V. Con menos voltaje no se calienta a la temperatura adecuada
y se reduce la sensibilidad.

Lecturas obtenidas:  
Aire limpio 0.3 V  
Saturado con gas de encendedor 4.0 V (alimentado con 3.3 V solo alcanzó 0.6 V)  
Es poco probable que se alcance ese voltaje en funcionamiento normal.
El aire estaría contaminado a niveles muy peligrosos.  

Tutoriales para calibrar sensor  
https://www.youtube.com/watch?v=qcNfXSe9CTI  
https://naylampmechatronics.com/blog/42_tutorial-sensores-de-gas-mq2-mq3-mq7-y-mq135.html  

### Sonido
La forma más fácil de representar nivel de sonido es con una gráfica de barras.
En este circuito se encienden LEDs en realación lineal al offset de voltaje  
https://www.epa.gov/sites/production/files/documents/uviguide.pdf  
Pasos mostrados: Ajustar el pot hasta que se apague el LED (umbral de silencio).
Al girar el pot cambia el voltaje de silencio.  

Si se alimenta con 5 V, el LED se apaga cuando la salida es de 2.4 V aprox.
El voltaje de salida puede ir de 0.6 a 4.7 V. El voltaje alto puede dañar el ADC.  

Si se alimenta con 3.3 V, el LED se apaga cuando la salida es 1.6 V aprox.
El voltaje de salida puede ir de 0.6 a 3.3 V.

Para evitar un voltaje alto hay varias opciones:

+ Alimentar con 3.3 V (desde la rasp o usar un regulador independiente)
+ Colocar un diodo rectificador que de una caída de 0.7 V
+ Colocar un diodo zener
+ Usar un divisor de voltaje
+ Poner un opamp en configuración seguidor, alimentado con 3.3 V

¿Cuál es más eficiente?

Este documento muestra un código para calcular decibeles, pero está mal  
http://www.scielo.org.co/pdf/pml/v12n1/1909-0455-pml-12-01-00081.pdf  
`float DB = (20 * log(10)) * (5 / voltageSensor);`  



## Notas
Medir tiempo transcurrido en Python  
https://stackoverflow.com/questions/7370801/how-to-measure-elapsed-time-in-python  

El programa `ads.py` muestra varias propiedades que se pueden configurar en la librería.  
La config por defecto es 128 SPS (samples per second), ganancia 1, modo 256 (¿qué 
significa?) y muestras de 16 bits.  
Cada lectura tarda de 13 a 16 ms, no cambia al cambiar el valor de SPS, pero no 
se probó con valor menor a 128.

Escribir superindices en markdown  
https://stackoverflow.com/questions/15155778/superscript-in-markdown-github-flavored  

Formas de ejecutar tareas periodicas en Python  
https://medium.com/greedygame-engineering/an-elegant-way-to-run-periodic-tasks-in-python-61b7c477b679  
