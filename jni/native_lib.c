#include <stdlib.h>
#include<Android/log.h>
//��ӡlogͷ�ļ�
#include <jni.h>
#include <stdio.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include "string.h"
#define TAG "TSL" // ������Զ����LOG�ı�ʶ
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__) // ����LOGD����
//#define LOGI(...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__) // ����LOGD����
#define JNIREG_CLASS "com/example/android_app_javashell/Shell"//ָ��Ҫע�����
//ʵ������һ��classloader
static jobject mydexClassLoaderJObject;
//����һ��char�ַ�
static char *g_strtdf;
//������������
jstring getPackname(JNIEnv *env, jobject obj);
void decodate(JNIEnv *env, jobject context);

//�˷���������ȡassetĿ¼�µ��ļ�������
void decodate(JNIEnv *env, jobject context) {
	//��jni�л�ȡAssetManager����
	jobject assetobj = (*env)->GetObjectClass(env, context);
	jmethodID v6 = (*env)->GetMethodID(env, assetobj, "getAssets",
			"()Landroid/content/res/AssetManager;");
	jobject assetManager = (*env)->CallObjectMethod(env, context, v6);
	AAssetManager* asMg = AAssetManager_fromJava(env, assetManager);
	if (asMg == NULL) {
		LOGD(" %s", "AAssetManager==NULL");
	}
	LOGD("%s", "getassetmanager sucess ");
	//�ҵ��ļ�������

	//��AssetĿ¼�µ�cdodj.jar�ļ�
	AAsset* as = AAssetManager_open(asMg, "payload.apk", 2);
	LOGD("%s", "delerjar sucess ");
	//jar�ļ�������
	if (as == NULL) {
		LOGD(" %s", "asset==NULL");
	}
	//��ȡ�ļ��Ĵ�С
	int bufferSize = AAsset_getLength(as);
	LOGD("file size   : %d", bufferSize);
	//����������
	char *buffer = (char *) malloc(bufferSize + 1);
	//�ļ�Ҫ��ȡ�Ĵ���
	buffer[bufferSize] = 0;
	int numBytesRead = AAsset_read(as, buffer, bufferSize);
	LOGD("file size : %d\n", numBytesRead);
	//��ȡ���ļ�
	FILE *fp = fopen("/data/data/com.example.android_app_javashell/payload.apk",
			"w");
	//LOGD("open file :%s\n", strerror(errno));
	//����Ҫ�ĳ��ֽ���
	// size_t fwrite(const void* buffer, size_t size, size_t count, FILE* stream);
	fwrite(buffer, numBytesRead, 1, fp);
	free(buffer);
	fclose(fp);
	/*�ر��ļ�*/
	AAsset_close(as);
	LOGD(": %s", "copy finshed");
}

//��ȡ����
jstring getPackname(JNIEnv *env, jobject obj)    //�ڸ�objΪcontext
{
	jclass native_class = (*env)->GetObjectClass(env, obj);
	jmethodID mId = (*env)->GetMethodID(env, native_class, "getPackageName",
			"()Ljava/lang/String;");
	jstring packName = (jstring) (*env)->CallObjectMethod(env, obj, mId);
	char *c_msg = (char*) (*env)->GetStringUTFChars(env, packName, 0);
	LOGD("signsture: %s", c_msg);    //jstringתc����Ȼ���ӡpackagemanger
	free(c_msg);
	return packName;
}

