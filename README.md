# CwisardRP
Change Detection by WisardRP

> Authors: Maurizio Giordano and Massimo De Gregorio
> Institution:  Consiglio Nazionale delle Ricerche (Italy)

----------------------
Description
----------------------

CwisardRP is a change detection method in video processing based on a weightless neural system, 
namely WiSARDrp, with the aim of exploiting its features of being highly adaptive and 
noise–tolerance at runtime.
In CwisardRP, the changing pixel colors in a video are processed by a an incremental 
learning neural network with a limited-in-time memory-retention mechanism that allow the
proposed system to absorb small variations of the learned model (background) 
in the steady state of operation as well as to  fastly adapt to background 
changes during the video timeline.


----------------------
License
----------------------
  
The source code is provided without any warranty of fitness for any purpose.
You can redistribute it and/or modify it under the terms of the
GNU General Public License (GPL) as published by the Free Software Foundation,
either version 3 of the License or (at your option) any later version.
A copy of the GPL license is provided in the "GPL.txt" file.

----------------------
Compile/Run CwisardRP source (Linux, Mac OSX)
----------------------

To run the code the following libraries are required:

1. OpenCV 3.0 (later versions may also work)

2. CMake  3.0  (later version may also work)

3. C++ Compiler (tested only with GCC 5.x or later versions)

```
$ cmake .
$ make
$ cwisard  -P <video-folder>
```
