#include <cstdarg>
#include "jni.h"
#include "Cranked.hpp"

using namespace cranked;

#if __ANDROID__
#include <android/log.h>

static void logVA(LogLevel level, const char *format, va_list args) {
    __android_log_vprint(ANDROID_LOG_INFO, "Cranked", format, args);
}

static void log(LogLevel level, const char *format, ...) {
    va_list args;
    va_start(args, format);
    logVA(level, format, args);
    va_end(args);
}
#else
static void logVA(LogLevel level, const char *format, va_list args) {
    auto string = std::string("Cranked") + format;
    vprintf(string.c_str(), args);
}

static void log(LogLevel level, const char *format, ...) {
    va_list args;
    va_start(args, format);
    logVA(level, format, args);
    va_end(args);
}
#endif

static JNIEnv* environment;
static jclass crankedClass;
static jmethodID updateCallbackMethodId;

extern "C" {

JNIEXPORT jlong JNICALL Java_com_thelogicmaster_cranked_Cranked_initialize(JNIEnv *env, jobject object, jstring appDataPath) {
    if (!crankedClass) {
        environment = env;
        crankedClass = (jclass) env->NewGlobalRef(env->GetObjectClass(object));
        updateCallbackMethodId = env->GetMethodID(crankedClass, "updateCallback", "()V");
    }

    auto cranked = new Cranked;
    cranked->config.userdata = env->NewGlobalRef(object);
    cranked->config.updateCallback = [](Cranked &cranked) {
        environment->CallVoidMethod((jobject) cranked.config.userdata, updateCallbackMethodId);
    };
    cranked->config.loggingCallback = [](Cranked &emulator, LogLevel logLevel, const char *format, va_list args){
        logVA(logLevel, format, args);
    };

    auto dataPathChars = env->GetStringUTFChars(appDataPath, nullptr);
    cranked->files.appDataPath = dataPathChars;
    env->ReleaseStringUTFChars(appDataPath, dataPathChars);

    return (intptr_t) cranked;
}

JNIEXPORT void JNICALL Java_com_thelogicmaster_cranked_Cranked_destroy(JNIEnv *env, jclass clazz, jlong ptr) {
    auto cranked = (Cranked *) (intptr_t) ptr;
    env->DeleteGlobalRef((jobject) cranked->config.userdata);
    delete cranked;
}

JNIEXPORT void JNICALL Java_com_thelogicmaster_cranked_Cranked_load(JNIEnv *env, jclass clazz, jlong ptr, jstring path) {
    auto cranked = (Cranked *) (intptr_t) ptr;
    auto pathChars = env->GetStringUTFChars(path, nullptr);
    cranked->load(pathChars);
    env->ReleaseStringUTFChars(path, pathChars);
}

JNIEXPORT void JNICALL Java_com_thelogicmaster_cranked_Cranked_unload(JNIEnv *env, jclass clazz, jlong ptr) {
    ((Cranked *) (intptr_t) ptr)->unload();
}

JNIEXPORT void JNICALL Java_com_thelogicmaster_cranked_Cranked_start(JNIEnv *env, jclass clazz, jlong ptr) {
    ((Cranked *) (intptr_t) ptr)->start();
}

JNIEXPORT void JNICALL Java_com_thelogicmaster_cranked_Cranked_update(JNIEnv *env, jclass clazz, jlong ptr) {
    ((Cranked *) (intptr_t) ptr)->update();
}

JNIEXPORT void JNICALL Java_com_thelogicmaster_cranked_Cranked_stop(JNIEnv *env, jclass clazz, jlong ptr) {
    ((Cranked *) (intptr_t) ptr)->stop();
}

JNIEXPORT void JNICALL Java_com_thelogicmaster_cranked_Cranked_terminate(JNIEnv *env, jclass clazz, jlong ptr) {
    ((Cranked *) (intptr_t) ptr)->terminate();
}

JNIEXPORT void JNICALL Java_com_thelogicmaster_cranked_Cranked_reset(JNIEnv *env, jclass clazz, jlong ptr) {
    ((Cranked *) (intptr_t) ptr)->reset();
}

JNIEXPORT jint JNICALL Java_com_thelogicmaster_cranked_Cranked_getState(JNIEnv *env, jclass clazz, jlong ptr) {
    return (int)((Cranked *) (intptr_t) ptr)->state;
}

JNIEXPORT void JNICALL Java_com_thelogicmaster_cranked_Cranked_copyDisplayBuffer(JNIEnv *env, jclass clazz, jlong ptr, jobject buffer) {
    constexpr int size = DISPLAY_HEIGHT * DISPLAY_WIDTH * sizeof(jint);
    void *bufferPtr = env->GetDirectBufferAddress(buffer);
    if (env->GetDirectBufferCapacity(buffer) < size) return;
    auto cranked = (Cranked *) (intptr_t) ptr;
    memcpy(bufferPtr, cranked->graphics.displayBufferRGBA, size);
}

}
