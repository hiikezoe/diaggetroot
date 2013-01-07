LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= \
	jni.c \
	diag.c 
LOCAL_CFLAGS += -std=c99
LOCAL_MODULE := diaggetroot
LOCAL_LDLIBS := -llog
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= \
	jni.c \
	diag.c 
LOCAL_CFLAGS += -std=c99
LOCAL_MODULE := diaggetrootjni
LOCAL_LDLIBS := -llog
include $(BUILD_SHARED_LIBRARY)
