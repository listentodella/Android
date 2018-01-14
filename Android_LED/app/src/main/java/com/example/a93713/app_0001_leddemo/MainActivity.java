package com.example.a93713.app_0001_leddemo;

import android.os.Bundle;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.Toast;
//import com.example.hardlibrary.*;

import android.os.ILedService;

public class MainActivity extends AppCompatActivity {
    private boolean ledon = false;
    private Button button = null;
    private CheckBox checkBoxLed1 = null;
    private CheckBox checkBoxLed2 = null;
    private CheckBox checkBoxLed3 = null;
    private CheckBox checkBoxLed4 = null;

    private  ILedService iLedService = null;


    class MyButtonListener implements View.OnClickListener{
        public void onClick(View v) {

            iLedService iLedService = new iLedService();

            ledon = !ledon;
            if(ledon){
                button.setText("ALL OFF");
                checkBoxLed1.setChecked(true);
                checkBoxLed2.setChecked(true);
                checkBoxLed3.setChecked(true);
                checkBoxLed4.setChecked(true);
                try {
                    for(int i = 0; i < 4; i++)
                        iLedService.ledCtrl(i, 1);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }else {
                button.setText("ALL ON");
                checkBoxLed1.setChecked(false);
                checkBoxLed2.setChecked(false);
                checkBoxLed3.setChecked(false);
                checkBoxLed4.setChecked(false);
                try {
                    for(int i = 0; i < 4; i++)
                        iLedService.ledCtrl(i, 0);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }
    }

    public void onCheckboxClicked(View view){
        //is the view now checked?
        boolean checked = ((CheckBox) view).isChecked();

        try {
            //check which checkbox was clicked
            switch (view.getId()) {
                case R.id.LED1:
                    if(checked) {
                        Toast.makeText(getApplicationContext(), "LED1 ON", Toast.LENGTH_SHORT).show();
                        iLedService.ledCtrl(0, 1);
                    } else {
                        Toast.makeText(getApplicationContext(), "LED1 OFF", Toast.LENGTH_SHORT).show();
                        iLedService.ledCtrl(0, 0);
                    }

                case R.id.LED2:
                    if(checked) {
                        Toast.makeText(getApplicationContext(), "LED2 ON", Toast.LENGTH_SHORT).show();
                        iLedService.ledCtrl(1, 1);
                    } else {
                        Toast.makeText(getApplicationContext(), "LED2 OFF", Toast.LENGTH_SHORT).show();
                        iLedService.ledCtrl(1, 1);
                    }

                case R.id.LED3:
                    if(checked) {
                        Toast.makeText(getApplicationContext(), "LED3 ON", Toast.LENGTH_SHORT).show();
                        iLedService.ledCtrl(2, 1);
                    } else {
                        Toast.makeText(getApplicationContext(), "LED3 OFF", Toast.LENGTH_SHORT).show();
                        iLedService.ledCtrl(2, 1);
                    }

                case R.id.LED4:
                    if(checked) {
                        Toast.makeText(getApplicationContext(), "LED4 ON", Toast.LENGTH_SHORT).show();
                        iLedService.ledCtrl(3, 1);
                    } else {
                        Toast.makeText(getApplicationContext(), "LED4 OFF", Toast.LENGTH_SHORT).show();
                        iLedService.ledCtrl(3, 1);
                    }

                break;
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }



    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

       FloatingActionButton fab = (FloatingActionButton) findViewById(R.id.fab);
        fab.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Snackbar.make(view, "Replace with your own action", Snackbar.LENGTH_LONG)
                        .setAction("Action", null).show();
            }
        });
        /*利用Find id 获取实例化对象*/
        button = (Button)findViewById(R.id.BUTTON);

        iLedService = ILedService.Stub.asInterface(ServiceManager.getService("led"));

        /*静态方法不需要实例化对象*/
        //iLedService.ledOpen();

        checkBoxLed1 = (CheckBox)findViewById(R.id.LED1);
        checkBoxLed2 = (CheckBox)findViewById(R.id.LED2);
        checkBoxLed3 = (CheckBox)findViewById(R.id.LED3);
        checkBoxLed4 = (CheckBox)findViewById(R.id.LED4);

        button.setOnClickListener(new MyButtonListener());
/*
        button.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                // Code here executes on main thread after user presses button
                ledon = !ledon;
                if(ledon)
                    button.setText("ALL OFF");
                else
                    button.setText("ALL ON");
            }
        });
*/


    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }
}
