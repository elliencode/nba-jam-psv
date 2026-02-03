#include <falso_jni/FalsoJNI.h>
#include <falso_jni/FalsoJNI_Impl.h>
#include <falso_jni/FalsoJNI_Logger.h>

/*
 * JNI Methods
*/

NameToMethodID nameToMethodId[] = {
	{ 1, "getAppVersion", METHOD_TYPE_OBJECT },
	{ 2, "getAppVersionCode", METHOD_TYPE_OBJECT },
	{ 3, "getAppName", METHOD_TYPE_OBJECT },
	{ 4, "getAppID", METHOD_TYPE_OBJECT },
	{ 5, "getObbPath", METHOD_TYPE_OBJECT },
	{ 6, "getInternalDataPath", METHOD_TYPE_OBJECT },
	{ 7, "getPublicSourceDir", METHOD_TYPE_OBJECT },
	{ 8, "getPrefferedDeviceLanguages", METHOD_TYPE_OBJECT },
	{ 9, "getDeviceRegion", METHOD_TYPE_OBJECT },
	{ 10, "isVibratorAvailable", METHOD_TYPE_BOOLEAN },
	{ 11, "isTouchScreenAvailable", METHOD_TYPE_BOOLEAN },
	{ 12, "isGamePadAvailable", METHOD_TYPE_BOOLEAN },
	{ 13, "isTV", METHOD_TYPE_BOOLEAN },
	{ 14, "getActivityInstance", METHOD_TYPE_OBJECT },
	{ 15, "isWiFiConnected", METHOD_TYPE_BOOLEAN },
};

jobject getAppVersion(jmethodID id, va_list args) {
	return jni->NewStringUTF(&jni, "v.4.00.80");
}

jobject getAppName(jmethodID id, va_list args) {
	return jni->NewStringUTF(&jni, "NBA Jam");
}

jobject getAppID(jmethodID id, va_list args) {
	return jni->NewStringUTF(&jni, "com.eamobile.nbajam_row_wf");
}

jobject getObbPath(jmethodID id, va_list args) {
	return jni->NewStringUTF(&jni, "");
}

jobject getInternalDataPath(jmethodID id, va_list args) {
	return jni->NewStringUTF(&jni, DATA_PATH);
}

jobject getActivityInstance(jmethodID id, va_list args) {
	return (jobject)0x42424242;
}

jobject getPrefferedDeviceLanguages(jmethodID id, va_list args) {
	JavaDynArray * res = jda_alloc(1, FIELD_TYPE_OBJECT);
	jstring * data = res->array;
	data[0] = jni->NewStringUTF(&jni, "en_US");
	data[0] = jni->NewStringUTF(&jni, "en");
	data[0] = jni->NewStringUTF(&jni, "english");
	return data;
}

jobject getDeviceRegion(jmethodID id, va_list args) {
	return jni->NewStringUTF(&jni, "Japan");
}

jboolean isVibratorAvailable(jmethodID id, va_list args) {
	return JNI_FALSE;
}

jboolean isTouchScreenAvailable(jmethodID id, va_list args) {
	return JNI_TRUE;
}

jboolean isGamePadAvailable(jmethodID id, va_list args) {
	return JNI_TRUE;
}

jboolean isTV(jmethodID id, va_list args) {
	return JNI_FALSE;
}

jboolean isWiFiConnected(jmethodID id, va_list args) {
	return JNI_FALSE;
}

MethodsBoolean methodsBoolean[] = {
	{ 10, isVibratorAvailable },
	{ 11, isTouchScreenAvailable },
	{ 12, isGamePadAvailable },
	{ 13, isTV },
	{ 15, isWiFiConnected },

};
MethodsByte methodsByte[] = {};
MethodsChar methodsChar[] = {};
MethodsDouble methodsDouble[] = {};
MethodsFloat methodsFloat[] = {};
MethodsInt methodsInt[] = {};
MethodsLong methodsLong[] = {};
MethodsObject methodsObject[] = {
	{ 1, getAppVersion },
	{ 2, getAppVersion },
	{ 3, getAppName },
	{ 4, getAppID },
	{ 5, getObbPath },
	{ 6, getInternalDataPath },
	{ 7, getInternalDataPath },
	{ 8, getPrefferedDeviceLanguages },
	{ 9, getDeviceRegion },
	{ 14, getActivityInstance }
};
MethodsShort methodsShort[] = {};
MethodsVoid methodsVoid[] = {};

/*
 * JNI Fields
*/

// System-wide constant that applications sometimes request
// https://developer.android.com/reference/android/content/Context.html#WINDOW_SERVICE
char WINDOW_SERVICE[] = "window";

// System-wide constant that's often used to determine Android version
// https://developer.android.com/reference/android/os/Build.VERSION.html#SDK_INT
// Possible values: https://developer.android.com/reference/android/os/Build.VERSION_CODES
const int SDK_INT = 19; // Android 4.4 / KitKat

NameToFieldID nameToFieldId[] = {
		{ 0, "WINDOW_SERVICE", FIELD_TYPE_OBJECT }, 
		{ 1, "SDK_INT", FIELD_TYPE_INT },
};

FieldsBoolean fieldsBoolean[] = {};
FieldsByte fieldsByte[] = {};
FieldsChar fieldsChar[] = {};
FieldsDouble fieldsDouble[] = {};
FieldsFloat fieldsFloat[] = {};
FieldsInt fieldsInt[] = {
		{ 1, SDK_INT },
};
FieldsObject fieldsObject[] = {
		{ 0, WINDOW_SERVICE },
};
FieldsLong fieldsLong[] = {};
FieldsShort fieldsShort[] = {};

__FALSOJNI_IMPL_CONTAINER_SIZES
