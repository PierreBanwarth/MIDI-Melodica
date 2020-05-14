# MIDI-Melodica
Goal is to create a MIDI Melodica from scratch using cherry Mx key

I have already done a MIDI concertina, goal of the melodica is to make a playable and expressive instrument wich is pretty easy to use and fit in a bag for travels


the final goal of this project is to sell some MIDI melodica to peoples who play accordion and concertina

- user need to upload his keyboard configuration to the Melodica, 
- air in and out need to be optional, some melodica use just in-way 
- One of the important thing, the melodica need to be playable even without a computer (not only MIDI)

## In term of electronic

### I think about a Arduino Mega maybe connected to a Raspberry.
There will be at least 30 Cherry Mx switches all connected to one of the arduino input. Like in the Concertina MIDI we will not use Multiplexing technic cause we need to push as input as we 

### Cherry MX Switchs.
Because cherry MX are cheap, easy to connect to Arduino Board (concertina MIDI) and cases are pretty easy to make.

usefull links :
https://shop.laserboost.com/en/create

### Pressure control
We need to use one pressure sensor outside the box and an other inside to compare values and detect pressure delta.

### Multi-Mode 
How to switch beetwen both options
using on off switch ?
detecting wich is plugged ?
#### MIDI  
for midi we need MIDI out port
#### Travel mode
for playing only with the device we need a headset plug-and play.

### Battery
We need to use battery for the travel mode.

