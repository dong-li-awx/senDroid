package com.fudan.sensormonitor;
import android.util.Log;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.os.Bundle;
import android.os.SystemClock;
import android.support.v4.app.Fragment;
import android.support.v7.app.AppCompatActivity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.graphics.drawable.ColorDrawable;
import android.graphics.drawable.Drawable;
import android.widget.ImageView;
import android.content.pm.PackageInfo;
import android.content.pm.ApplicationInfo;
import android.widget.ListView;
import android.widget.SimpleAdapter;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;


import lecho.lib.hellocharts.gesture.ContainerScrollType;
import lecho.lib.hellocharts.gesture.ZoomType;
import lecho.lib.hellocharts.model.Axis;
import lecho.lib.hellocharts.model.AxisValue;
import lecho.lib.hellocharts.model.Line;
import lecho.lib.hellocharts.model.LineChartData;
import lecho.lib.hellocharts.model.PointValue;
import lecho.lib.hellocharts.model.ValueShape;
import lecho.lib.hellocharts.model.Viewport;
import lecho.lib.hellocharts.view.LineChartView;

import android.widget.SimpleAdapter.ViewBinder;
import android.graphics.Canvas;

import com.fudan.sensormonitor.tool.AppInfo;
import com.fudan.sensormonitor.tool.xmlReader1;
import com.fudan.sensormonitor.tool.xmlReader2;
import com.fudan.sensormonitor.usage.UsageContent;

/**
 * Created by Spencer on 2017/7/9.
 */

public class CameraUsageFragment extends Fragment {
    private LineChartView lineChartView;
    public static final String ARG_ITEM_ID = "item_id";
    String[] time = {"-24", "-23", "-22", "-21", "-20", "-19", "-18", "-17", "-16", "-15", "-14", "-13", "-12",
            "-11", "-10", "-9", "-8", "-7", "-6", "-5", "-4", "-3", "-2", "-1", "0"};//X轴的标注

    int[] score;
    HashMap usageMap;
    int[] colors = new int[]{Color.parseColor("#99cc33"), Color.parseColor("#6699ff"),
            Color.parseColor("#da70d6"), Color.parseColor("#f6c390"), Color.parseColor("#b0e0e6"),
            Color.parseColor("#ffcc00"), Color.parseColor("#808080"), Color.parseColor("#990099"),
            Color.parseColor("#ffccff"), Color.parseColor("#660000"), Color.parseColor("#0000ff"),
            Color.parseColor("#cc0000"), Color.parseColor("#330033"), Color.parseColor("#006699"),
            Color.parseColor("#3399ff"), Color.parseColor("#cc6666")};


    ArrayList<Line> lines = new ArrayList<Line>();
    private List<PointValue> mPointValues = new ArrayList<PointValue>();
    private List<AxisValue> mAxisXValues = new ArrayList<AxisValue>();

    private ListView listView = null;
    private static ArrayList<Map<String, Object>> appData;
    static Map<String, Object> item;
    SimpleAdapter adapter;

