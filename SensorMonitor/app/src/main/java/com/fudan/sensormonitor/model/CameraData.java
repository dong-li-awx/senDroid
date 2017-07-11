package com.fudan.sensormonitor.model;

/**
 * Created by Spencer on 2017/7/4.
 */

public class CameraData extends Data{
    int usage;
    //int type;// 0 for recording and 1 for previewing.

    public int getUsage() {
        return usage;
    }

    public void setUsage(int usage) {
        this.usage = usage;
    }

/*
    public int getType() {
        return type;
    }

    public void setType(int type) {
        this.type = type;
    }*/

    public CameraData(String time_start, String time_end, int usage, int uid) {
        this.time_start = time_start;
        this.time_end = time_end;
        this.usage = usage;
        this.uid = uid;
    }
}
