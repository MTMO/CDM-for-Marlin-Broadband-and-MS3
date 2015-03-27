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

public class TaskInfo {

    public enum TaskType {
        REGISTRATION("registrationActionToken"), LICENSE("licenseActionToken"), DEREGISTRATION(
                "deregistrationActionToken"), SUBS_REGISTRATION(
                "subscriptionActionToken"), SUBS_DEREGISTRATION(
                "unsubscriptionActionToken");
        private final String path;

        private TaskType(String path) {
            this.path = path;
        }

        public Object getTargetPath() {
            return path;
        }
    }

    public TaskType taskType;
    public String accountId = null;
    public String serviceId = null;
    public String serviceTokenPath = null;

    public TaskInfo(TaskType taskType, String serviceTokenPath) {
        super();
        this.taskType = taskType;
        this.serviceTokenPath = serviceTokenPath;
    }

    public TaskInfo(TaskType taskType, String accountId, String serviceId,
            String serviceTokenPath) {
        super();
        this.taskType = taskType;
        this.accountId = accountId;
        this.serviceId = serviceId;
        this.serviceTokenPath = serviceTokenPath;
    }

    public TaskInfo(TaskInfo taskInfo) {
        this(taskInfo.taskType, taskInfo.accountId, taskInfo.serviceId,
                taskInfo.serviceTokenPath);
    }
}
