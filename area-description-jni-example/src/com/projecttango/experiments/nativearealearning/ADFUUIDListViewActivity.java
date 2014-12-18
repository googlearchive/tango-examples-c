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
import com.projecttango.experiments.nativearealearning.SetADFNameDialog.SetNameAndUUIDCommunicator;

/**
 * This class lets you manage ADFs between this class's Application Package folder 
 * and API private space. This show cases mainly three things:
 * Import, Export, Delete an ADF file from API private space to any known and accessible file path.
 *
 */
public class ADFUUIDListViewActivity extends Activity implements SetNameAndUUIDCommunicator{
  private static final String INTENT_CLASSPACKAGE = "com.projecttango.tango";
  private static final String INTENT_REQUESTPERMISSION_CLASSNAME =
      "com.google.atap.tango.RequestPermissionActivity";
  private static final String INTENT_IMPORTEXPORT_CLASSNAME =
      "com.google.atap.tango.RequestImportExportActivity";

  // startActivityForResult requires a code number.
  public static final int TANGO_INTENT_ACTIVITYCODE = 1129;
  private static final String EXTRA_KEY_SOURCEUUID = "SOURCE_UUID";
  private static final String EXTRA_KEY_DESTINATIONFILE = "DESTINATION_FILE";
  private static final String EXTRA_KEY_SOURCEFILE = "SOURCE_FILE";
  public static final String EXTRA_KEY_DESTINATIONUUID = "DESTINATION_UUID";

  public static final int TANGO_ERROR_INVALID = -2;
  public static final int TANGO_ERROR_ERROR = -1;
  public static final int TANGO_ERROR_SUCCESS = -0;

  private ADFDataSource mADFDataSource;
  private ListView mUUIDListView, mAppSpaceUUIDListView;
  private ADFUUIDArrayAdapter mADFAdapter, mAppSpaceADFAdapter;
  private String[] mUUIDList, mUUIDNames, mAppSpaceUUIDList, mAppSpaceUUIDNames;
  private String[] mAPISpaceMenuStrings, mAppSpaceMenuStrings;
  private String mAppSpaceADFFolder;

  private Activity thisActivity;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

    int err = TangoJNINative.initialize(this);
    if (err != TANGO_ERROR_SUCCESS) {
      if (err == TANGO_ERROR_INVALID) {
        Toast.makeText(this, 
          "Tango Service version mismatch", Toast.LENGTH_SHORT).show();
      } else {
        Toast.makeText(this, 
          "Tango Service initialize internal error", Toast.LENGTH_SHORT).show();
      }
    }

    setContentView(R.layout.uuid_listview);
    mAPISpaceMenuStrings = getResources().getStringArray(R.array.SetDialogMenuItemsAPISpace);
    mAppSpaceMenuStrings = getResources().getStringArray(R.array.SetDialogMenuItemsAppSpace);

    updateADFList();

