#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
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

using namespace cv;
using namespace std;

void process_image(string path)
{
    Mat image, small;

    image = imread( path, IMREAD_GRAYSCALE );
    if ( !image.data )
    {
        cout<<"No image data\n";
        return;
    }
    path.erase(path.find('.'));

    // transpose(image, image);
    // flip(image, image,1);

    Size size(1920,1080);
    resize(image,small,size);
    imwrite(path+"_small.jpg",small);
}

int main( int argc, char** argv )
{
    String Image_path;

    for(int i=0; i<=0; ++i)
    {
        for(int j=1; j<=55; ++j)
        {
            Image_path = "data_images/";
            Image_path += patch::to_string(i);
            Image_path += "/";
            Image_path += patch::to_string(i);
            Image_path += "_";
            Image_path += patch::to_string(j);
            Image_path += ".jpg";
            cout<<"Image :: "<<Image_path<<endl;
            process_image(Image_path);
        }
    }
    return 0;
}
