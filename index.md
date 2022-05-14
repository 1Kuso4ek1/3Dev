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


## Setting up a new project
First, let's create a new folder and `CMakeLists.txt` in it
```
mkdir cool-project
cd cool-project
micro CMakeLists.txt # I use micro, you can use something else
```
Here's the most basic file (probably will not work on windows)
```
cmake_minimum_required(VERSION 3.17)

project(cool-project VERSION 1.0.0 LANGUAGES CXX)

find_package(3Dev REQUIRED)

add_executable(cool-project main.cpp)
target_link_libraries(cool-project PUBLIC 3Dev ${3DEV_DEPS})
target_include_directories(cool-project
    PUBLIC
    ${3DEV_INCLUDE_DIR}
)
```
Now we can start making the game...
## Basic application
We haven't created the main file yet, so let's do this first
```
micro main.cpp
```
Now let's include the `Engine.hpp` header and create `main` function
```
#include <Engine.hpp>

int main()
{
}
```
There's how to create a window...
```
...
    Engine engine;
	
    engine.GetSettings().depthBits = 24;
    engine.CreateWindow(1280, 720, "cool game");
    engine.Init();
...
```
This code creates a window with 1280x720 resolution and `cool game` title  
`engine.Init();` function just initializes OpenGL, nothing will work without it  
Allright, we have a window... Which closes instantly...  
Why? Basically, we didn't call the `engine.Launch();` function, but even if we call it, we will get an exception
```
terminate called after throwing an instance of 'std::bad_function_call'
  what():  bad_function_call
```
This is because we don't have main loop and event loop  
Here's how to create an event loop
```
...
    engine.EventLoop([&](sf::Event& event)
    {
        if(event.type == sf::Event::Closed) // If the window should close...
            engine.Close(); // We're closing it!
    });
...
```
And the last thing is a main loop. For now, it will be empty
```
...
    engine.Loop([&]() 
    {
   	    
    });
...
```
Now we have everything for a successful launch!  
In the end of `main` write 
```
...
    engine.Launch();
}
```
Let's build it
```
mkdir build && cd build
cmake ..
make
```