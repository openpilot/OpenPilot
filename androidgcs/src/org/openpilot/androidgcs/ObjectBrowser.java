package org.openpilot.androidgcs;

import android.app.Activity;
import android.os.Bundle;

import android.widget.*;

public class ObjectBrowser extends Activity {
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        setContentView(R.layout.objectbrowser);
        ExpandableListAdapter mAdapter;
        ExpandableListView epView = (ExpandableListView) findViewById(R.id.objects);
        mAdapter = new ObjBrowserExpandableListAdapter(this);
       	epView.setAdapter(mAdapter);

    }
}