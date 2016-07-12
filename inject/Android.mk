LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := inject
LOCAL_SRC_FILES := \
    inject.c\
    mxml/mxml-attr.c\
    mxml/mxml-entity.c\
    mxml/mxml-file.c\
    mxml/mxml-get.c\
    mxml/mxml-index.c\
    mxml/mxml-node.c\
    mxml/mxml-private.c\
    mxml/mxml-search.c\
    mxml/mxml-set.c\
    mxml/mxml-string.c\

#shellcode.s

LOCAL_SHARED_LIBRARIES := libcutils
LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog
LOCAL_CFLAGS += -pie -fPIE
LOCAL_LDFLAGS += -pie -fPIE
#LOCAL_FORCE_STATIC_EXECUTABLE := true

include $(BUILD_EXECUTABLE)
