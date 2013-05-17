ifeq ($(SOMC_CFG_SENSORS_COMPASS_AK8973_AKMTASTIC),yes)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libAKMtastic
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := akmtastic.cpp
include $(BUILD_STATIC_LIBRARY)

endif
