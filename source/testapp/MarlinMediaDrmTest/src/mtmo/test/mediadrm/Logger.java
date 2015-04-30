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

public class Logger {
    /**
     * If you want to get the log for enter/exit/verbose, please set to true.
     */
    public static final boolean DEBUG_MODE = true;

    public static final int NONE = -1;

    public static final int ERROR = 0;

    public static final int WARNING = 1;

    public static final int INFO = 2;

    public static final int DEBUG = 3;

    public static final int VERBOSE = 4;

    public static final int METHOD = 5;

    public static final int ALL = METHOD + 1;

    public static final boolean TRACEVIEW = false;

    private String mTag;

    private String mSubtag;

    private int mLevel;

    private Logger(String tag, String subtag, int level) {
        mTag = tag;
        mSubtag = subtag + ": ";
        mLevel = level;
    }

    private Logger(String tag, String subtag) {
        this(tag, subtag, ALL);
    }

    public static Logger createLog(String classname) {
        return createLog(classname, ALL);
    }

    public static Logger createLog(String classname, int level) {
        int index = classname.lastIndexOf('.');
        if (index != -1) {
            return new Logger(Constants.LOG_TAG,
                    classname.substring(index + 1), level);
        } else {
            return new Logger(Constants.LOG_TAG, classname, level);
        }
    }

    public static Logger getDefaultLog() {
        return DefaultLogHolder.sLogger;
    }

    /* Singleton */
    static class DefaultLogHolder {
        static Logger sLogger = createLog("DefaultLogHolder");
    }

    public void e(String msg) {
        android.util.Log.e(mTag, mSubtag + msg);
    }

    public void e(String msg, Throwable tr) {
        android.util.Log.e(mTag, mSubtag + msg, tr);
    }

    public void w(String msg) {
        android.util.Log.w(mTag, mSubtag + msg);
    }

    public void w(String msg, Throwable tr) {
        android.util.Log.w(mTag, mSubtag + msg, tr);
    }

    public void i(String msg) {
        if (atINFO()) {
            android.util.Log.i(mTag, mSubtag + msg);
        }
    }

    public void i(String msg, Throwable tr) {
        if (atINFO()) {
            android.util.Log.i(mTag, mSubtag + msg, tr);
        }
    }

    public void d(String msg) {
        if (atDEBUG()) {
            android.util.Log.d(mTag, mSubtag + msg);
        }
    }

    public void d(String msg, Throwable tr) {
        if (atDEBUG()) {
            android.util.Log.d(mTag, mSubtag + msg, tr);
        }
    }

    public void v(String msg) {
        if (atVERBOSE() && DEBUG_MODE) {
            android.util.Log.v(mTag, mSubtag + msg);
        }
    }

    public void enter(String method) {
        if (atMETHOD() && DEBUG_MODE) {
            android.util.Log.v(mTag, mSubtag + "[Enter]" + method);
        }
    }

    public void exit(String method) {
        if (atMETHOD() && DEBUG_MODE) {
            android.util.Log.v(mTag, mSubtag + "[Exit]" + method);
        }
    }

    public boolean atINFO() {
        return (INFO <= mLevel || INFO <= logLevel());
    }

    public boolean atDEBUG() {
        return (DEBUG <= mLevel || DEBUG <= logLevel());
    }

    public boolean atVERBOSE() {
        return (VERBOSE <= mLevel || VERBOSE <= logLevel());
    }

    public boolean atMETHOD() {
        return (METHOD <= mLevel || METHOD <= logLevel());
    }

    public boolean atTRACEVIEW() {
        return TRACEVIEW;
    }

    private int logLevel() {
        return Integer.parseInt(System.getProperty(
                "persist.dtcp_test.logger.level", "0"));
    }
}
