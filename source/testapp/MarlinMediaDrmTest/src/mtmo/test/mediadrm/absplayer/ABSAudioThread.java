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

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.media.MediaCodec;
import android.media.MediaCodec.BufferInfo;
import android.media.MediaCodec.CryptoInfo;
import android.media.MediaCrypto;
import android.media.MediaCryptoException;
import android.media.MediaExtractor;
import android.media.MediaFormat;

public class ABSAudioThread extends Thread {
    private static final Logger mLogger = Logger.createLog(ABSAudioThread.class
            .getSimpleName());

    private MediaExtractor extractor;

    private ABSPlayer.MediaInfo mMediaInfo;

    private CountDownLatch latch;

    private static final int STATE_STOPPED = 0;

    private static final int STATE_RUNNING = 1;

    private static final int STATE_PAUSED = 2;

    private int state = STATE_STOPPED;

    public ABSAudioThread(ABSPlayer.MediaInfo mediaInfo) {
        this.mMediaInfo = mediaInfo;
        mLogger.d("new MediaExtractor");
        extractor = new MediaExtractor();
    }

    @Override
    public void run() {
        setState(STATE_RUNNING);
        MediaCodec decoder = null;
        AudioTrack mAudioTrack = null;
        MediaCrypto mediaCrypto = null;

        if (mMediaInfo.absinfo == null) {
            mLogger.e("Can't find abs info!");
            return;
        }

        try {
            // extractor.setDataSource(mMediaInfo.path);
            extractor.setDataSource(mMediaInfo.absinfo.getAudioPath());
            CryptoInfo info = null;

            if (mMediaInfo.isProtected) {
                info = new CryptoInfo();
                mLogger.d("psshsize: " + mMediaInfo.pssh_audio.length);

                mLogger.d("### new MediaCrypto");
                mediaCrypto = new MediaCrypto(mMediaInfo.uuid,
                        mMediaInfo.getAudioPssh());
            }
            for (int i = 0; i < extractor.getTrackCount(); i++) {
                MediaFormat format = extractor.getTrackFormat(i);
                String mime = format.getString(MediaFormat.KEY_MIME);
                if (mime.startsWith("audio/")) {
                    extractor.selectTrack(i);
                    mLogger.d("Mime: " + mime);
                    int buffsize = AudioTrack.getMinBufferSize(
                            format.getInteger(MediaFormat.KEY_SAMPLE_RATE),
                            AudioFormat.CHANNEL_OUT_MONO,
                            AudioFormat.ENCODING_PCM_16BIT);
                    mLogger.d("new AudioTrack");
                    mAudioTrack = new AudioTrack(AudioManager.STREAM_MUSIC,
                            format.getInteger(MediaFormat.KEY_SAMPLE_RATE),
                            AudioFormat.CHANNEL_CONFIGURATION_STEREO,
                            AudioFormat.ENCODING_PCM_16BIT, buffsize,
                            AudioTrack.MODE_STREAM);

                    mLogger.d("createDecoderByType");
                    decoder = MediaCodec.createDecoderByType(mime);
                    mLogger.d("configure");
                    decoder.configure(format, null, mediaCrypto, 0);
                    // flag 0 for decoder
                    break;
                }
            }

            if (decoder == null) {
                mLogger.e("Can't find audio info!");
                return;
            }

            decoder.start();
            mAudioTrack.play();

            mLogger.d("getInputBuffers");
            ByteBuffer[] inputBuffers = decoder.getInputBuffers();
            mLogger.d("getOutputBuffers");
            ByteBuffer[] outputBuffers = decoder.getOutputBuffers();

            mLogger.d("new BufferInfo");
            BufferInfo bufferInfo = new BufferInfo();
            boolean isEOS = false;
            long startMs = System.currentTimeMillis();
            while (!Thread.interrupted()) {
                if (!isEOS) {
                    mLogger.d("new dequeueInputBuffer");
                    int inIndex = decoder
                            .dequeueInputBuffer(ABSPlayer.TIMEOUT_US);
                    if (inIndex >= 0) {
                        ByteBuffer buffer = inputBuffers[inIndex];
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

                int outIndex = decoder.dequeueOutputBuffer(bufferInfo,
                        ABSPlayer.TIMEOUT_US);
                switch (outIndex) {
                case MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED:
                    mLogger.d("INFO_OUTPUT_BUFFERS_CHANGED");
                    outputBuffers = decoder.getOutputBuffers();
                    break;
                case MediaCodec.INFO_OUTPUT_FORMAT_CHANGED:
                    mLogger.d("Audio: New format " + decoder.getOutputFormat());
                    final MediaFormat oformat = decoder.getOutputFormat();
                    mAudioTrack.setPlaybackRate(oformat
                            .getInteger(MediaFormat.KEY_SAMPLE_RATE));
                    break;
                case MediaCodec.INFO_TRY_AGAIN_LATER:
                    mLogger.d("Audio: dequeueOutputBuffer timed out!");
                    break;
                default:
                    ByteBuffer buffer = outputBuffers[outIndex];
                    /*
                     * Log.v(TAG,
                     * "Audio: We can't use this buffer but render it due to the API limit, "
                     * + buffer);
                     */
                    mLogger.d("bufferInfo.size: " + bufferInfo.size);
                    final byte[] chunk = new byte[bufferInfo.size];
                    buffer.get(chunk);
                    buffer.clear();
                    if (chunk.length > 0) {
                        mAudioTrack.write(chunk, 0, chunk.length);
                    }

                    // We use a very simple clock to keep the video FPS,
                    // or the video playback will be too fast
                    while (bufferInfo.presentationTimeUs / 1000 > System
                            .currentTimeMillis() - startMs) {
                        sleep(10);
                    }// */
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
        } catch (IOException e1) {
            e1.printStackTrace();
        } catch (MediaCryptoException e) {
            e.printStackTrace();
        } catch (IllegalStateException e) {
            mLogger.d("IllegalStateException");
            e.printStackTrace();
            // byffers in wrong state when trying to que,
            // most pobably because of interupted Thread -> stop rendering
        } catch (InterruptedException e) {
            mLogger.d("InteruptedException");
            // Thread was interupted stop rendering
            e.printStackTrace();

        } finally {
            if (mAudioTrack != null) {
                if (mAudioTrack.getPlayState() == AudioTrack.PLAYSTATE_PLAYING) {
                    mAudioTrack.stop();
                }
                mAudioTrack.release();
            }

            if (mMediaInfo.isProtected && mediaCrypto != null) {
                mediaCrypto.release();
            }

            if (decoder != null) {
                decoder.stop();
                decoder.release();
                decoder = null;
            }
            extractor.release();
        }
    }

    public void pause() {
        if (this.state == STATE_RUNNING) {
            this.latch = new CountDownLatch(1);
            setState(STATE_PAUSED);
        } else {
            if (this.latch != null)
                this.latch.countDown();
            setState(STATE_RUNNING);
        }
    }

    private synchronized void setState(int state) {
        this.state = state;
    }
}
