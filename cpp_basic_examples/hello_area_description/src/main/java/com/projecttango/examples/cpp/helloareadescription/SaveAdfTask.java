/*
 * Copyright 2015 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.projecttango.examples.cpp.helloareadescription;

import android.content.Context;
import android.os.AsyncTask;

/**
 * Saves the ADF on a background thread and shows a progress dialog while saving.
 */
public class SaveAdfTask extends AsyncTask<Void, Integer, String> {

    /**
     * Listener to check if save ADF is successed.
     */
    public interface SaveAdfListener {
        void onSaveAdfFailed(String adfName);
        void onSaveAdfSuccess(String adfName, String adfUuid);
    }

    Context mContext;
    SaveAdfListener mCallbackListener;
    SaveAdfDialog mProgressDialog;
    String mAdfName;

    SaveAdfTask(Context context, SaveAdfListener callbackListener, String adfName) {
        mContext = context;
        mCallbackListener = callbackListener;
        mAdfName = adfName;
        mProgressDialog = new SaveAdfDialog(context);
    }

    /**
     * Call this method to marshall progress updates to the UI thread.
     */
    public void publishProgress(int progress) {
        super.publishProgress(progress);
    }


    /**
     * Sets up the progress dialog.
     */
    @Override
    protected void onPreExecute() {
        if (mProgressDialog != null) {
            mProgressDialog.show();
        }
    }

    /**
     * Performs long-running save in the background.
     */
    @Override
    protected String doInBackground(Void... params) {
        // This example implementation returns an empty-string on failure.
        String adfUuid = TangoJniNative.saveAdf();
        if (!adfUuid.isEmpty()) {
            TangoJniNative.setAdfMetadataValue(adfUuid, "name", mAdfName);
        }
        return adfUuid;
    }

    /**
     * Responds to progress updates events by updating the UI.
     */
    @Override
    protected void onProgressUpdate(Integer... progress) {
        if (mProgressDialog != null) {
            mProgressDialog.setProgress(progress[0]);
        }
    }

    /**
     * Dismisses the progress dialog and call the activity.
     */
    @Override
    protected void onPostExecute(String adfUuid) {
        if (mProgressDialog != null) {
            mProgressDialog.dismiss();
        }
        if (mCallbackListener != null) {
            if (adfUuid.isEmpty()) {
                mCallbackListener.onSaveAdfFailed(mAdfName);
            } else {
                mCallbackListener.onSaveAdfSuccess(mAdfName, adfUuid);
            }
        }
    }
}
