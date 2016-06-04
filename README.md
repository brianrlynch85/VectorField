README for VectorField
======================
(c) Brian Lynch June, 2016
--------------------------

This software was written for educational purposes. I wrote/hacked this 
code while teaching myself a little bit about openGL. There may be
errors. Please let me know if you encounter any. I am definitely not
an expert in openGL (in fact I have no idea what I'm doing). The tutorials
found at:

https://en.wikibooks.org/wiki/OpenGL_Programming#Setting_Up_OpenGL

are a useful resource.

You may need to install the following:

      -"sudo apt-get install cmake"
      -"sudo apt-get install freeglut3-dev"
      -"sudo apt-get install libglew-dev"

This software works on Ubuntu 12.04 & 14.04 and has not been verified on
other operating systems.

   possible cmake options are (will put the executables in build/bin):
   
      -"mkdir build"
      -"cd build"
      -"cmake -DCMAKE_BUILD_TYPE=Debug ../"
      -"make"
      -"cd ../"
      
   Now you have done and out of source build, which leaves the original
   source directories clean. Example calling command:
   
      -"build/bin/vector_field -f ExampleData/ExampleData.dat"