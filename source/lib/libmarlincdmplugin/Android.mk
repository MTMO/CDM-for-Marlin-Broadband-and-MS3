# -----------------------------------------------------------------------------
# CDM top level makefile
#
LOCAL_PATH := $(call my-dir)

# -----------------------------------------------------------------------------
# Builds libmarlincdmplugin.so
#
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
  src/MarlinCDMSingleton.cpp \
  src/MarlinCreatePluginFactories.cpp \
  src/MarlinCryptoFactory.cpp \
  src/MarlinDrmFactory.cpp \
  src/IonBuf.cpp \

LOCAL_C_INCLUDES := \
  $(TOP)/bionic \
  $(TOP)/external/stlport/stlport \
  $(TOP)/external/sqlite/dist \
  $(TOP)/frameworks/av/include \
  $(TOP)/frameworks/native/include \
  $(LOCAL_PATH)/mediacrypto \
  $(LOCAL_PATH)/mediadrm \
  $(LOCAL_PATH)/include \
  $(LOCAL_PATH)/cdm/include \
  $(LOCAL_PATH)/cdm/core/include/cdmsessionmanager \
  $(LOCAL_PATH)/cdm/core/include/common \
  $(LOCAL_PATH)/cdm/core/include/initdataparser \
  $(LOCAL_PATH)/cdm/core/include/marlincdmengine \
  $(LOCAL_PATH)/cdm/core/include/marlincrypto \
  $(LOCAL_PATH)/cdm/core/include/marlinfunction \
  $(LOCAL_PATH)/cdm/core/include/marlinlicense \
  $(LOCAL_PATH)/cdm/core/include/marlinmediaformat \
  $(LOCAL_PATH)/cdm/core/include/marlinrightsmanager \
  $(LOCAL_PATH)/../marlin-agent-handler/marlinagenthandler/include \

LOCAL_STATIC_LIBRARIES := \
  libmarlinmediacryptoplugin \
  libmarlinmediadrmplugin \

LOCAL_SHARED_LIBRARIES := \
  libmarlincdm \
  libcrypto \
  libdl \
  liblog \
  libutils \
  libsqlite \
  libstagefright_foundation \
  libstlport \

LOCAL_MODULE := libmarlincdmplugin

LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)/mediadrm

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

include $(call all-makefiles-under, $(LOCAL_PATH))

