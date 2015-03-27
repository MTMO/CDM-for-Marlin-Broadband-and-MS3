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

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.ProtocolException;
import java.net.URL;
import java.util.HashMap;
import java.util.Map;

import mtmo.test.mediadrm.TaskListener.TaskResult;
import mtmo.test.mediadrm.info.TaskInfo;
import mtmo.test.mediadrm.info.TaskInfo.TaskType;
import android.content.Context;
import android.media.MediaDrm;
import android.media.MediaDrm.KeyRequest;
import android.media.NotProvisionedException;
import android.media.UnsupportedSchemeException;
import android.os.AsyncTask;

public class DrmProcessExecutor extends AsyncTask<TaskInfo, Void, TaskResult> {
    private static final Logger mLogger = Logger.createLog(
            DrmProcessExecutor.class.getSimpleName(), Logger.DEBUG);

    public static final String LICENSE_FILENAME = "license.dat";
    public static final String PROCESSED_DATA_TEMP = "processed_data.dat";
    public static final int NETWORK_TIMEOUT = 30 * 1000;// ms
    public static final int READ_TIMEOUT = 50 * 1000;// ms
    public static final int BUFFER_SIZE = 1024 * 8;// bytes

    private final Context mContext;
    private final TaskListener listener;
    private TaskInfo currTask;

    public DrmProcessExecutor(Context mContext, TaskListener listener) {
        this.mContext = mContext;
        this.listener = listener;
    }

    @Override
    protected TaskResult doInBackground(TaskInfo... params) {
        mLogger.enter("DrmProcessTask#doInBackground");
        HashMap<String, String> optionalParameters = new HashMap<String, String>();
        KeyRequest keyRequest = null;
        byte[] request = null;
        byte[] atkn = null;
        byte[] sessionId = null;
        byte[] response_body = null;
        byte[] response = null;
        MediaDrm mediaDrm;
        RequestParser parser;
        int http_status = -1;

        try {
            mediaDrm = new MediaDrm(Constants.MBB_UUID);
            sessionId = mediaDrm.openSession();
        } catch (UnsupportedSchemeException e1) {
            e1.printStackTrace();
            return TaskResult.DRM_PROCESS_INTERRUPTED;
        } catch (NotProvisionedException e) {
            e.printStackTrace();
            return TaskResult.DRM_PROCESS_INTERRUPTED;
        } catch (RuntimeException e) {
            e.printStackTrace();
            return TaskResult.DRM_PROCESS_INTERRUPTED;
        }

        currTask = params[0];
        atkn = Utils.readActionTokenFromFile(currTask.serviceTokenPath);
        if (atkn == null) {
            mLogger.e("ActionToken[ " + currTask.serviceTokenPath +" ] is empty");
            return TaskResult.DRM_PROCESS_INTERRUPTED;
        }
        if (currTask.serviceId != null && !currTask.serviceId.isEmpty()) {
            mLogger.d("serviceId : " + currTask.serviceId);
            optionalParameters.put("service.id", currTask.serviceId);
        }

        if (currTask.accountId != null && !currTask.accountId.isEmpty()) {
            mLogger.d("accountId : " + currTask.accountId);
            optionalParameters.put("account.id",
                    Utils.accountIdToMarlinFormat(currTask.accountId));
        }

        // Generate request with action token
        try {
            keyRequest = getKeyRequest(mediaDrm, sessionId, atkn,
                    optionalParameters, currTask.taskType);
            if (keyRequest == null || keyRequest.getData() == null
                    || keyRequest.getData().length <= 0) {
                mLogger.e("getKeyRequest failure");
                return getActualTaskResultFromError();
            }
        } catch (IllegalArgumentException e) {
            mediaDrm.closeSession(sessionId);
            e.printStackTrace();
            return TaskResult.DRM_PROCESS_FAILED_ILLEGAL_ARGUMENT_OR_STATEMENT;
        } catch (IllegalStateException e) {
            mediaDrm.closeSession(sessionId);
            e.printStackTrace();
            return TaskResult.DRM_PROCESS_FAILED_UNKNOWN;
        } catch (Exception e) {
            mediaDrm.closeSession(sessionId);
            e.printStackTrace();
            return TaskResult.DRM_PROCESS_FAILED_UNKNOWN;
        }

        request = keyRequest.getData();
        parser = new RequestParser(request);
        if (!parser.parse()) {
            mLogger.e("parse failure");
            mediaDrm.closeSession(sessionId);
            return TaskResult.DRM_PROCESS_INTERRUPTED;
        }
        http_status = httpProcess(parser);
        if (http_status < 0) {
            mLogger.e("http Process failure");
        }

        while (parser.getRemaining_step() != 0) {
            try {
                response_body = readProcessedDataFromFile();
                response = InitData.getResponseTableForAndroid(response_body,
                        http_status);
                request = mediaDrm.provideKeyResponse(sessionId, response);

                parser = new RequestParser(request);
                if (!parser.parse()) {
                    mLogger.e("parse failure");
                    mediaDrm.closeSession(sessionId);
                    return TaskResult.DRM_PROCESS_INTERRUPTED;
                }
                if (RequestParser.MESSAGE_TYPE_NONE.equals(parser.getType())) {
                    mLogger.d("Format Type is None.");
                    break;
                }
                if (RequestParser.MESSAGE_TYPE_LICENSE.equals(parser.getType())) {
                    mLogger.d("Format Type is License.");
                    storeLicenseFile(parser.getLicense());
                    break;
                }
                http_status = httpProcess(parser);
                if (http_status < 0) {
                    mLogger.e("http Process failure");
                }
            } catch (IllegalArgumentException e) {
                mediaDrm.closeSession(sessionId);
                e.printStackTrace();
                return TaskResult.DRM_PROCESS_FAILED_ILLEGAL_ARGUMENT_OR_STATEMENT;
            } catch (IllegalStateException e) {
                mediaDrm.closeSession(sessionId);
                e.printStackTrace();
                return TaskResult.DRM_PROCESS_FAILED_UNKNOWN;
            } catch (NotProvisionedException e) {
                mediaDrm.closeSession(sessionId);
                e.printStackTrace();
                return TaskResult.DRM_PROCESS_INTERRUPTED;
            } catch (Exception e) {
                mediaDrm.closeSession(sessionId);
                e.printStackTrace();
                return TaskResult.DRM_PROCESS_FAILED_UNKNOWN;
            }
        }

        mediaDrm.closeSession(sessionId);
        mLogger.exit("DrmProcessTask#doInBackground");
        return TaskResult.DRM_PROCESS_SUCCESS;
    }

