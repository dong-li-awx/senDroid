package com.fudan.sensormonitor.model;

/**
 * Created by Spencer on 2017/7/4.
 */

public class GpsData extends Data{
    int usage;

    public int getUsage() {
        return usage;
    }

    public void setUsage(int usage) {
        this.usage = usage;
    }

    public GpsData(String time_start, String time_end, int usage, int uid) {
        this.time_start = time_start;
        this.time_end = time_end;
        this.usage = usage;
        this.uid = uid;
    }

}