//����assetĿ¼�µ�jar�ļ���apk��/data/data/packagename/Ŀ¼�²����úö�̬���ػ���
static void copyassettodata(JNIEnv* env, jobject claz, jobject context) {
	jclass ft1;
	//�ҵ�ActivityThread�����
	jclass ActivityThreadclazz = (*env)->FindClass(env,
			"android/app/ActivityThread");
	//map��
	jclass map = (*env)->FindClass(env, "java/util/Map");
	//ϵͳ��dexclassloader
	jclass dexClassLoaderClazz = (*env)->FindClass(env,
			"dalvik/system/DexClassLoader");
	//�ҵ�һ��clazz��֪��Ҫ����
	jclass clazz = (*env)->FindClass(env, "java/lang/Class");
	//���Զ����classloader�滻��ǰ������mclassload
	jclass loadapk = (*env)->FindClass(env, "android/app/LoadedApk");
	//������ȡapi�汾
	jclass os = (*env)->FindClass(env, "android/os/SystemProperties");
	//���ö�̬���ػ���
	//�Ȼ�ȡӦ�õİ���,����ӡ
	char *packagename = (char*) (*env)->GetStringUTFChars(env,
			getPackname(env, context), 0);
	LOGD("mypackagename: %s", packagename);
	//��ȡ���̶߳���
	jmethodID currentActivityThreadMethodID = (*env)->GetStaticMethodID(env,
			ActivityThreadclazz, "currentActivityThread",
			"()Landroid/app/ActivityThread;");
	jobject activityThreadObject = (*env)->CallStaticObjectMethod(env,
			ActivityThreadclazz, currentActivityThreadMethodID);
	//�õ�ϵͳgetint��������ȡsdk�汾��
	jmethodID myId = (*env)->GetStaticMethodID(env, os, "getInt",
			"(Ljava/lang/String;I)I");
	jstring sdk = (*env)->NewStringUTF(env, "ro.build.version.sdk");
	int sdkversion = (int) (*env)->CallStaticIntMethod(env, os, myId, sdk);
	LOGD("sdk version : %d", sdkversion);
	//������ȫƥ�䣬����api���ݻ��������
	if (sdkversion < 19) {
		//using hashmap
		ft1 = "Ljava/util/HashMap;";

	} else {
		//using arraymap
		ft1 = "Landroid/util/ArrayMap;";
	}
	//�õ�mPackages����
	jfieldID mPackagesFieldID = (*env)->GetFieldID(env, ActivityThreadclazz,
			"mPackages", ft1);
	//��ǰ�̶߳���
	jobject mPackagesJObject = (*env)->GetObjectField(env, activityThreadObject,
			mPackagesFieldID);
	jclass mPackagesClazz = (*env)->GetObjectClass(env, mPackagesJObject);
	jmethodID getMethodID = (*env)->GetMethodID(env, mPackagesClazz, "get",
			"(Ljava/lang/Object;)Ljava/lang/Object;");
	//����WeakReference����
	jobject weakReferenceJObject = (*env)->CallObjectMethod(env,
			mPackagesJObject, getMethodID, getPackname(env, context), 0);
	jclass weakReferenceJClazz = (*env)->GetObjectClass(env,
			weakReferenceJObject);
	jmethodID getweakMethodID = (*env)->GetMethodID(env, weakReferenceJClazz,
			"get", "()Ljava/lang/Object;");
	jobject loadedApkJObject = (*env)->CallObjectMethod(env,
			weakReferenceJObject, getweakMethodID);
	//�в���λ���иĶ�
	jclass loadedApkJClazz = (*env)->GetObjectClass(env, loadedApkJObject);
	//jmethodID  mClassLoaderFieldID  =(*env)->GetFieldID(env, loadedApkJClazz, "mClassLoader", "Ljava/lang/ClassLoader;");
	//jobject mClassLoaderJObject = (*env)->GetObjectField(env,loadedApkJObject, mClassLoaderFieldID);
	//��apk������Ĵ���
	jfieldID otherClassLoaderFieldID = (*env)->GetFieldID(env, loadapk,
			"mClassLoader", "Ljava/lang/ClassLoader;");
	jobject otherClassLoaderJObject = (*env)->GetObjectField(env,
			loadedApkJObject, otherClassLoaderFieldID);

	decodate(env, context);
	//�˷�����������jar������������ǿ�������
	LOGD("copy: %s", "copy to data finshed");
	//����dexclassloader���ػ���
	jstring dexPath = (*env)->NewStringUTF(env,
			"/data/data/com.example.android_app_javashell/payload.apk");
	jstring dexOptPath = (*env)->NewStringUTF(env,
			"/data/data/com.example.android_app_javashell/");
	//����DexClassLoader�ಢ�Ҵ������������Ż����dex
	jmethodID initDexLoaderMethod =
			(*env)->GetMethodID(env, dexClassLoaderClazz, "<init>",
					"(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/ClassLoader;)V");

	LOGD("copy: %s", "initDexLoaderMethod start");
	LOGD("copy: %s", "start to changed the dexclassloader");
	//�����滻ԭ����mclassloader��Ϊ�Լ�dexclassloader
	jobject dexClassLoaderJObject = (*env)->NewObject(env, dexClassLoaderClazz,
			initDexLoaderMethod, dexPath, dexOptPath, NULL,
			otherClassLoaderJObject);
	//Ϊȫ�ֱ�������һ����
	mydexClassLoaderJObject = (*env)->NewGlobalRef(env, dexClassLoaderJObject);
	//�ѵ�ǰ���̵�DexClassLoader ���ó��˱��ӿ�apk��DexClassLoader   ԭ�е�myclassload   WeakReference/applicationinfo   ϵͳ��ǰ��classload   �Զ���classload
	(*env)->SetObjectField(env, loadedApkJObject, otherClassLoaderFieldID,
			mydexClassLoaderJObject);
	//(*env) ->DeleteLocalRef(env,  g_objl);//�����ɾ����Ĳ���,�������
	//unlink(&g_strtdf);//ɾ����ɾ���������ļ�(�и��Ż��Ĳ���ط�����)
}

