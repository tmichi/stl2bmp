# stl2bmp
![teaser image](images/overview.png)
* stl2bmp voxelizes (descretizes) STL files as stacked 1bit bitmap images. 
* The user can control pixel resolution by DPI.
* Computation is reasonably fast by using OpenGL Frame buffer object.
# Build
## Dependencies 
* C++17 
* OpenGL
* GLFW (3.3 or later)
* GLEQ (2.1 or later)
* Eigen (3.4 or later)
## Build with cmake
```shell
% mkdir build    
% cd build 
% cmake ..
% make 
% 
```
## Usage 
```shell
% stl2bmp input.stl {dpi}
```
* input.stl: Input STL file (binary). The mesh must be closed and clean. 
* {dpi}: DPI value. Default value is 360 [dpi].

Voxelized images are saved as 1bit Bitmap images in input360/ (stem of the input file and dpi). 

### Example
``` shell
% ./stl2bmp torus.stl
stl2bmp v.1.0.0
dpi:360
    29/29
29 images(128x128,360dpi) saved to "E:\\stl2bmp_build\\torus360".
%
```
## Brief introduction to the algorithm
The program just renders the input mesh from the specific viewpoint by parallel projection.
It carefully sets a clipping plane so that the near plane corresponds to the sampling plane.
When the background colors, front face color and back face color are assigned to black, black and white respectively, the rendered images will be voxelized image of the sampling plane. 
By changing the distance of near clipping plane, we can obtain voxelized images of the mesh. 
We suppose that the input mesh is closed and clean ( no skiny triangles). You may need repair meshes created by some softwares (e.g. Polymender, Meshlab).
![Principle](images/principle.pngj)
# Notes
 * Minimum size of image is 128x128. 
# License
* MIT License (See LICENSE.txt)
# Author
* Takashi Michikawa <peppery-eternal.0b@icloud.com> : Image Processing Team, RIKEN Center for Advanced Photonics.

