# 3Dev
![](https://img.shields.io/badge/3Dev-1.2.6-blue)
![](https://img.shields.io/badge/3Dev_Editor-1.2.0-blue)
![](https://img.shields.io/badge/language-C++-green)
![](https://img.shields.io/github/license/1kuso4ek1/3Dev)
![](https://img.shields.io/github/issues/1kuso4ek1/3Dev)
![](https://img.shields.io/github/downloads/1kuso4ek1/3Dev/total)  
![](logo.png)
## About
- 3Dev is a simple and lightweight game engine, which uses SFML, OpenGL, ReactPhysics3D and Assimp
## Features
- Rigid body physics
- Loading a lot of model formats (.obj, .dae, .fbx and much more...)
- Skeletal animation (troubleshooting below)
- Post processing support
- HDR, tone mapping and gamma-correction
- FXAA
- Physically Based Rendering
- Image Based Lighting
- Shadow mapping
- Diffuse, normal, metalness, emission, roughness, ambient occlusion and opacity maps
- Audio
- Scripting with Angelscript
- 3Dev Editor
- Node-based hierarchy
## TODO
TODO list is now in the [projects](https://github.com/1Kuso4ek1/3Dev/projects/1)!
## Building
All building instructions are [here](https://1kuso4ek1.github.io/3Dev/build.html)!
## Tutorial
[English](https://telegra.ph/Developing-games-with-3Dev-01-11) and [Russian](https://telegra.ph/Razrabotka-igr-s-pomoshchyu-3Dev-01-06)!
## Screenshots
![](./Screenshots/scr.png)
![](./Screenshots/scr1.png)
![](./Screenshots/scr2.png)
![](./Screenshots/scr3.png)
## Troubleshooting
### Skeletal animation
There's an issue: animations are just weird. Soon I will start working on a brand new animation system (3Dev 1.3.0), but for now, that's what you must do to get decent results:  
1. Open your model in blender  
2. Select 'weights paint' mode  
3. Go weights -> Limit total -> Set limit to 1  ![image](https://user-images.githubusercontent.com/53074863/223760687-35435c8a-a78a-4c09-9f71-93a57250ccaa.png)  
4. Export  
5. Open it in the editor  
6. Enjoy!  
![image](https://user-images.githubusercontent.com/53074863/223761197-f381b4ef-311a-4653-835d-afd9d60c32b2.png)
