plugins {
    id("com.android.library")
    kotlin("android")
}

android {
    namespace = "com.example.minimap"
    compileSdk = 35

    defaultConfig {
        minSdk = 26
        externalNativeBuild {
            cmake {
                cppFlags += listOf("-std=c++20")
            }
        }
    }

    externalNativeBuild {
        cmake {
            path = file("CMakeLists.txt")
        }
    }
}

dependencies {
    implementation(kotlin("stdlib"))
    implementation(project(":shared"))
}
