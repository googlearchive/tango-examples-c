/*
 * Copyright 2014 Google Inc. All Rights Reserved.
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

import android.app.Activity;
import android.app.FragmentManager;
import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Environment;
import android.os.IBinder;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ListView;
import android.widget.Toast;

import java.io.File;
import java.util.ArrayList;
import java.util.StringTokenizer;

import com.projecttango.examples.cpp.util.TangoInitializationHelper;

/**
 * Creates a ListView to manage ADFs owned by the Tango Service.
 * Showcases:
 * - Importing an ADF to the Tango Service from this class's Application Package folder.
 * - Exporting an ADF from the Tango Service to this class's Application Package folder.
 * - Deleting an ADF owned by the Tango Service.
 */
public class AdfUuidListViewActivity extends Activity implements SetAdfNameDialog.CallbackListener {
    private static final String INTENT_CLASS_PACKAGE = "com.google.tango";
    private static final String INTENT_DEPRECATED_CLASS_PACKAGE = "com.projecttango.tango";
    private static final String INTENT_REQUEST_PERMISSION_CLASSNAME =
            "com.google.atap.tango.RequestPermissionActivity";
    private static final String INTENT_IMPORT_EXPORT_CLASSNAME =
            "com.google.atap.tango.RequestImportExportActivity";

    private static final int TANGO_INTENT_ACTIVITY_CODE = 3;
    private static final String EXTRA_KEY_SOURCE_UUID = "SOURCE_UUID";
    private static final String EXTRA_KEY_DESTINATION_FILE = "DESTINATION_FILE";
    private static final String EXTRA_KEY_SOURCE_FILE = "SOURCE_FILE";

