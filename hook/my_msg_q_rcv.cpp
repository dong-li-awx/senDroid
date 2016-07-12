#define LOG_TAG "DEBUG"
#include "log_util.h"

#include "my_msg_q_rcv.h"

#include <sys/syscall.h>
#include <time.h>
#include <stdio.h>
#include "mxml/mxml.h"

#define gettid() syscall(__NR_gettid)

msq_q_err_type my_msg_q_rcv(void* msg_q_data, void** msg_obj) {
    loc_eng_msg_report_position *rpMsg;
    loc_eng_msg *msg;
    GpsLocation *location;
    msq_q_err_type result;
    time_t current_time;
    FILE *fSensorUsage;
    mxml_node_t *tree;
    mxml_node_t *node;
    mxml_node_t *time_start;
    mxml_node_t *time_end;
    mxml_node_t *usage;

    result = msg_q_rcv(msg_q_data, msg_obj);

    msg = (loc_eng_msg*)(*msg_obj);
    if (msg->msgid == LOC_ENG_MSG_REPORT_POSITION) {
        rpMsg = (loc_eng_msg_report_position*)(*msg_obj);
        location = (GpsLocation*)&(rpMsg->location);
        LOGD("LOC_ENG_MSG_REPORT_POSITION: tid: %d, latitude: %f, longitude: %f", gettid(), location->latitude, location->longitude);
        if (location->latitude != 0 || location->longitude != 0) {
            gu.gps_count ++;
        }
    }
    return result;
}