    thisActivity = this;
  }
  
  @Override
  protected void onResume() {
    super.onResume();
    // Update App space ADF Listview.
    mAppSpaceUUIDList = getAppSpaceADFList();
    mAppSpaceADFAdapter = new ADFUUIDArrayAdapter(this, mAppSpaceUUIDList, null);
    mAppSpaceUUIDListView.setAdapter(mAppSpaceADFAdapter);
    
    // Update API ADF Listview.
    mUUIDList = mADFDataSource.getFullUUIDList();
    mUUIDNames = mADFDataSource.getUUIDNames();
    mADFAdapter = new ADFUUIDArrayAdapter(this, mUUIDList, mUUIDNames);
    mUUIDListView.setAdapter(mADFAdapter);
  }

  @Override
  protected void onPause() {
    super.onPause();
  }

  @Override
  public void onCreateContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo) {
    if (v.getId() == R.id.uuidlistviewAPI) {
      AdapterView.AdapterContextMenuInfo info = (AdapterView.AdapterContextMenuInfo) menuInfo;
      
      menu.setHeaderTitle(mUUIDList[info.position]);
      menu.add(mAPISpaceMenuStrings[0]);
      menu.add(mAPISpaceMenuStrings[1]);
      menu.add(mAPISpaceMenuStrings[2]);
    }
    
    if (v.getId() == R.id.uuidlistviewApplicationSpace) {
      AdapterView.AdapterContextMenuInfo info = (AdapterView.AdapterContextMenuInfo) menuInfo;
      menu.setHeaderTitle(mAppSpaceUUIDList[info.position]);
      menu.add(mAppSpaceMenuStrings[0]);
      menu.add(mAppSpaceMenuStrings[1]);
    }
  }
  
  @Override
  public boolean onContextItemSelected(MenuItem item) {
    AdapterView.AdapterContextMenuInfo info = 
        (AdapterView.AdapterContextMenuInfo) item.getMenuInfo();
    String itemName = (String) item.getTitle();
    if (itemName.equals(mAPISpaceMenuStrings[0])) {
      // Delete the ADF from API storage and update the API ADF Listview.
      showSetNameDialog(mUUIDList[info.position]);
    } else if (itemName.equals(mAPISpaceMenuStrings[1])) {
      // Delete the ADF from API storage and update the API ADF Listview.
      mADFDataSource.deleteADFandUpdateList(mUUIDList[info.position]);
      updateADFList();
    } else if (itemName.equals(mAPISpaceMenuStrings[2])) {
      // Export the ADF into application package folder and update the Listview.
      Intent exportIntent = new Intent();
      exportIntent.setClassName(INTENT_CLASSPACKAGE, INTENT_IMPORTEXPORT_CLASSNAME);
      exportIntent.putExtra(EXTRA_KEY_SOURCEUUID, mUUIDList[info.position]);
      exportIntent.putExtra(EXTRA_KEY_DESTINATIONFILE, mAppSpaceADFFolder);
      thisActivity.startActivityForResult(exportIntent, TANGO_INTENT_ACTIVITYCODE);
    } else if (itemName.equals(mAppSpaceMenuStrings[0])) {
      // Delete an ADF from App space and update the App space ADF Listview.
      File file = new File(mAppSpaceADFFolder + File.separator + mAppSpaceUUIDList[info.position]);
      file.delete();
      updateADFList();
    } else if (itemName.equals(mAppSpaceMenuStrings[1])) {
      // Import an ADF into API private Storage and update the API ADF Listview.
      String filepath = mAppSpaceADFFolder + File.separator + mAppSpaceUUIDList[info.position];
      Intent importIntent = new Intent();
      importIntent.setClassName(INTENT_CLASSPACKAGE, INTENT_IMPORTEXPORT_CLASSNAME);
      importIntent.putExtra(EXTRA_KEY_SOURCEFILE, filepath);
      thisActivity.startActivityForResult(importIntent, TANGO_INTENT_ACTIVITYCODE);
    }
    return true;
  }
  
  @Override
  protected void onActivityResult(int requestCode, int resultCode, Intent data) {
    // Check which request we're responding to
    if (requestCode == TANGO_INTENT_ACTIVITYCODE) {
        // Make sure the request was successful
        if (resultCode == RESULT_CANCELED) {
            Toast.makeText(this, R.string.no_permissions, Toast.LENGTH_LONG)
                    .show();
        }
    }
    updateADFList();
  }

  /* Returns maps storage location in the App package folder. 
   * Creates a folder called Maps, if it doesnt exist.
   */
  private String getAppSpaceADFFolder() {
    String mapsFolder = getFilesDir().getAbsolutePath() + File.separator + "Maps";
    File file = new File(mapsFolder);
    if (!file.exists()) {
      file.mkdirs();
    }
    return mapsFolder;
  }
  
  private void updateADFList() {
    // Get API ADF ListView ready.
    mUUIDListView = (ListView) findViewById(R.id.uuidlistviewAPI);
    mADFDataSource = new ADFDataSource(this);
    mUUIDList =  mADFDataSource.getFullUUIDList();
    mUUIDNames = mADFDataSource.getUUIDNames();
    mADFAdapter = new ADFUUIDArrayAdapter(this, mUUIDList, mUUIDNames);
    mUUIDListView.setAdapter(mADFAdapter);
    registerForContextMenu(mUUIDListView);
    
    // Get apps space ADF list ready.
    mAppSpaceUUIDListView = (ListView) findViewById(R.id.uuidlistviewApplicationSpace);
    mAppSpaceADFFolder = getAppSpaceADFFolder();
    mAppSpaceUUIDList = getAppSpaceADFList();
    mAppSpaceADFAdapter = new ADFUUIDArrayAdapter(this, mAppSpaceUUIDList, null);
    mAppSpaceUUIDListView.setAdapter(mAppSpaceADFAdapter);
    registerForContextMenu(mAppSpaceUUIDListView);
  }

  /*
   * Returns the names of all ADFs in String array in the
   * files/maps folder.
   */
  private String[] getAppSpaceADFList() {
    File file = new File(mAppSpaceADFFolder);
    File[] adfFileList = file.listFiles();
    String[] appSpaceADFList = new String[adfFileList.length];
    for (int i = 0; i < appSpaceADFList.length; ++i) {
      appSpaceADFList[i] = adfFileList[i].getName();
    }
    Arrays.sort(appSpaceADFList);
    return appSpaceADFList;
  }
  
  private void showSetNameDialog(String mCurrentUUID) {
    Bundle bundle = new Bundle();
    String name = TangoJNINative.getUUIDMetadataValue(mCurrentUUID, "name");
    if (name != null) {
      bundle.putString("name", name);
    }
    bundle.putString("id", mCurrentUUID);
    FragmentManager manager = getFragmentManager();
    SetADFNameDialog setADFNameDialog = new SetADFNameDialog();
    setADFNameDialog.setArguments(bundle);
    setADFNameDialog.show(manager, "ADFNameDialog");
  }

  @Override
  public void setNameAndUUID(String name, String uuid) {
    TangoJNINative.setUUIDMetadataValue(uuid, "name", name.length(), name);

    mUUIDList = mADFDataSource.getFullUUIDList();
    mUUIDNames = mADFDataSource.getUUIDNames();
    mADFAdapter = new ADFUUIDArrayAdapter(this, mUUIDList, mUUIDNames);
    mUUIDListView.setAdapter(mADFAdapter);
  }
}

