from paho.mqtt import client as mqtt_client

#MQTT settings
broker='10.0.0.2'
topic='/Weather/Data'
client_id='Sniffer'

def subscribe(client:client_id):
    def on_message(client, userdata,msg):
        print(f"Received '{msg.payload.decode()}' from {msg.topic}' topic")
        
    client.subscribe (topic)
    client.on_message=on_message

def connect_mqtt():
    def on_connect(client,userdata,flags,rc):
        if rc ==0:
            print('Connected to MQTT broker')
        else:
            print('Failed to connect to MQTT broker')
    
    client = mqtt_client.Client(client_id)
    client.on_connect=on_connect
    client.connect(broker, 1883)
    return client

def run():
    client=connect_mqtt()
    subscribe(client)
    client.loop_forever()

if __name__ == '__main__':
    run()