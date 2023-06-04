# Build
If you're using debian, then almost everything can be obtained through apt
```
sudo apt install build-essential cmake git g++ libassimp-dev libglew-dev libglm-dev libjsoncpp-dev
```
There are 5 libraries that you must build from source: ReactPhysics3D, LightLog, Angelscript, SFML 3 and TGUI, they are included in 3Dev as submodules
```
git clone --recursive https://github.com/1Kuso4ek1/3Dev.git
cd 3Dev/3Dev/libraries
```
Build every library in `libraries` folder like that
```
cd library
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release # Debug, RelWithDebInfo, etc.
sudo make install -j $(nproc)
```
Note: When building TGUI don't forget to also write `-DTGUI_BACKEND=SFML_OPENGL3`, when calling cmake
### 3Dev
Now, when we have all the dependencies, we can build 3Dev. First, cd to `3Dev/3Dev`
```
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release # Debug, RelWithDebInfo, etc.
sudo make install -j $(nproc)
```
### 3Dev Editor
3Dev Editor will help you create a cool game...
```
cd ../../3Dev-Editor
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release # Debug, RelWithDebInfo, etc.
make install # No sudo! Otherwise, resources will be installed only for root user
```
