cmake_minimum_required(VERSION 2.8)
project(augmented_reality_jni_example)

find_package(redwood_catkin REQUIRED)
redwood_build_only_if(ANDROID)

catkin_simple()

# This macro will also generate a depends.inc file with useful source and
# dependency information to pass on to ndk-build.
build_apk(AugmentedRealityJNIExample
          SOURCE tango_augmented_reality.cpp    # Assumed to be in jni directory.
            tango_data.cpp
          TRACK # Files to track, trigger rebuilds when changed.
            jni/tango_data.h
            src/com/projecttango/augmentedrealitynative/AugmentedRealityActivity.java
            src/com/projecttango/augmentedrealitynative/AugmentedRealityView.java
            src/com/projecttango/augmentedrealitynative/TangoJNINative.java
            res/layout/activity_augmented_reality.xml
            res/values/strings.xml
)
