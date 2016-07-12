/* Copyright (c) 2011, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of Code Aurora Forum, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#ifndef LOC_ENG_MSG_H
#define LOC_ENG_MSG_H

#ifdef __cplusplus

#include <stdlib.h>
#include <string.h>
#include "loc_eng_msg_id.h"
#include "loc_eng_log.h"

struct loc_eng_msg {
    const void* owner;
    const int msgid;
    inline loc_eng_msg(void* instance, int id) :
        owner(instance), msgid(id)
    {
        LOC_LOGV("creating msg %s", loc_get_msg_name(msgid));
    }
    virtual ~loc_eng_msg()
    {
        LOC_LOGV("deleting msg %s", loc_get_msg_name(msgid));
    }
};

struct loc_eng_msg_report_position : public loc_eng_msg {
    const GpsLocation location;
    const void* locationExt;
    const enum loc_sess_status status;
    inline loc_eng_msg_report_position(void* instance, GpsLocation &loc, void* locExt,
                                       enum loc_sess_status st) :
        loc_eng_msg(instance, LOC_ENG_MSG_REPORT_POSITION),
        location(loc), locationExt(locExt), status(st)
    {
#ifdef QCOM_FEATURE_ULP
        LOC_LOGV("flags: %d\n  source: %d\n  latitude: %f\n  longitude: %f\n  altitude: %f\n  speed: %f\n  bearing: %f\n  accuracy: %f\n  timestamp: %lld\n  rawDataSize: %d\n  rawData: %p\n  Session status: %s",
                 location.flags, location.position_source, location.latitude, location.longitude,
                 location.altitude, location.speed, location.bearing, location.accuracy,
                 location.timestamp, location.rawDataSize, location.rawData,
                 loc_get_position_sess_status_name(status));
#else
        LOC_LOGV("flags: %d\n  latitude: %f\n  longitude: %f\n  altitude: %f\n  speed: %f\n  bearing: %f\n  accuracy: %f\n  timestamp: %lld\n  Session status: %s",
                 location.flags, location.latitude, location.longitude,
                 location.altitude, location.speed, location.bearing, location.accuracy,
                 location.timestamp, loc_get_position_sess_status_name(status));
#endif
    }
};

#endif

#endif /* LOC_ENG_MSG_H */
