#ifndef __MY_MSG_Q_RCV_H__
#define __MY_MSG_Q_RCV_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <time.h>
#include "hardware/gps.h"
#include "msg_q.h"
#include "loc.h"
#include "loc_eng_msg.h"
#include "linked_list.h"

struct gps_usage {
    int gps_count;
    struct timeval gps_start_time;
    struct timeval gps_end_time;
    int uid;
    char* package_name;
}gu;

msq_q_err_type my_msg_q_rcv(void* msg_q_data, void** msg_obj);

#ifdef __cplusplus
}
#endif

#endif
