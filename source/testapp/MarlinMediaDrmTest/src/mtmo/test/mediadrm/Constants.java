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

import java.util.UUID;

import android.os.Environment;

public class Constants {
    public static final String APP_NAME = "MarlinMediaDrmTest";
    public static final String LOG_TAG = APP_NAME;

    public static final String CONFIG_DATA_DIRECTORY_PATH = Environment
            .getExternalStorageDirectory() + "/TestData/";
    public static final String VIDEO_DIRECTORY_PATH = CONFIG_DATA_DIRECTORY_PATH + "/VIDEO/";
    public static final String BOX_DATA_DIRECTORY_PATH = CONFIG_DATA_DIRECTORY_PATH + "/box_data/";
    public static final String ACTION_DIRECTORY_PATH = CONFIG_DATA_DIRECTORY_PATH + "/actiontoken/";
    public static final String ABS_CONTENT_LIST = CONFIG_DATA_DIRECTORY_PATH + "/abs_content_list";
    public static final String PROPERTY_FILE_PATH = CONFIG_DATA_DIRECTORY_PATH + "/test_prop";

    /*
     * required name for call the getPropertyString()
     */
    public static final String QUERY_NAME_DUID = "duid";
    public static final String QUERY_NAME_REGISTERED_STATE = "registered_state";

    /*
     * required mime type for call the getKeyRequest()
     */
    public static final String REQUEST_MIMETYPE_REGISTRATION = "application/vnd.marlin.drm.actiontoken+xml+registration";
    public static final String REQUEST_MIMETYPE_LICENSE = "application/vnd.marlin.drm.actiontoken+xml+license";
    public static final String REQUEST_MIMETYPE_DEREGISTRATION = "application/vnd.marlin.drm.actiontoken+xml+deregistration";
    public static final String REQUEST_MIMETYPE_QUERY_PROPERTY = "application/vnd.marlin.drm.metadata.property";

    /*
     * UUID
     */
    public static final UUID MBB_UUID = new UUID(0x4fc955b3b9344111L,
            0x9de98ff7dad07170L);

    /*
     * app mode
     */
    public static final String PREFERENCE_APP_MODE = "app_mode";
    public static final int APP_MODE_OFFLINE = 0;
    public static final int APP_MODE_ABS = 1;
}
