/************************************************************************************

Filename    :   MainActivity.java
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2014 Oculus VR, LLC. All Rights reserved.

*************************************************************************************/
package com.oculusvr.vrscene;

import android.os.Bundle;
import android.util.Log;
import android.widget.Toast;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.IntentFilter;

import com.oculusvr.vrlib.VrActivity;
import com.oculusvr.vrlib.VrLib;
import android.hardware.usb.*;

import java.util.*;
import android.content.Context;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;

public class MainActivity extends VrActivity {

	public static final String TAG = "VrScene";
	private byte[] bytes;
	private boolean forceClaim = true; 
	UsbDeviceConnection connection = null;
	UsbInterface gyroDataIntf_ = null;
	UsbInterface hidInputIntf_ = null;
	UsbEndpoint hidEndpoint_ = null;
	boolean buttonDown_ = false;
	private static final String ACTION_USB_PERMISSION ="com.oculusvr.USB_PERMISSION";
	
	/** Load jni .so on initialization */
	static {
		Log.d( TAG, "LoadLibrary" );
		System.loadLibrary( "vrscene" );
	}

	public static native long nativeSetAppInterface( VrActivity act, String fromPackageNameString, String commandString, String uriString );

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		Log.d( TAG, "onCreate" );
		super.onCreate(savedInstanceState);

		Intent intent = getIntent();
		String commandString = VrLib.getCommandStringFromIntent( intent );
		String fromPackageNameString = VrLib.getPackageStringFromIntent( intent );
		String uriString = VrLib.getUriStringFromIntent( intent );

		appPtr = nativeSetAppInterface( this, fromPackageNameString, commandString, uriString );

		//InitUsbDevice();
        IntentFilter filter = new IntentFilter();
        filter.addAction(ACTION_USB_PERMISSION);
        filter.addAction(UsbManager.ACTION_USB_DEVICE_ATTACHED);
        filter.addAction(UsbManager.ACTION_USB_DEVICE_DETACHED);
        registerReceiver(mUsbReceiver, filter);
        
