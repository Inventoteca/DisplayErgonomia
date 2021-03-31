Esta carpeta contiene el flujo de Node-RED para leer los sensores y 
programas de python que se ejecutan desde Node-RED.  

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

La de NCD no funcionó. Se debe seleccionar un valor de una lista desplegable, pero la lista está vacía. ¿Qué falta?  
https://flows.nodered.org/node/ncd-red-ads1115  

La de cinqueon no tiene documentación. No se ha probado.  
https://flows.nodered.org/node/node-red-contrib-ads1x15  

La de redplc necesita un complemento llamado `module`, pero no hay instrucciones de uso.  
https://flows.nodered.org/node/node-red-contrib-redplc-rpi-ads1x15  
https://www.npmjs.com/package/node-red-contrib-redplc-module  

