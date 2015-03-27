# ----------------------------------------------------------------
# Builds libmarlincdm
#
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_CPPFLAGS := -DJSON_IS_AMALGAMATION -fno-exceptions

LOCAL_C_INCLUDES := \
  $(TOP)/bionic \
  $(TOP)/external/stlport/stlport \
  $(TOP)/external/sqlite/dist \
  $(TOP)/frameworks/av/include \
  $(TOP)/frameworks/native/include \
  $(LOCAL_PATH)/mediacrypto \
  $(LOCAL_PATH)/mediadrm \
  $(LOCAL_PATH)/include \
  $(LOCAL_PATH)/core/include/cdmsessionmanager \
  $(LOCAL_PATH)/core/include/common \
  $(LOCAL_PATH)/core/include/initdataparser \
  $(LOCAL_PATH)/core/include/marlincdmengine \
  $(LOCAL_PATH)/core/include/marlincrypto \
  $(LOCAL_PATH)/core/include/marlinfunction \
  $(LOCAL_PATH)/core/include/marlinlicense \
  $(LOCAL_PATH)/core/include/marlinmediaformat \
  $(LOCAL_PATH)/core/include/marlinrightsmanager \
  $(LOCAL_PATH)/../../marlin-agent-handler/marlinagenthandler/include \
  $(LOCAL_PATH)/../jsoncpp/include \

SRC_DIR := src
CORE_SRC_DIR := core/src
LOCAL_SRC_FILES := \
    $(SRC_DIR)/MarlinCdmInterface.cpp \
    $(CORE_SRC_DIR)/cdmsessionmanager/CdmSessionManager.cpp \
    $(CORE_SRC_DIR)/commmon/ContentDataParser.cpp \
    $(CORE_SRC_DIR)/commmon/CTime.cpp \
    $(CORE_SRC_DIR)/commmon/MarlinCdmUtils.cpp \
    $(CORE_SRC_DIR)/commmon/MarlinError.cpp \
    $(CORE_SRC_DIR)/initdataparser/AtknInitFormatData.cpp \
    $(CORE_SRC_DIR)/initdataparser/CencInitFormatData.cpp \
    $(CORE_SRC_DIR)/initdataparser/CommonHeaderParser.cpp \
    $(CORE_SRC_DIR)/initdataparser/IpmpInitFormatData.cpp \
    $(CORE_SRC_DIR)/initdataparser/LTPInitFormatData.cpp \
    $(CORE_SRC_DIR)/initdataparser/MS3InitFormatData.cpp \
    $(CORE_SRC_DIR)/initdataparser/ResponseParser.cpp \
    $(CORE_SRC_DIR)/initdataparser/PropInitFormatData.cpp \
    $(CORE_SRC_DIR)/marlincdmengine/MarlinCdmEngine.cpp \
    $(CORE_SRC_DIR)/marlincrypto/MarlinCrypto.cpp \
    $(CORE_SRC_DIR)/marlincrypto/MBBCrypto.cpp \
    $(CORE_SRC_DIR)/marlincrypto/MS3Crypto.cpp \
    $(CORE_SRC_DIR)/marlincrypto/MarlinCryptoHandler.cpp \
    $(CORE_SRC_DIR)/marlinfunction/IMarlinFunction.cpp \
    $(CORE_SRC_DIR)/marlinfunction/MBBFunction.cpp \
    $(CORE_SRC_DIR)/marlinfunction/MS3Function.cpp \
    $(CORE_SRC_DIR)/marlinlicense/IMarlinLicense.cpp \
    $(CORE_SRC_DIR)/marlinlicense/MarlinLicenseManager.cpp \
    $(CORE_SRC_DIR)/marlinlicense/MessageCreator.cpp \
    $(CORE_SRC_DIR)/marlinlicense/MBBLicense.cpp \
    $(CORE_SRC_DIR)/marlinlicense/MS3License.cpp \
    $(CORE_SRC_DIR)/marlinmediaformat/Cenc.cpp \
    $(CORE_SRC_DIR)/marlinmediaformat/Ipmp.cpp \
    $(CORE_SRC_DIR)/marlinrightsmanager/MarlinRightsManager.cpp \
    $(CORE_SRC_DIR)/marlinrightsmanager/RightsDb.cpp \
    $(CORE_SRC_DIR)/marlinrightsmanager/SQLiteQueryResult.cpp \

LOCAL_SHARED_LIBRARIES := \
    libmarlinagenthandler \
    libjsoncpp \
    libcutils \
    liblog \
    libstlport \
    libsqlite \

LOCAL_MODULE := libmarlincdm
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
