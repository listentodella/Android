package com.example.a93713.app_addons_0001_message;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.Button;
import android.view.View;
import android.util.Log;
import android.os.HandlerThread;

public class MainActivity extends AppCompatActivity {

	private Button mButton;
	private final String TAG = "MessageTest";
	private int ButtonCount = 0;
	private Thread myThread;
	private Thread myThread2;
	private Handler mHandler;
	private Handler mHandler3;
	private int mMessageCount = 0;
	private HandlerThread myThread3;

	class MyRunnable implements Runnable {
		public void run() {
			int count = 0;
			for(;;) {
				Log.d(TAG, "MyThread " + count);
				count++;
				try {
					Thread.sleep(3000);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
		}
	}

	class MyThread extends Thread {
		private Looper mLooper;

		public void run() {
			super.run();
			Looper.prepare();
			synchronized (this){
				mLooper = Looper.myLooper();
				notifyAll();
			}
			mLooper = Looper.myLooper();
			Looper.loop();
		}

		public Looper getLooper() {
			if(!isAlive()) {
				return null;
			}

			synchronized (this) {
				while(isAlive() && mLooper == null) {
					try {
						wait();
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}
			}

			return Looper.myLooper();
		}
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		mButton = (Button)findViewById(R.id.button);
		mButton.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				Log.d(TAG, "Send Message " +ButtonCount);
				ButtonCount++;
				Message msg = new Message();
				mHandler.sendMessage(msg);

				mHandler3.post(new Runnable() {
					@Override
					public void run() {
						Log.d(TAG, "get Message for Thread3 "+ mMessageCount);
						mMessageCount++;
					}
				});
			}
		});

		myThread = new Thread(new MyRunnable(), "MessageTestThread");
		myThread.start();

		myThread2 = new MyThread();
		myThread2.start();

		mHandler = new Handler(myThread2.getLooper(), new Handler.Callback() {
			@Override
			public boolean handleMessage(Message message) {
				Log.d(TAG, "get Message "+ mMessageCount);
				mMessageCount++;
				return false;
			}
		});

		myThread3 = new HandlerThread("MessageTestThread3");
		myThread3.start();
		mHandler3 = new Handler(myThread3.getLooper());

	}
}
