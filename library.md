# Library
## Setting up a new project
First, let's create a new folder and `CMakeLists.txt` in it
```
mkdir cool-project
cd cool-project
micro CMakeLists.txt # I use micro, you can use something else
```
Here's the most basic file
```
cmake_minimum_required(VERSION 3.17)

project(cool-project VERSION 1.0.0 LANGUAGES CXX)

find_package(3Dev REQUIRED)

find_package(assimp REQUIRED)
find_package(OpenGL REQUIRED)
find_package(ReactPhysics3D REQUIRED)
find_package(glm REQUIRED)
find_package(jsoncpp REQUIRED)
find_package(Angelscript REQUIRED)
find_package(Threads REQUIRED)

add_executable(cool-project main.cpp)

target_link_libraries(cool-project PUBLIC 3Dev::3Dev)
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
If you launch your project, you'll see just a blank screen, and... That's kinda boring, right? So let's initialize the most important thing: Renderer.
Renderer class is a singleton, so we can access the only possible object with
```
Renderer::GetInstance()->...;
```
The function we need is `Renderer::Init`, which needs at least 2 parameters: size of viewport, and a path to the HDR environment. You can look for cool HDRIs on [polyhaven](https://polyhaven.com/) or [ambientcg](https://ambientcg.com/).  
Found a good environment? Let's move on and initialize renderer right before event loop
```
Renderer::GetInstance()->Init(engine.GetWindow().getSize(), "cool_hdri.hdr");
```
Now we're gonna have all the shaders, framebuffers and textures.  
Let's create a camera objects right after renderer initialization.  
Camera's constructor needs at least 1 parameter - pointer to the sf::Window object.
```
Camera camera(&engine.GetWindow());
```

