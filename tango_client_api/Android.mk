# Copyright 2014 Google Inc. All Rights Reserved.
# Distributed under the Project Tango Preview Development Kit (PDK) Agreement.
# CONFIDENTIAL. AUTHORIZED USE ONLY. DO NOT REDISTRIBUTE.
# NDK import module for resolving symbols from selected private Android APIs.
### IMPORTANT ###
# This module has a non-standard extra step. Usage:
#   $(call import-module,AndroidSystemLibs)
#   $(call module-add-static-depends,<MY_MODULE>,AndroidSystemLibs)
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := tango_client_api


ifeq ($(TARGET_ARCH), x86)
  LOCAL_EXPORT_LDLIBS := -L$(LOCAL_PATH)/lib/x86 -ltango_client_api
endif

ifeq ($(TARGET_ARCH), arm64)
  LOCAL_EXPORT_LDLIBS := -L$(LOCAL_PATH)/lib/arm64-v8a -ltango_client_api
endif

ifeq ($(TARGET_ARCH), arm)
  LOCAL_EXPORT_LDLIBS := -L$(LOCAL_PATH)/lib/armeabi-v7a -ltango_client_api
endif

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_SRC_FILES := lib/libtango_client_stub.a

include $(PREBUILT_STATIC_LIBRARY)
