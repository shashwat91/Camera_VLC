#include <jni.h>
#include <opencv2/opencv.hpp>
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <android/log.h>
#include <time.h>
#include <iomanip>

#define  LOG_TAG    "JNI_PART"

#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

using namespace std;
using namespace cv;

Mat * mCanny = NULL;

extern "C"
{
	JNIEXPORT jintArray JNICALL Java_com_example_testvlc_ProcessImage_decode(JNIEnv* env, jobject thiz, jint width, jint height, jint centerRow, jint centerColumn, jint blobRadius );
  	void detector(Mat input, vector< vector<int> > &returnMatrix);
  	Mat clahe(Mat input);
  	Mat blur(Mat input);
  	Mat adaptiveThreshold(Mat input);
  	void decodeBits(vector<int>& inputPixels, vector<int>& detectedBits);
  	int avoidBlobOffset(Mat input, int centerRadius[3]);
  	void getCorrectedPixelsOffset(Mat input, int centerRadius[3], int offset, vector<int>& correctPixels);
	
	JNIEXPORT jintArray JNICALL Java_com_example_testvlc_ProcessImage_decode(JNIEnv* env, jobject thiz, jint width, jint height, jint centerRow, jint centerColumn, jint blobRadius )
	{
		LOGE("c++ code initialised");
		string file_path = "/storage/emulated/0/blobtest/";
		Mat grey_img;
		grey_img = imread(file_path+"original_.jpg",CV_LOAD_IMAGE_GRAYSCALE);
		transpose(grey_img, grey_img);
		flip(grey_img, grey_img,1);
		LOGE("image converted to grayScal");
		//imwrite(file_path + "rec_image.jpg",grey_img);

		vector<vector<int> > returnMatrix(5, vector<int>(3)); // contains center positions and radii of blobs
		LOGE("Starting detector function");
		detector(grey_img, returnMatrix);
		LOGE("Circles plotted");
		imwrite(file_path + "detector_image.jpg", grey_img);

		int centerRadius[3];
		centerRadius[0] = returnMatrix[0][0];
		centerRadius[1] = returnMatrix[0][1];
		centerRadius[2] = ceil(returnMatrix[0][2]*1.5);

		Mat adaptive_equalized = clahe(grey_img);
		LOGE("adaptive equalisation completed");
		Mat blurred = blur(adaptive_equalized);
		LOGE("blur completed");
		adaptive_equalized.release();
		Mat adaptive_threshold = adaptiveThreshold(blurred);
		LOGE("adaptive threshold completed");
		imwrite(file_path + "adaptive_threshold_image.jpg", adaptive_threshold);

		int offset = avoidBlobOffset(adaptive_threshold, centerRadius);
		LOGE("offset calculated");
		vector<int> correctedPixels;
		getCorrectedPixelsOffset(adaptive_threshold, centerRadius, offset, correctedPixels);
		imwrite(file_path + "line_image.jpg", correctedPixels);
		vector<int> detectedBits;

		decodeBits(correctedPixels, detectedBits);
		jintArray result;

		if(detectedBits.size() == 0)
		{
		    LOGE("Image not decoded properly");
		}

		LOGE("bits decoded");


		jint fill[detectedBits.size()];

		for(int i=0; i<detectedBits.size(); i++)
		  fill[i] = detectedBits[i];

		result = env->NewIntArray(detectedBits.size());
		env->SetIntArrayRegion(result, 0, detectedBits.size(), fill);

		return result;
	}

	void detector(Mat input, vector< vector<int> > &returnMatrix)
	{
	  Mat blurred_image;
	  blur(input, blurred_image, Size(100, 100));

	  Mat otsu;
	  threshold(blurred_image, otsu, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	  blurred_image.release();

	  Mat cimg = input;

	  vector<vector<Point> > contours;
	  vector<Point2i> center;
	  vector<int> radius;

	  findContours(otsu.clone(), contours, CV_RETR_TREE, CV_CHAIN_APPROX_NONE);
	  otsu.release();
	  LOGE("countours found");

	  size_t count = contours.size();

	  for( size_t i=0; i < count; i++)
	  {
		  Point2f c;
		  float r;
		  minEnclosingCircle( contours[i], c, r);

		  if (r >= 50)
		  {
			  center.push_back(c);
			  radius.push_back(r);
		  }
	  }
	  int count2 = center.size();
	  Scalar red(255,255,255);

	  returnMatrix.resize(count2);

	  for( int i = 0; i < count2; i++)
	  {
		  circle(input, center[i], radius[i], red, 3);
		  returnMatrix[i][0] = center[i].x;
		  returnMatrix[i][1] = center[i].y;
		  returnMatrix[i][2] = radius[i];
		  //cout << radius[i];
	  }
	}

	Mat clahe(Mat input)
	{

		Ptr<CLAHE> clahe = createCLAHE();
		clahe->setClipLimit(10);
		clahe->setTilesGridSize(Size(8,8));

		Mat adaptive_equalized;
		clahe->apply(input,adaptive_equalized);


		return adaptive_equalized;
	}

	Mat blur(Mat input)
	{
		Mat blurred_image;
		blur(input, blurred_image, Size(3, 3));

		return blurred_image;
	}

	Mat adaptiveThreshold(Mat input)
	{
		Mat adaptive;
		adaptiveThreshold(input, adaptive, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 105, 2);
		return adaptive;
	}

	void decodeBits(vector<int>& inputPixels, vector<int>& detectedBits)
	{
	    vector<int> peaks;
	    vector<int> falls;

	    int dif;

	    for (unsigned int i = 0; i < inputPixels.size()-1; i++)
	    {
	        if(inputPixels[i] == 0 && inputPixels[i+1] == 255)
	        {
	            peaks.push_back(i);
	            //cout<<"on "<<i<<endl;
	        }
	        else if(inputPixels[i] == 255 && inputPixels[i+1] == 0 && peaks.size() !=0) //Peaks.size()!=0 makes sure that first transition is rising edge
	        {
	            falls.push_back(i);
	            //cout<<"off "<<i<<endl;
	        }
	    }

	    if(peaks.size() > falls.size())
	        peaks.erase(peaks.begin() + peaks.size() - 1);      //Removes the last peak if no fall is associated to it.

	    // for (int i=0; i<=peaks.size()-1; ++i) 
	    //     cout <<setw(3)<< i <<" :: " << setw(4) << peaks[i] << " , " << setw(4) << falls[i]<<" : "<<setw(4)<<falls[i]-peaks[i]<<endl;

	    int j = 1;
	    int firstPreamble=0;
	    int secondPreamble=0;

	    for (unsigned int i = 0; i < peaks.size(); i++)
	    {
	        dif = falls[i] - peaks[i];
	        //cout<<"diff :: "<<dif;
	        if ( dif >= 35 ) //2.5*16=40
	        { //freq = 3000 -> 2.5*8
	          //  cout<<"\tPreamble detected";
	            //**Change this to something more adaptable
	            if( j == 1 )
	            {
	                if(i == 0)
	                    continue;
	                firstPreamble = i;
	                j++;
	            }
	            else
	            {
	                secondPreamble = i;
	                break;
	            }
	        }
	        // cout<<endl;
	    }

	    if(secondPreamble == 0)
	        return;

	    // cout << "\nfirst " << firstPreamble << " second " << secondPreamble<< "\n" ;

	    j = 1;
	    int addZero = 0;

	    while(true)
	    {
	        dif = peaks[firstPreamble +j] - falls[firstPreamble + j -1];
	        if(dif > 1.3 * 16 ) // 1.3*16=20.8
	        { //freq = 3000 -> 2.5*8
	            if(j == 1 || (firstPreamble + j == secondPreamble))
	            {
	                // cout<<"0";
	                detectedBits.push_back(0);
	            }
	            else
	            {
	                // cout << "0";
	                // cout << "/0";
	                detectedBits.push_back(0);
	                detectedBits.push_back(0);
	            }
	        }
	        else
	        {
	            if((j != 1 && firstPreamble + j != secondPreamble) || addZero == 1)
	            {
	                // cout << "0";
	                detectedBits.push_back(0);
	            }
	        }
	        if (firstPreamble + j == secondPreamble)
	            break;

	        dif = falls[firstPreamble +j] - peaks[firstPreamble + j];

	        if(dif > 1.5 * 16 ) // 1.5*16=24
	        { //freq = 3000 -> 2.5*8
	            detectedBits.push_back(1);
	            detectedBits.push_back(1);
	            if(firstPreamble + j +1 == secondPreamble)
	                addZero = 1;
	            // cout << "1";
	            // cout << "/1";
	        }
	        else
	        {
	            detectedBits.push_back(1);
	            // cout<<"1";
	        }
	        j++;
	    }
	    // cout<<endl;
	}

	int avoidBlobOffset(Mat input, int centerRadius[3])
	{
		float factor = 1;
		int newRadius = ceil(centerRadius[2] * factor);
		int top = centerRadius[1] - newRadius;
		int bottom = centerRadius[1] + newRadius;

		if(top < 0){
			top = 0;
		}
		if(bottom > input.rows){
			bottom = input.rows;
		}

		int column = centerRadius[0];

		while(1)
		{
			vector<int> pixels;
			vector<int> peaks;
			vector<int> falls;
			for(int i = top ; i< bottom ; i++)
			{
				//cout<<"i::"<<i<<"input at i::"<<column;
				pixels.push_back(input.at<unsigned char>(i, column));
			}

			for (unsigned int i = 0; i < pixels.size()-1; i++)
			{
				if(pixels[i] == 0 && pixels[i+1] == 255)
					peaks.push_back(i);
				else if(pixels[i] == 255 && pixels[i+1] == 0 && peaks.size() !=0)
					falls.push_back(i);
			}

			int noBlob = 0;

			for (unsigned int i = 0; i < falls.size(); i++)
			{
				int dif = falls[i] - peaks[i];
				if (dif > 16*3)
				{ //freq = 3000 -> 3*8
					column = column - 1;
					noBlob = 1;
				}
			}

			if(noBlob == 0)
				break;
		}

		return centerRadius[0] - column;
	}

	void getCorrectedPixelsOffset(Mat input, int centerRadius[3], int offset, vector<int>& correctPixels )
	{
		float factor = 1;
		int newRadius = ceil(centerRadius[2] * factor);

		int top = centerRadius[1] - newRadius;
		int bottom = centerRadius[1] + newRadius;

		if(top < 0){
			top = 0;
		}
		if(bottom > input.rows){
			bottom = input.rows;
		}
		cout<<"new radius::"<<newRadius<<endl;

		for(int i = top ; i< bottom ; i++){
			correctPixels.push_back(input.at<unsigned char>(i, centerRadius[0] - offset) );
		}
	}
}
