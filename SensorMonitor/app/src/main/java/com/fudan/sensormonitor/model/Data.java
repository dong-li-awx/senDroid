package com.fudan.sensormonitor.model;

/**
 * Created by Spencer on 2017/7/4.
 */

public class Data {
    private static final long serialVersionUID=1L;
    String time_start;
    String time_end;
    int uid;

    public String getTime_start() {
        return time_start;
    }

    public void setTime_start(String time_start) {
        this.time_start = time_start;
    }

    public String getTime_end() {
        return time_end;
    }

    public void setTime_end(String time_end) {
        this.time_end = time_end;
    }

    public int getUid() {
        return uid;
    }

    public void setUid(int uid) {
        this.uid = uid;
    }

    public Data(){

    }
}
