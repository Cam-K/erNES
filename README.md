# erNES
A Nintendo Entertainment System emulator, written in C.

<p align="center">
  <img src="https://i.imgur.com/RoONmfS.png" alt="Super Mario Bros">
  <img src="https://i.imgur.com/qPbXxxN.png" alt="Castlevania">
  <img src="https://i.imgur.com/0TF6qSS.png" alt="Zelda">
</p>

## Description

A NES emulator using SDL2 as it's graphics library that is able to boot games such as Mega Man 2 and Super Mario Bros.

There is no APU imeplementation.

Currently a work in progress.

### Mapper Support:
- Mapper 0 (NROM)
- Mapper 1 (MMC1) (Partial Support) 
- Mapper 2 (UxROM)
- Mapper 3 (CNROM)
- Mapper 7 (AxROM)


## How to compile on a Linux system
```
git clone https://github.com/Cam-K/ernes.git
cd erNES
make
```
This will create a directory ``./build`` and generate the executable in it

## Libraries Needed (Ubuntu)

erNES requires cjson and SDL2 to be installed on your system. These can be installed with the following command on Ubuntu:

``sudo apt install libcjson-dev libcjson1 libsdl2-dev libsdl2-2.0-0``

## Usage
### To print help
``./ernes -h``

### To run a game
``./ernes -n [FILE]``



## Controls
| Keyboard      | NES Controller   |
|---------------|------------------|
| Arrow Keys    | Control Pad      |
| Z Key         | B Button         |
| X Key         | A Button         |
| Enter         | Start            |
| Shift         | Select           | 
 