    @Override
    protected void onPostExecute(TaskResult result) {
        mContext.deleteFile(PROCESSED_DATA_TEMP);
        listener.onTaskFinished(result);
    }

    protected KeyRequest getKeyRequest(MediaDrm mediaDrm, byte[] sessionId,
            byte[] atkn, HashMap<String, String> optionalParameters,
            TaskType taskType) throws IllegalArgumentException, Exception {
        KeyRequest request = null;
        if (taskType.equals(TaskType.REGISTRATION)) {
            request = mediaDrm.getKeyRequest(sessionId,
                    InitData.getAktionTokenTableForAndroid(atkn),
                    Constants.REQUEST_MIMETYPE_REGISTRATION,
                    MediaDrm.KEY_TYPE_OFFLINE, optionalParameters);
        } else if (taskType.equals(TaskType.LICENSE)) {
            request = mediaDrm.getKeyRequest(sessionId,
                    InitData.getAktionTokenTableForAndroid(atkn),
                    Constants.REQUEST_MIMETYPE_LICENSE,
                    MediaDrm.KEY_TYPE_OFFLINE, optionalParameters);
        } else if (taskType.equals(TaskType.DEREGISTRATION)) {
            request = mediaDrm.getKeyRequest(sessionId,
                    InitData.getAktionTokenTableForAndroid(atkn),
                    Constants.REQUEST_MIMETYPE_DEREGISTRATION,
                    MediaDrm.KEY_TYPE_RELEASE, optionalParameters);
        }
        return request;
    }

