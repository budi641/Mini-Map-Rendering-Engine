package com.example.minimap

import kotlin.math.atan
import kotlin.math.exp
import kotlin.math.ln
import kotlin.math.tan

object Projection {
    private const val EARTH_RADIUS = 6378137.0
    private const val ORIGIN_SHIFT = 20037508.342789244
    private const val PI = 3.141592653589793

    data class LatLng(val lat: Double, val lng: Double)
    data class WorldPoint(val x: Double, val y: Double)

    fun latLngToWorld(latLng: LatLng): WorldPoint {
        val lat = latLng.lat.coerceIn(-85.05112878, 85.05112878)
        val x = latLng.lng * ORIGIN_SHIFT / 180.0
        val y = ln(tan((90.0 + lat) * PI / 360.0)) * EARTH_RADIUS
        return WorldPoint(x, y)
    }

    fun worldToLatLng(world: WorldPoint): LatLng {
        val lng = world.x / ORIGIN_SHIFT * 180.0
        val lat = 180.0 / PI * (2.0 * atan(exp(world.y / EARTH_RADIUS)) - PI / 2.0)
        return LatLng(lat, lng)
    }
}
