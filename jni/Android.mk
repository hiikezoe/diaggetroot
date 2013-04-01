LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= jni.c
LOCAL_CFLAGS += -std=c99
LOCAL_MODULE := diaggetroot
LOCAL_LDLIBS := -llog
LOCAL_MODULE_TAGS := optional
LOCAL_STATIC_LIBRARIES := libdiagexploit
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= jni.c
LOCAL_CFLAGS += -std=c99
LOCAL_MODULE := diaggetrootjni
LOCAL_LDLIBS := -llog
LOCAL_MODULE_TAGS := optional
LOCAL_STATIC_LIBRARIES := libdiagexploit
include $(BUILD_SHARED_LIBRARY)

include $(call all-makefiles-under,$(LOCAL_PATH))
