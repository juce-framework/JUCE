The source code for lilv and its dependent libraries have been copied into this
directory. The following modifications were made:

- Removed files not strictly required to build the lilv library, including
  generated config headers
- Added handwritten config headers
- Removed the include of dlfcn.h in world.c
- Fixed a leak of world->opt.lv2_path when calling lilv_world_set_option
  multiple times
- Fixed a leak of plugin classes when calling lilv_world_load_plugin_classes
  multiple times
- Added clang-tidy warning suppression comments to atom/util.h

Remember to update the versions in the config headers if you ever update the
library versions!

