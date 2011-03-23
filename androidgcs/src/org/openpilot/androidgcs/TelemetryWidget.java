package org.openpilot.androidgcs;

import android.appwidget.AppWidgetManager;
import android.appwidget.AppWidgetProvider;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.widget.RemoteViews;

public class TelemetryWidget extends AppWidgetProvider {
	
	@Override
	public void onReceive(Context context, Intent intent) {
		if(intent.getAction().equals(OPTelemetryService.INTENT_ACTION_CONNECTED)) {
			changeStatus(context, true);
		}
		if(intent.getAction().equals(OPTelemetryService.INTENT_ACTION_DISCONNECTED)) {
			changeStatus(context, false);
		}		
        
		super.onReceive(context, intent);
	}
	
	public void changeStatus(Context context, boolean status) {
		RemoteViews updateViews =  new RemoteViews(context.getPackageName(), R.layout.telemetry_widget);
		updateViews.setTextViewText(R.id.telemetryWidgetStatus, "Connection status: " + status);
		ComponentName thisWidget = new ComponentName(context, TelemetryWidget.class);
        AppWidgetManager manager = AppWidgetManager.getInstance(context);
        manager.updateAppWidget(thisWidget, updateViews);

	}

	public void onUpdate(Context context, AppWidgetManager appWidgetManager, int[] appWidgetIds) {
        final int N = appWidgetIds.length;

        // Perform this loop procedure for each App Widget that belongs to this provider
        for (int i=0; i<N; i++) {
            int appWidgetId = appWidgetIds[i];

            // Get the layout for the App Widget and attach an on-click listener to the button
            RemoteViews views = new RemoteViews(context.getPackageName(), R.layout.telemetry_widget);

            // Tell the AppWidgetManager to perform an update on the current App Widget
            appWidgetManager.updateAppWidget(appWidgetId, views);
        }
    }
    

}
