package com.example.gles_3_helloworld;

import android.app.Activity;
import android.os.Bundle;

public class MainActivity extends Activity {

	private GLES3View mView;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		mView = new GLES3View(getApplication());
        setContentView(mView);
	}
	
	@Override
	protected void onPause() {
        super.onPause();
        mView.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        mView.onResume();
    }	
}
