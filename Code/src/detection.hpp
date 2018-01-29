#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"

using namespace cv;

void detector(Mat input, vector< vector<int> > &returnMatrix);

Mat clahe(Mat input);

Mat adaptiveThreshold(Mat input);

int avoidBlobOffset(Mat input, int centerRadius[3]);

void getCorrectedPixelsOffset(Mat input, int centerRadius[3], int offset, vector<int>& correctPixels );

void decodeBits(vector<int>& inputPixels, vector<int>& detectedBits);

void decodeBits_encoding1();

void decodeBits_encoding2(vector<int>& inputPixels, vector<int>& detectedBits);

void decodeBits_MAE(vector<int>& inputPixels, vector<int>& detectedBits);