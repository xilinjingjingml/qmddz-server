LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := bdLordRobotAi_static

LOCAL_MODULE_FILENAME := libLordRobotAi

MY_SRCS := $(wildcard $(LOCAL_PATH)/*.cpp)
	
MY_SRCS := $(MY_SRCS:$(LOCAL_PATH)/%=%) 

LOCAL_SRC_FILES :=  $(MY_SRCS)

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/ 

LOCAL_C_INCLUDES := $(LOCAL_PATH)/ 
	
LOCAL_CFLAGS := -g				            
include $(BUILD_STATIC_LIBRARY)

########################################################

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := bdLordRobotAi

MY_SRCS := $(wildcard $(LOCAL_PATH)/*.cpp) 

	
MY_SRCS := $(MY_SRCS:$(LOCAL_PATH)/%=%) 

LOCAL_SRC_FILES := $(MY_SRCS)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/ 

                   
LOCAL_LDLIBS := 
LOCAL_CFLAGS := -g				            
include $(BUILD_SHARED_LIBRARY)