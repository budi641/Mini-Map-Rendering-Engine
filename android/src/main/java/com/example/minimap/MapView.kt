package com.example.minimap

class MapView : AutoCloseable {
    private val bridge = NativeBridge()
    private var handle: Long = bridge.nativeCreate()

    fun setCenter(lat: Double, lng: Double) {
        check(handle != 0L) { "MapView has been released." }
        bridge.nativeSetCenter(handle, lat, lng)
    }

    fun setZoom(level: Float) {
        check(handle != 0L) { "MapView has been released." }
        bridge.nativeSetZoom(handle, level)
    }

    override fun close() {
        if (handle != 0L) {
            bridge.nativeDestroy(handle)
            handle = 0L
        }
    }

    fun release() = close()
}
