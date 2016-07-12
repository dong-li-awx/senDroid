LOCAL_PATH := $(call my-dir)
LOCAL_CPP_EXTENSION := .cpp
include $(CLEAR_VARS)

LOCAL_MODULE    := hook
LOCAL_SRC_FILES := \
    hook.c\
    msg_q.c\
    linked_list.c\
    my_msg_q_rcv.cpp\
    loc_eng_log.cpp\
    loc_log.cpp\
    binder.c\
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

LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog

include $(BUILD_SHARED_LIBRARY)
