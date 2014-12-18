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

import java.util.ArrayList;
import java.util.List;
import java.util.StringTokenizer;

import android.content.Context;
import android.widget.Toast;

/**
 * This class interfaces a Tango Object and maintains a 
 * full list of ADF UUIds. Whenever an ADF is deleted or added,
 * getFullUUIDList needs to be called to update the UUIDList within this class.
 *
 */
public class ADFDataSource {
  private ArrayList<String> mFullUUIDList;
  private Context mContext;
  public ADFDataSource(Context context) {
    mContext = context;
    mFullUUIDList = new ArrayList<String>();
  }
  
  public String[] getFullUUIDList() {
    String allUUID = TangoJNINative.getAllUUIDs();
    StringTokenizer tok = new StringTokenizer(allUUID, ",");
    mFullUUIDList.clear();
    while (tok.hasMoreElements()) {
      mFullUUIDList.add(tok.nextToken());
    }
    if (mFullUUIDList.size() == 0) {
      Toast.makeText(mContext, R.string.no_adfs_tango_error, Toast.LENGTH_SHORT).show();
    }
    return mFullUUIDList.toArray(new String[mFullUUIDList.size()]);
  }
  
  public String[] getUUIDNames() {
    ArrayList<String> list = new ArrayList<String>();

    for (String s : mFullUUIDList) {
      String name = TangoJNINative.getUUIDMetadataValue(s, "name");
      list.add(name);
    }
    
    return list.toArray(new String[list.size()]);
  }
  
  public void deleteADFandUpdateList(String uuid) {
    TangoJNINative.deleteADF(uuid);
    mFullUUIDList.clear();
    String allUUID = TangoJNINative.getAllUUIDs();
    StringTokenizer tok = new StringTokenizer(allUUID, ",");

    while (tok.hasMoreElements()) {
      mFullUUIDList.add(tok.nextToken());
    }
  }

}
