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

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.concurrent.CountDownLatch;

import mtmo.test.mediadrm.TaskListener.TaskResult;
import mtmo.test.mediadrm.info.ABSContentInfo;
import mtmo.test.mediadrm.info.TaskInfo;
import mtmo.test.mediadrm.info.TaskInfo.TaskType;

import org.json.JSONException;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences.Editor;
import android.media.MediaDrm;
import android.media.MediaDrm.KeyRequest;
import android.media.NotProvisionedException;
import android.media.UnsupportedSchemeException;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.text.format.Time;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity extends Activity {
    private static final Logger mLogger = Logger.createLog(MainActivity.class
            .getSimpleName());

    private static final String DIALOG_TASK_PROCESSING = "task_processing";
    private static final String DIALOG_SWITCH_MODE = "switch_mode";
    private static final int MENU_ID_MENU1 = 0;

    private Spinner mSpinnerABSContentList = null;
    private Spinner mSpinnerOfflineContentList = null;
    private Spinner mSpinnerActionTokenList = null;

    private Context mContext;
    private String mAccountId = null;
    private String mServiceId = null;
    private ArrayList<String> mATKNFilenames;
    private ArrayList<String> mOfflineFilenames;
    private ArrayAdapter<String> mActionTokenAdapter;
    private ArrayAdapter<String> mOfflineContentAdapter;
    private String mCurrentATKNFilePath;
    private String mCurrentOfflineContentPath;
    private Handler mHandler;
    private TaskRunner mTaskRunner = null;
    private int appMode;

    private class TaskStarter implements Runnable {
        private final TaskInfo taskInfo;

        public TaskStarter(TaskInfo taskInfo) {
            this.taskInfo = taskInfo;
        }

        @Override
        public void run() {
            boolean isStartedRunner = false;

            synchronized (MainActivity.this) {
                if (mTaskRunner == null) {
                    mTaskRunner = new TaskRunner(taskInfo);
                    new Thread(mTaskRunner).start();
                    isStartedRunner = true;
                }
            }

            if (isStartedRunner) {
                TaskProgressDialog progressDialog = new TaskProgressDialog();
                progressDialog.title = taskInfo.taskType.name();
                progressDialog.show(MainActivity.this.getFragmentManager(),
                        DIALOG_TASK_PROCESSING);
            } else {
                Toast.makeText(MainActivity.this,
                        "Anoteher process is already running. Ignored.",
                        Toast.LENGTH_SHORT).show();
            }
        }

    }

    private class TaskRunner implements Runnable {
        private final TaskInfo taskInfo;
        AsyncTask<TaskInfo, Void, TaskResult> asynTask;
        private TaskListenerImpl mTaskListener;

        public TaskRunner(TaskInfo taskInfo) {
            this.taskInfo = taskInfo;
            this.mTaskListener = new TaskListenerImpl();
        }

        @Override
        public void run() {
            mLogger.d("Start task for " + taskInfo.taskType.name());

            /*
             * Execute DRM Process
             */
            final CountDownLatch taskForDrmProcess = new CountDownLatch(1);
            mTaskListener.setCountDownLatch(taskForDrmProcess);

            asynTask = new DrmProcessExecutor(MainActivity.this, mTaskListener);
            asynTask.execute(taskInfo);

            try {
                taskForDrmProcess.await();
            } catch (InterruptedException e) {
            }

            mLogger.d("Execut DRM Process finished: "
                    + mTaskListener.getResult().name());
            mLogger.d("Finish task for " + taskInfo.taskType.name());

            MainActivity.this.onTaskFinished(convertResultToMessage(taskInfo,
                    mTaskListener.getResult()));
            return;
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        mLogger.enter("onCreate");

        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_INDETERMINATE_PROGRESS);

        appMode = getSharedPreferences(Constants.APP_NAME, MODE_PRIVATE)
                .getInt(Constants.PREFERENCE_APP_MODE,
                        Constants.APP_MODE_OFFLINE);

        switch (appMode) {
        case Constants.APP_MODE_ABS:
            setContentView(R.layout.main_abs);
            break;
        case Constants.APP_MODE_OFFLINE:
            setContentView(R.layout.activity_main);
            break;
        default:
            // setContentView(R.layout.main);
            break;
        }

        mContext = getApplicationContext();
        mHandler = new Handler();

        readPropFromFile();
        setupDropDownList();
        listFiles();
        setupDrmProcessButton(appMode);
        setupPlayerButton();

        checkMarlinPluginInfo();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        menu.add(Menu.NONE, MENU_ID_MENU1, Menu.NONE, "Switch Mode");
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
        case MENU_ID_MENU1:
            SwitchModeDialogFragment switchMode = new SwitchModeDialogFragment();
            switchMode.show(getFragmentManager(), DIALOG_SWITCH_MODE);
            break;
        default:
            break;
        }
        return true;
    }

    private void checkMarlinPluginInfo() {
        try {
            MediaDrm mediaDrm = new MediaDrm(Constants.MBB_UUID);
            String duid = mediaDrm.getPropertyString(Constants.QUERY_NAME_DUID);
            mLogger.d("duid: " + duid);
        } catch (UnsupportedSchemeException e) {
            Toast.makeText(MainActivity.this, "Unsupported Scheme",
                    Toast.LENGTH_LONG).show();
            e.printStackTrace();
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        }
        return;
    }

    @Override
    protected void onStop() {
        mLogger.enter("onStop");
        super.onStop();
        Editor editor = getSharedPreferences(Constants.APP_NAME, MODE_PRIVATE)
                .edit();
        editor.apply();
    }

    private void setupDropDownList() {
        mSpinnerABSContentList = (Spinner) findViewById(R.id.dropdownlist_abs_content_id);
        mSpinnerOfflineContentList = (Spinner) findViewById(R.id.dropdownlist_offline_content_id);
        mSpinnerActionTokenList = (Spinner) findViewById(R.id.select);

        if (mSpinnerABSContentList != null) {
            File file = new File(Constants.ABS_CONTENT_LIST);

            if (file.exists()) {
                List<ABSContentInfo> absContentInfo = null;
                try {
                    absContentInfo = ABSContentInfo
                            .parseFromProeprties(new FileInputStream(file));
                } catch (FileNotFoundException e) {
                    mLogger.e("Could not found : " + Constants.ABS_CONTENT_LIST);
                    e.printStackTrace();
                }
                if (absContentInfo != null) {
                    ArrayAdapter<ABSContentInfo> adapter = new ArrayAdapter<ABSContentInfo>(
                            mContext, android.R.layout.simple_list_item_1,
                            absContentInfo);
                    mSpinnerABSContentList.setAdapter(adapter);
                }
            } else {
                mLogger.e("Could not found : " + Constants.ABS_CONTENT_LIST);
            }
        }

        if (mSpinnerOfflineContentList != null) {
            mSpinnerOfflineContentList
            .setOnItemSelectedListener(new OnItemSelectedListener() {
                @Override
                public void onItemSelected(AdapterView<?> arg0,
                        View arg1, int arg2, long arg3) {
                    if (mOfflineFilenames != null && !mOfflineFilenames.isEmpty()) {
                        mCurrentOfflineContentPath = Constants.VIDEO_DIRECTORY_PATH + "/" + mOfflineFilenames.get(arg2);
                    }
                }
                @Override
                public void onNothingSelected(AdapterView<?> arg0) {
                }
            });
        }

        if (mSpinnerActionTokenList != null) {
            mSpinnerActionTokenList
                    .setOnItemSelectedListener(new OnItemSelectedListener() {
                        @Override
                        public void onItemSelected(AdapterView<?> arg0,
                                View arg1, int arg2, long arg3) {
                            if (mATKNFilenames != null && !mATKNFilenames.isEmpty()) {
                                mCurrentATKNFilePath = Constants.ACTION_DIRECTORY_PATH + "/" + mATKNFilenames.get(arg2);
                            }
                        }
                        @Override
                        public void onNothingSelected(AdapterView<?> arg0) {
                        }
                    });
        }
    }

    private ABSContentInfo getABSContentInfo() {
        Object selectedItem = mSpinnerABSContentList.getSelectedItem();
        if (selectedItem == null) {
            return null;
        }
        if (selectedItem instanceof ABSContentInfo) {
            return (ABSContentInfo) selectedItem;
        }
        return null;
    }

    private void onTaskFinished(final String message) {
        mLogger.d("Teardown after processing");

        mHandler.post(new Runnable() {
            @Override
            public void run() {
                DialogFragment fragment = (DialogFragment) getFragmentManager()
                        .findFragmentByTag(DIALOG_TASK_PROCESSING);
                if (fragment != null) {
                    fragment.dismiss();
                }

                Toast.makeText(MainActivity.this, message, Toast.LENGTH_LONG)
                        .show();
                mTaskRunner = null;
            }
        });

    }

    private void setupDrmProcessButton(final int appMode) {
        final Button btnRegistration = (Button) findViewById(R.id.btn_registration);
        final Button btnSaveLicense = (Button) findViewById(R.id.btn_license);
        final Button btnDeregistration = (Button) findViewById(R.id.btn_deregistration);
        final Button btnCheckRights = (Button) findViewById(R.id.btn_check_rights);
        final Button btnRemoveRights = (Button) findViewById(R.id.btn_remove_rights);
        final Button btnStatus = (Button) findViewById(R.id.btn_check_regist);

        if (btnRegistration != null) {
            btnRegistration.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View v) {
                    mLogger.enter("requeseted registration...");

                    final TaskInfo taskInfo = new TaskInfo(
                            TaskType.REGISTRATION, mAccountId, mServiceId,
                            mCurrentATKNFilePath);
                    mHandler.post(new TaskStarter(taskInfo));
                }
            });
        }
        if (btnSaveLicense != null) {
            btnSaveLicense.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View v) {
                    mLogger.d("requeseted getting License...");
                    final TaskInfo taskInfo = new TaskInfo(TaskType.LICENSE,
                            mAccountId, mServiceId, mCurrentATKNFilePath);
                    mHandler.post(new TaskStarter(taskInfo));
                }
            });
        }
        if (btnDeregistration != null) {
            btnDeregistration.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View v) {
                    mLogger.d("requeseted deregistration...");
                    final TaskInfo taskInfo = new TaskInfo(
                            TaskType.DEREGISTRATION, mAccountId, mServiceId,
                            mCurrentATKNFilePath);
                    mHandler.post(new TaskStarter(taskInfo));
                }
            });
        }
        if (btnCheckRights != null) {
            btnCheckRights.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View v) {
                    mLogger.d("requeseted check License...");
                    TextView log = (TextView) findViewById(R.id.log);
                    byte[] sessionId = null;
                    MediaDrm mediaDrm = null;
                    byte[] contentData = null;
                    log.setText("");

                    try {
                        mediaDrm = new MediaDrm(Constants.MBB_UUID);
                        sessionId = mediaDrm.openSession();

                        switch (appMode) {
                        case Constants.APP_MODE_ABS:
                            ABSContentInfo absContentInfo = getABSContentInfo();
                            contentData = Utils.readPsshDataFromFile(true);
                            mediaDrm.restoreKeys(sessionId, InitData
                                    .getPSSHTableForAndroid(contentData, absContentInfo.getVideoKid()));
                            break;
                        case Constants.APP_MODE_OFFLINE:
                            contentData = Utils.readIPMPDataFromFile(true);
                            mediaDrm.restoreKeys(sessionId, InitData
                                    .getIPMPTableForAndroid(contentData));
                            break;
                        default:
                            Toast.makeText(mContext, "Unknown App Mode",
                                    Toast.LENGTH_SHORT).show();
                            return;
                        }
                        HashMap<String, String> infoMap = mediaDrm
                                .queryKeyStatus(sessionId);

                        if (infoMap != null && infoMap.size() > 0) {
                            StringBuilder sb = new StringBuilder();
                            Iterator<String> iterator = infoMap.keySet()
                                    .iterator();
                            log.setText("");
                            Time time = new Time();
                            while (iterator.hasNext()) {
                                String name = iterator.next();
                                time.set(Long.valueOf(infoMap.get(name)));
                                mLogger.d("\t" + name + " = "
                                        + infoMap.get(name) + " ["
                                        + time.format2445() + "]");
                                sb.append("\n\t" + name + " = "
                                        + infoMap.get(name) + " ["
                                        + time.format2445() + "]");
                            }
                            log.append(sb);
                        }
                        mediaDrm.closeSession(sessionId);
                        sessionId = null;
                        Toast.makeText(MainActivity.this,
                                "queryKeyStatus finished", Toast.LENGTH_LONG)
                                .show();
                        return;
                    } catch (IllegalArgumentException e) {
                        e.printStackTrace();
                    } catch (IllegalStateException e) {
                        e.printStackTrace();
                    } catch (UnsupportedSchemeException e) {
                        e.printStackTrace();
                    } catch (NotProvisionedException e) {
                        e.printStackTrace();
                    } catch (JSONException e) {
                        e.printStackTrace();
                    }
                    mediaDrm.closeSession(sessionId);
                    sessionId = null;
                    Toast.makeText(MainActivity.this, "Failure",
                            Toast.LENGTH_LONG).show();
                }
            });
        }
        if (btnRemoveRights != null) {
            btnRemoveRights.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View v) {
                    mLogger.d("requeseted removing License...");
                    Toast.makeText(MainActivity.this, "Not implementation",
                            Toast.LENGTH_LONG).show();
                }
            });
        }
        if (btnStatus != null) {
            btnStatus.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View v) {
                    mLogger.enter("Check registration Status...");

                    RequestParser parser = null;
                    MediaDrm mediaDrm = null;
                    byte[] sessionid = null;
                    HashMap<String, String> optionalParameters = null;
                    try {
                        mediaDrm = new MediaDrm(Constants.MBB_UUID);
                        sessionid = mediaDrm.openSession();
                        KeyRequest keyRequest = mediaDrm.getKeyRequest(
                                sessionid,
                                InitData.getPropertyTableForAndroid(
                                        Constants.QUERY_NAME_REGISTERED_STATE,
                                        Utils.accountIdToMarlinFormat(mAccountId),
                                        mServiceId),
                                Constants.REQUEST_MIMETYPE_QUERY_PROPERTY,
                                MediaDrm.KEY_TYPE_OFFLINE, optionalParameters);
                        parser = new RequestParser(keyRequest.getData());

                        if (parser.parse()) {
                            Toast.makeText(MainActivity.this,
                                    parser.getProperty(), Toast.LENGTH_LONG)
                                    .show();
                        } else {
                            Toast.makeText(MainActivity.this, "Failure",
                                    Toast.LENGTH_LONG).show();
                        }
                        return;
                    } catch (IllegalArgumentException e) {
                        e.printStackTrace();
                    } catch (IllegalStateException e) {
                        e.printStackTrace();
                    } catch (UnsupportedSchemeException e) {
                        e.printStackTrace();
                    } catch (JSONException e) {
                        e.printStackTrace();
                    } catch (NotProvisionedException e) {
                        e.printStackTrace();
                    }
                    Toast.makeText(MainActivity.this, "Failure",
                            Toast.LENGTH_LONG).show();
                }
            });
        }
    }

    private void setupPlayerButton() {
        final Button btnPlayer = (Button) findViewById(R.id.btn_launch_movie_app);
        final Button btnABSShowMPD = (Button) findViewById(R.id.btn_show_mpd);

        if (btnPlayer != null) {
            btnPlayer.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View v) {
                    mLogger.d("requeseted starting playback of offline...");
                    if (appMode == Constants.APP_MODE_OFFLINE) {

                        // play by local app
                        final String contentPath = mCurrentOfflineContentPath;
                        Intent intent = new Intent(MainActivity.this,
                                mtmo.test.mediadrm.player.DecodeActivity.class);
                        intent.putExtra("file", contentPath);
                        startActivity(intent);
                    } else if (appMode == Constants.APP_MODE_ABS) {
                        ABSContentInfo contentInfo = getABSContentInfo();
                        if (contentInfo == null) {
                            mLogger.e("Could not get ABS Content Info. Unable to start content.");
                            return;
                        }
                        // play by local app
                        Intent intent = new Intent(
                                MainActivity.this,
                                mtmo.test.mediadrm.absplayer.ABSDecodeActivity.class);
                        intent.putExtra("ABSContentInfo", contentInfo);
                        startActivity(intent);
                    }
                }
            });
        }

        if (btnABSShowMPD != null) {
            btnABSShowMPD.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View v) {
                    ABSContentInfo absContentInfo = getABSContentInfo();

                    if (absContentInfo == null) {
                        mLogger.e("Could not get ABS Content Info. Unable to show MPD.");
                        return;
                    }

                    Intent intent = new Intent(Intent.ACTION_VIEW);
                    intent.setData(Uri.parse(absContentInfo.getMpdUrl()));
                    startActivity(intent);
                }
            });
        }
    }

    private void listFiles() {
        File file = new File(Constants.ACTION_DIRECTORY_PATH);
        if (file.exists()) {
            File[] files = file.listFiles();
            mATKNFilenames = new ArrayList<String>();
            for (int i = 0; i < files.length; i++) {
                if (!files[i].isDirectory()) {
                    mATKNFilenames.add(files[i].getName());
                }
            }
            if (mActionTokenAdapter == null
                    && mSpinnerActionTokenList != null) {
                mActionTokenAdapter = new ArrayAdapter<String>(this,
                        android.R.layout.simple_spinner_item, mATKNFilenames);
                mActionTokenAdapter
                        .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
                mSpinnerActionTokenList.setAdapter(mActionTokenAdapter);
            }

            if (mActionTokenAdapter != null) {
                mActionTokenAdapter.notifyDataSetChanged();
            }
        } else {
            mLogger.e("No actiontoken folder");
        }

        file = new File(Constants.VIDEO_DIRECTORY_PATH);
        if (file.exists()) {
            File[] files = file.listFiles();
            mOfflineFilenames = new ArrayList<String>();
            for (int i = 0; i < files.length; i++) {
                if (!files[i].isDirectory()) {
                    mOfflineFilenames.add(files[i].getName());
                }
            }
            if (mOfflineContentAdapter == null
                    && mSpinnerOfflineContentList != null) {
                mOfflineContentAdapter = new ArrayAdapter<String>(this,
                        android.R.layout.simple_spinner_item, mOfflineFilenames);
                mOfflineContentAdapter
                        .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
                mSpinnerOfflineContentList.setAdapter(mOfflineContentAdapter);
            }

            if (mOfflineContentAdapter != null) {
                mOfflineContentAdapter.notifyDataSetChanged();
            }
        } else {
            mLogger.e("No video folder");
        }
    }

    private String convertResultToMessage(TaskInfo taskInfo, TaskResult result) {
        String ret;
        switch (result) {
        case ACTION_TOKEN_DOWNLOAD_SUCCESS:
            ret = "Download ActionToken was finished";
            break;
        case ACTION_TOKEN_DOWNLOAD_NEVER_EXECUTED:
        case ACTION_TOKEN_DOWNLOAD_ILLEGAL_ARGUMENTS:
            ret = "Could not start download ActionToken";
            break;
        case ACTION_TOKEN_DOWNLOAD_FAILED:
            ret = "Failed to download ActionToken";
            break;
        case DRM_PROCESS_SUCCESS:
            ret = "Process was finished successfully";
            break;
        case DRM_PROCESS_NO_RIGHT_DATA:
            ret = "Could not get License Data";
            break;
        case DRM_PROCESS_FAILED_REGISTER_ACCOUNT:
            ret = "Could not register account";
            break;
        case DRM_PROCESS_FAILED_REGISTER_SUBSCRIPTION:
            ret = "Could not register subscription";
            break;
        case DRM_PROCESS_FAILED_SAVE_LICENSE:
            ret = "Could not save License";
            break;
        case DRM_PROCESS_FAILED_ACQUIRE_LICENSE:
            ret = "Could not acquire License";
            break;
        case DRM_PROCESS_FAILED_DEREGISTER_ACCOUNT:
            ret = "Could not deregister Account";
            break;
        case DRM_PROCESS_FAILED_DEREGISTER_SUBSCRIPTION:
            ret = "Could not deregister Subscription";
            break;
        case DRM_PROCESS_FAILED_ILLEGAL_ARGUMENT_OR_STATEMENT:
            ret = "Could not execute DRM Process due to illegal arguments or illegal state";
            break;
        case DRM_PROCESS_FAILED_UNKNOWN:
        case DRM_PROCESS_INTERRUPTED:
        default:
            ret = "Unkonown problem occurs";
            break;
        }
        return ret;
    }

    private class TaskProgressDialog extends DialogFragment {
        String title;

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            setProgressBarIndeterminateVisibility(true);
        }

        @Override
        public void onDestroyView() {
            super.onDestroyView();
            setProgressBarIndeterminateVisibility(false);

        }

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container,
                Bundle savedInstanceState) {
            TaskProgressDialog.this.getDialog().setTitle(title);
            return inflater.inflate(R.layout.task_progress, container, false);
        }

    }

    private class SwitchModeDialogFragment extends DialogFragment {
        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
            builder.setTitle("Select Mode").setSingleChoiceItems(
                    R.array.app_mode, appMode,
                    new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            getSharedPreferences(Constants.APP_NAME,
                                    MODE_PRIVATE)
                                    .edit()
                                    .putInt(Constants.PREFERENCE_APP_MODE,
                                            which).commit();
                            SwitchModeDialogFragment.this.dismiss();
                            Intent intent = MainActivity.this.getIntent();
                            MainActivity.this.finish();
                            startActivity(intent);
                        }
                    });
            return builder.create();
        }
    }

    private boolean readPropFromFile() {
        boolean ret = false;
        FileReader fr = null;
        BufferedReader br = null;
        try {
            fr = new FileReader(Constants.PROPERTY_FILE_PATH);
            br = new BufferedReader(fr);

            String line;
            for (int i = 0; (line = br.readLine()) != null; i++) {
                switch (i) {
                case 0:
                    mServiceId = line;
                    break;
                case 1:
                    mAccountId = line;
                    break;
                default:
                }
            }
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            try {
                if (br != null)
                    br.close();
                if (fr != null)
                    fr.close();
                ret = true;
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        return ret;
    }
}
