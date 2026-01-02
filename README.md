# Nintendo Entertainment System Emulator
An NES emulator, written in C and SDL2.

Only compatible with mapper 0 games at the moment.
Able to boot games such as Balloon Fight and Donkey Kong.

Features a CPU tester to test against the Tom Harte tests with the appropriate JSON data.

Currently a work in progress.


## How to compile on a Linux system
``make`` 


## Libraries Needed (Ubuntu)
``sudo apt install libcjson-dev libcjson1 libsdl2-dev libsdl2-2.0-0``

## How to run
### To print help
``./nesemu -h``

### To run a game
``./nesemu -n [FILE]``


## Controls
| Keyboard      | NES Controller   |
|---------------|------------------|
| Arrow Keys    | Control Pad      |
| Z Key         | B Button         |
| X Key         | A Button         |
| Enter         | Start            |
| Shift         | Select           | 
 



## Screenshots

![Donkey Kong](https://i.imgur.com/gbqdqQ5.png)
![Bomberman](https://i.imgur.com/hcdkWeJ.png)