/**
 ******************************************************************************
 * @file       TelemetryWidget.java
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      A widget that shows the status of telemetry.
 * @see        The GNU Public License (GPL) Version 3
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
package org.openpilot.androidgcs;

import org.openpilot.androidgcs.telemetry.OPTelemetryService;

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

	@Override
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
