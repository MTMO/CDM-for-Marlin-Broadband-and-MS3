#
# (c) 2015 - Copyright Marlin Trust Management Organization
#
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := tests
LOCAL_PACKAGE_NAME := MarlinMediaDrmTest
LOCAL_SRC_FILES := $(call all-java-files-under,src)

include $(BUILD_PACKAGE)