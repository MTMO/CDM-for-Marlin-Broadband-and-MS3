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
import java.nio.ByteBuffer;
import java.util.concurrent.CountDownLatch;

import mtmo.test.mediadrm.Logger;

import android.media.MediaCodec;
import android.media.MediaCodec.BufferInfo;
import android.media.MediaCodec.CryptoInfo;
import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.media.MediaCrypto;
import android.media.MediaCryptoException;
import android.media.MediaExtractor;
import android.media.MediaFormat;
import android.view.Surface;
import android.view.SurfaceHolder;

public class ABSVideoThread extends Thread {
    private static final Logger mLogger = Logger.createLog(ABSVideoThread.class
            .getSimpleName());

    private MediaExtractor extractor;

    private Surface surface;

    private ABSPlayer.MediaInfo mMediaInfo;

    private CountDownLatch latch;

    private static final int STATE_STOPPED = 0;

    private static final int STATE_RUNNING = 1;

    private static final int STATE_PAUSED = 2;

    private int state = STATE_STOPPED;

    private long totalPause = 0;

    private long thisPause = 0;

    public ABSVideoThread(Surface surface, ABSPlayer.MediaInfo mediaInfo) {
        this.surface = surface;
        this.mMediaInfo = mediaInfo;
        mLogger.d("new MediaExtractor");
        extractor = new MediaExtractor();
    }

    private String getSecureDecoderNameForMime(String mime) {
        int n = MediaCodecList.getCodecCount();
        for (int i = 0; i < n; ++i) {
            MediaCodecInfo info = MediaCodecList.getCodecInfoAt(i);

            if (info.isEncoder()) {
                continue;
            }

            String[] supportedTypes = info.getSupportedTypes();

            for (int j = 0; j < supportedTypes.length; ++j) {
                if (supportedTypes[j].equalsIgnoreCase(mime)) {
                    mLogger.e("getSecureDecoderNameForMime returned: "
                                    + info.getName() + ".secure");
                    return info.getName() + ".secure";
                }
            }
        }
        return null;
    }

