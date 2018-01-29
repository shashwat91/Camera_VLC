package com.example.testvlc;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;

import android.media.Image;
import android.util.Log;

public class ProcessImage 
{
	private Image image=null;
	private File file;
	private String filePath = "/storage/emulated/0/blobtest/";
	private DateFormat dateFormat;
	private Date date;
	private static final String TAG = "ProcessingAPI";
	private String path;
	 
	static 
	{
		System.loadLibrary("jni_imgPros");
	}

	public native int[] decode(int width, int height, int centerRow, int centerColumn, int blobRadius);
	
	ProcessImage(Image input)
	{
		assert input != null;
		image=input;
		dateFormat = new SimpleDateFormat("hhmm_yyMMdd");
		date = new Date();
		Log.d(TAG, "Processing object created");
	}
	
	public int[] processframe() throws IOException
	{
		Log.d(TAG, "Strting Blob processing");
		Log.d(TAG, "Dimensions of image :: "+image.getWidth()+image.getHeight());
		byte[] bytes = save(image,"original_");
		
		int row=0,col=0,radius=0;
		int[] result = decode(image.getWidth(), image.getHeight(), row, col, radius);
		Log.d(TAG, "Finished Blob processing");
		//System.out.println(result[0]);
		//Log.d(TAG, "Decoded bits" + result[0]);
		if(result.length == 0)
		{
			System.out.println("Image not decoded properly");
			return null;
		}
		Log.d(TAG, "Printing Results of size:"+result.length);
		for(int i=0; i<result.length; ++i)
		{
			System.out.println(result[i]);
		}
		return result;
	}
	
	private byte[] save(Image imagetosave, String name) throws IOException 
    {
		ByteBuffer buffer = imagetosave.getPlanes()[0].getBuffer();
        byte[] bytes = new byte[buffer.capacity()];
        buffer.get(bytes);
       
        //path = filePath+dateFormat.format(date)+name+".jpg";
        //file = new File(path);	//With date tag
        path = filePath+name+".jpg";
        file = new File(path); //Without date tag
        OutputStream output = null;
        try
        {
            output = new FileOutputStream(file);
            output.write(bytes);
        }
        finally 
        {
            if (null != output)
                output.close();
        }
        return bytes;
    }
}
