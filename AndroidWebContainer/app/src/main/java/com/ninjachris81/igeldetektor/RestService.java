package com.ninjachris81.igeldetektor;

import android.app.Activity;
import android.app.DownloadManager;
import android.app.Service;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.IBinder;
import android.util.Log;
import android.widget.TextView;

import com.android.volley.Request;
import com.android.volley.RequestQueue;
import com.android.volley.Response;
import com.android.volley.VolleyError;
import com.android.volley.toolbox.JsonObjectRequest;
import com.android.volley.toolbox.Volley;

import org.json.JSONException;
import org.json.JSONObject;

import androidx.localbroadcastmanager.content.LocalBroadcastManager;

public class RestService extends Service {

    public static final String INTENT_FILTER = "new_data";
    public static final IntentFilter NEW_DATA = new IntentFilter(INTENT_FILTER);

    private static final String TAG = "RestService";

    private Handler handler = new Handler();

    RequestQueue requestQueue;

    private Runnable runnableCode = new Runnable() {
        @Override
        public void run() {
            updateData();
            handler.postDelayed(this, 10000);
        }
    };

    public RestService() {
        Log.d(TAG, "New Rest Service");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId){
        Log.d(TAG, "New Rest Service started");

        requestQueue = Volley.newRequestQueue(this);

        //onTaskRemoved(intent);

        handler.post(runnableCode);

        return START_STICKY;
    }

    @Override
    public IBinder onBind(Intent arg0) {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public void onTaskRemoved(Intent rootIntent) {
        Intent restartServiceIntent = new Intent(this ,this.getClass());
        restartServiceIntent.setPackage(getPackageName());
        startService(restartServiceIntent);
        super.onTaskRemoved(rootIntent);
    }

    private void updateData() {
        Log.d(TAG, "updateData");

        JsonObjectRequest jsonObjectRequest = new JsonObjectRequest
                (Request.Method.GET, "http://rpi-server.fritz.box:9090/data/60", null, new Response.Listener<JSONObject>() {

                    @Override
                    public void onResponse(final JSONObject response) {
                        Log.d(TAG, "response: " + response.toString());

                        if (!response.isNull("ts")) {
                            Intent intent = new Intent(INTENT_FILTER);
                            try {
                                intent.putExtra("ts", response.getLong("ts"));
                                intent.putExtra("last", response.getInt("last"));
                                intent.putExtra("min", response.getInt("min"));
                                intent.putExtra("avg", response.getDouble("avg"));
                                intent.putExtra("max", response.getInt("max"));
                            } catch (JSONException e) {
                                e.printStackTrace();
                            }

                            LocalBroadcastManager.getInstance(getApplicationContext()).sendBroadcast(intent);
                        } else {
                            // nothing
                        }
                }
                }, new Response.ErrorListener() {

                    @Override
                    public void onErrorResponse(VolleyError error) {
                        // TODO: Handle error
                        Log.e(TAG, "Failed to request " + error.toString());
                    }
                });

        requestQueue.add(jsonObjectRequest);
    }
}
