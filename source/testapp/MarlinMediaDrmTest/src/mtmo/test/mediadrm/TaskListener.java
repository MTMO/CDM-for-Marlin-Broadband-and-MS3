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

public interface TaskListener {

    public void onTaskFinished(TaskResult result);

    public enum TaskResult {
        /**
        *
        */
        UNKNOWN,
        /**
        *
        */
        ACTION_TOKEN_DOWNLOAD_SUCCESS,
        /**
         *
         */
        ACTION_TOKEN_DOWNLOAD_FAILED,
        /**
         *
         */
        ACTION_TOKEN_DOWNLOAD_NEVER_EXECUTED,
        /**
         *
         */
        ACTION_TOKEN_DOWNLOAD_ILLEGAL_ARGUMENTS,
        /**
         *
         */
        CONTENT_DOWNLOAD_SUCCESS,
        /**
         *
         */
        CONTENT_DOWNLOAD_FAILED,
        /**
         *
         */
        DRM_PROCESS_SUCCESS,
        /**
         *
         */
        DRM_PROCESS_INTERRUPTED,
        /**
         *
         */
        DRM_PROCESS_NO_RIGHT_DATA,
        /**
         *
         */
        DRM_PROCESS_FAILED_REGISTER_ACCOUNT,
        /**
         *
         */
        DRM_PROCESS_FAILED_REGISTER_SUBSCRIPTION,
        /**
         *
         */
        DRM_PROCESS_FAILED_SAVE_LICENSE,
        /**
         *
         */
        DRM_PROCESS_FAILED_UNKNOWN,
        /**
         *
         */
        DRM_PROCESS_FAILED_ILLEGAL_ARGUMENT_OR_STATEMENT,
        /**
         *
         */
        DRM_PROCESS_FAILED_ACQUIRE_LICENSE,
        /**
         *
         */
        DRM_PROCESS_FAILED_DEREGISTER_ACCOUNT,
        /**
         *
         */
        DRM_PROCESS_FAILED_DEREGISTER_SUBSCRIPTION,
    }

}
