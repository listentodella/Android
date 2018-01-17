package com.example.a93713.app_addons_0001_message;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.Button;
import android.view.View;
import android.util.Log;

public class MainActivity extends AppCompatActivity {

	private Button mButton;
	private final String TAG = "MessageTest";
	private int ButtonCount = 0;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		mButton = (Button)findViewById(R.id.button);
		mButton.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				Log.d(TAG, "Send Message " +ButtonCount);
				ButtonCount++;
			}
		});
	}
}
