#ifndef __ALOG_H__
#define __ALOG_H__

#ifdef __cplusplus
extern "C"
{
#endif
#include<android/log.h>

#define LOGI(fmt, args...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, fmt, ##args)
#define LOGD(fmt, args...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, fmt, ##args)
#define LOGE(fmt, args...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, fmt, ##args)
#define LOGV(fmt, args...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, fmt, ##args)

#ifdef __cplusplus
}
#endif

#endif
