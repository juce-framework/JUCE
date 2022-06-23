The source code for lilv and its dependent libraries have been copied into this
directory. The following modifications were made:

- Removed files not strictly required to build the lilv library,
  including generated config headers
- Added handwritten config headers
- Removed the include of dlfcn.h in world.c

Remember to update the versions in the config headers if you ever update
the library versions!

