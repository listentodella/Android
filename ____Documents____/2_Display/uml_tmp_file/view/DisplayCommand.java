/*
* Copyright (C) 2012 Freescale Semiconductor, Inc. All Rights Reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

package android.view;
import android.util.Slog;
import android.os.IBinder;
import android.os.Parcel;
import android.os.ServiceManager;
import android.os.RemoteException;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.FileNotFoundException;
import android.content.Context;
import android.util.AndroidException;

public class DisplayCommand {
    static final String TAG = "DisplayCommand";
    ConfigParam mCfgParam = new ConfigParam();

    public static final int SETTING_MODE_FULL_SCREEN = 0x1000;
    public static final int SETTING_MODE_KEEP_PRIMARY_RATE = 0x2000;
    public static final int SETTING_MODE_KEEP_16_9_RATE = 0x4000;
    public static final int SETTING_MODE_KEEP_4_3_RATE = 0x8000;

    public DisplayCommand() {
    }

    public static class ConfigParam {
        public int rotation;
        public int xOverScan;
        public int yOverScan;
        public int mirror;
        public int colorDepth;
        public int keepRate;
        public int resolution;
	public int rotAngle;
	public int navBarVisible;
        public String mode;

        public ConfigParam() {
            mode = "test";
            rotation = 0;
            xOverScan = 0;
            yOverScan = 0;
            mirror = 0;
            colorDepth = 0;
            keepRate = SETTING_MODE_KEEP_PRIMARY_RATE;
            resolution = 10806011;
	        rotAngle = 1;
            navBarVisible = 1;
        }

        public void writeToParcel(Parcel out) {
            out.writeInt(rotation);
            out.writeInt(xOverScan);
            out.writeInt(yOverScan);
            out.writeInt(mirror);
            out.writeInt(colorDepth);
            out.writeInt(keepRate);
            out.writeInt(resolution);
            out.writeInt(rotAngle);
            out.writeInt(navBarVisible);
            out.writeString(mode);
        }
    }


    private int setMainDisplay(String mode) {
        int ret = 0;
        Slog.d(TAG, "setMainDisplay: " + mode);
        return ret;
    }

    public int enable(String mode, int rotation,
                          int xoverScan, int yoverScan, int mirror, int colorDepth, int keepRate, int resolution, int rotAngle, int navBarVisible) {
        int ret = 0;

        mCfgParam.mode = mode;
        mCfgParam.rotation = rotation;
        mCfgParam.xOverScan = xoverScan;
        mCfgParam.yOverScan = yoverScan;
        mCfgParam.mirror = mirror;
        mCfgParam.colorDepth = colorDepth;
        mCfgParam.keepRate = keepRate;
        mCfgParam.resolution = resolution;
        mCfgParam.rotAngle = rotAngle;
        mCfgParam.navBarVisible = navBarVisible;
        Slog.d(TAG, "enable: " + mode);

        return ret;
    }

    public int disable() {
        int ret = 0;
        Slog.d(TAG, "disable");
        return ret;
    }

    public int setResolution(String mode) {
        int ret = 0;
        mCfgParam.mode = mode;
        Slog.d(TAG, "setResolution: " + mode);
        return ret;
    }

    public int setOverScan(int Xratio, int Yratio) {
        int ret = 0;
        mCfgParam.xOverScan = Xratio;
        mCfgParam.yOverScan = Yratio;
        Slog.d(TAG, "setOverScan: " + mCfgParam.xOverScan + " " + mCfgParam.yOverScan);
        return ret;
    }

    public int getKeepRate() {
        return mCfgParam.keepRate;
    }
    public int getResolution() {
        return mCfgParam.resolution;
    }
    public int getRotAngle() {
        return mCfgParam.rotAngle;
    }
    public int getNavBarVisible() {
        return mCfgParam.navBarVisible;
    }
    public int getXOverScan() {
        return mCfgParam.xOverScan;
    }
    public int getYOverScan() {
        return mCfgParam.yOverScan;
    }

    public int setMirror(int mirror) {
        int ret = 0;
        mCfgParam.mirror = mirror;
        Slog.d(TAG, "setMirror: " + mirror);
        return ret;
    }

    public int setColorDepth(int depth) {
        int ret = 0;
        mCfgParam.colorDepth = depth;
        Slog.d(TAG, "setColorDepth: " + depth);
        return ret;
    }

    public int setRotation(int rotation) {
        int ret = 0;
        mCfgParam.rotation = rotation;
        Slog.d(TAG, "setRotation: " + rotation);
        return ret;
    }

    public int setKeepRate(int keepRate) {
        int ret = 0;
        mCfgParam.keepRate = keepRate;
        Slog.d(TAG, "setKeepRate: " + keepRate);
        return ret;
    }


    public int setHDMIResolution(int resolution) {
        int ret = 0;
        mCfgParam.resolution = resolution;
        Slog.d(TAG, "setHDMIResolution: " + resolution);
        return ret;
    }

    public int setRotAngle(int rotAngle) {
        int ret = 0;
        mCfgParam.rotAngle = rotAngle;
        Slog.d(TAG, "setRotAngle: " + rotAngle);
        return ret;
    }

    public int setNavBarVisible(int navBarVisible) {
        int ret = 0;
        mCfgParam.navBarVisible = navBarVisible;
        Slog.d(TAG, "setNavBarVisible: " + navBarVisible);
        return ret;
    }

    public int broadcastEvent() {
        int ret = 0;
        return ret;
    }

}



