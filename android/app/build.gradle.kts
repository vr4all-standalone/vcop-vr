plugins { id("com.android.application") }
android {
    namespace = "com.vrlightgunhub.vcop"
    ndkVersion = "29.0.14206865"
    compileSdk { version = release(36) { minorApiLevel = 1 } }
    defaultConfig {
        applicationId = "com.vrlightgunhub.vcop"
        minSdk = 29
        targetSdk = 36
        versionCode = 10
        versionName = "0.1.9"
        ndk { abiFilters += "arm64-v8a" }
    }
    signingConfigs { create("local") {
        storeFile = rootProject.file("../private/vcop.keystore")
        storePassword = "android"
        keyAlias = "vcop"
        keyPassword = "android"
    } }
    buildTypes { getByName("release") { signingConfig = signingConfigs.getByName("local"); isMinifyEnabled = false } }
    sourceSets.getByName("main").assets.srcDirs("../../assets")
    sourceSets.getByName("main").jniLibs.srcDirs(providers.gradleProperty("vcopNativeDir").orNull ?: rootProject.file("../build/quest/jniLibs"))
    packaging { jniLibs { useLegacyPackaging = true } }
}

layout.buildDirectory.set(rootProject.layout.projectDirectory.dir("../build/android/app"))
