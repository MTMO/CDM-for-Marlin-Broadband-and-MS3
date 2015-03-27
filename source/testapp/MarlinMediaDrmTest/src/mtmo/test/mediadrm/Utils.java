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

package mtmo.test.mediadrm;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Locale;

import org.apache.http.util.ByteArrayBuffer;

public class Utils {

    private static final int BYTES_OF_ACCOUNT_ID = 8;

    private static final String AUDIO_SINF = Constants.BOX_DATA_DIRECTORY_PATH + "audio.sinf";
    private static final String VIDEO_SINF = Constants.BOX_DATA_DIRECTORY_PATH + "video.sinf";
    private static final String AUDIO_PSSH = Constants.BOX_DATA_DIRECTORY_PATH + "cenc_audio_pssh_box.dat";
    private static final String VIDEO_PSSH = Constants.BOX_DATA_DIRECTORY_PATH + "cenc_video_pssh_box.dat";

    public static String accountIdToMarlinFormat(final String accountId) {
        ByteBuffer accountIdBuf = ByteBuffer.allocate(BYTES_OF_ACCOUNT_ID);
        try {
            accountIdBuf.putLong(Long.valueOf(accountId));
        } catch (Exception e) {
            return null;
        }
        accountIdBuf.order(ByteOrder.LITTLE_ENDIAN);
        return String.format(Locale.US, "%016x", accountIdBuf.getLong(0));
    }

    public static byte[] readActionTokenFromFile(String path) {
        StringBuilder builder = new StringBuilder();
        FileInputStream fis = null;
        byte[] buff = new byte[1024];
        int ret = 0;
        File file = null;

        if (path.isEmpty()) {
            return null;
        }

        file = new File(path);
        if (!file.exists()) {
            return null;
        }

        try {
            fis = new FileInputStream(file);
            while ((ret = fis.read(buff)) > 0) {
                builder.append(new String(buff, 0, ret));
            }
        } catch (IOException e) {
            return null;
        } finally {
            if (fis != null) {
                try {
                    fis.close();
                } catch (IOException e) {
                }
                fis = null;
            }
        }
        return builder.toString().getBytes();
    }

    public static byte[] readIPMPDataFromFile(boolean isVideo) {
        ByteArrayBuffer buffer = new ByteArrayBuffer(0);
        FileInputStream fis = null;
        byte[] buff = new byte[1024];
        int ret = 0;
        File file = null;
        String path;

        if (isVideo) {
            path = VIDEO_SINF;
        } else {
            path = AUDIO_SINF;
        }

        file = new File(path);
        if (!file.exists()) {
            return null;
        }

        try {
            fis = new FileInputStream(file);
            while ((ret = fis.read(buff)) > 0) {
                buffer.append(buff, 0, ret);
            }
        } catch (IOException e) {
            return null;
        } finally {
            if (fis != null) {
                try {
                    fis.close();
                } catch (IOException e) {
                }
                fis = null;
            }
        }
        return buffer.buffer();
    }

    public static byte[] readPsshDataFromFile(boolean isVideo) {
        ByteArrayBuffer buffer = new ByteArrayBuffer(0);
        FileInputStream fis = null;
        byte[] buff = new byte[1024];
        int ret = 0;
        File file = null;
        String path;

        if (isVideo) {
            path = VIDEO_PSSH;
        } else {
            path = AUDIO_PSSH;
        }

        file = new File(path);
        if (!file.exists()) {
            return null;
        }

        try {
            fis = new FileInputStream(file);
            while ((ret = fis.read(buff)) > 0) {
                buffer.append(buff, 0, ret);
            }
        } catch (IOException e) {
            return null;
        } finally {
            if (fis != null) {
                try {
                    fis.close();
                } catch (IOException e) {
                }
                fis = null;
            }
        }
        return buffer.buffer();
    }

    public static String asHex(byte bytes[]) {
        StringBuffer strbuf = null;

        if (bytes == null) {
            return "";
        }
        strbuf = new StringBuffer(bytes.length * 2);

        for (int index = 0; index < bytes.length; index++) {
            int bt = bytes[index] & 0xff;
            if (bt < 0x10) {
                strbuf.append("0");
            }
            strbuf.append(Integer.toHexString(bt));
        }
        return strbuf.toString();
    }

}
