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

import java.util.HashMap;
import java.util.Map;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

public class RequestParser {
    private static final Logger mLogger = Logger.createLog(
            RequestParser.class.getSimpleName(), Logger.DEBUG);
    public static final String MESSAGE_TYPE_NONE = "none";
    public static final String MESSAGE_TYPE_LICENSE = "license";
    public static final String MESSAGE_TYPE_REQUEST = "httpRequest";
    public static final String MESSAGE_TYPE_PROPERTY = "prop";

    public static final String METHOD_TYPE_GET = "GET";
    public static final String METHOD_TYPE_POST = "POST";

    private static byte[] mRequest = null;
    private static String version = "";
    private static String type;
    private static String url = "";
    private static int remaining_step = 0;
    private static String method;
    private static Map<String, String> header_list;
    private static byte[] request;
    private static String license = "";
    private static String property = "";;

    private final static String REQUEST_MESSAGE_KEY_PROPERTY = "properties";
    private final static String REQUEST_MESSAGE_KEY_VERSION = "version";
    private final static String REQUEST_MESSAGE_KEY_STEPS = "remaining_steps";
    private final static String REQUEST_MESSAGE_KEY_MSG_TYPE = "message_type";
    private final static String REQUEST_MESSAGE_KEY_REQUEST = "httpRequest";
    private final static String REQUEST_MESSAGE_KEY_METHOD = "method";
    private final static String REQUEST_MESSAGE_KEY_URL = "url";
    private final static String REQUEST_MESSAGE_KEY_HEADERS = "headers";
    private final static String REQUEST_MESSAGE_KEY_HEADER_NAME = "name";
    private final static String REQUEST_MESSAGE_KEY_HEADER_VALUE = "value";
    private final static String REQUEST_MESSAGE_KEY_BODY = "body";
    private final static String REQUEST_MESSAGE_KEY_LICENSE = "license";
    private final static String REQUEST_MESSAGE_KEY_PROP = "prop";

    public RequestParser(byte[] request) {
        mRequest = request;
    }

    public boolean parse() {
        JSONObject root, property, http_request = null;

        try {
            root = new JSONObject(new String(mRequest));
            if (root.isNull(REQUEST_MESSAGE_KEY_PROPERTY)) {
                return false;
            }
            mLogger.d("request: \n" + root.toString(4));
            property = root.getJSONObject(REQUEST_MESSAGE_KEY_PROPERTY);
        } catch (JSONException e) {
            e.printStackTrace();
            return false;
        }

        if (!readVersion(property)) {
            return false;
        }
        if (!readFormatType(property)) {
            return false;
        }
        if (!readRemainingStep(property)) {
            return false;
        }

        if (!MESSAGE_TYPE_NONE.equals(type)) {
            if (MESSAGE_TYPE_LICENSE.equals(type)) {
                readLicense(property);
            } else if (type.equals(MESSAGE_TYPE_REQUEST)) {
                try {
                    if (property.isNull(REQUEST_MESSAGE_KEY_REQUEST)) {
                        return false;
                    }
                    http_request = property
                            .getJSONObject(REQUEST_MESSAGE_KEY_REQUEST);
                } catch (JSONException e) {
                    e.printStackTrace();
                    return false;
                }

                if (!readMethod(http_request) || !readUrl(http_request)
                        || !readOptionParameter(http_request)
                        || !readRequestBody(http_request)) {
                    return false;
                }
            } else if (MESSAGE_TYPE_PROPERTY.equals(type)) {
                readProperty(property);
            } else {
                return false;
            }
        }
        return true;
    }

