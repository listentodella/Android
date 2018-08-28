package com.example.a93713.app_0001_leddemo;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.widget.Toast;

/**
 * Created by 93713 on 2018/8/9.
 */

public class MyBroadcastReceiver extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
        Toast.makeText(context, "Get BroadcastReceiver", Toast.LENGTH_SHORT).show();

        //在BroadcastReceiver中启动Activity的
        Intent intentNewTask = new Intent(context,MainActivity.class);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        context.startActivity(intentNewTask);
    }
}

