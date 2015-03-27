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

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

public class InitData {
    private static final Logger mLogger = Logger.createLog(
            InitData.class.getSimpleName(), Logger.DEBUG);

    // common key
    private final static String INIT_DATA_KEY_TITLE = "title";
    private final static String INIT_DATA_KEY_PROPATIES = "properties";
    private final static String INIT_DATA_KEY_PROP_NAME = "name";
    private final static String INIT_DATA_KEY_PROP_VERSION = "version";
    private final static String INIT_DATA_KEY_PROCESSTYPE = "process_type";
    private final static String INIT_DATA_KEY_DATATYPE = "data_type";

    // init data key
    private final static String INIT_DATA_KEY_OPTIONAL_PARAMETERS = "optional_parameters";
    private final static String INIT_DATA_KEY_PROP = "prop";

    // IPMP
    private final static String INIT_DATA_KEY_IPMP = "ipmp";
    private final static String INIT_DATA_KEY_SINF = "sinf";

    // CENC
    private final static String INIT_DATA_KEY_CENC = "cenc";
    private final static String INIT_DATA_KEY_PSSH = "pssh";
    private final static String INIT_DATA_KEY_KIDS = "kids";
    private final static String INIT_DATA_KEY_LICENSE_EMBBEDED = "license_embbeded";

    // ATKN
    private final static String INIT_DATA_KEY_ATKN = "atkn";
    private final static String INIT_DATA_KEY_MATD = "matd";
    private final static String INIT_DATA_KEY_MATU = "matu";

    // reponse key
    private final static String RESPONSE_DATA_KEY_MSG_TYPE = "message_type";
    private final static String RESPONSE_DATA_KEY_HTTP_RESPNSE = "httpResponse";
    private final static String RESPONSE_DATA_KEY_STATUS_CODE = "statusCode";
    private final static String RESPONSE_DATA_KEY_RESPNSE_BODY = "body";

    private final static String INIT_DATA_TITLE = "marlincdm_initData";
    private final static String RESPONSE_DATA_TITLE = "marlincdm_json_message";

    private static final String CURRENT_VERSION = "1.0";
    private static final String PROCESS_TYPE_ANDROID = "android";
    private static final String PROCESS_TYPE_EME = "eme";
    private static final String DATA_TYPE_NONE = "none";
    private static final String DATA_TYPE_CENC = "cenc";
    private static final String DATA_TYPE_IPMP = "ipmp";
    private static final String DATA_TYPE_ATKN = "atkn";
    private static final String DATA_TYPE_PROP = "prop";

    private static final String MESSAGE_TYPE_RESPONSE = "httpResponse";
    private static final String MESSAGE_TYPE_LICENSE = "license";

    public static byte[] getAktionTokenTableForAndroid(byte[] atkn)
            throws JSONException {
        JSONObject root = new JSONObject();
        JSONObject property = new JSONObject();
        JSONObject atkn_data = new JSONObject();

        root.put(INIT_DATA_KEY_TITLE, INIT_DATA_TITLE); // root
        property.put(INIT_DATA_KEY_PROP_NAME, "getkeyRequest_initdata"); // name
        property.put(INIT_DATA_KEY_PROP_VERSION, CURRENT_VERSION); // version
        property.put(INIT_DATA_KEY_PROCESSTYPE, PROCESS_TYPE_ANDROID); // process
                                                                       // type
        property.put(INIT_DATA_KEY_DATATYPE, DATA_TYPE_ATKN); // data type

        atkn_data.put(INIT_DATA_KEY_MATD, new String(atkn)); // actiontoken

        property.put(INIT_DATA_KEY_ATKN, atkn_data);
        root.put(INIT_DATA_KEY_PROPATIES, property);
        mLogger.d("Json Init Data(ATKN) length[" + root.toString().length()
                + "]: \n" + root.toString(4));
        return root.toString().getBytes();
    }

