# MinecraftAlter

## Intro
This is a project made in C++ with OpenGL. The goal is to create a small scene that mimics the Minecraft world.

Main Features:
-	Terrain Generation
-	Blinn-Phone lighting model
- **[New] Shadow Mapping**
-	Simple wave simulation
- **[New] Player Movement**
- **[New] Player Collision**
-	Controllable Sunlight position
-	Controllable Orbiting Camera

Used libraries: GLFW, Glad, glm, stb.

![Current Progress](./images/230108.png?raw=true)
*Fig: Current Progress*

![First Person View](./images/230108_fpv.png?raw=true)
*Fig: First Person View*

## Controls

### Universal

| Key | Operation |
| --- | --- |
| Q / E | Sun orbit to West / East |
| F1 | Default rendering |
| F2 | Show wireframes |
| F3 | Show normal (camera view) |
| F5 | Toggle Observer / First Person view |
| F10 | Toggle Player Floating |
| F11 | Toggle full screen |
| F12 | Randomize terrain |
| Esc | Exit |

### Observer View

| Mouse | Operation |
| ----- | --- |
| Hold LMB & Move | Orbit camera around world |
| Scroll Up / Down | Zoom in / out |

### First Person View

| Mouse | Operation |
| ----- | --- |
| Move | Look around |

| Key | Operation |
| --- | --- |
| W / S | move forward / backward |
| A / D | move to left / right |
| LShift | sprinting |
| Space | Jump |
| Space / LCtrl | Floating Mode: Player move up / down |

## To-do

- [ ] Rework sun
- [ ] Light intensity changes with sun position
- [ ] A bigger world (procedurally loaded and unloaded chunks)
- [ ] Screen-space reflection (for the water)
- [ ] Ambient occlusion
- [ ] Anti-aliasing
- [ ] Controllable player
  - [x] Movements
  - [x] Collision
  - [ ] Block placing & destroying
- [x] Performance improvement
- [x] Shadow Mapping (use depth map)
- [x] Sky color changes with sun position
- [x] Texturing
  - [x] Basic
  - [x] Multiple textures for one block

## Implementation

### Terrain Generation

The information of the world is stored in a 72\*72\*72 int multi-dimensional array. The int is the id of different types of blocks. Currently, the id only determines the color of the cubes. A map from int to vec3 tells the program how to color the cubes.

As the program start, a terrain generation code is run. The program loop over the x-z plane (the horizontal plane) and do Perlin noise calculation to get the altitude of the ground surface. 

Then, assign the color of cubes to make it looks like terrain. If the Altitude ≤ 25, color the surface as sand, if 25 < Altitude < 35, color the surface as grass, above that, color the surface as snow. Above each surface, if the y ≤ 23, filled with water blocks, else filled with air (not rendered later).

### Rendering

When rendering each frame, it updates the background color according to the light source position, which mimics the sky. Then, it calculates the camera transform based on the global variables that store the camera position. The variable is influenced by keyboard input (with functions provided by GLFW). Similarly, light position transform is also handled here.

After linking the variable to the shader uniforms, the function drawWorldCubes() is called to loop over the world info and render each cube. In every iteration, it first checks if the block is air, or if the 6 sides of the block are all covered by other blocks, then skip the render. After excluding the unnecessary blocks, it creates a translation matrix, to transform the unit cube vertices (centered at the origin, with the length = 1) to their proper coordinates, and pass that matrix to the shader. If the block is a water surface, it does a shrinking scale to make the water surface a bit lower than the shore, and also passes some variables to the shader for the specular lighting and wave simulation. Finally, it passes the preset cube color as the diffuse color to the shader.

![Wireframes of the scene](./images/221221_wireframes.png?raw=true)
*Fig: Wireframe of the scene. The cubes and faces that are fully covered by other opaque cubes are not rendered to save performance. Face culling is also enabled.*

The shader is relatively simple. Most of the cubes use diffuse lighting, while the water cube uses the Blinn-Phone model to add a specular light. In addition, multiple sine wave rotations are applied to the fragment normal of the water surface, to create a simple wave simulation for the water.

![Normals of the scene](./images/221221_normals.png?raw=true)
*Fig: Normals of the scene (Camera space).*

![Showcase of wave simulation](./images/221221_waves.png?raw=true)
*Fig: Showcase of wave simulation.*

### References

- OpenGL basics
  - https://learnopengl.com/
- Perlin Noise
  - https://en.wikipedia.org/wiki/Perlin_noise
  - https://rtouti.github.io/graphics/perlin-noise-algorithm
  - https://www.bilibili.com/video/BV11V4y1M7N6
- Rotation Transformation about an arbitrary axis
  - https://www.neilmendoza.com/glsl-rotation-about-an-arbitrary-axis/
- Single header image loading lib
  - https://github.com/nothings/stb/blob/master/stb_image.h
