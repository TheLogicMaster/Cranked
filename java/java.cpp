#include <cstdarg>
#include "jni.h"
#include "Emulator.hpp"

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

    auto emulator = new Emulator([](Emulator *emulator) {
        environment->CallVoidMethod((jobject) emulator->internalUserdata, updateCallbackMethodId);
    }, env->NewGlobalRef(object));

    auto dataPathChars = env->GetStringUTFChars(appDataPath, nullptr);
    emulator->loggingCallback = [](Emulator *emulator, LogLevel logLevel, const char *format, va_list args){
        logVA(logLevel, format, args);
    };

    emulator->files.appDataPath = dataPathChars;
    env->ReleaseStringUTFChars(appDataPath, dataPathChars);

    return (intptr_t) emulator;
}

JNIEXPORT void JNICALL Java_com_thelogicmaster_cranked_Cranked_destroy(JNIEnv *env, jclass clazz, jlong ptr) {
    auto emulator = (Emulator *) (intptr_t) ptr;
    env->DeleteGlobalRef((jobject) emulator->internalUserdata);
    delete emulator;
}

JNIEXPORT void JNICALL Java_com_thelogicmaster_cranked_Cranked_load(JNIEnv *env, jclass clazz, jlong ptr, jstring path) {
    auto emulator = (Emulator *) (intptr_t) ptr;
    auto pathChars = env->GetStringUTFChars(path, nullptr);
    emulator->load(pathChars);
    env->ReleaseStringUTFChars(path, pathChars);
}

JNIEXPORT void JNICALL Java_com_thelogicmaster_cranked_Cranked_unload(JNIEnv *env, jclass clazz, jlong ptr) {
    ((Emulator *) (intptr_t) ptr)->unload();
}

JNIEXPORT void JNICALL Java_com_thelogicmaster_cranked_Cranked_start(JNIEnv *env, jclass clazz, jlong ptr) {
    ((Emulator *) (intptr_t) ptr)->start();
}

JNIEXPORT void JNICALL Java_com_thelogicmaster_cranked_Cranked_update(JNIEnv *env, jclass clazz, jlong ptr) {
    ((Emulator *) (intptr_t) ptr)->update();
}

JNIEXPORT void JNICALL Java_com_thelogicmaster_cranked_Cranked_stop(JNIEnv *env, jclass clazz, jlong ptr) {
    ((Emulator *) (intptr_t) ptr)->stop();
}

JNIEXPORT void JNICALL Java_com_thelogicmaster_cranked_Cranked_terminate(JNIEnv *env, jclass clazz, jlong ptr) {
    ((Emulator *) (intptr_t) ptr)->terminate();
}

JNIEXPORT void JNICALL Java_com_thelogicmaster_cranked_Cranked_reset(JNIEnv *env, jclass clazz, jlong ptr) {
    ((Emulator *) (intptr_t) ptr)->reset();
}

JNIEXPORT jint JNICALL Java_com_thelogicmaster_cranked_Cranked_getState(JNIEnv *env, jclass clazz, jlong ptr) {
    return (int)((Emulator *) (intptr_t) ptr)->state;
}

JNIEXPORT void JNICALL Java_com_thelogicmaster_cranked_Cranked_copyDisplayBuffer(JNIEnv *env, jclass clazz, jlong ptr, jobject buffer) {
    constexpr int size = DISPLAY_HEIGHT * DISPLAY_WIDTH * sizeof(jint);
    void *bufferPtr = env->GetDirectBufferAddress(buffer);
    if (env->GetDirectBufferCapacity(buffer) < size) return;
    auto emulator = (Emulator *) (intptr_t) ptr;
    memcpy(bufferPtr, emulator->graphics.displayBufferRGBA, size);
}

}
