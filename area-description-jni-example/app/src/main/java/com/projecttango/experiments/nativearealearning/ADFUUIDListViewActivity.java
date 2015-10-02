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

package com.projecttango.experiments.nativearealearning;

import java.io.File;
import java.util.Arrays;
import java.util.ArrayList;
import java.util.StringTokenizer;
import java.util.Collections;

import android.app.Activity;
import android.app.FragmentManager;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.os.Bundle;
import android.util.Log;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

/**
 * Contains an ADF Name and its UUID.
 */
class AdfData {
  public String mUuid = new String(); 
  public String mName = new String(); 

  public AdfData(String uuid, String name) {
    mUuid = uuid;
    mName = name;
  }
}

/**
 * Maps AdfData to Strings for display in the ListView.
 */
class AdfUuidArrayAdapter extends ArrayAdapter<String> {
  Context mContext;
  private String[] mUuidArray, mNameArray;

  public AdfUuidArrayAdapter(Context context, ArrayList<AdfData> adfDataList) {
    super(context, R.layout.adf_list_row);
    mContext = context;
    SetAdfData(adfDataList);
  }
  
  public void SetAdfData(ArrayList<AdfData> adfDataList) {
    int size = adfDataList.size();
    mUuidArray = new String[size];
    mNameArray = new String[size];
    for (int i = 0; i < size; ++i) {
      mUuidArray[i] = adfDataList.get(i).mUuid;
      mNameArray[i] = adfDataList.get(i).mName;
    }
  }

  @Override
  public int getCount() {
      return mUuidArray.length;
  }

  @Override
  public View getView(int position, View convertView, ViewGroup parent) {
    LayoutInflater inflator = (LayoutInflater) mContext.getSystemService(
        Context.LAYOUT_INFLATER_SERVICE);
    View row = inflator.inflate(R.layout.adf_list_row, parent, false);
    TextView uuid = (TextView) row.findViewById(R.id.adf_uuid);
    TextView name = (TextView) row.findViewById(R.id.adf_name);

    uuid.setText(mUuidArray[position]);

    if (mNameArray != null) {
      name.setText(mNameArray[position]);
    } else {
      name.setText("Metadata cannot be read");
    }
    return row;
  }
}

/**
 * Creates a ListVIew to manage ADFs owned by the Tango Service.
 * Showcases:
 * - Importing an ADF to the Tango Service from this class's Application Package folder.
 * - Exporting an ADF from the Tango Service to this class's Application Package folder.
 * - Deleting an ADF owned by the Tango Service.
 */