    public CameraUsageFragment() {
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (getArguments().containsKey(ARG_ITEM_ID)) {
            usageMap = getUsage(getArguments().getString(ARG_ITEM_ID));
            score = (int[]) usageMap.get("data");
        }
        Log.i("","On create------------------------------------------");
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        Log.i("","On createview------------------------------------------");
        View rootView = inflater.inflate(R.layout.fragment_camera_detail, container, false);
        appData = new ArrayList<Map<String, Object>>();
        ArrayList<Map<String, Object>> appView = new ArrayList<Map<String, Object>>();
        HashMap<String, Object> temp = new HashMap<String, Object>();
        listPackages();

        lineChartView = (LineChartView) rootView.findViewById(R.id.camera_line_chart);
        listView = (ListView) rootView.findViewById(R.id.camera_listview);
        getAxisXLables();//获取x轴的标注
        int i = 0;
        lines = initLine(lines, score, i, 4);
        Iterator iter = ((HashMap) usageMap.get("appList")).entrySet().iterator();
        while (iter.hasNext()) {
            i++;
            Map.Entry entry = (Map.Entry) iter.next();
            int uid = (int) entry.getKey();
            for (int j = 0; j < appData.size(); j++) {
                if (uid == (int) appData.get(j).get("uid")) {
                    temp = new HashMap<String, Object>();
                    temp.put("icon", (Object) appData.get(j).get("icon"));
                    temp.put("appname", (String) appData.get(j).get("appname"));
                    temp.put("pname", (String) appData.get(j).get("pname"));

                    ColorDrawable drawable = new ColorDrawable(colors[i % 16]);
                    Bitmap bitmap = Bitmap.createBitmap(10, 25,
                            Bitmap.Config.ARGB_8888);
                    bitmap.eraseColor(colors[i % 16]);//填充颜色
                    Canvas canvas = new Canvas(bitmap);
                    drawable.draw(canvas);
                    temp.put("color", drawable);
                    appView.add(temp);
                    break;
                }
            }
            int[] subscore = (int[]) entry.getValue();
            lines = initLine(lines, subscore, i, 2);
            i++;
        }
        initChart(lines);

        adapter = new SimpleAdapter(this.getActivity(), appView, R.layout.app_list_entry,
                new String[]{"icon", "appname", "pname", "color"},
                new int[]{R.id.app_list_entry_icon, R.id.app_list_entry_title, R.id.app_list_entry_subtitle, R.id.app_list_entry_color});
        listView.setAdapter(adapter);
        adapter.setViewBinder(new ViewBinder() {
            public boolean setViewValue(View view, Object data, String textRepresentation) {
                if (view instanceof ImageView && data instanceof Drawable) {
                    ImageView iv = (ImageView) view;
                    iv.setImageDrawable((Drawable) data);
                    return true;
                } else return false;
            }
        });

        return rootView;
    }


    private void getAxisXLables() {
        for (int i = 0; i < time.length; i++) {
            mAxisXValues.add(new AxisValue(i).setLabel(time[i]));
        }
    }

    /**
     * 图表的每个点的显示
     */
    private void getAxisPoints(int[] scores) {
        mPointValues = new ArrayList<PointValue>();
        for (int i = 0; i < scores.length; i++) {
            mPointValues.add(new PointValue(i, scores[i]));
        }
    }

    private ArrayList<Line> initLine(ArrayList<Line> lines, int[] score, int color, int stroke) {
        getAxisPoints(score);
        Line line = new Line(mPointValues);
        if (color == 0) {
            line.setColor(Color.parseColor("#ff0000"));
        } else {
            line.setColor(colors[color % 16]);
        }
        line.setShape(ValueShape.CIRCLE);//折线图上每个数据点的形状  这里是圆形 （有三种 ：ValueShape.SQUARE  ValueShape.CIRCLE  ValueShape.DIAMOND）
        line.setCubic(true);//曲线是否平滑，即是曲线还是折线
        line.setFilled(false);//是否填充曲线的面积
        line.setStrokeWidth(stroke);
        //line.setHasLabels(false);//曲线的数据坐标是否加上备注
        line.setHasLabelsOnlyForSelected(true);//点击数据坐标提示数据（设置了这个line.setHasLabels(true);就无效）
        line.setHasLines(true);//是否用线显示。如果为false 则没有曲线只有点显示
        line.setHasPoints(true);//是否显示圆点 如果为false 则没有原点只有点显示（每个数据点都是个大的圆点）
        line.setPointRadius(5);
        if (stroke == 2) {
            line.setShape(ValueShape.SQUARE);
            line.setHasPoints(true);
            line.setPointRadius(2);
        }
        lines.add(line);
        return lines;
    }

