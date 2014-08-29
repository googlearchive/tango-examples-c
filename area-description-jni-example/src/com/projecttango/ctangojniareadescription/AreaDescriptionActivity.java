package com.projecttango.ctangojniareadescription;

import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.app.Activity;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.RelativeLayout;
import android.widget.TextView;

public class AreaDescriptionActivity extends Activity {
	GLSurfaceView glView;
	RelativeLayout layout;
	TextView device2StartText;
	TextView device2ADFText;
	TextView start2ADFText;
	TextView adf2StartText;
	
	TextView learningModeText;
	TextView uuidText;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setContentView(R.layout.activity_area_description);
		glView = (GLSurfaceView) findViewById(R.id.surfaceview);
		glView.setRenderer(new Renderer());

		device2StartText = (TextView) findViewById(R.id.device_start);
		device2ADFText = (TextView) findViewById(R.id.device_adf);
		start2ADFText = (TextView) findViewById(R.id.start_adf);
		start2ADFText = (TextView) findViewById(R.id.adf_start);
		
		learningModeText = (TextView) findViewById(R.id.learning_mode);
		uuidText = (TextView) findViewById(R.id.uuid);
		
		final Button button = (Button) findViewById(R.id.save_adf);
		button.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                TangoJNINative.SaveADF();
            }
        });
		
		final Button button1 = (Button) findViewById(R.id.remove_adf);
		button1.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                TangoJNINative.RemoveAllAdfs();
            }
        });
		
		new Thread(new Runnable() {
			@Override
			public void run() {
				while (true) {
					try {
						Thread.sleep(10);
						final String d_t_s = String.valueOf(TangoJNINative.GetCurrentTimestamp(0));
						final String d_t_a = String.valueOf(TangoJNINative.GetCurrentTimestamp(1));
						final String s_t_a = String.valueOf(TangoJNINative.GetCurrentTimestamp(2));
						final String a_t_s = String.valueOf(TangoJNINative.GetCurrentTimestamp(3));
						runOnUiThread(new Runnable() {
							@Override
							public void run() {
								try {
									device2StartText.setText(d_t_s);
									device2ADFText.setText(d_t_a);
									start2ADFText.setText(s_t_a);
									start2ADFText.setText(a_t_s);
									
									learningModeText.setText(String.valueOf(TangoJNINative.GetEnabledLearn()));
									uuidText.setText(TangoJNINative.GetUUID());

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
	}

	@Override
	protected void onPause() {
		super.onPause();
		TangoJNINative.OnPause();
	}

	protected void onDestroy() {
		super.onDestroy();
		TangoJNINative.OnDestroy();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		getMenuInflater().inflate(R.menu.area_description, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) {
		case R.id.load_adf:
			TangoJNINative.OnCreate(0);
			TangoJNINative.OnResume();
			return true;
		case R.id.record_adf:
			TangoJNINative.OnCreate(1);
			TangoJNINative.OnResume();
			return true;
		default:
			return super.onOptionsItemSelected(item);
		}
	}
}