public class ADFUUIDListViewActivity extends Activity implements
      SetADFNameDialog.CallbackListener {
  private static final String INTENT_CLASS_PACKAGE = "com.projecttango.tango";
  private static final String INTENT_REQUEST_PERMISSION_CLASSNAME =
      "com.google.atap.tango.RequestPermissionActivity";
  private static final String INTENT_IMPORT_EXPORT_CLASSNAME =
      "com.google.atap.tango.RequestImportExportActivity";

  private static final int TANGO_INTENT_ACTIVITY_CODE = 3;
  private static final String EXTRA_KEY_SOURCE_UUID = "SOURCE_UUID";
  private static final String EXTRA_KEY_DESTINATION_FILE = "DESTINATION_FILE";
  private static final String EXTRA_KEY_SOURCE_FILE = "SOURCE_FILE";
  private static final String EXTRA_KEY_DESTINATION_UUID = "DESTINATION_UUID";

  private ListView mTangoSpaceAdfListView, mAppSpaceAdfListView;
  private AdfUuidArrayAdapter mTangoSpaceAdfListAdapter, mAppSpaceAdfListAdapter;
  private ArrayList<AdfData> mTangoSpaceAdfDataList, mAppSpaceAdfDataList;
  private String[] mTangoSpaceMenuStrings, mAppSpaceMenuStrings;
  private String mAppSpaceADFFolder;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

    setContentView(R.layout.adf_listview);
    mTangoSpaceAdfListView = (ListView) findViewById(R.id.uuidlistviewAPI);
    mAppSpaceAdfListView =
        (ListView) findViewById(R.id.uuidlistviewApplicationSpace);

    mTangoSpaceMenuStrings =
        getResources().getStringArray(R.array.SetDialogMenuItemsAPISpace);
    mAppSpaceMenuStrings =
        getResources().getStringArray(R.array.SetDialogMenuItemsAppSpace);

    mAppSpaceADFFolder =
        getFilesDir().getAbsolutePath() + File.separator + "Maps";
    File file = new File(mAppSpaceADFFolder);
    if (!file.exists()) {
      file.mkdirs();
    }

    mTangoSpaceAdfDataList = new ArrayList<AdfData>();
    mAppSpaceAdfDataList = new ArrayList<AdfData>();

    mTangoSpaceAdfListAdapter = 
        new AdfUuidArrayAdapter(this, mTangoSpaceAdfDataList);
    mAppSpaceAdfListAdapter =
        new AdfUuidArrayAdapter(this, mAppSpaceAdfDataList);

    mTangoSpaceAdfListView.setAdapter(mTangoSpaceAdfListAdapter);
    mAppSpaceAdfListView.setAdapter(mAppSpaceAdfListAdapter);

    registerForContextMenu(mTangoSpaceAdfListView);
    registerForContextMenu(mAppSpaceAdfListView);
  }
  
  @Override
  protected void onResume() {
    super.onResume();
    updateList();
  }

  @Override
  public void onCreateContextMenu(ContextMenu menu, View v,
      ContextMenuInfo menuInfo) {
    if (v.getId() == R.id.uuidlistviewAPI) {
      AdapterView.AdapterContextMenuInfo info =
          (AdapterView.AdapterContextMenuInfo) menuInfo;
      menu.setHeaderTitle(mTangoSpaceAdfDataList.get(info.position).mUuid);
      menu.add(mTangoSpaceMenuStrings[0]);
      menu.add(mTangoSpaceMenuStrings[1]);
      menu.add(mTangoSpaceMenuStrings[2]);
    }

    if (v.getId() == R.id.uuidlistviewApplicationSpace) {
      AdapterView.AdapterContextMenuInfo info =
          (AdapterView.AdapterContextMenuInfo) menuInfo;
      menu.setHeaderTitle(mAppSpaceAdfDataList.get(info.position).mUuid);
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
      showSetNameDialog(mTangoSpaceAdfDataList.get(index).mUuid);
    } else if (itemName.equals(mTangoSpaceMenuStrings[1])) {
      // Delete the ADF from Tango space and update the Tango ADF Listview.
      deleteAdfFromTangoSpace(mTangoSpaceAdfDataList.get(index).mUuid);
    } else if (itemName.equals(mTangoSpaceMenuStrings[2])) {
      // Export the ADF into application package folder and update the Listview.
      exportAdf(mTangoSpaceAdfDataList.get(index).mUuid);
    } else if (itemName.equals(mAppSpaceMenuStrings[0])) {
      // Delete an ADF from App space and update the App space ADF Listview.
      deleteAdfFromAppSpace(mAppSpaceAdfDataList.get(index).mUuid);
    } else if (itemName.equals(mAppSpaceMenuStrings[1])) {
      // Import an ADF from app space to Tango space.
      importAdf(mAppSpaceAdfDataList.get(index).mUuid);
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
            Toast.makeText(this, R.string.no_permissions, Toast.LENGTH_LONG)
                    .show();
        }
    }
    updateList();
  }

  /**
   * Implementation of callback listener interface in SetADFNameDialog.
   */
  @Override
  public void onAdfNameOk(String name, String uuid) {
    TangoJNINative.setAdfMetadataValue(uuid, "name", name);
    updateList();
  }

  /**
   * Implementation of callback listener interface in SetADFNameDialog.
   */
  @Override
  public void onAdfNameCancelled() {
  }

  /**
   * Import an ADF from app space to Tango space.
   */
  private void importAdf(String uuid) {
    String filepath = mAppSpaceADFFolder + File.separator + uuid;
    Intent importIntent = new Intent();
    importIntent.setClassName(INTENT_CLASS_PACKAGE, INTENT_IMPORT_EXPORT_CLASSNAME);
    importIntent.putExtra(EXTRA_KEY_SOURCE_FILE, filepath);
    startActivityForResult(importIntent, TANGO_INTENT_ACTIVITY_CODE);
  }

  /**
   * Export an ADF from Tango space to app space.
   */
  private void exportAdf(String uuid) {
    Intent exportIntent = new Intent();
    exportIntent.setClassName(INTENT_CLASS_PACKAGE, INTENT_IMPORT_EXPORT_CLASSNAME);
    exportIntent.putExtra(EXTRA_KEY_SOURCE_UUID, uuid);
    exportIntent.putExtra(EXTRA_KEY_DESTINATION_FILE, mAppSpaceADFFolder);
    startActivityForResult(exportIntent, TANGO_INTENT_ACTIVITY_CODE);
  }

  private void deleteAdfFromTangoSpace(String uuid) {
    TangoJNINative.deleteAdf(uuid);
  }

  private void deleteAdfFromAppSpace(String uuid) {
    File file = new File(mAppSpaceADFFolder + File.separator + uuid);
    file.delete();
  }

  private void updateAppSpaceAdfList() {
    File file = new File(mAppSpaceADFFolder);
    File[] adfFileList = file.listFiles();
    mAppSpaceAdfDataList.clear();

    for (int i = 0; i < adfFileList.length; ++i) {
      mAppSpaceAdfDataList.add(new AdfData(adfFileList[i].getName(), ""));
    }
  }

  private void updateTangoSpaceAdfList() {
    StringTokenizer tok =
        new StringTokenizer(TangoJNINative.getAllAdfUuids(), ",");
    mTangoSpaceAdfDataList.clear();
    while (tok.hasMoreElements()) {
      String uuid = tok.nextToken();
      String name = TangoJNINative.getAdfMetadataValue(uuid, "name");
      mTangoSpaceAdfDataList.add(new AdfData(uuid, name));
    }
  }

  private void updateList() {
    // Update App space ADF Listview.
    updateAppSpaceAdfList();
    mAppSpaceAdfListAdapter.SetAdfData(mAppSpaceAdfDataList);
    mAppSpaceAdfListAdapter.notifyDataSetChanged();

    updateTangoSpaceAdfList();
    mTangoSpaceAdfListAdapter.SetAdfData(mTangoSpaceAdfDataList);
    mTangoSpaceAdfListAdapter.notifyDataSetChanged();
  }

  private void showSetNameDialog(String mCurrentUUID) {
    Bundle bundle = new Bundle();
    String name = TangoJNINative.getAdfMetadataValue(mCurrentUUID, "name");
    if (name != null) {
      bundle.putString("name", name);
    }
    bundle.putString("id", mCurrentUUID);
    FragmentManager manager = getFragmentManager();
    SetADFNameDialog setADFNameDialog = new SetADFNameDialog();
    setADFNameDialog.setArguments(bundle);
    setADFNameDialog.show(manager, "ADFNameDialog");
  }
}


