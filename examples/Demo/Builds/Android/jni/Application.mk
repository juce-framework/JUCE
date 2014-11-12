# Automatically generated makefile, created by the Introjucer
# Don't edit this file! Your changes will be overwritten when you re-save the Introjucer project!

APP_STL := gnustl_static
APP_CPPFLAGS += -fsigned-char -fexceptions -frtti -Wno-psabi
APP_PLATFORM := android-10
NDK_TOOLCHAIN_VERSION := 4.8

ifeq ($(NDK_DEBUG),1)
    APP_ABI := armeabi armeabi-v7a
else
    APP_ABI := armeabi armeabi-v7a
endif
