# Build
## Building on Debian
Almost everything we need can be obtained through apt
```
sudo apt install build-essential cmake git g++ libsfml-dev libassimp-dev libglew-dev libglm-dev libjsoncpp-dev
```
There are 4 libraries that we must build from source: ReactPhysics3D, LightLog, Angelscript and TGUI, they are included in 3Dev as submodules
```
git clone --recursive https://github.com/1Kuso4ek1/3Dev.git
cd 3Dev/3Dev/libraries
```
### ReactPhysics3D
```
cd ReactPhysics3D
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release # Debug, RelWithDebInfo, etc.
sudo make install -j4
```
### LightLog
```
cd LightLog
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release # Debug, RelWithDebInfo, etc.
sudo make install -j4
```
### TGUI
```
cd TGUI
mkdir build && cd build
cmake .. -DTGUI_BACKEND=SFML_OPENGL3 -DCMAKE_BUILD_TYPE=Release # Debug, RelWithDebInfo, etc.
sudo make install -j4
```
### 3Dev
Now, when we have all the dependencies, we can build 3Dev. First, cd to `3Dev/3Dev`
```
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release # Debug, RelWithDebInfo, etc.
sudo make install -j4
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
### 3Dev Editor
3Dev Editor will help you create a cool game...
```
cd ../../3Dev-Editor
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release # Debug, RelWithDebInfo, etc.
make install # No sudo! Otherwise, resources will be installed only for root user
```