    private byte[] readProcessedDataFromFile() {
        StringBuilder builder = new StringBuilder();
        FileInputStream fis = null;
        byte[] buff = new byte[1024];
        int ret = 0;

        try {
            fis = mContext.openFileInput(PROCESSED_DATA_TEMP);
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

    private boolean storeLicenseFile(String license) {
        OutputStream str_out;
        try {
            str_out = mContext.openFileOutput(LICENSE_FILENAME,
                    Context.MODE_WORLD_READABLE);
            PrintWriter writer = new PrintWriter(
                    new OutputStreamWriter(str_out));
            writer.println(license);
            writer.close();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
        return true;
    }

    protected int httpProcess(RequestParser parser) {
        int statusCode = -1;
        BufferedInputStream in = null;
        FileOutputStream fos = null;
        String type = parser.getType();
        String method = parser.getMethod();
        byte[] body = parser.getRequest();
        Map<String, String> header_list = parser.getHeader_list();

        mContext.deleteFile(PROCESSED_DATA_TEMP);
        try {
            if (RequestParser.MESSAGE_TYPE_REQUEST.equals(type)
                    && RequestParser.METHOD_TYPE_GET.equals(method)) {
                URL url = new URL(parser.getUrl());
                HttpURLConnection http = (HttpURLConnection) url
                        .openConnection();
                http.setRequestMethod("GET");
                http.connect();

                in = new BufferedInputStream(http.getInputStream());
                OutputStream str_out = mContext.openFileOutput(
                        PROCESSED_DATA_TEMP, Context.MODE_WORLD_READABLE);
                String line = "";
                BufferedReader reader = new BufferedReader(
                        new InputStreamReader(http.getInputStream()));
                statusCode = http.getResponseCode();
                PrintWriter writer = new PrintWriter(new OutputStreamWriter(
                        str_out, "UTF-8"));
                while ((line = reader.readLine()) != null) {
                    writer.append(line);
                }
                writer.close();

            } else if (RequestParser.MESSAGE_TYPE_REQUEST.equals(type)
                    && RequestParser.METHOD_TYPE_POST.equals(method)) {
                OutputStream os = null;
                URL url = new URL(parser.getUrl());
                HttpURLConnection http = (HttpURLConnection) url
                        .openConnection();
                http.setRequestMethod("POST");

                http.setConnectTimeout(NETWORK_TIMEOUT);
                http.setReadTimeout(NETWORK_TIMEOUT);
                http.setDoOutput(true);
                http.setDoInput(true);

                if ((header_list != null) && (header_list.size() > 0)) {
                    for (Map.Entry<String, String> entry : header_list
                            .entrySet()) {
                        String key = entry.getKey();
                        String value = entry.getValue();

                        mLogger.d("setRequestProperty( " + key + " , " + value
                                + " )");
                        http.setRequestProperty(key, value);
                    }
                }
                if (body != null) {
                    try {
                        os = new BufferedOutputStream(http.getOutputStream());
                        os.write(body);
                        os.flush();

                        statusCode = http.getResponseCode();
                        mLogger.d("httpStatusCode: " + statusCode);

                        in = new BufferedInputStream(http.getInputStream());
                        fos = mContext.openFileOutput(PROCESSED_DATA_TEMP,
                                Context.MODE_WORLD_READABLE);

                        byte[] buffer = new byte[BUFFER_SIZE];
                        int size = -1;
                        while ((size = in.read(buffer)) > 0) {
                            fos.write(buffer, 0, size);
                        }
                    } catch (IOException e) {
                        mLogger.e("Could not open stream.", e);
                        statusCode = -1;
                    } finally {
                        if (os != null) {
                            try {
                                os.close();
                            } catch (IOException e) {
                            }
                            os = null;
                        }
                        if (fos != null) {
                            try {
                                fos.close();
                            } catch (IOException e) {
                            }
                            fos = null;
                        }
                    }
                }
            } else {
                statusCode = -1;
            }
        } catch (MalformedURLException e) {
            e.printStackTrace();
            statusCode = -1;
        } catch (ProtocolException e) {
            e.printStackTrace();
            statusCode = -1;
        } catch (IOException e) {
            e.printStackTrace();
            statusCode = -1;
        }
        return statusCode;
    }

    protected TaskResult getActualTaskResultFromError() {

        switch (currTask.taskType) {
        case REGISTRATION: {
            return TaskResult.DRM_PROCESS_FAILED_REGISTER_ACCOUNT;
        }
        case SUBS_REGISTRATION: {
            return TaskResult.DRM_PROCESS_FAILED_REGISTER_SUBSCRIPTION;
        }
        case LICENSE:
            return TaskResult.DRM_PROCESS_FAILED_ACQUIRE_LICENSE;
        case DEREGISTRATION:
            return TaskResult.DRM_PROCESS_FAILED_DEREGISTER_ACCOUNT;
        case SUBS_DEREGISTRATION:
            return TaskResult.DRM_PROCESS_FAILED_DEREGISTER_SUBSCRIPTION;
        default:
            return TaskResult.DRM_PROCESS_FAILED_UNKNOWN;
        }
    }
}
