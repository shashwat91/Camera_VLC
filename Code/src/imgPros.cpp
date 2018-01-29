#include "detection.hpp"

#include <iostream>
#include <stdio.h>
#include <fstream>
#include <iomanip>

using namespace cv;
using namespace std;

String Image_path;
int main( int argc, char** argv )
{
    Mat grey_img;
    
    if( argc < 2)
    {
     cout <<" Usage: display_image ImageToLoadAndDisplay" << endl;
     return -1;
    }

    grey_img = imread( argv[1], 0);
    if ( !grey_img.data )
    {
        cout<<"No image data\n";
        return -1;
    }
    Image_path = argv[1];
    Image_path.erase(Image_path.find('.'));

    cout<<"Image dimension::"<<grey_img.rows<<"x"<<grey_img.cols<<endl;
    /*	//--Applied nominal threshold filter
    Mat bin;
    threshold( grey_img, bin, 100,255,THRESH_BINARY );
    imwrite("threshold_image(nominal).jpg", bin);
    namedWindow("binary",1);
    imshow("binary", bin);
    waitKey(0);
    */

    // transpose(grey_img, grey_img);
    // flip(grey_img, grey_img,1);
    	//--Original detection method
    vector<vector<int> > returnMatrix(5, vector<int>(3)); // contains center positions and radii of blobs
	detector(grey_img, returnMatrix);
	cout<<"Circles plotted"<<endl;
	imwrite(Image_path+"1_detector_image.jpg", grey_img);

	int centerRadius[3];
	centerRadius[0] = returnMatrix[0][0];
	centerRadius[1] = returnMatrix[0][1];
	centerRadius[2] = ceil(returnMatrix[0][2]*1.5);

	Mat adaptive_equalized = clahe(grey_img);
	imwrite(Image_path+"2_adaptive_equalized.jpg", adaptive_equalized);
	cout<<"adaptive equalisation completed"<<endl;

	Mat blurred;
	blur(adaptive_equalized, blurred, Size(1, 2));
	imwrite(Image_path+"3_blurred.jpg", blurred);
	cout<<"blur completed"<<endl;
	
	adaptive_equalized.release();
	Mat adaptive_threshold2 = adaptiveThreshold(blurred);
	Mat adaptive_threshold;
	threshold( grey_img, adaptive_threshold, 65,255,THRESH_BINARY );

	cout<<"adaptive threshold completed"<<endl;
	imwrite(Image_path+"4_threshold_image.jpg", adaptive_threshold);
	imwrite(Image_path+"4_adaptive_threshold_image.jpg", adaptive_threshold2);

	int offset = avoidBlobOffset(adaptive_threshold2, centerRadius);
	cout<<"offset calculated"<<endl;
	
	vector<int> correctedPixels;
	getCorrectedPixelsOffset(adaptive_threshold, centerRadius, offset, correctedPixels);
	cout<<"Pixels corrected\n";
	imwrite(Image_path+"5_line_image.jpg", correctedPixels);

	vector<int> detectedBits;
	decodeBits_MAE(correctedPixels, detectedBits);

	// vector<int> detectedBits;
	// decodeBits(correctedPixels, detectedBits);
	if(detectedBits.size() == 0)
	{
	    cout<<"Image not decoded properly"<<endl;
	}
	else
	{
		cout<<"Bits decoded"<<endl;
		for(int i=0; i<detectedBits.size(); i++)
	  		cout<<std::hex<<detectedBits[i]<<endl;
	}

    return 0;
}
