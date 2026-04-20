plugins {
    kotlin("multiplatform")
}

kotlin {
    androidTarget()
    jvm()

    sourceSets {
        val commonMain by getting
        val commonTest by getting
    }
}
