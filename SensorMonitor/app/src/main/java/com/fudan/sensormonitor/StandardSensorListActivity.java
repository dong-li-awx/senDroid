package com.fudan.sensormonitor;

import android.content.Intent;
import android.os.Bundle;
import android.os.SystemClock;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.MenuItem;
import android.util.Log;
/**
 * Created by Spencer on 2017/7/7.
 */

public class StandardSensorListActivity extends AppCompatActivity implements StandardSensorListFragment.Callbacks {

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        /*setContentView(R.layout.activity_standard_list);

            ((StandardSensorListFragment) getSupportFragmentManager()
                    .findFragmentById(R.id.standard_sensor_list))
                    .setActivateOnItemClick(true);*/
        setContentView(R.layout.activity_standard_sensor_app_bar);

        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        toolbar.setTitle("Sensor Type");
       /* ((StandardSensorListFragment) getSupportFragmentManager()
                .findFragmentById(R.id.standard_list))
                .setActivateOnItemClick(true);*/

    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int id = item.getItemId();
        if (id == android.R.id.home) {
            // This ID represents the Home or Up button. In the case of this
            // activity, the Up button is shown. For
            // more details, see the Navigation pattern on Android Design:
            //
            // http://developer.android.com/design/patterns/navigation.html#up-vs-back
            //
            navigateUpTo(new Intent(this, SensorListActivity.class));
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    public void onItemSelected(String id) {
        Intent detailIntent = new Intent(this, StandardSensorUsageActivity.class);
        detailIntent.putExtra(StandardSensorUsageFragment.ARG_ITEM_ID, id);
        startActivity(detailIntent);
        Log.i("","jjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjj");
    }
}
