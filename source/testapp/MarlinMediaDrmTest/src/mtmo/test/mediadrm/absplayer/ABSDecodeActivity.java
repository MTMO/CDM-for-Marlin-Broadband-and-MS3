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

package mtmo.test.mediadrm.absplayer;

import mtmo.test.mediadrm.info.ABSContentInfo;
import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.SurfaceView;
import android.view.WindowManager;

public class ABSDecodeActivity extends Activity {

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        SurfaceView sv = new SurfaceView(this);
        setContentView(sv);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        ABSPlayer mPlayer = new ABSPlayer(sv);

        Intent intent = getIntent();
        if (intent != null) {
            ABSContentInfo info = null;
            info = (ABSContentInfo)intent.getSerializableExtra("ABSContentInfo");
            if (info == null) {
                finish();
            }

            mPlayer.setDataSource(info, getApplicationContext());
            mPlayer.play();
        }
    }
}
