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
	TextView relocalizeText;
	TextView uuidText;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setContentView(R.layout.activity_area_description);
		glView = (GLSurfaceView) findViewById(R.id.surfaceview);
		glView.setRenderer(new Renderer());

		relocalizeText = (TextView) findViewById(R.id.relocalizationStatus);
		uuidText = (TextView) findViewById(R.id.adfUuid);

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
						final int status = TangoJNINative.GetCurrentStatus();
						if(status == 0)
						{
							Log.i("jjj", "initialize");
						}
						runOnUiThread(new Runnable() {
							@Override
							public void run() {
								try {
									switch (status) {
									case 0:
										relocalizeText.setText(" TANGO_POSE_INITIALIZING");
										break;
									case 1:
										relocalizeText.setText(" TANGO_POSE_VALID");
										break;
									case 2:
										relocalizeText.setText(" TANGO_POSE_INVALID");
										break;
									default:
										relocalizeText.setText(" n/a");
										break;
									}
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
