import time

from datetime import datetime
from paho.mqtt import client as mqtt_client

#Weather station data
TimeStamp=datetime.now()
Temp=20
WindDir='NW'
WindSpd=34.5
RainFall=0
Pressure=100.4
Humidity=10
WX_Data={'time_stamp':TimeStamp,'temp':Temp,'wind_dir':WindDir,'wind_spd':WindSpd,'rain':RainFall,'pressure':Pressure,'humidity':Humidity}

#MQTT settings
broker='10.0.0.2'
topic='/Weather/Data'

def connect_mqtt():
    def on_connect(client,userdata,flags,rc):
        if rc ==0:
            print('Connected to MQTT broker')
        else:
            print('Failed to connect to MQTT broker')
    client = mqtt_client.Client('FakeWeatherStation')
    client.on_connect=on_connect
    client.connect(broker, 1883)
    return client

def publish(client):
    while True:
        time.sleep(1)
        WX_Data['time_stamp']=datetime.now().strftime('%Y/%m/%d %H:%M:%S')
        msg=str(WX_Data)
        result=client.publish(topic,msg)
        status = result[0]
        if status == 0:
            print(f"Send '{msg}' to topic '{topic}'")
        else:
            print (f"Failed to send messageto topic {topic}")
        time.sleep(4)
        
def run():
    client=connect_mqtt()
    client.loop_start()
    publish(client)
    
if __name__ == '__main__':
    run()