    public static byte[] getPSSHTableForAndroid(byte[] pssh, String kid)
            throws JSONException {
        JSONObject root = new JSONObject();
        JSONObject property = new JSONObject();
        JSONObject cenc = new JSONObject();
        JSONArray kids = new JSONArray();

        root.put(INIT_DATA_KEY_TITLE, INIT_DATA_TITLE); // root
        property.put(INIT_DATA_KEY_PROP_NAME, "getkeyRequest_initdata"); // name
        property.put(INIT_DATA_KEY_PROP_VERSION, CURRENT_VERSION); // version
        property.put(INIT_DATA_KEY_PROCESSTYPE, PROCESS_TYPE_ANDROID); // process
                                                                       // type
        property.put(INIT_DATA_KEY_DATATYPE, DATA_TYPE_CENC); // data type

        kids.put(kid); // kid
        cenc.put(INIT_DATA_KEY_PSSH, Utils.asHex(pssh)); // pssh
        cenc.put(INIT_DATA_KEY_KIDS, kids);

        property.put(INIT_DATA_KEY_CENC, cenc);
        root.put(INIT_DATA_KEY_PROPATIES, property);
        mLogger.d("Json Init Data(CENC) length[" + root.toString().length()
                + "]: \n" + root.toString(4));
        return root.toString().getBytes();
    }

    public static byte[] getIPMPTableForAndroid(byte[] ipmp)
            throws JSONException {
        JSONObject root = new JSONObject();
        JSONObject property = new JSONObject();
        JSONObject sinf = new JSONObject();

        root.put(INIT_DATA_KEY_TITLE, INIT_DATA_TITLE); // root
        property.put(INIT_DATA_KEY_PROP_NAME, "getkeyRequest_initdata"); // name
        property.put(INIT_DATA_KEY_PROP_VERSION, CURRENT_VERSION); // version
        property.put(INIT_DATA_KEY_PROCESSTYPE, PROCESS_TYPE_ANDROID); // process
                                                                       // type
        property.put(INIT_DATA_KEY_DATATYPE, DATA_TYPE_IPMP); // data type

        sinf.put(INIT_DATA_KEY_SINF, Utils.asHex(ipmp)); // ipmp

        property.put(DATA_TYPE_IPMP, sinf);
        root.put(INIT_DATA_KEY_PROPATIES, property);
        mLogger.d("Json Init Data(IPMP) length[" + root.toString().length()
                + "]: \n" + root.toString(4));
        return root.toString().getBytes();
    }

    public static byte[] getPropertyTableForAndroid(String name,
            String accountid, String serviceid) throws JSONException {
        JSONObject root = new JSONObject();
        JSONObject property = new JSONObject();
        JSONObject propData = new JSONObject();
        JSONArray options = new JSONArray();
        JSONObject option = new JSONObject();

        root.put(INIT_DATA_KEY_TITLE, INIT_DATA_TITLE); // root
        property.put(INIT_DATA_KEY_PROP_NAME, "getkeyRequest_initdata"); // name
        property.put(INIT_DATA_KEY_PROP_VERSION, CURRENT_VERSION); // version
        property.put(INIT_DATA_KEY_PROCESSTYPE, PROCESS_TYPE_ANDROID); // process
                                                                       // type
        property.put(INIT_DATA_KEY_DATATYPE, DATA_TYPE_PROP); // data type

        propData.put(INIT_DATA_KEY_PROP_NAME, name);
        options.put(new JSONObject().put("account.id", accountid));
        options.put(new JSONObject().put("service.id", serviceid));

        propData.put(INIT_DATA_KEY_OPTIONAL_PARAMETERS, options);
        property.put(INIT_DATA_KEY_PROP, propData);
        root.put(INIT_DATA_KEY_PROPATIES, property);
        mLogger.d("Json Init Data(PROP) length[" + root.toString().length()
                + "]: \n" + root.toString(4));
        return root.toString().getBytes();
    }

    public static byte[] getResponseTableForAndroid(byte[] response, int status)
            throws JSONException {
        JSONObject root = new JSONObject();
        JSONObject property = new JSONObject();
        JSONObject http_response = new JSONObject();

        root.put(INIT_DATA_KEY_TITLE, RESPONSE_DATA_TITLE); // root
        property.put(INIT_DATA_KEY_PROP_NAME,
                "getkeyRequest_json_response_message"); // name
        property.put(INIT_DATA_KEY_PROP_VERSION, CURRENT_VERSION); // version
        property.put(RESPONSE_DATA_KEY_MSG_TYPE, MESSAGE_TYPE_RESPONSE); // massage
                                                                         // type

        http_response.put(RESPONSE_DATA_KEY_STATUS_CODE, status); // status code
        http_response.put(RESPONSE_DATA_KEY_RESPNSE_BODY, new String(response)); // response

        property.put(RESPONSE_DATA_KEY_HTTP_RESPNSE, http_response);
        root.put(INIT_DATA_KEY_PROPATIES, property);
        mLogger.d("Json Response Data length[" + root.toString().length()
                + "]: \n" + root.toString(4));
        return root.toString().getBytes();
    }
}
