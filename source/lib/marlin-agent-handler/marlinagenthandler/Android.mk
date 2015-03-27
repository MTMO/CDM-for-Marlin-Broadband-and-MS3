# ----------------------------------------------------------------
# Builds libmarlinagenthandler
#
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES := \
  $(TOP)/bionic \
  $(TOP)/external/stlport/stlport \
  $(LOCAL_PATH)/include \
  $(LOCAL_PATH)/../../libmarlincdmplugin/cdm/core/include/common \

SRC_DIR := src
LOCAL_SRC_FILES := \
    $(SRC_DIR)/MarlinAgentHandler.cpp \

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog \
    libstlport \

LOCAL_MODULE := libmarlinagenthandler
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
