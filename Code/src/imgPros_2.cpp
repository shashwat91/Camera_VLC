#include "detection.hpp"

#include <iostream>
#include <stdio.h>
#include <fstream>
#include <iomanip>
#include <string.h> 
#include <sstream>

namespace patch
{
    template < typename T > std::string to_string( const T& n )
    {
        std::ostringstream stm ;
        stm << n ;
        return stm.str() ;
    }
}

using namespace cv;
using namespace std;

void image_detection(String path)
{
	Mat grey_img;

    grey_img = imread( path, 0);
    if ( !grey_img.data )
    {
        cout<<"No image data\n";
        return ;
    }
    // String Image_path = "circles";
    // Image_path.erase(Image_path.find('.'));

    	//--Original detection method
    vector<vector<int> > returnMatrix(5, vector<int>(3)); // contains center positions and radii of blobs
	detector(grey_img, returnMatrix);
	// cout<<"Circles plotted"<<endl;
	// imwrite("../circles/"+path, grey_img);

	int centerRadius[3];
	centerRadius[0] = returnMatrix[0][0];
	centerRadius[1] = returnMatrix[0][1];
	centerRadius[2] = ceil(returnMatrix[0][2]*1.5);

	Mat adaptive_equalized = clahe(grey_img);
	// imwrite(Image_path+"_2adaptive_equalized.jpg", adaptive_equalized);
	// cout<<"adaptive equalisation completed"<<endl;

	Mat blurred;
	blur(adaptive_equalized, blurred, Size(1, 2));
	// imwrite(Image_path+"_3blurred.jpg", blurred);
	// cout<<"blur completed"<<endl;
	
	adaptive_equalized.release();
	//Mat adaptive_threshold = adaptiveThreshold(blurred);
	Mat adaptive_threshold;
	threshold( grey_img, adaptive_threshold, 80,255,THRESH_BINARY );

	// cout<<"adaptive threshold completed"<<endl;
	// imwrite(Image_path+"_4adaptive_threshold.jpg", adaptive_threshold);

	int offset = avoidBlobOffset(adaptive_threshold, centerRadius);
	// cout<<"offset calculated"<<endl;
	
	vector<int> correctedPixels;
	getCorrectedPixelsOffset(adaptive_threshold, centerRadius, offset, correctedPixels);
	// imwrite(Image_path+"_5line.jpg", correctedPixels);

	vector<int> detectedBits;
	decodeBits_MAE(correctedPixels, detectedBits);
}

int main( int argc, char** argv )
{
    String Image_path;
//MAE_50_1_small.jpg
    for(int i=1;i<=10;++i)
    {
        // for(int j=1;j<=3;++j)
        // {
            // Image_path = "Images_encoding1/Image";
            // Image_path += patch::to_string(i);
            // Image_path += "_";
            // Image_path += patch::to_string(j);
            // Image_path += "_small.jpg";
            // // cout<<"\n\n\n\n\t\t\tImage :: "<<Image_path<<endl;
            // cout<<"Image :: "<<Image_path<<endl;

            // Image_path = "Videos/";
            Image_path = "/home/shashwat/Desktop/MAE_4/MAE_";
            Image_path += patch::to_string(i);
            Image_path += "_small.jpg";
            // Image_path += patch::to_string(j);
            // Image_path += "_small.jpg";
            // // cout<<"\n\n\n\n\t\t\tImage :: "<<Image_path<<endl;
            cout<<"Image :: "<<Image_path<<endl;
            image_detection(Image_path);
        // }
    }
    return 0;
}
