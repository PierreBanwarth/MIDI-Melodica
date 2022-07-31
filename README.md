# MIDI Melodica concertina Keyboard.

the first version was made thanks to
https://www.koopinstruments.com/instrument-projects/melodicade-mx
Koop made a great work inspiring this one.

I was already working on MIDI instruments like a real concertina but there was some issue with the differential pressure sensor.

So the idea was to put all the keys on a single instrument to make a kind of melodica.

first one is built with a Arduino Mega, 36 cherry mx keys an oled screen and a rotatory encoder.

There is no multiplexing. And there is two modes (MIDI and Synth)

# Improvements

## MOZZI

the first difference with my first Concertina is the use of MOZZI wich is a great library to play moogy things.

With an output circuit :
https://sensorium.github.io/Mozzi/learn/output/

I can play directly with a headset, speaker with jack input, or directly in a DI on stage.

Issue : the synth is something like 'monophonic' you can only play one note at a time.
(not the case with the MIDI mode)

## Menu and Synth

Second improvement is clearly the menu screen and encoder are great.
Thanks to them you can choose wich waveform, octave, and volume you want.
There is 4 oscillators
2 for the main thème (30 keys)
2 for the drones (6 keys => on/off drones)

You can choose between sin, cos, square, tri and saw for each oscillators
With the synth you can apply a modifier on global octave for each oscillator. -3 -2 -1 0 1 2 3


through this menu you can choose between MIDI and Synth mode

you can choose wich octave you are playing (DMIDI and Synth)
but also you can shift you keyboard by halftone to play as concertina keyboard you want and any scale.

I have made some presets in a library that can be saved, displayed and loaded (7 presets)

# Next Goals

## Short goals :
- menus :

-- adding possibility to display scrollable menu (more entries) (more presets)
-- polyphonic synth (FIFO ?)


## V2.0
- goal is to make a new version physically.
- first one is made with a classic arduino mega.
- In order to suppress magnetic disorders we can integrate an Arduino Mego or a Teensy board directly to the cherry mx pcb.
- this made we can also add the output circuitery on the board.
- screen + encoder + output circuits on the board.

### TEENSY vs Arduino Mega.

- Teensy looks great, many entries, enough with multiplexing.
- Sound library look to work with polyphonic synth but mozzi code could be out.
- Need to check more about polyphonic mozzi synth.
