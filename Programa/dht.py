import Adafruit_DHT

DHT_SENSOR = Adafruit_DHT.DHT22
DHT_PIN = 4

humidity, temperature = Adafruit_DHT.read_retry(DHT_SENSOR, DHT_PIN)

if humidity is not None and temperature is not None:
    print('{"t":' + str(temperature) + ',"h":' + str(humidity) + '}')
    #print("Temp={0:0.1f}*C  Humidity={1:0.1f}%".format(temperature, humidity))
    #msg.payload.t = temperature
    #msg.payload.h = humidity
else:
    print('{"t":0, "h":0}')
    #print("Failed to retrieve data from humidity sensor")
    #msg.payload = "Failed"

#return msg
