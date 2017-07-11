package com.fudan.sensormonitor.usage;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Helper class for providing sample content for user interfaces created by
 * Android template wizards.
 */
public class UsageContent {

    private static UsageContent instance;
    private UsageContent(){}
    public static synchronized UsageContent getInstance(String[] sensorsName) {
        if (instance == null) {
            instance = new UsageContent();
            for (int i = 0; i < sensorsName.length; i++) {
                addItem(createDummyItem(sensorsName[i]));
            }
        }
        return instance;
    }

    /**
     * An array of sample (dummy) items.
     */
    public static List<UsageItem> SENSORS = new ArrayList<UsageItem>();

    /**
     * A map of sample (dummy) items, by ID.
     */
    public static Map<String, UsageItem> SENSOR_MAP = new HashMap<String, UsageItem>();

    private static void addItem(UsageItem item) {
        SENSORS.add(item);
        SENSOR_MAP.put(item.id, item);
    }

    private static UsageItem createDummyItem(String sensor) {
        return new UsageItem(sensor, sensor, sensor);
    }

    /**
     * A dummy item representing a piece of content.
     */
    public static class UsageItem {
        public String id;
        public String content;
        public String details;

        public UsageItem(String id, String content, String details) {
            this.id = id;
            this.content = content;
            this.details = details;
        }

        @Override
        public String toString() {
            return content;
        }
    }
}
