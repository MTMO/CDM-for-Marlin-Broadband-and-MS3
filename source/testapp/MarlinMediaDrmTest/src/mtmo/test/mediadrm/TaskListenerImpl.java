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

import java.util.concurrent.CountDownLatch;

public class TaskListenerImpl implements TaskListener {
    private CountDownLatch mLatch = null;
    private TaskResult result = TaskResult.UNKNOWN;

    public long getCount() {
        return mLatch.getCount();
    }

    public TaskResult getResult() {
        return result;
    }

    @Override
    public synchronized void onTaskFinished(TaskResult result) {
        this.result = result;
        if (mLatch != null && mLatch.getCount() != 0) {
            mLatch.countDown();
        }
    }

    public synchronized void setCountDownLatch(CountDownLatch latch) {
        if (this.mLatch != null && this.mLatch.getCount() != 0) {
            return;
        }
        this.mLatch = latch;
        result = TaskResult.UNKNOWN;
    }
}
