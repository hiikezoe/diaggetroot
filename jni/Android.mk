LOCAL_PATH:= $(call my-dir)

common_sources := common.c
COMMAND_LINE_TOOL_NAME := diaggetroot

include $(CLEAR_VARS)
LOCAL_SRC_FILES := $(common_sources) main.c
LOCAL_CFLAGS += -std=c99
LOCAL_MODULE := $(COMMAND_LINE_TOOL_NAME)
LOCAL_MODULE_TAGS := optional
LOCAL_STATIC_LIBRARIES := libdiagexploit
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := $(common_sources) jni.c
LOCAL_CFLAGS += -std=c99 -DHELPER_COMMAND_NAME=\"$(COMMAND_LINE_TOOL_NAME)\"
LOCAL_MODULE := diaggetrootjni
LOCAL_LDLIBS := -llog
LOCAL_MODULE_TAGS := optional
LOCAL_STATIC_LIBRARIES := libdiagexploit
include $(BUILD_SHARED_LIBRARY)

include $(call all-makefiles-under,$(LOCAL_PATH))
