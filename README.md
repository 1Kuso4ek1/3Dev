# 3Dev
Simple 3D game engine, that uses SFML, OpenGL, Assimp. 
## 3Dev Editor
3Dev Editor is in development, but now, if you on Windows, you can launch exe file in 3Dev Editor folder and make your scene.
## Tutorial
If you don't know how to make your scene, you can read small tutorial (only Russian language).
## Features
Loading many model formats (.obj, .fbx, .dae, etc.)

Loading animations

Loading textures

GUI (Buttons, Text box)

Class for lighting

Class for camera

Small class for simple shapes

Skybox

3Dev Editor (soon)
## TODO
- [x] Make it cross-platform
- [ ] Add lights (3Dev Editor)
- [ ] Add shapes (3Dev Editor)
- [ ] Add export function (3Dev Editor)
- [ ] Add simple physics (optional, if advanced physics will work fast)
- [ ] Add advanced physics
- [x] Add text input (Gui) 
## Building test game (Linux)
In 3Dev folder you have model, skybox, code and building files. First you need to download assimp sources in 3Dev folder, execute
```
git clone https://github.com/assimp/assimp
```
in terminal, you don't need to build it. Then download and build https://github.com/1Kuso4ek1/ibs, move executable near main.cpp, then execute
```
./ibs build.ibs libraries.ibs
```
in terminal. 
