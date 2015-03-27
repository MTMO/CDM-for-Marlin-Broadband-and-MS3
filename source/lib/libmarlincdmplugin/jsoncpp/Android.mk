LOCAL_PATH := $(call my-dir)
# ----------------------------------------------------------------
# Prebuilt libstlport_shared
#

# ----------------------------------------------------------------
# Builds libjsoncpp
#
include $(CLEAR_VARS)

LOCAL_CPPFLAGS := -DJSON_IS_AMALGAMATION -fno-exceptions

LOCAL_C_INCLUDES := \
  $(TOP)/bionic \
  $(TOP)/external/stlport/stlport \
  $(LOCAL_PATH)/include \

LOCAL_SRC_FILES := \
  jsoncpp.cpp \

LOCAL_MODULE := libjsoncpp
LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES := \
  libstlport \

include $(BUILD_SHARED_LIBRARY)
