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

package mtmo.test.mediadrm.absplayer;

import java.io.IOException;
import java.io.InputStream;
import java.util.UUID;

import mtmo.test.mediadrm.Constants;
import mtmo.test.mediadrm.InitData;
import mtmo.test.mediadrm.Logger;
import mtmo.test.mediadrm.Utils;
import mtmo.test.mediadrm.info.ABSContentInfo;

import org.apache.http.util.ByteArrayBuffer;
import org.json.JSONException;

import android.content.Context;
import android.content.res.AssetManager;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.View.OnClickListener;

public class ABSPlayer implements SurfaceHolder.Callback {
    private static final Logger mLogger = Logger.createLog(ABSPlayer.class
            .getSimpleName());

    protected static int TIMEOUT_US = 1000;

    private SurfaceView mSurfaceView;

    private ABSAudioThread mAudioThread;

    private ABSVideoThread mVideoThread;

    private MediaInfo mMediaInfo;

    public ABSPlayer(SurfaceView surfaceView) {
        surfaceView.getHolder().addCallback(this);
        mSurfaceView = surfaceView;
        mSurfaceView.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                pause();
            }
        });
    }

    public void setDataSource(ABSContentInfo info, Context context) {
        mLogger.d("setDataSource");
        mMediaInfo = new MediaInfo(info);
        mMediaInfo.uuid = Constants.MBB_UUID;
        byte[] pssh = Utils.readPsshDataFromFile(true);
        mMediaInfo.setIpmpVideo(pssh);
        pssh = Utils.readPsshDataFromFile(false);
        mMediaInfo.setIpmpAudio(pssh);

        mLogger.d("create new Threads");
        mVideoThread = new ABSVideoThread(
                mSurfaceView.getHolder().getSurface(), mMediaInfo);
        mAudioThread = new ABSAudioThread(mMediaInfo);
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

        public ABSContentInfo absinfo;

        public byte[] sessionId;

        public boolean isProtected;

        public byte[] pssh_video;

        public byte[] pssh_audio;

        UUID uuid;

        public MediaInfo(ABSContentInfo info) {
            this.absinfo = info;
            sessionId = null;
            pssh_video = null;
            pssh_audio = null;
            uuid = null;
        }

        public void setIpmpVideo(byte[] pssh) {
            this.pssh_video = pssh;
            if (pssh != null) {
                isProtected = true;
            }
        }

        public void setIpmpAudio(byte[] pssh) {
            this.pssh_audio = pssh;
            if (pssh != null) {
                isProtected = true;
            }
        }

        public byte[] getVideoPssh() {
            try {
                return InitData.getPSSHTableForAndroid(pssh_video,
                        absinfo.getVideoKid());
            } catch (JSONException e) {
                e.printStackTrace();
            }
            return null;
        }

        public byte[] getAudioPssh() {
            try {
                return InitData.getPSSHTableForAndroid(pssh_audio,
                        absinfo.getAudiokid());
            } catch (JSONException e) {
                e.printStackTrace();
            }
            return null;
        }
    }
}