/**
 * This is an adapter class which maps the ListView with 
 * a Data Source(Array of strings).
 *
 */
class ADFUUIDArrayAdapter extends ArrayAdapter<String> {
  Context mContext;
  private String[] mUUIDStringArray, mUUIDNamesStringArray;
  public ADFUUIDArrayAdapter(Context context, String[] uuids, String[] uuidNames) {
    super(context, R.layout.uuid_view, R.id.uuid, uuids);
    mContext = context;
    mUUIDStringArray = uuids;
    if (uuidNames != null) {
      mUUIDNamesStringArray = uuidNames;
    }
  }
  
  @Override
  public View getView(int position, View convertView, ViewGroup parent) {
    LayoutInflater inflator = (LayoutInflater) mContext.getSystemService(
        Context.LAYOUT_INFLATER_SERVICE);
    View row = inflator.inflate(R.layout.uuid_view, parent, false);
    TextView uuid = (TextView) row.findViewById(R.id.uuid);
    TextView uuidName = (TextView) row.findViewById(R.id.adfName);
    uuid.setText(mUUIDStringArray[position]);
    
    if (mUUIDNamesStringArray != null) {
      uuidName.setText(mUUIDNamesStringArray[position]);
    } else {
      uuidName.setText("Metadata cannot be read");
    }
    return row;
  }
}