    @Override
    public void run() {
        mLogger.d("run");
        setState(STATE_RUNNING);
        MediaCodec decoder = null;
        MediaCrypto mediaCrypto = null;

        if (mMediaInfo.absinfo == null) {
            mLogger.e("Can't find abs info!");
            return;
        }

        try {
            extractor.setDataSource(mMediaInfo.absinfo.getVideoPath());
            CryptoInfo info = null;

            if (mMediaInfo.isProtected) {
                mLogger.d("psshsize: " + mMediaInfo.pssh_video.length);
                info = new CryptoInfo();
                mediaCrypto = new MediaCrypto(mMediaInfo.uuid,
                        mMediaInfo.getVideoPssh());
            }

            for (int i = 0; i < extractor.getTrackCount(); i++) {
                MediaFormat format = extractor.getTrackFormat(i);
                String mime = format.getString(MediaFormat.KEY_MIME);
                if (mime.startsWith("video/")) {
                    extractor.selectTrack(i);
                    mLogger.d("Mime: " + mime);
                    if (mMediaInfo.isProtected) {
                        decoder = MediaCodec.createDecoderByType(mime);
                    } else {
                        decoder = MediaCodec.createDecoderByType(mime);
                    }
                    if (decoder != null) {
                        decoder.configure(format, surface, mediaCrypto, 0);
                        // flag 0 for decoder
                    } else {
                        mLogger.d("Failed to create decoder");
                    }
                    break;
                }
            }

            if (decoder == null) {
                mLogger.e("Can't find video info!");
                return;
            }

            decoder.start();

            ByteBuffer[] inputBuffers = decoder.getInputBuffers();

            BufferInfo bufferInfo = new BufferInfo();
            boolean isEOS = false;
            long startMs = System.currentTimeMillis();
            while (!Thread.interrupted()) {
                if (!isEOS) {
                    mLogger.d("dequeueInputBuffer");
                    int inIndex = decoder.dequeueInputBuffer(0);
                    mLogger.d("inIndex: " + inIndex);
                    if (inIndex >= 0) {
                        ByteBuffer buffer = inputBuffers[inIndex];
                        mLogger.d("readSampleData");
                        int sampleSize = extractor.readSampleData(buffer, 0);
                        if (sampleSize < 0) {
                            // We shouldn't stop the playback at this point,
                            // just pass the EOS flag to decoder,
                            // we will get it again from
                            // the dequeueOutputBuffer
                            mLogger.d("InputBuffer BUFFER_FLAG_END_OF_STREAM");
                            decoder.queueInputBuffer(inIndex, 0, 0, 0,
                                    MediaCodec.BUFFER_FLAG_END_OF_STREAM);

                            isEOS = true;
                        } else {
                            if (mMediaInfo.isProtected) {
                                boolean ret = extractor
                                        .getSampleCryptoInfo(info);
                                mLogger.d("queueSecureInputBuffer");
                                decoder.queueSecureInputBuffer(inIndex, 0,
                                        info, extractor.getSampleTime(),
                                        MediaCodec.BUFFER_FLAG_SYNC_FRAME);
                            } else {
                                decoder.queueInputBuffer(inIndex, 0,
                                        sampleSize, extractor.getSampleTime(),
                                        MediaCodec.BUFFER_FLAG_SYNC_FRAME);
                            }
                            extractor.advance();
                        }
                    }
                }
                mLogger.d("dequeueOutputBuffer");
                int outIndex = decoder.dequeueOutputBuffer(bufferInfo, 10000);
                switch (outIndex) {
                case MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED:
                    mLogger.d("INFO_OUTPUT_BUFFERS_CHANGED");
                    break;
                case MediaCodec.INFO_OUTPUT_FORMAT_CHANGED:
                    mLogger.d("Video: New format " + decoder.getOutputFormat());
                    break;
                case MediaCodec.INFO_TRY_AGAIN_LATER:
                    mLogger.d("Video: dequeueOutputBuffer timed out!");
                    break;
                default:
                    // ByteBuffer buffer = outputBuffers[outIndex];
                    // Log.v(TAG,
                    // "Video: We can't use this buffer but render it due to the API limit, "
                    // + buffer);
                    // We use a very simple clock to keep the video FPS,
                    // or the video playback will be too fast
                    while (bufferInfo.presentationTimeUs / 1000 > System
                            .currentTimeMillis() - totalPause - startMs) {
                        sleep(10);
                    }
                    decoder.releaseOutputBuffer(outIndex, true);
                    break;
                }

                // All decoded frames have been rendered,
                // we can stop playing now
                if ((bufferInfo.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {
                    mLogger.d("OutputBuffer BUFFER_FLAG_END_OF_STREAM");
                    break;
                }

                /* simple pause */
                if (this.latch != null)
                    latch.await();
            }
        } catch (IOException e) {
            e.printStackTrace();
        } catch (MediaCryptoException e) {
            e.printStackTrace();
        } catch (IllegalStateException e) {
            mLogger.d("IllegalStateException");
            e.printStackTrace();
            // byffers in wrong state when trying to que,
            // most probably because of interupted Thread -> stop rendering
        } catch (InterruptedException e) {
            mLogger.d("InteruptedException");
            // Thread was interupted stop rendering
            e.printStackTrace();
        } finally {
            if (decoder != null) {
                decoder.stop();
                decoder.release();
            }

            if (mMediaInfo.isProtected && mediaCrypto != null) {
                mediaCrypto.release();
            }
        }
        extractor.release();
    }

    public void pause() {
        if (this.state == STATE_RUNNING) {
            this.latch = new CountDownLatch(1);
            setState(STATE_PAUSED);
            thisPause = System.currentTimeMillis();
        } else {
            totalPause = totalPause + (System.currentTimeMillis() - thisPause);
            if (this.latch != null)
                this.latch.countDown();
            setState(STATE_RUNNING);
        }
    }

    private synchronized void setState(int state) {
        this.state = state;
    }

    public void configureSurface(SurfaceHolder holder) {
        this.surface = holder.getSurface();
    }
}
