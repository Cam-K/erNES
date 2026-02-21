# erNES
A Nintendo Entertainment System emulator, written in C.


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


## Libraries Needed (Ubuntu)
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
 



## Screenshots

![Super Mario Bros](https://i.imgur.com/ehnudBK.png)
![Castlevania](https://i.imgur.com/eMoT9Kq.png)

