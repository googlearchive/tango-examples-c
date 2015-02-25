LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := app
LOCAL_SRC_FILES := \
	/home/xuguo/development/RedwoodInternal/Redwood/3rdparty/tango-examples-c/hello-tango-jni-example/app/src/main/jni/tango_native.c \
	/home/xuguo/development/RedwoodInternal/Redwood/3rdparty/tango-examples-c/hello-tango-jni-example/app/src/main/jni/Application.mk \
	/home/xuguo/development/RedwoodInternal/Redwood/3rdparty/tango-examples-c/hello-tango-jni-example/app/src/main/jni/Android.mk \

LOCAL_C_INCLUDES += /home/xuguo/development/RedwoodInternal/Redwood/3rdparty/tango-examples-c/hello-tango-jni-example/app/src/main/jni
LOCAL_C_INCLUDES += /home/xuguo/development/RedwoodInternal/Redwood/3rdparty/tango-examples-c/hello-tango-jni-example/app/src/debug/jni

include $(BUILD_SHARED_LIBRARY)
