import pygame
import time

class ColoursLib():
    def __init__(self):
        self.rgb={'black':(0,0,0), 'green':(0,255,0), 'blue':(0,0,128), 'white':(255,255,255)}
        self.background=self.rgb['black']

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
        
def initialize(resolution):
    pass
    
#    screen=pygame.display.set_mode(size=resolution,flags=pygame.NOFRAME)

pygame.init()
window_size=(1027,768)
screen=pygame.display.set_mode(window_size)
colours=ColoursLib()

pygame.display.set_caption('Text test')
font=pygame.font.Font('freesansbold.ttf', 32)
text=font.render('Test string', True, colours.rgb['white'], colours.background)
textRect=text.get_rect()
textRect.center=(window_size[0]//2,window_size[1]//2)

while True:
    screen.fill(colours.background)
    screen.blit(text,textRect)
    for event in pygame.event.get():
        if event.type==pygame.QUIT:
            pygame.quit()
            quit()
        pygame.display.update()
