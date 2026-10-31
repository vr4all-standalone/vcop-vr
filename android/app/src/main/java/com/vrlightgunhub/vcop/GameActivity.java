// SPDX-License-Identifier: GPL-2.0-or-later
package com.vrlightgunhub.vcop;
import org.libsdl.app.SDLActivity;
import android.os.Environment;
import android.content.Intent;
import java.io.File;

public final class GameActivity extends SDLActivity {
    @Override protected void onNativeMainReturned() {
        Intent setup=new Intent(Intent.ACTION_MAIN).setClass(this,SetupActivity.class).putExtra("returned",true);
        setup.addCategory("com.oculus.intent.category.2D");
        setup.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK|Intent.FLAG_ACTIVITY_CLEAR_TASK);
        startActivity(setup); finishAndRemoveTask();
    }
    @Override protected String[] getLibraries() { return new String[]{"SDL2","main"}; }
    @Override protected String[] getArguments() {
        String root=getFilesDir().getAbsolutePath();
        File download=new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS),"vcop-vr");
        nativeSetenv("VCOP_VR","1");
        nativeSetenv("VCOP_XR_LIBRARY",getApplicationInfo().nativeLibraryDir+"/libvcop_xr.so");
        nativeSetenv("VCOP_ASSETS",root+"/assets");
        nativeSetenv("VCOP_SETTINGS",root+"/settings.ini");
        File logs=getExternalFilesDir(null);
        nativeSetenv("VCOP_XR_LOG",new File(logs!=null?logs:getFilesDir(),"vcop-xr.log").getAbsolutePath());
        return new String[]{"vcop","-rompath",download.getAbsolutePath(),
            "-nvram_directory",root+"/nvram","-cfg_directory",root+"/cfg","-state_directory",root+"/state","-snapshot_directory",root+"/snap",
            "-video","none","-sound","sdl","-skip_gameinfo","-noreadconfig","-nomouse","-numprocessors","2","-throttle"};
    }
}
