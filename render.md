# 3Dev Render
3Dev Render is a small console program, that helps you draw your scene or animation to the .jpg or <.mp4, etc.> file.
## Basics
You can see usage and arguments by writing `render --help`:
```
Usage: render [options] -s path/to/scene
  --help            Display this information
  -s <file>         Path to the scene file
  -a <name>         Name of animation
  -o <file>         Name for the output image/video (output.jpg/.mp4 is default)
  -e <file>         Path to the hdri environment (${HOME}/.3Dev-Editor/default/hdri.hdr is default)
  -w <int>          Width of the output (1280 is default)
  -h <int>          Height of the output (720 is default)
  -b <int>          Size of a skybox side (512 is default)
  -f <int>          Output video framerate (30 is default)
  -x <float>        Exposure (1.5 is default)
  -r <number>       Shadow map resolution (4096 is default)
```
There's a typical way to render an image:
```
render -s /home/user/projects/scene/scene.json -o scene.jpg -e /home/user/environment.hdr -w 1280 -h 720 -x 1.0
```
## Animations
To render animations you need **FFmpeg**, that included in the 1.3.0-rc release. With `-a` argument you can either specify one animation or make a **queue**:
```
render -s /..../scene.json -o scene.mp4 -e /..../environment.hdr .... -a cameraMovement:walk,cameraMovement1
```
In that case, **cameraMovement** will play first, and then **walk** and **cameraMovement1** will play simultaneously.  
Here's a small example of what can be rendered:


https://user-images.githubusercontent.com/53074863/236879971-24dcfd5a-19e1-40e0-86a0-2ee20c214490.mp4

