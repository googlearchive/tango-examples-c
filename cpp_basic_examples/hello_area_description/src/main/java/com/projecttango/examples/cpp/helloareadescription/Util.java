/*
 * Copyright 2014 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.projecttango.examples.cpp.helloareadescription;

import android.content.Context;
import android.database.Cursor;
import android.net.Uri;

/**
 * Util class provides handy utility functions.
 */
public class Util {

    /**
     * Checks if the calling app has the specified permission.
     * It is recommended that an app check if it has a permission before trying
     * to request it; this will save time by avoiding re-requesting permissions
     * that have already been granted.
     *
     * @param context        The context of the calling app.
     * @param permissionType The type of permission to request; either
     *                       PERMISSIONTYPE_MOTION_TRACKING or PERMISSIONTYPE_ADF_LOAD_SAVE.
     * @return boolean Whether or not the permission was already granted.
     */
    public static boolean hasPermission(Context context, String permissionType) {
        Uri uri = Uri.parse("content://com.google.atap.tango.PermissionStatusProvider/" +
                permissionType);
        Cursor cursor = context.getContentResolver().query(uri, null, null, null, null);
        if (cursor == null) {
            return false;
        } else {
            return true;
        }
    }
}
