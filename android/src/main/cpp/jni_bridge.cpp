#include "Map/Camera.h"

#include <jni.h>
#include <mutex>

namespace {

class NativeMapHandle {
public:
    void SetCenter(double lat, double lng) { camera_.SetCenter({lat, lng}); }
    void SetZoom(float zoom) { camera_.SetZoom(zoom); }

private:
    minimap::map::Camera camera_ {};
};

std::mutex g_mutex;

}  // namespace

extern "C" JNIEXPORT jlong JNICALL
Java_com_example_minimap_NativeBridge_nativeCreate(JNIEnv*, jobject) {
    auto* handle = new NativeMapHandle();
    return reinterpret_cast<jlong>(handle);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_minimap_NativeBridge_nativeDestroy(JNIEnv*, jobject, jlong ptr) {
    std::lock_guard<std::mutex> lock(g_mutex);
    auto* handle = reinterpret_cast<NativeMapHandle*>(ptr);
    delete handle;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_minimap_NativeBridge_nativeSetCenter(JNIEnv*, jobject, jlong ptr, jdouble lat, jdouble lng) {
    std::lock_guard<std::mutex> lock(g_mutex);
    auto* handle = reinterpret_cast<NativeMapHandle*>(ptr);
    if (handle != nullptr) {
        handle->SetCenter(lat, lng);
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_minimap_NativeBridge_nativeSetZoom(JNIEnv*, jobject, jlong ptr, jfloat zoom) {
    std::lock_guard<std::mutex> lock(g_mutex);
    auto* handle = reinterpret_cast<NativeMapHandle*>(ptr);
    if (handle != nullptr) {
        handle->SetZoom(zoom);
    }
}
