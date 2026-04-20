package com.example.minimap

internal class NativeBridge {
    external fun nativeCreate(): Long
    external fun nativeDestroy(ptr: Long)
    external fun nativeSetCenter(ptr: Long, lat: Double, lng: Double)
    external fun nativeSetZoom(ptr: Long, zoom: Float)

    companion object {
        init {
            System.loadLibrary("minimap_android")
        }
    }
}
