package com.projecttango.ctangojniareadescription;

import android.os.Bundle;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.RelativeLayout;
import android.widget.TextView;

@SuppressLint("ResourceAsColor")
public class AreaDescriptionActivity extends Activity {
	AreaDescriptionView areaDescriptionView;
	RelativeLayout layout;
	TextView text;
	
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		layout = new RelativeLayout(this);
		areaDescriptionView = new AreaDescriptionView(getApplication());
		
		text= new TextView(this);
		text.setText("Status");
		text.setWidth(250);
		text.setHeight(70);
		text.setPadding(10, 10, 10, 10);
		text.setBackgroundColor(R.color.black);
		
		layout.addView(areaDescriptionView);
		layout.addView(text);
		
		setContentView(layout);
		TangoJNINative.OnCreate();
		
		new Thread(new Runnable() {
			@Override
			public void run() {
				while(true){
					try {
						Thread.sleep(10);
						runOnUiThread(new Runnable() {
							@Override
							public void run() {
								try {
									String status = "";
									int statusValue = TangoJNINative.GetCurrentStatus();
									switch (statusValue) {
									case 0:
										status = "Status: Initializing";
										break;
									case 1:
										status = "Status: Valid";
										break;
									case 2:
										status = "Status: Invalid";
										break;
									default:
										status = "Status: Unknown";
										break;
									}
									text.setText(status);
								} catch (Exception e) {
									e.printStackTrace();
								}
							}
						});
					} catch (Exception e) {
						e.printStackTrace();
					}
				}			
			}
		}).start();
	}

	@Override
	protected void onResume() {
		super.onResume();
		areaDescriptionView.onResume();
		TangoJNINative.OnResume();
	}

	@Override
	protected void onPause() {
		super.onPause();
		areaDescriptionView.onPause();
		TangoJNINative.OnPause();
	}

	protected void onDestroy() {
		super.onDestroy();
		TangoJNINative.OnDestroy();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.area_description, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) {
		case R.id.action_first_camera:
			TangoJNINative.SetCamera(0);
			return true;
		case R.id.action_third_camera:
			TangoJNINative.SetCamera(1);
			return true;
		case R.id.action_top_camera:
			TangoJNINative.SetCamera(2);
			return true;
		default:
			return super.onOptionsItemSelected(item);
		}
	}
}
