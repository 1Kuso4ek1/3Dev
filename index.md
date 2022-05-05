# Hello, 3Dev!
Learn how to create your first project using 3Dev engine
## Setup
### Building on Debian
Almost everything we need can be obtained through apt
```
sudo apt install build-essential cmake git g++ libsfml-dev libassimp-dev libglew-dev libglm-dev
```
There are 2 libraries that we must build from source: ReactPhysics3D and LightLog
### ReactPhysics3D
```
git clone https://github.com/DanielChappuis/reactphysics3d.git
cd reactphysics3d
mkdir build && cd build
cmake ..
make -j4
sudo make install
```
### LightLog
```
git clone https://github.com/1Kuso4ek1/LightLog.git
cd LightLog
mkdir build && cd build
cmake ..
make -j4
sudo make install
```
### 3Dev
Now, when we have all the dependencies, we can build 3Dev
```
git clone https://github.com/1Kuso4ek1/3Dev.git
cd 3Dev/3Dev
mkdir build && cd build
cmake ..
make -j4
sudo make install
```
There also a small demo scene, you can launch it with
```
./physics
```
> INFO: On a Raspberry Pi you will probably get an error when compiling shaders.  
> This is because 3Dev uses GLSL 3.30, that RPI does not support.
> The only solution is to launch it with
> ```
> MESA_GL_VERSION_OVERRIDE=3.3 ./physics
> ```  
If you haven't got any errors, congrats!