    private void initChart(ArrayList<Line> lines) {
        LineChartData data = new LineChartData();
        data.setLines(lines);

        //坐标轴
        Axis axisX = new Axis(); //X轴
        axisX.setHasTiltedLabels(true);  //X坐标轴字体是斜的显示还是直的，true是斜的显示
        axisX.setTextColor(Color.GRAY);  //设置字体颜色
        axisX.setName(getArguments().getString(ARG_ITEM_ID) + " usages in the past 24 hours.(The total usage is RED)");  //表格名称
        axisX.setTextSize(10);//设置字体大小
        axisX.setMaxLabelChars(10); //最多几个X轴坐标，意思就是你的缩放让X轴上数据的个数7<=x<=mAxisXValues.length
        axisX.setValues(mAxisXValues);  //填充X轴的坐标名称
        //axisX.setName("hour(s)");
        data.setAxisXBottom(axisX); //x 轴在底部
        //data.setAxisXTop(axisX);  //x 轴在顶部
        axisX.setHasLines(true); //x 轴分割线

        // Y轴是根据数据的大小自动设置Y轴上限(在下面我会给出固定Y轴数据个数的解决方案)
        Axis axisY = new Axis();  //Y轴
        if(getArguments().getString(ARG_ITEM_ID).equalsIgnoreCase("preview")){
            axisY.setName("(k)frame(s)");
        }else if (getArguments().getString(ARG_ITEM_ID).equalsIgnoreCase("record")) {
            axisY.setName("(M)byte(s)");
        }else{
            axisY.setName("");
        }
        axisY.setTextSize(10);//设置字体大小
        data.setAxisYLeft(axisY);  //Y轴设置在左边
        //data.setAxisYRight(axisY);  //y轴设置在右边


        //设置行为属性，支持缩放、滑动以及平移
        lineChartView.setInteractive(true);
        lineChartView.setZoomType(ZoomType.HORIZONTAL);
        lineChartView.setMaxZoom((float) 2);//最大方法比例
        lineChartView.setContainerScrollEnabled(true, ContainerScrollType.HORIZONTAL);
        lineChartView.setLineChartData(data);
        lineChartView.setVisibility(View.VISIBLE);
        /**注：下面的7，10只是代表一个数字去类比而已
         * 当时是为了解决X轴固定数据个数。见（http://forum.xda-developers.com/tools/programming/library-hellocharts-charting-library-t2904456/page2）;
         */
        Viewport v = new Viewport(lineChartView.getMaximumViewport());
        v.left = 0;
        v.right = 7;
        lineChartView.setCurrentViewport(v);
    }

    private HashMap getUsage(String sensorType) {
        Process process = null;
        try {
            process = Runtime.getRuntime().exec("su");
        } catch (IOException e) {
            e.printStackTrace();
        }
        HashMap usageMap = xmlReader2.readFromXML("/data/senDroid/sensor_usage.xml", "camera", getArguments().getString(ARG_ITEM_ID));

        return usageMap;
    }

    private void listPackages() {
        ArrayList<AppInfo> apps = getInstalledApps(true);

        for (int i = 0; i < apps.size(); i++) {
            apps.get(i).pPrint();
            item = new HashMap<String, Object>();
            item.put("appname", apps.get(i).appname);
            item.put("pname", apps.get(i).pname);
            item.put("icon", apps.get(i).icon);
            item.put("status", "");
            item.put("uid", apps.get(i).uid);
            appData.add(item);
            //System.out.print("app: "+appData.get(i).get("appname") + "\t" + appData.get(i).get("pname") + "\t" + appData.get(i).get("icon"));
        }
    }

    private ArrayList<AppInfo> getInstalledApps(boolean getSysPackages) {
        ArrayList<AppInfo> res = new ArrayList<AppInfo>();
        List<PackageInfo> appList = this.getActivity().getPackageManager().getInstalledPackages(0);
        for (int i = 0; i < appList.size(); i++) {
            PackageInfo info = appList.get(i);
            if (getSysPackages == false && (info.applicationInfo.flags & ApplicationInfo.FLAG_SYSTEM) != 0) {
                continue;
            }
            AppInfo newInfo = new AppInfo();
            newInfo.appname = info.applicationInfo.loadLabel(this.getActivity().getPackageManager()).toString();
            newInfo.pname = info.packageName;
            newInfo.versionName = info.versionName;
            newInfo.versionCode = info.versionCode;
            newInfo.icon = info.applicationInfo.loadIcon(this.getActivity().getPackageManager());
            newInfo.uid = info.applicationInfo.uid;
            res.add(newInfo);
        }
        return res;
    }
}
