# 3Dev
Simple 3D game engine, that uses SFML, OpenGL, Assimp. 
## 3Dev Editor
3Dev Editor's source code is now here!
## Tutorial
Check Wiki, if you want to learn how to make games with 3Dev.
## Features
Loading many model formats (.obj, .fbx, .dae, etc.)

Loading animations

Loading textures

GUI (Buttons, Text box)

Class for lighting

Class for camera

Class for simple shapes

3Dev Editor
## TODO
- [x] Make it cross-platform
- [x] Add lights (3Dev Editor)
- [ ] Add shapes (3Dev Editor)
- [x] Add export function (3Dev Editor)
- [x] Add physics
- [x] Improve GUI
- [x] Add text input (Gui) 
- [x] Animation states
- [ ] Materials
- [ ] Particle system
## Building test game (Linux)
In 3Dev folder you have model, skybox, code and building files. First you need to download assimp sources in 3Dev folder, execute
```
git clone https://github.com/assimp/assimp
```
in terminal, you don't need to build it. Then download and build https://github.com/1Kuso4ek1/ibs, write
```
sudo cp ibs /usr/bin
```
to use ibs everywhere you want, then execute
```
ibs build.ibs libraries.ibs
```
in terminal.
## Building 3Dev Editor (Linux)
It's really easy. Open terminal in 3Dev Editor forlder, then, if you already build ibs, just write
```
ibs editor.ibs libraries.ibs
```
wait, and enjoy! :)
