package oculus;

import java.io.File;
import java.net.MalformedURLException;
import java.util.ArrayList;

import android.os.Environment;
import android.util.Log;

//��װ��������
public class DiskEnum {
	
	static private void r_scan(File dir_handle,ArrayList<String> res)
	{
		Log.d("diskenum","r_scan:"+dir_handle.getPath());
		 File[] all_video_fn=dir_handle.listFiles();
	     if(all_video_fn!=null)
	     {
		     for(File fn:all_video_fn)
		     {
		    	 if(fn.isFile()){
		    		 if(fn.getName().endsWith(".mp4")){
		    			 res.add(fn.getPath ());
		    		 }
		    	 }
		    		 
		    	 else if(fn.isDirectory())
		    		 r_scan(fn,res);
		     }	     	    	 
	     }

	}
	
	
	//ɨ���ֻ�sd�������ⲿ sd ���� fancytechĿ¼���ҵ����е�mp4
	static public ArrayList<String> scanVideoAndUpdateUI()
    {
    	
    	ArrayList<String> mp4_fl=new ArrayList<String>();
    	
    	//����Ŀ¼���ҵ����е���Ƶ�ļ�
        File videoDirHandle=Environment.getExternalStorageDirectory();
        r_scan(videoDirHandle,mp4_fl);
    	
        return mp4_fl;
    	

    }
}