        ScanUsb();
	}
	//���讔ǰusb�˿��Ƿ����O���B�����������Ҳ��Ҫ��Ո����
	protected void ScanUsb(){
		UsbManager manager = (UsbManager) getSystemService(Context.USB_SERVICE);
		HashMap<String, UsbDevice> deviceList = manager.getDeviceList();
		Iterator<UsbDevice> deviceIterator = deviceList.values().iterator();
		while(deviceIterator.hasNext()){
			UsbDevice device=deviceIterator.next();
			PendingIntent mPermissionIntent = PendingIntent.getBroadcast(getApplicationContext(), 0, new Intent(ACTION_USB_PERMISSION), 0);
            manager.requestPermission(device, mPermissionIntent);
		}
	}
	protected final BroadcastReceiver mUsbReceiver = new BroadcastReceiver() {
		   public void onReceive(Context context, Intent intent) {
		        String action = intent.getAction();
		        //����usb attach ���û�ͬ����Ȩ�޵��¼�
		        if (ACTION_USB_PERMISSION.equals(action)) {
		            synchronized (this) {
		                UsbDevice device = (UsbDevice)intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
		                if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
		                    if(device != null)
		                    	InitUsbDevice(device);		                    
		                } 
		                else {
		                    Log.d(TAG, "permission denied for device " + device);
		                }
		            }
		        }
		        //����usb attach �󵯳��Ի������û�����Ȩ��
		        else if (UsbManager.ACTION_USB_DEVICE_ATTACHED.equals(action)) {
			           UsbDevice device = (UsbDevice) intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
			           String deviceName = device.getDeviceName();
			           Toast.makeText(getApplicationContext(), "UsbDevice attached ="+deviceName, 0).show();
			        PendingIntent mPermissionIntent = PendingIntent.getBroadcast(context, 0, new Intent(ACTION_USB_PERMISSION), 0);
                    UsbManager mUsbManager = (UsbManager) getSystemService(Context.USB_SERVICE);
                    mUsbManager.requestPermission(device, mPermissionIntent);
                    Log.d(TAG, "after requestPermission");
			    }

		        //����usb dettach ����¼�
			    else if (UsbManager.ACTION_USB_DEVICE_DETACHED.equals(action)) {
			           UsbDevice device = intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
			           String deviceName = device.getDeviceName();
			           Toast.makeText(getApplicationContext(), "UsbDevice dettached ="+deviceName, 0).show();  
			    }		        
		    }
		};
		
	protected int CheckUsbDevice(UsbDevice device) {
		int deviceType = -1;
		if (device.getVendorId() == 10291 && device.getProductId() == 1) {
			deviceType = 1;
		} else if ((device.getVendorId() == 1155 && device.getProductId() == 22336)
				|| (device.getVendorId() == 1155 && device.getProductId() == 22352)
				|| (device.getVendorId() == 949 && device.getProductId() == 1)) {
			deviceType = 0;
		}
		return deviceType;
	}
	protected int FindButtonGyro(UsbDevice device) {
		
		int nInterface = device.getInterfaceCount();
		if (nInterface < 1) {
			return -1;
		}

		Log.d(TAG, "usb device found interface count " + nInterface);
		for (int i = 0; i < nInterface; i++) {
			UsbInterface intf = device.getInterface(i);
			String str = String.format("interface %d %d.%d.%d", intf.getId(),
					intf.getInterfaceClass(), intf.getInterfaceSubclass(),
					intf.getInterfaceProtocol());
			Log.i(TAG, str);

			if (intf.getInterfaceClass() == 10
					&& intf.getInterfaceSubclass() == 0
					&& intf.getInterfaceProtocol() == 0) {
				gyroDataIntf_ = intf;
				Log.i(TAG, "found cdc interface " + intf.toString());
			}

			if (intf.getInterfaceClass() == 0x03
					&& intf.getInterfaceSubclass() == 0
					&& intf.getInterfaceProtocol() == 0) {
				this.hidInputIntf_ = intf;
				Log.i(TAG, "found hid interface " + intf.toString());
			}

			int numEndpoint = intf.getEndpointCount();
			for (int j = 0; j < numEndpoint; j++) {
				UsbEndpoint endpoint = intf.getEndpoint(j);
				String endpointInfo = String.format(
						"endpoint %d addr 0x%x number %d attri %d", j,
						endpoint.getAddress(), endpoint.getEndpointNumber(),
						endpoint.getAttributes());
				Log.i(TAG, endpointInfo);
				if (this.hidInputIntf_ == intf) {
					this.hidEndpoint_ = endpoint;
				}

			}
		}
		UsbManager manager = (UsbManager) getSystemService(Context.USB_SERVICE);
		connection = manager.openDevice(device);
		if (gyroDataIntf_ != null) {
			boolean bRes = connection.claimInterface(gyroDataIntf_, forceClaim);
			Log.i(TAG, String.format("claim gyro data interface %s",
					bRes ? "succeeded" : "failed"));
		}

		if (hidInputIntf_ != null) {
			boolean bRes = connection.claimInterface(hidInputIntf_, forceClaim);
			Log.i(TAG, String.format("claim hid input interface %s",
					bRes ? "succeeded" : "failed"));
		}
		return 1;
	}
		
	protected void InitUsbDevice(UsbDevice device) {

		//UsbManager manager = (UsbManager) getSystemService(Context.USB_SERVICE);

		int deviceType=CheckUsbDevice(device);
		if(deviceType==-1)
			return;

		int res=FindButtonGyro(device);
		if(res==-1)
			return;

		int fd = connection.getFileDescriptor();
		Log.i(TAG, String.format("FileDescriptor %d", fd));

		setupUsbDevice(fd, deviceType, false);

		// use native thread
		StartUsbIOThread();

		// 0xa1 : device to host / Type : class / Recipien : interface
		// request : 1 (get report)
		// value : 0x300 HID_FEATURE 0x3 FEATURE_CALIBRATE

		// size = connection.controlTransfer(0xa1, 1, 0x303, 0, bytes, 256, 0);
		// addText("control read " + size);

		// size = connection.controlTransfer(0xa1, 1, 0x304, 0, bytes, 256, 0);
		// addText("control read " + size);			

	}
	
	void HandleHidInputData(byte[] data, int size)
	{
		if (size == 0){
			return;
		}
					
		if (data[6] == (byte)0xf) {
			this.buttonDown_ = true;
		} else if (buttonDown_ && data[6] == (byte)0xf0) {			
			this.buttonDown_ = false;
			
			Log.i(TAG, "button clicked!!!");
		}		
	}
	
	void StartUsbIOThread() {
		bytes = new byte[256];
		
		new Thread(new Runnable() {
			public void run() {
							
				while (true) {	
					int size = connection.bulkTransfer(hidEndpoint_, bytes, bytes.length, 0);
					HandleHidInputData(bytes, size);
					//PushData(bytes, size);
				}				
			}
		}).start();
	}
	
		
    public native void PushData(byte[] buffer, int length);	
    public native void setupUsbDevice(int fd, int deviceType, boolean startThread);
	
}
