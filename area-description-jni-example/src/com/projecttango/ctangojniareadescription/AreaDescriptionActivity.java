package com.projecttango.ctangojniareadescription;

import android.R.bool;
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
import android.widget.ToggleButton;

public class AreaDescriptionActivity extends Activity {
	GLSurfaceView glView;
	RelativeLayout layout;
	
	TextView device2StartText;
	TextView device2ADFText;
	TextView start2ADFText;
	
	TextView learningModeText;
	TextView uuidText;
	TextView relocalizedText;
	
	TextView learning_mode_toggle_button_text;
	TextView load_adf_button_text;
	
	Button saveADFButton;
	Button startButton;
	
	ToggleButton isUsingADFToggleButton;
	ToggleButton isLearningToggleButton;
	
	boolean isUsingADF = false;
	boolean isLearning = false;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setContentView(R.layout.activity_area_description);
		glView = (GLSurfaceView) findViewById(R.id.surfaceview);
		glView.setRenderer(new Renderer());

		device2StartText = (TextView) findViewById(R.id.device_start);
		device2ADFText = (TextView) findViewById(R.id.device_adf);
		start2ADFText = (TextView) findViewById(R.id.start_adf);
		
		learningModeText = (TextView) findViewById(R.id.learning_mode);
		uuidText = (TextView) findViewById(R.id.uuid);
		relocalizedText = (TextView) findViewById(R.id.relocalized_text);
		
		learning_mode_toggle_button_text = (TextView) findViewById(R.id.learning_mode_toggle_button_text);
		load_adf_button_text = (TextView) findViewById(R.id.load_adf_button_text);
		
		saveADFButton = (Button) findViewById(R.id.save_adf_button);
		saveADFButton.setVisibility(View.GONE);
		saveADFButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                TangoJNINative.SaveADF();
            }
        });
		
		startButton = (Button) findViewById(R.id.start_button);
		
		startButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
            	if(isLearning)
                {
                	saveADFButton.setVisibility(View.VISIBLE);
                }
                TangoJNINative.Initialize(isLearning, isUsingADF);
                TangoJNINative.ConnectService();
                startButton.setVisibility(View.GONE);
                isUsingADFToggleButton.setVisibility(View.GONE);
                isLearningToggleButton.setVisibility(View.GONE);
                learning_mode_toggle_button_text.setVisibility(View.GONE);
                load_adf_button_text.setVisibility(View.GONE);
                
            }
        });
		
		isUsingADFToggleButton = (ToggleButton)findViewById(R.id.load_adf_toggle_button);
		isUsingADFToggleButton.setOnClickListener(new View.OnClickListener(){
		    public void onClick(View v) {
		    	isUsingADF = isUsingADFToggleButton.isChecked();
            }
		});
		isLearningToggleButton = (ToggleButton)findViewById(R.id.learning_mode_toggle_button);
		isLearningToggleButton.setOnClickListener(new View.OnClickListener(){
		    public void onClick(View v) {
		    	isLearning = isLearningToggleButton.isChecked();
            }
		});
		
		new Thread(new Runnable() {
			@Override
			public void run() {
				while (true) {
					try {
						Thread.sleep(100);
						runOnUiThread(new Runnable() {
							@Override
							public void run() {
								try {
									device2StartText.setText(getStringFromPoseStatusCode(TangoJNINative.GetCurrentStatus(0)) + ", " +
											TangoJNINative.GetPoseString(0));
									device2ADFText.setText(getStringFromPoseStatusCode(TangoJNINative.GetCurrentStatus(1)) + ", " +
											TangoJNINative.GetPoseString(1));
									start2ADFText.setText(getStringFromPoseStatusCode(TangoJNINative.GetCurrentStatus(2)) + ", " +
											TangoJNINative.GetPoseString(2));
									
									uuidText.setText(TangoJNINative.GetUUID());
									learningModeText.setText(String.valueOf(isLearning));
									relocalizedText.setText(TangoJNINative.GetIsRelocalized());
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
		TangoJNINative.DisconnectService();
	}

	protected void onDestroy() {
		super.onDestroy();
		TangoJNINative.OnDestroy();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
//		getMenuInflater().inflate(R.menu.area_description, menu);
		return true;
	}

	private String getStringFromPoseStatusCode(int poseStatus){
		
		String retString = "";
		switch (poseStatus){
		case 0:
			retString = "Initializing";
			break;
		case 1:
			retString = "Valid";
			break;
		case 2:
			retString = "Invalid";
			break;
		case 3:
			retString = "Unkown";
			break;	
		default:
			retString = "N/A";
			break;
		}
		return retString;
	}
//	@Override
//	public boolean onOptionsItemSelected(MenuItem item) {
//		switch (item.getItemId()) {
//		case R.id.load_adf:
//			TangoJNINative.Initialize();
//			TangoJNINative.ConnectService();
//			return true;
//		case R.id.record_adf:
//			TangoJNINative.Initialize(1);
//			saveADFButton.setVisibility(View.VISIBLE);
//			TangoJNINative.ConnectService();
//			return true;
//		default:
//			return super.onOptionsItemSelected(item);
//		}
//	}
}
