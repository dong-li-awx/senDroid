package com.fudan.sensormonitor;

import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;


/**
 * An activity representing a list of Sensors. This activity
 * has different presentations for handset and tablet-size devices. On
 * handsets, the activity presents a list of items, which when touched,
 * lead to a {@link UsageSummaryActivity} representing
 * item details. On tablets, the activity presents the list of items and
 * item details side-by-side using two vertical panes.
 * <p/>
 * The activity makes heavy use of fragments. The list of items is a
 * {@link SensorListFragment} and the item details
 * (if present) is a {@link UsageSummaryFragment}.
 * <p/>
 * This activity also implements the required
 * {@link SensorListFragment.Callbacks} interface
 * to listen for item selections.
 */
public class SensorListActivity extends AppCompatActivity
        implements SensorListFragment.Callbacks {

    /**
     * Whether or not the activity is in two-pane mode, i.e. running on a tablet
     * device.
     */
    private boolean mTwoPane;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_sensor_app_bar);

        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        toolbar.setTitle(getTitle());

        if (findViewById(R.id.sensor_detail_container) != null) {
            // The detail container view will be present only in the
            // large-screen layouts (res/values-large and
            // res/values-sw600dp). If this view is present, then the
            // activity should be in two-pane mode.
            mTwoPane = true;

            // In two-pane mode, list items should be given the
            // 'activated' state when touched.
            ((SensorListFragment) getSupportFragmentManager()
                    .findFragmentById(R.id.sensor_list))
                    .setActivateOnItemClick(true);
        }

        // TODO: If exposing deep links into your app, handle intents here.
    }

    /**
     * Callback method from {@link SensorListFragment.Callbacks}
     * indicating that the item with the given ID was selected.
     */
    @Override
    public void onItemSelected(String id) {
        if (id.equalsIgnoreCase("gps") | id.equalsIgnoreCase("microphone")) {
            if (mTwoPane) {
                // In two-pane mode, show the detail view in this activity by
                // adding or replacing the detail fragment using a
                // fragment transaction.
                Bundle arguments = new Bundle();
                arguments.putString(UsageSummaryFragment.ARG_ITEM_ID, id);
                UsageSummaryFragment fragment = new UsageSummaryFragment();
                fragment.setArguments(arguments);
                getSupportFragmentManager().beginTransaction()
                        .replace(R.id.sensor_detail_container, fragment)
                        .commit();

            } else {
                // In single-pane mode, simply start the detail activity
                // for the selected item ID.
                Intent detailIntent = new Intent(this, UsageSummaryActivity.class);
                detailIntent.putExtra(UsageSummaryFragment.ARG_ITEM_ID, id);
                startActivity(detailIntent);
            }
        } else if (id.equalsIgnoreCase("CAMERA")) {
            Intent detailIntent = new Intent(this, CameraListActivity.class);
            detailIntent.putExtra(CameraListFragment.ARG_ITEM_ID, id);
            startActivity(detailIntent);

        } else {
            // In single-pane mode, simply start the detail activity
            // for the selected item ID.
            Intent detailIntent = new Intent(this, StandardSensorListActivity.class);
            detailIntent.putExtra(StandardSensorListFragment.ARG_ITEM_ID, id);
            startActivity(detailIntent);

        }

    }
}
