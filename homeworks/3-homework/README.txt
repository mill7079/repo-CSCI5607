This resource file contains 3 folders

Examples/: 
  - Contains example files that can serve as input to the program alone with sample outputs

PGA/:
  - Contains an implementation of basic ray-tracing ideas using a 3D PGA Library

Vec3/
  - Contains an implementation of basic ray-tracing ideas using a simple Vec3 Library

You can use either the PGA or Vec3 version for the homework.

The assignment asks you to modify either parse_pga.h or parse_vec.h so that
the program reads in a renders a file. Your code must support the following commands:
   - output_file:, image_resolution:, sphere:
   - camera_pos:, camera_fwd:, camera_up:, camera_hfov:
Additionally, you should skip any line that starts with an "#" (as a comment)

As a general note, some things are easy with PGA others with Vec3.
Both the Vec3 and PGA libraries are pretty fast, when compiled with appropriate optimizations.
The Vec3 library is very small. This is good, because it's easy to use & understand, but
it not as full featured as the PGA library which has different distinct data structures
for points, planes, lines, and directions. 