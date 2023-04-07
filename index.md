# Hello, 3Dev!  
Learn how to create your first project using 3Dev engine  
[Build from sources](build.md)
## 3Dev Editor
### Editor interface
After running editor a window will appear with the main menu.
![image](https://user-images.githubusercontent.com/53074863/230631711-f5dfb1e7-303b-4499-a2f2-95aff4da6415.png)
When you click on New project, a small window opens where you need to enter the project name and the path to the folder where it will be stored. For example, if you enter "newProject" and "/home/user/projects", the "newProject" directory will be created in the "projects" folder.
![image](https://user-images.githubusercontent.com/53074863/230631874-a8ad95b9-ee19-47f6-a43c-73c3db5efc86.png)
When you click on Load project, a window opens with a list of recent projects and a field for entering the path to the project file. The button to the right of it opens file explorer.
![image](https://user-images.githubusercontent.com/53074863/230631913-f9042635-7ac7-4d49-a294-4d8b233e93d8.png)
When you create or open a project, the editor interface will appear in front of you.
![image](https://user-images.githubusercontent.com/53074863/230631964-5d4e527b-d466-4055-9b95-aa49fe489e5e.png)
In the middle is a 3D viewport where your scene is drawn. At the bottom - a window with logs. On the left - buttons for adding scripts and various objects: graphic, physical, and audio. Top right - the scene tree. At the bottom right - object property editor or save scene menu, if nothing is selected in the tree.

### Controlling the camera
To control the camera, hold down the right mouse button. To move it, use WASD. If you hold down Shift, the camera will move 2 times faster, if you hold down Ctrl - 2 times slower.

### Models
The left panel has buttons for creating both an empty model object and a cube. The created model will immediately be selected in the tree, and many items will appear in the properties panel: model name, position, rotation, and size. Below are the material slots, the type of physical body, a check mark that is responsible for enabling / disabling object rendering, and three buttons: open file, reload and delete.
![image](https://user-images.githubusercontent.com/53074863/230632046-492a4618-4481-4dc2-9bb0-06d0a55c754d.png)
When you click on Open file, an explorer window opens, allowing you to select the model file.
![image](https://user-images.githubusercontent.com/53074863/230632087-73e70a31-652f-4f94-9c8d-d1571383a134.png)
After loading a model that contains more than one mesh / material, there will be more slots for materials. Models can be copied and pasted using Ctrl+C and Ctrl+V.

### Materials
Any new project must have standard material, and you can create a new one by clicking the appropriate button on the left panel.
![image](https://user-images.githubusercontent.com/53074863/230632161-8537f737-b4cd-4832-9922-03edbbc849e1.png)
In the properties panel, you can change the material name, color, metalness, roughness, normal map, ambient occlusion map, emission, and opacity. The Pick buttons open a color selection window, and the Load texture buttons open a file explorer where you can select a texture.
![image](https://user-images.githubusercontent.com/53074863/230632205-118db001-8786-49fa-9bc3-a9d912bfb7a1.png)

### Physics
In the Physics section on the left panel, you can select the shape of the collider for the selected model (when creating a cube, and not an empty model, it will already have the desired collider).

### Light sources
The Light button on the left panel is responsible for creating light sources. In the properties panel, you can change the position, direction, color, attenuation, cutoff, and presence of shadows from the source.
![image](https://user-images.githubusercontent.com/53074863/230632256-1f7abdf3-c819-4085-99b3-bed85f8c3dd4.png)

### Audio sources
When you click the Sound source button, the file explorer opens, where you can select a file with sound (only .ogg or .wav). In the properties panel, you can change the name of the sound, its position, volume, attenuation, the minimum distance at which the sound will not be muted, the check mark responsible for looping the sound, the audio control buttons, and the delete sound button.
![image](https://user-images.githubusercontent.com/53074863/230632295-14e8383b-e5ac-44cb-8245-a7be87a43135.png)

### Saving the scene
The save scene menu is shown in the properties panel if no object is selected in the tree. 
![image](https://user-images.githubusercontent.com/53074863/230632369-9c72813e-79e4-4b1d-9295-ddd505e8d888.png)
You can either press Save or Ctrl+S to save the scene.

### Scripts
3Dev supports writing scripts in ![Angelscript](https://www.angelcode.com/angelscript/sdk/docs/manual/). When you click the Script button, the file explorer opens, allowing you to select the ".as" file. In the properties panel, you can build all scripts, run them, change the values of variables (only after successful build), and delete the script.
![image](https://user-images.githubusercontent.com/53074863/230632439-197a4c32-0df1-4951-8cca-9e263861c951.png)

### Object mode
This mode allows you to change the position, rotation, and size of objects without using the properties panel. You can enter and exit it by pressing Ctrl+G. An inscription will appear in the lower-left corner of the viewport.
![image](https://user-images.githubusercontent.com/53074863/230632602-be2e110a-e45d-45c9-b1ac-46990896a33d.png)
By default, we are in X-axis movement mode. You can change the axis using the X, Y, and Z keys. To switch to a specific mode, use one of the following combinations: Alt+M - move mode, Alt+R - rotate mode, Alt+S - scale mode. The value of the property on the selected axis is changed by arrows: up or right - increase, down or left - decrease.

### Angelscript
Angelscript is a beautiful scripting language that resembles C++ in syntax. All the main aspects of Angelscript are well described in the official documentation.  

You can start writing code in any editor. There are extensions for vscode, atom, sublime, etc. that add syntax highlighting. But, for example, codeblocks has it by default.  

Any file with Angelscript code has the extension ".as". Let's create, for example, "script.as" in "project/assets/scripts" and start writing.  

In order for the script to run in 3Dev, you need 2 functions: void Start(), which is executed once, and void Loop(), which is executed every frame.
```
void Start()
{

}

void Loop()
{

}
```
Let's start with a simple scene: floor and player.
![image](https://user-images.githubusercontent.com/53074863/230632799-3fd11c6c-7f65-440b-b8b3-f1734f220c3b.png)
Pay attention to the properties panel - the floor's body type must be static so that it is not affected by any physical forces.  

We can get models and all other objects in scripts from the scene object as pointers. Unlike C++, pointers in Angelscript are created as follows: `Type@ name;`  

Another difference is that by default, all operators interact not with the pointer, but with the object itself:
```
Type@ object;
Type@ anotherObject;

...

object = anotherObject; // The = operator overloaded in the Type class is used
@object = @anotherObject; // A pointer to object is assigned a pointer to anotherObject
object.doSomething(); // You don't need to dereference the pointer
```
Now that you know the specifics of pointers, you can continue writing the script.  

The scene object is located in the Game namespace. In addition to the scene object, there are some other objects and variables that we'll talk about later.  

The SceneManager class has many different useful functions. Now we definitely need the GetModel function, which allows us to get a pointer to the model by its name.
```
Model@ model;

void Start()
{
    @model = @Game::scene.GetModel("player");
}

void Loop()
{

}
```
Now, having a pointer to the model, you can do a lot of interesting things with it: move, rotate, get some information about it, etc.  

For example, when you press a certain key, let's try to move the model along the X-axis. For working with the keyboard, there is the Keyboard namespace, where all keys are defined, and the isKeyPressed function, which allows you to find out if a certain key is pressed. To move the model, we will use the Move function.
```
Model@ model;

void Start()
{
    @model = @Game::scene.GetModel("player");
}

void Loop()
{
    if(Keyboard::isKeyPressed(Keyboard::Q))
        model.Move(Vector3(1, 0, 0));
}
```
Now you can go back to the editor, load this script, build it with the Build button, and run it with the Start / stop button. When you click on Q, the "player" will move along the X-axis. If you press Start / stop again, everything will fall back into place.  

Time to complicate our script again. For example, you can make an FPS controller. To do this, you don't need to write a lot of conditions for the WASD keys.You can use the camera located in the `Game` namespace and the Move function built into it, which will give the camera's movement vector.

```
Model@ model;

void Start()
{
    @model = @Game::scene.GetModel("player");
}

void Loop()
{
    Vector3 vec = Game::camera.Move(1, true);
}
```

Instead of `1` in the first argument, it is better to use delta time, but you can leave it like that for now. The second argument tells our camera that we just want it to give the displacement vector. In case if there's `true`, camera's position will not change.  

By default, the Move function is called by the editor/launcher every frame, so `Game` has the `manageCameraMovement` variable. In order for the camera to be controlled only by the script, set it to false in the `Start()` function.
```
Model@ model;

void Start()
{
    Game::manageCameraMovement = false;

    @model = @Game::scene.GetModel("player");
}

void Loop()
{
    Vector3 vec = Game::camera.Move(1, true);
}
```
Now that you know the displacement vector, you can either just use `Model::Move`, or do something smarter using physics.  

The Model class has a method for getting a pointer to its physical body - `GetRigidBody`. In turn, the `RigidBody` class has a huge number of functions.  

Control can be done in two ways: directly change the speed of the physical body or apply a certain force to it. Both options in the simplest version are not perfect. In the first case, this does not have a very good effect on the object's response to a collision, and in the second case, the control turns out to be too unresponsive. But [here's](https://github.com/1Kuso4ek1/FPSController) my best version of character controller!  

Now we will analyze the 1st method. Using the setLinearVelocity and getLinearVelocity functions, we set the correct speed for the object based on the direction.
```
Model@ model;

void Start()
{
    Game::manageCaneraMovement = false;

    @model = @Game::scene.GetModel("player");
}

void Loop()
{
    // By changing the 1st argument we will change the speed
    Vector3 vec = Game::camera.Move(10, true);

    // So the player will be affected by gravity
    vec.y = model.GetRigidBody().getLinearVelocity().y;

    model.GetRigidBody().setLinearVelocity(vec);

    // So the player will not fall down
    model.SetOrientation(Quaternion(0, 0, 0, 1));

    // 2.7 - player's height
    Game::camera.SetPosition(model.GetPosition() + Vector3(0.0, 2.7, 0.0));
}
```

That's a pretty good movement system. The only thing is that you don't need to draw the player model. This is can be done in the editor.  

Do you want to do something more? Look at the ScriptManager.cpp, while also looking at the classes in the include folder. Yes, new information will appear in the article, but it's so interesting to study and try something yourself! :)  

Since version 1.3.0 of the editor, a built-in code editor has appeared in it. Now, when adding a script through the button on the left panel, you can not only select a file with the code, but also create a new one. In any case, after selecting or creating a file, the code editor itself will open. There's nothing special about it. You can save the file using Ctrl+S, and exit using the Esc key. Also, when selecting a script, the Edit script button appeared in the scene tree, which will open the selected script in the code editor.
![image](https://user-images.githubusercontent.com/53074863/230636062-bed6c16c-eb21-434d-803e-fa99bf7d95be.png)
