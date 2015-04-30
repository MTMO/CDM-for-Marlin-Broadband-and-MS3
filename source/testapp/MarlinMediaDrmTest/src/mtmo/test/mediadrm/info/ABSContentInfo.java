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

package mtmo.test.mediadrm.info;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

import mtmo.test.mediadrm.Logger;
import android.text.TextUtils;

public class ABSContentInfo implements Serializable {
    protected static final Logger mLogger = Logger
            .createLog(ABSContentInfo.class.getSimpleName());

    private final String displayName;
    private final String videoPath;
    private final String videoKid;
    private final String audioPath;
    private final String audiokid;
    private final String mpdUrl;

    public ABSContentInfo(String displayName, String VideoPath, String VideoKid,
            String AudioPath, String Audiokid, String mpdUrl) {
        this.displayName = displayName;
        this.videoPath = VideoPath;
        this.videoKid = VideoKid;
        this.audioPath = AudioPath;
        this.audiokid = Audiokid;
        this.mpdUrl = mpdUrl;
    }

    @Override
    public String toString() {
        return displayName;
    }

    public String getDisplayName() {
        return displayName;
    }

    public String getVideoPath() {
        return videoPath;
    }

    public String getVideoKid() {
        return videoKid;
    }

    public String getAudioPath() {
        return audioPath;
    }

    public String getAudiokid() {
        return audiokid;
    }

    public String getMpdUrl() {
        return mpdUrl;
    }

    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 1;
        result = prime * result + ((displayName == null) ? 0 : displayName.hashCode());
        result = prime * result + ((videoPath == null) ? 0 : videoPath.hashCode());
        result = prime * result + ((videoKid == null) ? 0 : videoKid.hashCode());
        result = prime * result + ((audioPath == null) ? 0 : audioPath.hashCode());
        result = prime * result + ((audiokid == null) ? 0 : audiokid.hashCode());
        result = prime * result + ((mpdUrl == null) ? 0 : mpdUrl.hashCode());
        return result;
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj)
            return true;
        if (obj == null)
            return false;
        if (getClass() != obj.getClass())
            return false;
        ABSContentInfo other = (ABSContentInfo) obj;
        if (displayName == null) {
            if (other.displayName != null)
                return false;
        } else if (!displayName.equals(other.displayName))
            return false;
        if (videoPath == null) {
            if (other.videoPath != null)
                return false;
        } else if (!videoPath.equals(other.videoPath))
            return false;
        if (videoKid == null) {
            if (other.videoKid != null)
                return false;
        } else if (!videoKid.equals(other.videoKid))
            return false;
        if (audioPath == null) {
            if (other.audioPath != null)
                return false;
        } else if (!audioPath.equals(other.audioPath))
            return false;
        if (audiokid == null) {
            if (other.audiokid != null)
                return false;
        } else if (!audiokid.equals(other.audiokid))
            return false;
        if (mpdUrl == null) {
            if (other.mpdUrl != null)
                return false;
        } else if (!mpdUrl.equals(other.mpdUrl))
            return false;
        return true;
    }

    public static List<ABSContentInfo> parseFromProeprties(InputStream is) {
        BufferedReader br = null;
        List<ABSContentInfo> rtn = null;

        try {
            br = new BufferedReader(new InputStreamReader(is));
            String line;
            int current = 0;
            while ((line = br.readLine()) != null) {
                current++;

                String[] values = line.split(",");
                if (values == null || values.length < 6) {
                    mLogger.d("invalid line, ignored Line-" + current);
                    break;
                }

                if (TextUtils.isEmpty(values[0])) {
                    mLogger.d("invalid line, DisplayName is empty Line-"
                            + current);
                    break;
                }
                if (TextUtils.isEmpty(values[1])) {
                    mLogger.d("invalid line, video path is empty Line-"
                            + current);
                    break;
                }
                if (TextUtils.isEmpty(values[2])) {
                    mLogger.d("invalid line, Kid for video is empty Line-" + current);
                    break;
                }
                if (TextUtils.isEmpty(values[3])) {
                    mLogger.d("invalid line, audio path is empty Line-" + current);
                    break;
                }
                if (TextUtils.isEmpty(values[4])) {
                    mLogger.d("invalid line, Kid for audio is empty Line-" + current);
                    break;
                }
                if (TextUtils.isEmpty(values[5])) {
                    mLogger.d("invalid line, mpd path is empty Line-" + current);
                    break;
                }
                ABSContentInfo info = new ABSContentInfo(values[0], values[1],
                        values[2], values[3], values[4], values[5]);

                if (rtn == null) {
                    rtn = new ArrayList<ABSContentInfo>();
                }
                rtn.add(info);
                break;
            }
        } catch (IOException e) {
            mLogger.e("error occurred when parsing.", e);
            rtn = null;
        } finally {
            try {
                if (br != null) {
                    br.close();
                }
            } catch (IOException e) {
            }
        }

        return rtn;
    }

}
