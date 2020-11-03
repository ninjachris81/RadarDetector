package com.ninjachris81.igeldetektor;

import android.annotation.SuppressLint;

import androidx.annotation.RequiresApi;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.NotificationCompat;
import androidx.core.app.NotificationManagerCompat;
import androidx.localbroadcastmanager.content.LocalBroadcastManager;

import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Matrix;
import android.media.RingtoneManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.provider.Settings;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.webkit.ConsoleMessage;
import android.webkit.WebChromeClient;
import android.webkit.WebResourceError;
import android.webkit.WebResourceRequest;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import org.json.JSONException;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Random;

/**
 * An example full-screen activity that shows and hides the system UI (i.e.
 * status bar and navigation/system bar) with user interaction.
 */
public class FullscreenActivity extends AppCompatActivity {

    private static final String TAG = "MainActivity";
    private static final String CHANNEL_ID = "IgelDetektor";
    private ImageView mIgel;
    private TextView mInfoText1;
    private TextView mInfoText2;
    private TextView mTs;

    boolean lastAlarmState = false;
    double lastAvg = 0.0;

    int currentNotificationId = 0;

    private NotificationCompat.Builder builder;

    private DateFormat dateFormat = new SimpleDateFormat("dd.MM.yyyy - HH:mm:ss");

    private BroadcastReceiver onNotice= new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, final Intent intent) {
            // intent can contain anydata

            runOnUiThread(new Runnable() {
                @RequiresApi(api = Build.VERSION_CODES.Q)
                @Override
                public void run() {
                    double avg = intent.getDoubleExtra("avg", 0);

                    // enable to simulate data
                    //avg = new Random().nextInt(20);

                    String dateText;
                    Date ts = new Date(intent.getLongExtra("ts", 0));

                    if (new Date().getTime() - ts.getTime() < 30 * 1000) {
                        dateText = "Just now";
                    } else {
                        dateText = dateFormat.format(ts);
                    }

                    mTs.setText(dateText);
                    mInfoText1.setText("Current: " + intent.getIntExtra("last", 0) + "\n" + "Avg: " + String.format("%.1f", avg)  + "\n" );
                    mInfoText2.setText("Min: " + intent.getIntExtra("min", 0) + "\n" + "Max: " + intent.getIntExtra("max", 0) + "\n");

                    double scale = avg / 20;

                    Log.d(TAG, "AVG: " + avg);

                    //mIgel.animateTransform(newMatrix);
                    mIgel.setScaleX((float)scale);
                    mIgel.setScaleY((float)scale);
                    mIgel.setAlpha((float)scale);

                    boolean isAlarm = avg > 11;

                    NotificationManagerCompat notificationManager = NotificationManagerCompat.from(getApplicationContext());
                    if (isAlarm && lastAlarmState!=isAlarm) {
                        currentNotificationId++;
                        notificationManager.notify(currentNotificationId, builder.build());
                    } else {
                        notificationManager.cancelAll();
                    }

                    lastAlarmState = isAlarm;
                }
            });

        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_fullscreen);

        createNotificationChannel();

        Uri alarmSound = RingtoneManager.getDefaultUri(RingtoneManager.TYPE_NOTIFICATION);

        builder = new NotificationCompat.Builder(getApplicationContext(), CHANNEL_ID)
                .setSmallIcon(R.drawable.igel_icon)
                .setContentTitle("Igel sind da")
                .setContentText("Party Party!")
                .setPriority(NotificationCompat.PRIORITY_DEFAULT)
                .setSound(alarmSound)
                .setOnlyAlertOnce(true)
        ;

        mIgel = findViewById(R.id.igel);
        mIgel.setScaleX(0.0f);
        mIgel.setScaleY(0.0f);

        mTs = findViewById(R.id.ts);
        mInfoText1 = findViewById(R.id.infoText1);
        mInfoText2 = findViewById(R.id.infoText2);

        startService(new Intent(getApplicationContext(), RestService.class));

        IntentFilter iff= new IntentFilter(RestService.NEW_DATA);
        LocalBroadcastManager.getInstance(this).registerReceiver(onNotice, iff);
    }

    private void createNotificationChannel() {
        // Create the NotificationChannel, but only on API 26+ because
        // the NotificationChannel class is new and not in the support library
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            CharSequence name = getString(R.string.channel_name);
            String description = getString(R.string.channel_description);
            int importance = NotificationManager.IMPORTANCE_DEFAULT;
            NotificationChannel channel = new NotificationChannel(CHANNEL_ID, name, importance);
            channel.setDescription(description);
            // Register the channel with the system; you can't change the importance
            // or other notification behaviors after this
            NotificationManager notificationManager = getSystemService(NotificationManager.class);
            notificationManager.createNotificationChannel(channel);
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
    }
}