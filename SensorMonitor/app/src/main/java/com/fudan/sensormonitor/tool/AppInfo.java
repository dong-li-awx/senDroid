package com.fudan.sensormonitor.tool;

import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.graphics.drawable.Drawable;
import android.util.Log;

/**
 * Created by Spencer on 2017/7/6.
 */

public class AppInfo {
    public String appname = "";
    public String pname = "";
    public String versionName = "";
    public int versionCode = 0;
    public int uid;
    public Drawable icon;
    static PackageManager pm;

    public void pPrint() {
        Log.i("taskmanager", appname + "\t" + pname + "\t" + versionName + "\t" + versionCode + "\t");
    }

    public static Drawable getIcon(String packageName) throws NameNotFoundException {
        //pm = getPackageManager();
        ApplicationInfo info = pm.getApplicationInfo(packageName, 0);
        return info.loadIcon(pm);
    }

    public String getAppname() {
        return appname;
    }

    public void setAppname(String appname) {
        this.appname = appname;
    }

    public String getPname() {
        return pname;
    }

    public void setPname(String pname) {
        this.pname = pname;
    }

    public String getVersionName() {
        return versionName;
    }

    public void setVersionName(String versionName) {
        this.versionName = versionName;
    }

    public int getVersionCode() {
        return versionCode;
    }

    public void setVersionCode(int versionCode) {
        this.versionCode = versionCode;
    }

    public int getUid() {
        return uid;
    }

    public void setUid(int uid) {
        this.uid = uid;
    }

    public Drawable getIcon() {
        return icon;
    }

    public void setIcon(Drawable icon) {
        this.icon = icon;
    }

}
