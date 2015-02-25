cmake_minimum_required(VERSION 2.8)
project(native_activity_example)

find_package(redwood_catkin REQUIRED)
redwood_build_only_if(ANDROID)

catkin_simple()

# This macro will also generate a depends.inc file with useful source and
# dependency information to pass on to ndk-build.
build_apk(NativeActivityExample
          SOURCE # Assumed to be in jni directory.
          	native_activity.cpp
          TRACK # Files to track, trigger rebuilds when changed.
            jni/Android.mk
            AndroidManifest.xml
)
