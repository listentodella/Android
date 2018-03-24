//package com.example.a93713.app_0002_lightsdemo;
package com.example.a93713.app_android_lights_system;
import android.app.Notification;
import android.app.NotificationManager;
import android.os.Handler;
import android.provider.Settings;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.SeekBar;

import com.example.a93713.app_android_lights_system.R;


public class MainActivity extends AppCompatActivity {

    private Button mLightButton = null;
    boolean flashing = false;
    final private int LED_NOTIFICATION_ID = 123;

    private SeekBar mBacklightSeekBar = null;

    private Handler mLightHandler = new Handler();
    private LightRunnable mLightRunnable = new LightRunnable();

    class LightRunnable implements Runnable {
        @Override
        public void run() {
            if(flashing) {
                FlashingLight();
            } else {
                ClearLED();
            }
        }
    }

    private void FlashingLight()
    {
        NotificationManager nm = (NotificationManager)getSystemService(NOTIFICATION_SERVICE);
        Notification notif = new Notification();
        notif.flags = Notification.FLAG_SHOW_LIGHTS;
        notif.ledARGB = 0xFF0000ff;
        notif.ledOnMS = 1000;
        notif.ledOffMS = 2000;
        nm.notify(LED_NOTIFICATION_ID, notif);
    }

    private void ClearLED()
    {
        NotificationManager nm = (NotificationManager)getSystemService(NOTIFICATION_SERVICE);
        nm.cancel(LED_NOTIFICATION_ID);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mBacklightSeekBar = (SeekBar)findViewById(R.id.seekBar);

        //get the value of the system and set the seekBar
        try {
            //if auto-backlight is enable, we can't adjust the backlight manual
            Settings.System.putInt(getContentResolver(),
                    Settings.System.SCREEN_BRIGHTNESS_MODE,
                    Settings.System.SCREEN_BRIGHTNESS_MODE_MANUAL);

            int brightness = android.provider.Settings.System.getInt(getContentResolver(),
                    android.provider.Settings.System.SCREEN_BRIGHTNESS);
            mBacklightSeekBar.setProgress(brightness  * 100 / 255);
        } catch (Settings.SettingNotFoundException e) {
            e.printStackTrace();
        }


        mBacklightSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                int brightness = mBacklightSeekBar.getProgress();
                brightness = brightness * 255 / 100;

                //then set backlight
                android.provider.Settings.System.putInt(getContentResolver(),
                        android.provider.Settings.System.SCREEN_BRIGHTNESS,
                        brightness);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

        mLightButton = (Button)findViewById(R.id.button);
        mLightButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v){
                //Perform action on click
                flashing = !flashing;
                if(flashing) {
                    mLightButton.setText("Stop Flashing Light ");
                } else {
                    mLightButton.setText("Flashing Light 20s Later");
                }

                mLightHandler.postDelayed(mLightRunnable, 20000);
            }
        });
    }
}
