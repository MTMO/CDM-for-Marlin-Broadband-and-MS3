LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
  MarlinCryptoPlugin.cpp \

LOCAL_C_INCLUDES := \
  $(TOP)/bionic \
  $(TOP)/external/stlport/stlport \
  $(TOP)/frameworks/av/include \
  $(TOP)/frameworks/native/include \
  $(LOCAL_PATH)/../include \
  $(LOCAL_PATH)/../cdm/include \
  $(LOCAL_PATH)/../cdm/core/include/cdmsessionmanager \
  $(LOCAL_PATH)/../cdm/core/include/common \
  $(LOCAL_PATH)/../cdm/core/include/initdataparser \
  $(LOCAL_PATH)/../cdm/core/include/marlincdmengine \
  $(LOCAL_PATH)/../cdm/core/include/marlincrypto \
  $(LOCAL_PATH)/../cdm/core/include/marlinfunction \
  $(LOCAL_PATH)/../cdm/core/include/marlinlicense \
  $(LOCAL_PATH)/../cdm/core/include/marlinmediaformat \
  $(LOCAL_PATH)/../cdm/core/include/marlinrightsmanager \
  $(LOCAL_PATH)/../../marlin-agent-handler/marlinagenthandler/include \

LOCAL_MODULE := libmarlinmediacryptoplugin

LOCAL_MODULE_TAGS := optional

include $(BUILD_STATIC_LIBRARY)
