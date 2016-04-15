# Automatically generated makefile, created by the Projucer
# Don't edit this file! Your changes will be overwritten when you re-save the Projucer project!

APP_STL := gnustl_static
APP_CPPFLAGS += -fsigned-char -fexceptions -frtti -Wno-psabi
APP_PLATFORM := android-23
NDK_TOOLCHAIN_VERSION := 4.9

ifeq ($(NDK_DEBUG),1)
    APP_ABI := armeabi x86
else
    APP_ABI := armeabi armeabi-v7a x86
endif
