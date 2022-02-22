import time
import ast
import threading
import pygame
from datetime import datetime

from paho.mqtt import client as mqtt_client

#MQTT settings
broker='10.0.0.2'
topic='/Weather/Data'
client_id='WeatherStation'

#Display Settings
default_background=(0,0,0)
default_font=('freesansbold.ttf', 24)
resolution=(1024,768)

class ColoursLib():
    def __init__(self):
        self.rgb={'black':(0,0,0), 'white':(255,255,255),'red':(255,0,0), 'orange':(255,100,0), 'yellow':(255,255,0), 'green':(0,255,0), 'blue':(0,0,255)}

    def __str__(self):
        string=''
        for key,value in self.__dict__.items():
            string=string+'\n'+str(key)+' = '+str(value)
        return string

    def add_from_rgb(self,name,hexnum):
        string=['0x{}'.format(hexnum[:2]), '0x{}'.format(hexnum[2:4]), '0x{}'.format(hexnum[4:])]
        for x in range(3):
            string[x]=int(string[x], 16)
        self.rgb[name]=tuple(string)

class Weather:
    def __init__(self):
        self.time_stamp={'day':'None','date':'None','time':'None'}
        self.temp='None'
        self.wind_dir='None'
        self.wind_spd='None'
        self.rain='None'
        self.pressure='None'
        self.humidity='None'

    def __str__(self):
        string=''
        for key,value in self.__dict__.items():
            if key == 'time_stamp':
                string=string+'time_stamp'+'='+self.time_stamp['day']+', '+self.time_stamp['date']+' '+self.time_stamp['time']
            else:
                string=string+'\n'+str(key)+' = '+str(value)
        return string

    def update(self,msg):
        if isinstance(msg,dict) == True:
            for key, value in msg:
                self.Timestamp = 'New'
        else:
#            print('Old weather report:\n{}\n'.format(CurrentWeather))
#             print('Raw MQTT payload: {}'.format(msg))
            dictmsg=ast.literal_eval(msg)
#            print(str(dictmsg))
            daytime=datetime.strptime(dictmsg['time_stamp'], '%Y/%m/%d %H:%M:%S')
            day=daytime.strftime('%a')
            date=daytime.strftime('%b %d')
            time=daytime.strftime('%H:%M')
            self.time_stamp={'day':day,'date':date,'time':time}
            self.temp=dictmsg['temp']
            self.wind_dir=dictmsg['wind_dir']
            self.wind_spd=dictmsg['wind_spd']
            self.rain=dictmsg['rain']
            self.pressure=dictmsg['pressure']
            self.humidity=dictmsg['humidity']
            print('New weather report:\n{}\n'.format(CurrentWeather))
            return

class TextBlock():
    def __init__(self,txt,txtlocation,txtcolour,align,bgcolour=default_background,font=default_font):
        self.font=pygame.font.Font(font[0],font[1])
        self.text=self.font.render(txt, True, txtcolour, bgcolour)
        self.textRect=self.text.get_rect()
        if align == 'left':
#             print('Aligning to the left')
            self.textRect.midleft=(txtlocation[0],txtlocation[1])
        elif align == 'center':
            self.textRect.center=(txtlocation[0],txtlocation[1])
        elif align == 'right':
#             print('Aligning to the right')
            self.textRect.midright=(txtlocation[0],txtlocation[1])

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

def subscribe(client,topic):
    def on_message(client,userdata,msg):
        payload = msg.payload.decode()
#        print(f"Received '{payload}' from {msg.topic}' topic.\nParsing...\n\n")
        CurrentWeather.update(payload)
    client.subscribe (topic)
#    print("Subscribed to topic'{}'".format(topic))
    client.on_message=on_message

#Initialize Objects
CurrentWeather=Weather()

#Initialize MQTT
client=connect_mqtt()
# time.sleep(1)
subscribe(client,topic)

#Initialize Display
pygame.init()
window_size=(resolution)
screen=pygame.display.set_mode(window_size)
#screen=pygame.display.set_mode(window_size,pygame.NOFRAME)
pygame.display.set_caption('Weather Display')
colours=ColoursLib()

#Main Script

MQTTsub_daemon=threading.Thread(target=client.loop_forever,args=(1,),daemon=True)
MQTTsub_daemon.start()

while True:
    line=2*default_font[1]
    linespace=2*default_font[1]
    align=50
    screen.fill(default_background)
    
    title=str('Weather report as of {}, {} at {}'.format(CurrentWeather.time_stamp['day'],CurrentWeather.time_stamp['date'],CurrentWeather.time_stamp['time']))
    length=len(title)
    title=TextBlock(title,(window_size[0]//2,line),colours.rgb['white'],'center')
    screen.blit(title.text,title.textRect)
    
    line=line+2*linespace
    temp='Temperature: {}\N{DEGREE SIGN}C'.format(str(CurrentWeather.temp))
    temp=TextBlock(temp,(align,line),colours.rgb['white'],'left')
    screen.blit(temp.text,temp.textRect)
    
    line=line+linespace
    wind='Wind is out of the {} at {}km/h'.format(str(CurrentWeather.wind_dir),str(CurrentWeather.wind_spd))
    wind=TextBlock(wind,(align,line),colours.rgb['white'],'left')
    screen.blit(wind.text,wind.textRect)
    
    line=line+linespace
    rain='{}mm of rain has fallen in the last hour'.format(str(CurrentWeather.rain))
    rain=TextBlock(rain,(align,line),colours.rgb['white'],'left')
    screen.blit(rain.text,rain.textRect)
    
    line=line+linespace
    pressure='Barometric Pressure is sitting at {}kpa and holding steady'.format(str(CurrentWeather.pressure))
    pressure=TextBlock(pressure,(align,line),colours.rgb['white'],'left')
    screen.blit(pressure.text,pressure.textRect)
    
    line=line+linespace
    humidity='(Humidity is at {}%'.format(str(CurrentWeather.humidity))
    humidity=TextBlock(humidity,(align,line),colours.rgb['white'],'left')
    screen.blit(humidity.text,humidity.textRect)
    
    for event in pygame.event.get():
        if event.type==pygame.QUIT:
            pygame.quit()
            quit()
        pygame.display.update()
        time.sleep(1)