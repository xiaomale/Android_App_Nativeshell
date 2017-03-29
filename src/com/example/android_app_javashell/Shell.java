package com.example.android_app_javashell;

import android.content.Context;
import android.content.res.AssetManager;

public class Shell {
	static {
		System.loadLibrary("Nativeshell");
	}

//	public static native void load(String string);

//	public static native void copy(AssetManager assetManager);

//	public static native void run();
	
	public static native void preload(Context con);
	public static native void loaddex();
	

}
