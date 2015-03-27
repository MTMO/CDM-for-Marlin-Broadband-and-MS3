/*
 * (c) 2015 - Copyright Marlin Trust Management Organization
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package mtmo.test.mediadrm.player;

import java.io.IOException;
import java.io.InputStream;
import java.util.UUID;

import mtmo.test.mediadrm.Constants;
import mtmo.test.mediadrm.InitData;
import mtmo.test.mediadrm.Logger;
import mtmo.test.mediadrm.Utils;
import mtmo.test.mediadrm.absplayer.ABSAudioThread;

import org.apache.http.util.ByteArrayBuffer;
import org.json.JSONException;

import android.content.Context;
import android.content.res.AssetManager;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.View.OnClickListener;

public class Player implements SurfaceHolder.Callback {
    private static final Logger mLogger = Logger.createLog(Player.class
            .getSimpleName());

    protected static int TIMEOUT_US = 1000;

    private SurfaceView mSurfaceView;

    private AudioThread mAudioThread;

    private VideoThread mVideoThread;

    private MediaInfo mMediaInfo;

    public Player(SurfaceView surfaceView) {
        surfaceView.getHolder().addCallback(this);
        mSurfaceView = surfaceView;
        mSurfaceView.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                pause();
            }
        });
    }

    public void setDataSource(String path, Context context) {
        mLogger.d("setDataSource content path : " + path);
        mMediaInfo = new MediaInfo(path);
        mMediaInfo.uuid = Constants.MBB_UUID;
        byte[] sinf = Utils.readIPMPDataFromFile(true);
        mMediaInfo.setIpmpVideo(sinf);
        sinf = Utils.readIPMPDataFromFile(false);
        mMediaInfo.setIpmpAudio(sinf);
        mLogger.d("create new Threads");
        mAudioThread = new AudioThread(mMediaInfo);
        mVideoThread = new VideoThread(mSurfaceView.getHolder().getSurface(),
                mMediaInfo, mAudioThread);
    }

    public void play() {
        if (mAudioThread != null) {
            mAudioThread.start();
        }
        if (mVideoThread != null) {
            mVideoThread.start();
        }
    }

    public void pause() {
        if (mVideoThread != null) {
            mVideoThread.pause();
        }
        if (mAudioThread != null) {
            mAudioThread.pause();
        }
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        mLogger.d("surface created");
        if (mVideoThread != null) {
            mVideoThread.configureSurface(holder);
        }
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width,
            int height) {
        mLogger.d("something is happening with surface");
        if (mSurfaceView != null) {

        }
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        mLogger.d("surface destoyed");
        if (mVideoThread != null) {
            mVideoThread.interrupt();
        }
        if (mAudioThread != null) {
            mAudioThread.interrupt();
        }
    }

    class MediaInfo {

        public String path;

        public byte[] sessionId;

        public boolean isProtected;

        public byte[] video_sinf;

        public byte[] audio_sinf;

        UUID uuid;

        public MediaInfo(String path) {
            this.path = path;
            sessionId = null;
            video_sinf = null;
            audio_sinf = null;
            uuid = null;
        }

        public void setIpmpVideo(byte[] ipmp) {
            this.video_sinf = ipmp;
            if (ipmp != null) {
                isProtected = true;
            }
        }

        public void setIpmpAudio(byte[] ipmp) {
            this.audio_sinf = ipmp;
            if (ipmp != null) {
                isProtected = true;
            }
        }

        public byte[] getVideoSinf() {
            try {
                return InitData.getIPMPTableForAndroid(video_sinf);
            } catch (JSONException e) {
                e.printStackTrace();
            }
            return null;
        }

        public byte[] getAudioSinf() {
            try {
                return InitData.getIPMPTableForAndroid(audio_sinf);
            } catch (JSONException e) {
                e.printStackTrace();
            }
            return null;
        }
    }
}
