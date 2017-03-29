#include <stdlib.h>
#include <jni.h>
#include <stdio.h>

#include<Android/log.h>

#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include "string.h"
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#define TAG "LOGGGGGGGG"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__) // LOGD
static jobject mydexClassLoaderJObject;

//ת���ļ�
void copy(JNIEnv * env, jobject context) {

	//��jni�л�ȡAssetManager����
	jobject assetobj = (*env)->GetObjectClass(env, context);
	jmethodID getassets = (*env)->GetMethodID(env, assetobj, "getAssets",
			"()Landroid/content/res/AssetManager;");
	jobject assetManager = (*env)->CallObjectMethod(env, context, getassets);

	AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
	if (mgr == NULL) {
		return;
	}

	/*��ȡ�ļ�������*/
	AAsset* asset = AAssetManager_open(mgr, "payload.apk", AASSET_MODE_UNKNOWN);
	if (asset == NULL) {
		return;
	}
	/*��ȡ�ļ���С*/
	int bufferSize = AAsset_getLength(asset);
	char *buffer = (char *) malloc(bufferSize + 1);
	buffer[bufferSize] = 0;
	int numBytesRead = AAsset_read(asset, buffer, bufferSize);
	FILE *stream = fopen(
			"/data/data/com.example.android_app_javashell/payload.apk", "w");
	fwrite(buffer, bufferSize, 1u, stream);
	fclose(stream);
	free(buffer);
	/*�ر��ļ�*/
	AAsset_close(asset);
}
//����Դ����
static void loadApk(JNIEnv * env, jclass clazz, jobject context) {
	jclass activityThreadClazz;
	jmethodID currentActivityThreadMethodID;
	jobject activityThreadObject;
	const char *className;
	const char *methodName;
	int codeoff;

	jfieldID mPackagesFieldID;
	jobject mPackagesJObject;
	jclass mPackagesClazz;
	jmethodID getMethodID;

	jobject weakReferenceJObject;
	jclass weakReferenceJClazz;
	jmethodID getweakMethodID;

	jobject loadedApkJObject;
	jclass loadedApkJClazz;
	jfieldID mClassLoaderFieldID;
	jobject mClassLoaderJObject;
	jstring dexPath;
	jstring dexOptPath;

	jclass dexClassLoaderClazz;
	jmethodID initDexLoaderMethod;
	jobject dexClassLoaderJObject;
	copy(env, context); //�Ȱ�assets�����Դ����ת�Ƶ�Ӧ��Ŀ¼��
	activityThreadClazz = (*env)->FindClass(env, "android/app/ActivityThread");
	currentActivityThreadMethodID = (*env)->GetStaticMethodID(env,
			activityThreadClazz, "currentActivityThread",
			"()Landroid/app/ActivityThread;");
	activityThreadObject = (*env)->CallStaticObjectMethod(env,
			activityThreadClazz, currentActivityThreadMethodID); //currentActivityThread()��ȡ���̶߳���;
	LOGD("currentActivityThread: %s", "currentActivityThread");

	mPackagesFieldID = (*env)->GetFieldID(env, activityThreadClazz, "mPackages",
			"Landroid/util/ArrayMap;"); //Hashmap ArrayMap��������Ҫ����api�汾����ѡ��                  ������Ҫ�����ж�api�汾�Ĵ���........

	mPackagesJObject = (*env)->GetObjectField(env, activityThreadObject,
			mPackagesFieldID);
	LOGD("ArrayMap: %s", "ArrayMap");

	mPackagesClazz = (*env)->GetObjectClass(env, mPackagesJObject);
	LOGD("GetObjectClass: %s", "GetObjectClass");

	getMethodID = (*env)->GetMethodID(env, mPackagesClazz, "get",
			"(Ljava/lang/Object;)Ljava/lang/Object;");
	LOGD("GetMethodID: %s", "GetMethodID");

	weakReferenceJObject = (*env)->CallObjectMethod(env, mPackagesJObject,
			getMethodID,
			(*env)->NewStringUTF(env, "com.example.android_app_javashell"), 0); //WeakReference����
	LOGD("weakReferenceJObject: %s", "weakReferenceJObject");

	weakReferenceJClazz = (*env)->GetObjectClass(env, weakReferenceJObject);
	getweakMethodID = (*env)->GetMethodID(env, weakReferenceJClazz, "get",
			"()Ljava/lang/Object;");
	loadedApkJObject = (*env)->CallObjectMethod(env, weakReferenceJObject,
			getweakMethodID);
	LOGD("callweakReference:get %s", "get");

	loadedApkJClazz = (*env)->GetObjectClass(env, loadedApkJObject);
	mClassLoaderFieldID = (*env)->GetFieldID(env, loadedApkJClazz,
			"mClassLoader", "Ljava/lang/ClassLoader;");
	mClassLoaderJObject = (*env)->GetObjectField(env, loadedApkJObject,
			mClassLoaderFieldID);
	dexPath = (*env)->NewStringUTF(env,
			"/data/data/com.example.android_app_javashell/payload.apk"); //Ӳ���벻�Ǻܺ�
	dexOptPath = (*env)->NewStringUTF(env,                       //////////
			"/data/data/com.example.android_app_javashell/");
	dexClassLoaderClazz = (*env)->FindClass(env,
			"dalvik/system/DexClassLoader");
	initDexLoaderMethod =
			(*env)->GetMethodID(env, dexClassLoaderClazz, "<init>",
					"(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/ClassLoader;)V");
	LOGD("initDexLoaderMethod: %s", "initDexLoaderMethod");

	dexClassLoaderJObject = (*env)->NewObject(env, dexClassLoaderClazz,
			initDexLoaderMethod, dexPath, dexOptPath, NULL,
			mClassLoaderJObject);

	//ȫ�ֱ���
	mydexClassLoaderJObject = (*env)->NewGlobalRef(env, dexClassLoaderJObject);

	(*env)->SetObjectField(env, loadedApkJObject, mClassLoaderFieldID,
			mydexClassLoaderJObject);
	LOGD("NewGlobalRef: %s", "NewGlobalRef");

	//����Դ�������
	jclass Dexclassload = (*env)->FindClass(env,
			"dalvik/system/DexClassLoader");
	//˵�����ϰ汾��SDK��DexClassLoader��findClass�������°汾SDK����loadClass����
	jmethodID findclassMethodID = (*env)->GetMethodID(env, Dexclassload,
			"loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
	//���arg������֪����ɶ���������õ�MainAvtivity
	jstring arg = (*env)->NewStringUTF(env, "com.example.test.MainActivity");
	//����DexClassLoader��loadClass������������Ҫ���õ���͸���Ĳ���
	jobject actObj = (*env)->CallObjectMethod(env, mydexClassLoaderJObject,
			findclassMethodID, arg);

}
//��������Ǽ���Դ�������Զ����application��
static void running(JNIEnv * env, jclass clazz) {
	jclass activityThreadClazz;
	jmethodID currentActivityThreadMethodID;
	jobject activityThreadObject;

	jfieldID mBoundApplicationFieldID;
	jobject mBoundApplicationJObject;
	jclass mBoundApplicationClazz;
	jfieldID mInfoFieldID;
	jobject mInfoJObject;
	jclass mInfoClazz;

	jfieldID mApplicationFieldID;
	jobject mApplicationJObject;

	jfieldID mInitialApplicationFieldID;
	jobject mInitialApplicationJObject;

	jfieldID mAllApplicationsFieldID;
	jobject mAllApplicationsJObject;
	jclass mAllApplicationsClazz;
	jmethodID removeMethodID;

	jfieldID mApplicationInfoFieldID;
	jobject mApplicationInfoJObject;
	jclass mApplicationInfoClazz;

	jfieldID mBindApplicationInfoFieldID;
	jobject mBindApplicationInfoJObject;
	jclass mBindApplicationInfoClazz;

	jfieldID classNameFieldID;
	jfieldID mBindClassNameFieldID;
	jstring applicationName;

	jmethodID makeApplicationMethodID;
	jobject ApplicationJObject;
	jclass ApplicationClazz;
	jmethodID onCreateMethodID;
	LOGD("running: %s", "running");

	activityThreadClazz = (*env)->FindClass(env, "android/app/ActivityThread");
	currentActivityThreadMethodID = (*env)->GetStaticMethodID(env,
			activityThreadClazz, "currentActivityThread",
			"()Landroid/app/ActivityThread;");
	activityThreadObject = (*env)->CallStaticObjectMethod(env,
			activityThreadClazz, //��ȡ���̶߳���
			currentActivityThreadMethodID);
	mBoundApplicationFieldID = (*env)->GetFieldID(env, activityThreadClazz,
			"mBoundApplication", "Landroid/app/ActivityThread$AppBindData;");
	mBoundApplicationJObject = (*env)->GetObjectField(env, activityThreadObject,
			mBoundApplicationFieldID);
	mBoundApplicationClazz = (*env)->GetObjectClass(env,
			mBoundApplicationJObject);
	mInfoFieldID = (*env)->GetFieldID(env, mBoundApplicationClazz, "info",
			"Landroid/app/LoadedApk;");
	mInfoJObject = (*env)->GetObjectField(env, mBoundApplicationJObject,
			mInfoFieldID);
	mInfoClazz = (*env)->GetObjectClass(env, mInfoJObject);
	mApplicationFieldID = (*env)->GetFieldID(env, mInfoClazz, "mApplication",
			"Landroid/app/Application;");
	mApplicationJObject = (*env)->GetObjectField(env, mInfoJObject,
			mApplicationFieldID);
	(*env)->SetObjectField(env, mInfoJObject, mApplicationFieldID, NULL);
	mInitialApplicationFieldID = (*env)->GetFieldID(env, activityThreadClazz,
			"mInitialApplication", "Landroid/app/Application;");
	mInitialApplicationJObject = (*env)->GetObjectField(env,
			activityThreadObject, mInitialApplicationFieldID);
	mAllApplicationsFieldID = (*env)->GetFieldID(env, activityThreadClazz,
			"mAllApplications", "Ljava/util/ArrayList;");
	mAllApplicationsJObject = (*env)->GetObjectField(env, activityThreadObject,
			mAllApplicationsFieldID);
	mAllApplicationsClazz = (*env)->GetObjectClass(env,
			mAllApplicationsJObject);
	removeMethodID = (*env)->GetMethodID(env, mAllApplicationsClazz, "remove",
			"(Ljava/lang/Object;)Z");
	jboolean isTrue = (*env)->CallBooleanMethod(env, mAllApplicationsJObject,
			removeMethodID, mInitialApplicationJObject);
	mApplicationInfoFieldID = (*env)->GetFieldID(env, mInfoClazz,
			"mApplicationInfo", "Landroid/content/pm/ApplicationInfo;");
	mApplicationInfoJObject = (*env)->GetObjectField(env, mInfoJObject,
			mApplicationInfoFieldID);
	mApplicationInfoClazz = (*env)->GetObjectClass(env,
			mApplicationInfoJObject);
	mBindApplicationInfoFieldID = (*env)->GetFieldID(env,
			mBoundApplicationClazz, "appInfo",
			"Landroid/content/pm/ApplicationInfo;");
	mBindApplicationInfoJObject = (*env)->GetObjectField(env,
			mBoundApplicationJObject, mBindApplicationInfoFieldID);
	mBindApplicationInfoClazz = (*env)->GetObjectClass(env,
			mBindApplicationInfoJObject);
	classNameFieldID = (*env)->GetFieldID(env, mApplicationInfoClazz,
			"className", "Ljava/lang/String;");
	mBindClassNameFieldID = (*env)->GetFieldID(env, mBindApplicationInfoClazz,
			"className", "Ljava/lang/String;");
	applicationName = (*env)->NewStringUTF(env, "com.example.test.Myapp");
	(*env)->SetObjectField(env, mApplicationInfoJObject, classNameFieldID,
			applicationName);
	(*env)->SetObjectField(env, mBindApplicationInfoJObject,
			mBindClassNameFieldID, applicationName);
	makeApplicationMethodID = (*env)->GetMethodID(env, mInfoClazz,
			"makeApplication",
			"(ZLandroid/app/Instrumentation;)Landroid/app/Application;");
	ApplicationJObject = (*env)->CallObjectMethod(env, mInfoJObject,
			makeApplicationMethodID, JNI_FALSE, NULL);
	(*env)->SetObjectField(env, activityThreadObject,
			mInitialApplicationFieldID, ApplicationJObject);
	ApplicationClazz = (*env)->GetObjectClass(env, ApplicationJObject);
	onCreateMethodID = (*env)->GetMethodID(env, ApplicationClazz, "onCreate",
			"()V");
	(*env)->CallVoidMethod(env, ApplicationJObject, onCreateMethodID);
	LOGD("onCreate: %s", "onCreate");

}


static const JNINativeMethod s_methods[] = { { "preload",
		"(Landroid/content/Context;)V", (void*) loadApk }, { "loaddex", "()V",
		(void*) running }, };
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
	JNIEnv* env = NULL;
	if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_4) != JNI_OK) {
		return JNI_ERR;
	}

	jclass cls = (*env)->FindClass(env,
			"com/example/android_app_javashell/Shell");
	if (cls == NULL) {
		return JNI_ERR;
	}

	int len = sizeof(s_methods) / sizeof(s_methods[0]);
	if ((*env)->RegisterNatives(env, cls, s_methods, len) < 0) {
		return JNI_ERR;
	}

	return JNI_VERSION_1_4;
}