    // Tango service connection.
    ServiceConnection mTangoServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            TangoJniNative.onTangoServiceConnected(service, false, false);
            updateList();
        }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {
            // Handle this if you need to gracefully shutdown/retry
            // in the event that Tango itself crashes/gets upgraded while running.
        }
    };

    private ListView mTangoSpaceAdfListView, mAppSpaceAdfListView;
    private AdfUuidArrayAdapter mTangoSpaceAdfListAdapter, mAppSpaceAdfListAdapter;
    private ArrayList<AdfData> mTangoSpaceAdfDataList, mAppSpaceAdfDataList;
    private String[] mTangoSpaceMenuStrings, mAppSpaceMenuStrings;
    private String mAppSpaceAdfFolder;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.adf_list_view);
        mTangoSpaceMenuStrings = getResources().getStringArray(
                R.array.set_dialog_menu_items_api_space);
        mAppSpaceMenuStrings = getResources().getStringArray(
                R.array.set_dialog_menu_items_app_space);

        // Get API ADF ListView Ready
        mTangoSpaceAdfListView = (ListView) findViewById(R.id.uuid_list_view_tango_space);
        mTangoSpaceAdfDataList = new ArrayList<AdfData>();
        mTangoSpaceAdfListAdapter = new AdfUuidArrayAdapter(this, mTangoSpaceAdfDataList);
        mTangoSpaceAdfListView.setAdapter(mTangoSpaceAdfListAdapter);
        registerForContextMenu(mTangoSpaceAdfListView);

        // Get App Space ADF List View Ready
        mAppSpaceAdfListView = (ListView) findViewById(R.id.uuid_list_view_application_space);
        mAppSpaceAdfFolder = getAppSpaceAdfFolder();
        mAppSpaceAdfDataList = new ArrayList<AdfData>();
        mAppSpaceAdfListAdapter = new AdfUuidArrayAdapter(this, mAppSpaceAdfDataList);
        mAppSpaceAdfListView.setAdapter(mAppSpaceAdfListAdapter);
        registerForContextMenu(mAppSpaceAdfListView);
    }

    @Override
    protected void onResume() {
        super.onResume();
        TangoInitializationHelper.bindTangoService(this, mTangoServiceConnection);
    }

    @Override
    protected void onPause() {
        super.onPause();
        TangoJniNative.onPause();
        unbindService(mTangoServiceConnection);
    }

    @Override
    public void onCreateContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo) {
        AdapterView.AdapterContextMenuInfo info = (AdapterView.AdapterContextMenuInfo) menuInfo;
        if (v.getId() == R.id.uuid_list_view_tango_space) {
            menu.setHeaderTitle(mTangoSpaceAdfDataList.get(info.position).uuid);
            menu.add(mTangoSpaceMenuStrings[0]);
            menu.add(mTangoSpaceMenuStrings[1]);
            menu.add(mTangoSpaceMenuStrings[2]);
        }

        if (v.getId() == R.id.uuid_list_view_application_space) {
            menu.setHeaderTitle(mAppSpaceAdfDataList.get(info.position).uuid);
            menu.add(mAppSpaceMenuStrings[0]);
            menu.add(mAppSpaceMenuStrings[1]);
        }
    }

    @Override
    public boolean onContextItemSelected(MenuItem item) {
        AdapterView.AdapterContextMenuInfo info =
                (AdapterView.AdapterContextMenuInfo) item.getMenuInfo();
        String itemName = (String) item.getTitle();
        int index = info.position;

        if (itemName.equals(mTangoSpaceMenuStrings[0])) {
            // Delete the ADF from Tango space and update the Tango ADF Listview.
            showSetNameDialog(mTangoSpaceAdfDataList.get(index).uuid);
        } else if (itemName.equals(mTangoSpaceMenuStrings[1])) {
            // Delete the ADF from Tango space and update the Tango ADF Listview.
            deleteAdfFromTangoSpace(mTangoSpaceAdfDataList.get(index).uuid);
        } else if (itemName.equals(mTangoSpaceMenuStrings[2])) {
            // Export the ADF into application package folder and update the Listview.
            exportAdf(mTangoSpaceAdfDataList.get(index).uuid);
        } else if (itemName.equals(mAppSpaceMenuStrings[0])) {
            // Delete an ADF from App space and update the App space ADF Listview.
            deleteAdfFromAppSpace(mAppSpaceAdfDataList.get(index).uuid);
        } else if (itemName.equals(mAppSpaceMenuStrings[1])) {
            // Import an ADF from app space to Tango space.
            importAdf(mAppSpaceAdfDataList.get(index).uuid);
        }

        updateList();
        return true;
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        // Check which request we're responding to
        if (requestCode == TANGO_INTENT_ACTIVITY_CODE) {
            // Make sure the request was successful
            if (resultCode == RESULT_CANCELED) {
                Toast.makeText(this, R.string.no_permissions, Toast.LENGTH_LONG).show();
            }
        }
        updateList();
    }

    /**
     * Implementation of callback listener interface in SetADFNameDialog.
     */
    @Override
    public void onAdfNameOk(String name, String uuid) {
        TangoJniNative.setAdfMetadataValue(uuid, "name", name);
        updateList();
    }

    /**
     * Implementation of callback listener interface in SetADFNameDialog.
     */
    @Override
    public void onAdfNameCancelled() {
        // Nothing to do here.
    }

    /**
     * Import an ADF from app space to Tango space.
     */
    private void importAdf(String uuid) {
        String filepath = mAppSpaceAdfFolder + File.separator + uuid;
        Intent importIntent = new Intent();
        importIntent.setClassName(INTENT_CLASS_PACKAGE, INTENT_IMPORT_EXPORT_CLASSNAME);
        if (importIntent.resolveActivity(getApplicationContext().getPackageManager()) == null) {
            importIntent = new Intent();
            importIntent.setClassName(INTENT_DEPRECATED_CLASS_PACKAGE,
                                      INTENT_IMPORT_EXPORT_CLASSNAME);
        }
        importIntent.putExtra(EXTRA_KEY_SOURCE_FILE, filepath);
        startActivityForResult(importIntent, TANGO_INTENT_ACTIVITY_CODE);
    }

    /**
     * Export an ADF from Tango space to app space.
     */
    private void exportAdf(String uuid) {
        Intent exportIntent = new Intent();
        exportIntent.setClassName(INTENT_CLASS_PACKAGE, INTENT_IMPORT_EXPORT_CLASSNAME);
        if (exportIntent.resolveActivity(getApplicationContext().getPackageManager()) == null) {
            exportIntent = new Intent();
            exportIntent.setClassName(INTENT_DEPRECATED_CLASS_PACKAGE,
                    INTENT_IMPORT_EXPORT_CLASSNAME);
        }
        exportIntent.putExtra(EXTRA_KEY_SOURCE_UUID, uuid);
        exportIntent.putExtra(EXTRA_KEY_DESTINATION_FILE, mAppSpaceAdfFolder);
        startActivityForResult(exportIntent, TANGO_INTENT_ACTIVITY_CODE);
    }

    private void deleteAdfFromTangoSpace(String uuid) {
        TangoJniNative.deleteAdf(uuid);
    }

    private void deleteAdfFromAppSpace(String uuid) {
        File file = new File(mAppSpaceAdfFolder + File.separator + uuid);
        file.delete();
    }

    /*
     * Returns maps storage location in the App package folder. Creates a folder called Maps, if it
     * does not exist.
     */
    private String getAppSpaceAdfFolder() {
        String mapsFolder = Environment.getExternalStorageDirectory().getAbsolutePath()
                + File.separator + "Maps";
        File file = new File(mapsFolder);
        if (!file.exists()) {
            file.mkdirs();
        }
        return mapsFolder;
    }

    /**
     * Updates the list of AdfData corresponding to the App space.
     */
    private void updateAppSpaceAdfList() {
        File file = new File(mAppSpaceAdfFolder);
        File[] adfFileList = file.listFiles();
        mAppSpaceAdfDataList.clear();

        for (int i = 0; i < adfFileList.length; ++i) {
            mAppSpaceAdfDataList.add(new AdfData(adfFileList[i].getName(), ""));
        }
    }

    /**
     * Updates the list of AdfData corresponding to the Tango space.
     */
    private void updateTangoSpaceAdfList() {
        StringTokenizer tok = new StringTokenizer(TangoJniNative.getAllAdfUuids(), ",");
        mTangoSpaceAdfDataList.clear();
        while (tok.hasMoreElements()) {
            String uuid = tok.nextToken();
            String name = TangoJniNative.getAdfMetadataValue(uuid, "name");
            mTangoSpaceAdfDataList.add(new AdfData(uuid, name));
        }
    }

    /**
     * Updates the list of AdfData from Tango and App space, and sets it to the adapters.
     */
    private void updateList() {
        // Update App space ADF Listview.
        updateAppSpaceAdfList();
        mAppSpaceAdfListAdapter.setAdfData(mAppSpaceAdfDataList);
        mAppSpaceAdfListAdapter.notifyDataSetChanged();

        // Update Tango space ADF Listview.
        updateTangoSpaceAdfList();
        mTangoSpaceAdfListAdapter.setAdfData(mTangoSpaceAdfDataList);
        mTangoSpaceAdfListAdapter.notifyDataSetChanged();
    }

    private void showSetNameDialog(String mCurrentUuid) {
        Bundle bundle = new Bundle();
        String name = TangoJniNative.getAdfMetadataValue(mCurrentUuid, "name");
        if (name != null) {
            bundle.putString("name", name);
        }
        bundle.putString("id", mCurrentUuid);
        FragmentManager manager = getFragmentManager();
        SetAdfNameDialog setAdfNameDialog = new SetAdfNameDialog();
        setAdfNameDialog.setArguments(bundle);
        setAdfNameDialog.show(manager, "ADFNameDialog");
    }
}


