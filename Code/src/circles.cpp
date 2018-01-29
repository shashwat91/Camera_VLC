#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"

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

void detector(Mat input, vector< vector<int> > &returnMatrix)
{
	Mat blurred_image;
	Mat otsu;
	vector<vector<Point> > contours;
	Scalar red(255,0,0);
	size_t count=0;
	unsigned long totalArea=0;
	Point2f center;
	int rMax=0;
	float r;

	blur(input, blurred_image, Size(100, 100));
	threshold(blurred_image, otsu, 0, 255,  CV_THRESH_OTSU);
	findContours(otsu, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);


	count = contours.size();
	vector<Moments> mu(count);
	vector<Point2f> mc(count);

	for( size_t i=0; i < count; i++)
	{
		minEnclosingCircle( contours[i], center, r);
		if(r > rMax)
			rMax=r;
		mu[i] = moments( contours[i], false );
		if(mu[i].m00 == 0)
			continue;
		mc[i] = Point2f( mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00 );
		totalArea+=mu[i].m00;
	}


	center.x=0; center.y=0;
	for(size_t i=0; i < count; ++i)
	{
		float per = mu[i].m00 / totalArea;
		center.x += mc[i].x * per;
		center.y += mc[i].y * per;
	}

	if(count!=0)
	{
		// circle(input, center, rMax,red, 1);
		returnMatrix.resize(1);
		returnMatrix[0].resize(3);
		returnMatrix[0][0] = center.x;
		returnMatrix[0][1] = center.y;
		returnMatrix[0][2] = rMax;
	}
}

void saveCircle(Mat input, vector< vector<int> > returnMatrix, String Image_path)
{
	Image_path.erase(Image_path.find('.'));
	Image_path += "_circle.jpg";
	Rect roi;
	roi.x = returnMatrix[0][0] - returnMatrix[0][2];
	roi.y = returnMatrix[0][1] - returnMatrix[0][2] + 40;
	roi.width = 2*returnMatrix[0][2];
	roi.height = 2*returnMatrix[0][2] - 70;

	// Scalar red(255,0,0);
	// rectangle(input, roi, red, 1);
	Mat crop = input(roi);
	imwrite(Image_path, crop);
	// return crop;
}

int main( int argc, char** argv )
{
/*	namedWindow("Image",1);
	Mat image = imread( argv[1], IMREAD_GRAYSCALE);
	vector< vector<int> > Matrix;

	detector(image, Matrix);

	cout<<"Circle dimensions\n";
	cout<<"Center :: "<< Matrix[0][0] << " , " << Matrix[0][1] << endl;
	cout<<"Radius :: "<< Matrix[0][2] << endl;

	Mat circle = saveCircle(image, Matrix);

	imwrite("_circle.jpg", circle);
	// imshow("Image", image);
	// waitKey(0);
	return 0;*/

	String Image_path;
	for(int i=0;i<=7;++i)
	{
		for(int j=1;j<=55;++j)
		{
			Image_path = "data_images/";
			Image_path += patch::to_string(i);
			Image_path += "/";
			Image_path += patch::to_string(i);
			Image_path += "_";
			Image_path += patch::to_string(j);
			Image_path += "_small.jpg";
			cout<<"Image :: "<<Image_path<<endl;
			
			Mat image = imread( Image_path, IMREAD_GRAYSCALE);
			if ( !image.data )
			{
				cout<<"No image data\n";
				continue;
			}
			vector< vector<int> > Matrix;
			detector(image, Matrix);
			saveCircle(image, Matrix, Image_path);
		}
	}
	return 0;
}