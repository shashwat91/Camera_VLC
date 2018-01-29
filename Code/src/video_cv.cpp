#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "detection.hpp"

#include <iostream>
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

using namespace std;
using namespace cv;

int main(int argc, char const *argv[])
{
	VideoCapture inputVideo("Videos/PWM2_50.mp4");

	if (!inputVideo.isOpened())
    {
        cout  << "Could not open the input video: " << endl;
        return -1;
    }

    Mat camera;
    namedWindow("camera",1);
    // namedWindow("original",2);
    int i=0;
    inputVideo>>camera;
    cout<<"Video size :: "<<camera.rows<<"X"<<camera.cols<<endl;
    while(camera.data)
    {
        i++;
        // imshow("original", camera);
        // transpose(camera, camera);
        // flip(camera, camera,1);
        cout<<i<<"\t:: ";
        vector<vector<int> > returnMatrix(5, vector<int>(3)); // contains center positions and radii of blobs
        cvtColor(camera, camera, COLOR_RGB2GRAY);
        detector(camera, returnMatrix);
        imshow("camera", camera);
        // imwrite("Videos/"+patch::to_string(i)+"_camera.jpg",camera);
        inputVideo>>camera;
        if(waitKey(1)>0)
            break;
    }
    cout<<"number of frames :: "<<i<<endl;

	return 0;
}