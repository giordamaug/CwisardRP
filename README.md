# CwisardRP
Change Detection by Wisard<sup>rp</sup>

> Authors: Maurizio Giordano and Massimo De Gregorio
Institution: Istituto di Calcolo e Reti ad Alte Prestazioni (ICAR) - Consiglio Nazionale delle Ricerche (CNR) (Italy)
Institution: Istituto di Scienze Applicate e Sistemi Intelligenti "Eduardo Caianiello" (ISASI) - Consiglio Nazionale delle Ricerche (CNR) (Italy)

----------------------
Description
----------------------

CwisardRP is a change detection method in video processing based on a weightless neural system, 
namely WiSARD<sup>rp</sup>, with the aim of exploiting its features of being highly adaptive and 
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

1. OpenCV 2.7 (later versions may also work)

2. CMake  2.8  (later version may also work)

3. C++ Compiler (tested only with GCC 4.x or later versions)

```
$ cmake .
$ make
$ cwisard -P <video-folder>
```
If you have a C++ compiler supporting  OpenMP and you want to use parallelism
on multicore:
```
$ cmake . -DUSE_OMP=ON
$ make
$ cwisard -P <video-folder>
```