    private static boolean readVersion(JSONObject propaties) {
        try {
            if (propaties.isNull(REQUEST_MESSAGE_KEY_VERSION)) {
                return false;
            }
            version = propaties.getString(REQUEST_MESSAGE_KEY_VERSION);
            mLogger.d("version: " + version);
            return true;
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return false;
    }

    private static boolean readRemainingStep(JSONObject propaties) {
        try {
            if (propaties.isNull(REQUEST_MESSAGE_KEY_STEPS)) {
                return false;
            }
            remaining_step = propaties.getInt(REQUEST_MESSAGE_KEY_STEPS);
            mLogger.d("remaining step: " + remaining_step);
            return true;
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return false;
    }

    private static boolean readFormatType(JSONObject propaties) {
        try {
            if (propaties.isNull(REQUEST_MESSAGE_KEY_MSG_TYPE)) {
                return false;
            }
            type = propaties.getString(REQUEST_MESSAGE_KEY_MSG_TYPE);
            return true;
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return false;
    }

    private static boolean readUrl(JSONObject httpRequest) {
        try {
            if (httpRequest.isNull(REQUEST_MESSAGE_KEY_URL)) {
                return false;
            }
            url = httpRequest.getString(REQUEST_MESSAGE_KEY_URL);
            mLogger.d("url: " + url);
            return true;
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return false;
    }

    private static boolean readMethod(JSONObject httpRequest) {
        try {
            if (httpRequest.isNull(REQUEST_MESSAGE_KEY_METHOD)) {
                return false;
            }
            method = httpRequest.getString(REQUEST_MESSAGE_KEY_METHOD);
            mLogger.d("method: " + method);
            return true;
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return false;
    }

    private static boolean readOptionParameter(JSONObject httpRequest) {
        Map<String, String> optionPalameter = new HashMap<String, String>();
        try {
            if (httpRequest.isNull(REQUEST_MESSAGE_KEY_HEADERS)) {
                return true;
            }
            JSONArray itemArray = httpRequest
                    .getJSONArray(REQUEST_MESSAGE_KEY_HEADERS);
            int count = itemArray.length();
            JSONObject[] headers = new JSONObject[count];
            for (int i = 0; i < count; i++) {
                headers[i] = itemArray.getJSONObject(i);
                if (headers[i].isNull(REQUEST_MESSAGE_KEY_HEADER_NAME)
                        || headers[i].isNull(REQUEST_MESSAGE_KEY_HEADER_VALUE)) {
                    return false;
                }

                optionPalameter.put(
                        headers[i].getString(REQUEST_MESSAGE_KEY_HEADER_NAME),
                        headers[i].getString(REQUEST_MESSAGE_KEY_HEADER_VALUE));
            }
            header_list = optionPalameter;
            return true;
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return false;
    }

    private static boolean readRequestBody(JSONObject httpRequest) {
        String tmp = null;
        try {
            if (httpRequest.isNull(REQUEST_MESSAGE_KEY_BODY)) {
                return true;
            }
            tmp = httpRequest.getString(REQUEST_MESSAGE_KEY_BODY);
            request = tmp.getBytes();
            return true;
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return false;
    }

    private static boolean readLicense(JSONObject propaties) {
        try {
            if (propaties.isNull(REQUEST_MESSAGE_KEY_LICENSE)) {
                return false;
            }
            license = propaties.getString(REQUEST_MESSAGE_KEY_LICENSE);
            return true;
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return false;
    }

    private static boolean readProperty(JSONObject propaties) {
        try {
            if (propaties.isNull(REQUEST_MESSAGE_KEY_PROP)) {
                return false;
            }
            property = propaties.getString(REQUEST_MESSAGE_KEY_PROP);
            return true;
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return false;
    }

    public String getVersion() {
        return version;
    }

    public String getType() {
        return type;
    }

    public String getUrl() {
        return url;
    }

    public int getRemaining_step() {
        return remaining_step;
    }

    public String getMethod() {
        return method;
    }

    public Map<String, String> getHeader_list() {
        return header_list;
    }

    public byte[] getRequest() {
        return request;
    }

    public String getLicense() {
        return license;
    }

    public String getProperty() {
        return property;
    }
}