//��дjni����Ķ�̬����s
static void loaddex(JNIEnv *env, jobject obj, jobject context) {
	LOGD(": %s", "loadex start");
	//�ҵ�class��  �����뿴������������һ��jclass
	jclass myclazz = (*env)->FindClass(env, "java/lang/Class");
	//�ҵ�DexClassLoader��
	jclass Dexclassload = (*env)->FindClass(env,
			"dalvik/system/DexClassLoader");
	//˵�����ϰ汾��SDK��DexClassLoader��findClass�������°汾SDK����loadClass����
	jmethodID findclassMethodID = (*env)->GetMethodID(env, Dexclassload,
			"loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
	LOGD("��%s", "findclassMethod");
	//���arg������֪����ɶ���������õ�MainAvtivity
	jstring arg = (*env)->NewStringUTF(env,
			"com.example.test.MainActivity");
	//����DexClassLoader��loadClass������������Ҫ���õ���͸���Ĳ���
	jobject actObj = (*env)->CallObjectMethod(env, mydexClassLoaderJObject,
			findclassMethodID, arg);
	LOGD("��%s", "load over");
	/* //�����ǹ̶���·
	 jmethodID newinstanceID=  (*env)->GetMethodID(env, myclazz, "newInstance", "()Ljava/lang/Object;");
	 //new һ��v11����
	 jobject   mainactivity=    (*env)->CallObjectMethod(env, actObj, newinstanceID);
	 //����ĳ���������attachBaseContext������onCreate����
	 jobject  v17 = (*env)->GetObjectClass(env, mainactivity);
	 jmethodID attachbaseid=(*env)->GetMethodID(env, v17, "attachBaseContext", "(Landroid/content/Context;)V");
	 (*env)->CallVoidMethod(env, mainactivity, attachbaseid, context);
	 //���������onCreate����
	 jmethodID v21 = (*env)->GetMethodID(env, v17, "onCreate", "()V");
	 (*env)->CallVoidMethod(env, mainactivity, v21);*/
	LOGD(": %s", "loadex finshed");

}

/**
 * Table of methods associated with a single class.
 */
static JNINativeMethod gMethods[] = { { "loaddex",
		"(Landroid/content/Context;)V", (void*) loaddex },    //��java ��������
		// �ڶ���������()Ljava/lang/String:��,��ǩ�����ţ���˼�Ǹú���û�в���������һ���ַ�������������������Ҫ���õ� native ����
		{ "decodefromasset", "(Landroid/content/Context;)V",
				(void*) copyassettodata },        //��

		};

/*
 * Register several native methods for one class.
 */
static int registerNativeMethods(JNIEnv* env, const char* className,
		JNINativeMethod* gMethods, int numMethods) {
	jclass clazz;
	clazz = (*env)->FindClass(env, className);
	if (clazz == NULL) {
		return JNI_FALSE;
	}
	if ((*env)->RegisterNatives(env, clazz, gMethods, numMethods) < 0) {
		return JNI_FALSE;
	}

	return JNI_TRUE;
}

/*
 * Register native methods for all classes we know about.
 */
static int registerNatives(JNIEnv* env) {
	if (!registerNativeMethods(env, JNIREG_CLASS, gMethods,
			sizeof(gMethods) / sizeof(gMethods[0])))
		return JNI_FALSE;

	return JNI_TRUE;
}

/*
 * Set some test stuff up.
 *
 * Returns the JNI version on success, -1 on failure.
 */
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void* reserved) {
	JNIEnv* env = NULL;
	jint result = -1;

	if (JNI_OK != (*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_4)) {
		return -1;
	}
	assert(env != NULL);

	if (!registerNatives(env)) {        //ע���һ����
		return -1;
	}
	/* success -- return valid version number */
	result = JNI_VERSION_1_4;

	return result;
}
