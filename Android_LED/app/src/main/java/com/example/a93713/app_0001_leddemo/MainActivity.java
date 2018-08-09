package com.example.a93713.app_0001_leddemo;

import android.os.Bundle;
import android.os.IBinder;
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

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
//import com.example.hardlibrary.*;

//import android.os.ILedService;
//import android.os.ServiceManager;
import android.os.IBinder;

public class MainActivity extends AppCompatActivity {
    private boolean ledon = false;
    private Button button = null;
    private CheckBox checkBoxLed1 = null;
    private CheckBox checkBoxLed2 = null;
    private CheckBox checkBoxLed3 = null;
    private CheckBox checkBoxLed4 = null;

//    private  ILedService iLedService = null;
    Object proxy = null;
    Method ledCtrl = null;

    class MyButtonListener implements View.OnClickListener{
        public void onClick(View v) {

            //iLedService iLedService = new iLedService();

            ledon = !ledon;
            if(ledon){
                button.setText("ALL OFF");
                checkBoxLed1.setChecked(true);
                checkBoxLed2.setChecked(true);
                checkBoxLed3.setChecked(true);
                checkBoxLed4.setChecked(true);

                try {
                    for (int i = 0; i < 4; i++)
                        //iLedService.ledCtrl(i, 1);
                        //反射的方式访问硬件
                        ledCtrl.invoke(proxy, i, 1);
                } catch (IllegalAccessException e) {
                    e.printStackTrace();
                } catch (InvocationTargetException e) {
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
                        //iLedService.ledCtrl(i, 0);
                        //反射的方式访问硬件
                        ledCtrl.invoke(proxy, i, 0);
                } catch (IllegalAccessException e) {
                    e.printStackTrace();
                } catch (InvocationTargetException e) {
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
//                        iLedService.ledCtrl(0, 1);
                        //反射的方式访问硬件
                        ledCtrl.invoke(proxy, 0, 1);

                    } else {
                        Toast.makeText(getApplicationContext(), "LED1 OFF", Toast.LENGTH_SHORT).show();
//                        iLedService.ledCtrl(0, 0);
                        //反射的方式访问硬件
                        ledCtrl.invoke(proxy, 0, 0);
                    }

                case R.id.LED2:
                    if(checked) {
                        Toast.makeText(getApplicationContext(), "LED2 ON", Toast.LENGTH_SHORT).show();
//                        iLedService.ledCtrl(1, 1);
                        //反射的方式访问硬件
                        ledCtrl.invoke(proxy, 1, 1);
                    } else {
                        Toast.makeText(getApplicationContext(), "LED2 OFF", Toast.LENGTH_SHORT).show();
//                        iLedService.ledCtrl(1, 0);
                        //反射的方式访问硬件
                        ledCtrl.invoke(proxy, 1, 0);
                    }

                case R.id.LED3:
                    if(checked) {
                        Toast.makeText(getApplicationContext(), "LED3 ON", Toast.LENGTH_SHORT).show();
                        //iLedService.ledCtrl(2, 1);
                        //反射的方式访问硬件
                        ledCtrl.invoke(proxy, 2, 1);
                    } else {
                        Toast.makeText(getApplicationContext(), "LED3 OFF", Toast.LENGTH_SHORT).show();
//                        iLedService.ledCtrl(2, 0);
                        //反射的方式访问硬件
                        ledCtrl.invoke(proxy, 2, 0);
                    }

                case R.id.LED4:
                    if(checked) {
                        Toast.makeText(getApplicationContext(), "LED4 ON", Toast.LENGTH_SHORT).show();
//                        iLedService.ledCtrl(3, 1);
                        //反射的方式访问硬件
                        ledCtrl.invoke(proxy, 3, 1);
                    } else {
                        Toast.makeText(getApplicationContext(), "LED4 OFF", Toast.LENGTH_SHORT).show();
//                        iLedService.ledCtrl(3, 1);
                        //反射的方式访问硬件
                        ledCtrl.invoke(proxy, 3, 0);
                    }

                break;
            }
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        } catch (InvocationTargetException e) {
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

//        iLedService = ILedService.Stub.asInterface(ServiceManager.getService("led"));
        //使用反射的方法获取服务
        try {
            Method getService = Class.forName("android.os.ServiceManager").getMethod("getService", String.class);
            //getService本身是返回IBinder对象，不能忽略这里的getService是通过上面的Class.forName().getMethod得到的,它得到的Method类型
            //已经经过向上转换成为Object类型，所以只有通过强转才可以使用IBinder类型
            //IBinder ledService = (IBinder)getService.invoke(null, "led");
            Object ledService = getService.invoke(null, "led");
            Method asInterface = Class.forName("android.os.ILedService$Stub").getMethod("asInterface", IBinder.class);
            proxy = asInterface.invoke(null, ledService);
            ledCtrl = Class.forName("android.os.ILedService$Stub$Proxy").getMethod("ledCtrl", int.class, int.class);
        } catch (NoSuchMethodException e) {
            e.printStackTrace();
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        } catch (InvocationTargetException e) {
            e.printStackTrace();
        }

        try {
            //使用反射的方法操作硬件
            ledCtrl.invoke(proxy, 0, 1);
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        } catch (InvocationTargetException e) {
            e.printStackTrace();
        }

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
