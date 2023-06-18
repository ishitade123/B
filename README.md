# Phantom_Generator
Code for 3D phantom generation using real data as reference

Steps to Create Phantom:

1.  Open the program
2.  Load any image (the reference image)
3.  Click on Distance Transform
4.  Click/Select any point
5.  Click on Add Point
6.  Click/Select any point
7.  Click on Add Point
8.  Click/Select any point
9.  Click on Add Point
10. Click/Select any point
11. Click on Add Point
12. Click/Select any point
13. Click on Generate Curve, the generated curve will be shown in the display
14. If you want to add more curves repeat steps 4 to 13 again, else Click on Write Image
15. The phantom image will be written as <reference image name>_phantom.img [ You need to create the corresponding .hdr file yourself using ITK-SNAP or feel free to edit the code and add that feature


N.B: Make sure you save the generated phantom image somewhere else/ or rename it as it will be overwritten if you open the program with the same reference image as before.
