package com.example.android_app_javashell;

import android.app.Application;
import android.content.Context;

public class ProxyApplication extends Application {
	// ’‚ «context ∏≥÷µ
	@Override
	protected void attachBaseContext(Context base) {
		super.attachBaseContext(base);
		// Shell.copy(getAssets());
		// Shell.load("com.example.test");
		Shell.preload(base);

	}

	@Override
	public void onCreate() {
		// Shell.run();
		Shell.loaddex();
	}
}