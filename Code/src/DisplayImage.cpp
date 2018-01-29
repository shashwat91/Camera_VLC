#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>


using namespace cv;
using namespace std;

int main( int argc, char** argv )
{
    Mat image;
    
    if( argc < 2)
    {
     cout <<" Usage: display_image ImageToLoadAndDisplay" << endl;
     return -1;
    }

    image = imread( argv[1], 0);
    
    if ( !image.data )
    {
        cout<<"No image data\n";
        return -1;
    }

    cout<<"Original Image dimension::"<<image.rows<<"x"<<image.cols<<endl;
    Size size(1920,1080);
    Mat small;
    resize(image,small,size);
    cout<<"Small Image dimension::"<<small.rows<<"x"<<small.cols<<endl;
    imwrite("Images/small.jpg",small);

    namedWindow("Display Image", WINDOW_AUTOSIZE );
    imshow("Display Image", small);
    waitKey(0);
    return 0;
}
