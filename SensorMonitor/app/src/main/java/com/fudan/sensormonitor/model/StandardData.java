package com.fudan.sensormonitor.model;

/**
 * Created by Spencer on 2017/7/4.
 */

public class StandardData extends Data{
    /*
        0:accelerometer
        1:pressure
        2:light
        3:magnetic
        4:proximity
        5:gyroscope
         */
    String type;

    public String getType() {
        return type;
    }

    public void setType(String type) {
        this.type = type;
    }

    public StandardData(String time_start, String time_end, String type,int uid) {
        this.time_start = time_start;
        this.time_end = time_end;
        this.type = type;
        this.uid = uid;
    }
}
