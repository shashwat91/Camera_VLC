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

extern "C" {

float pow2roundup (int x);
int roundUp(int numToRound, int multiple);
Mat clahe(Mat gray_image);
Mat histogramEqualization(Mat input);
void detector(Mat input, vector< vector<int> > &returnMatrix);
Mat adaptiveThreshold(Mat input);
Mat blur(Mat input);
void getMiddlePixels(Mat input, int centerRadius[3], vector<int>& pixels);
void centerToZero(vector<int>& inputPixels, vector<int>& outputPixels);
void detectBits(vector<int>& inputPixels, vector<int>& detectedBits);
double averagePixelsForLightCheck (Mat input);

double averageBlobPixelsForLight(Mat input, int column, int row ,int radius);

void avoidBlobCenterOffset(vector<int>& inputPixels, vector<int>& offsetBlobStart);
void getCorrectedPixelsCenterOffset(Mat input, int centerRadius[3], vector<int>& offsetBlobStart, vector<int>& correctPixels );

int avoidBlobOffset(Mat input, int centerRadius[3]);
void getCorrectedPixelsOffset(Mat input, int centerRadius[3], int offset, vector<int>& correctPixels );


void decodeBits(vector<int>& inputPixels, vector<int>& detectedBits);
ofstream fout;
void start_log();

JNIEXPORT jintArray JNICALL Java_com_example_testhiddenpreview_Decoder_decode(JNIEnv* env, jobject thiz, jint width, jint height,jbyteArray NV21FrameData, jint centerRow, jint centerColumn, jint blobRadius ){

	start_log();
	jbyte * pNV21FrameData = env->GetByteArrayElements(NV21FrameData, 0);
	string file = "/storage/emulated/0/blobtest/";
	//imwrite(file + "first_image.jpg",pNV21FrameData);
	// get image
	Mat gray_image(height, width, CV_8UC1, (unsigned char *) pNV21FrameData);
	imwrite(file + "rec_image.jpg",gray_image);
	clock_t begin, end;
	double time_spent;

	vector<vector<int> > returnMatrix(5, vector<int>(3)); // contains center positions and radii of blobs
	detector(gray_image, returnMatrix);

	int centerRadius[3];

	centerRadius[0] = returnMatrix[0][0];
	centerRadius[1] = returnMatrix[0][1];
	centerRadius[2] = ceil(returnMatrix[0][2]*1.5);

	begin = clock();
	// enhance image contrast
	Mat adaptive_equalized = clahe(gray_image); //the image that will be processed

	end = clock();

	time_spent = (double) (end - begin) / CLOCKS_PER_SEC;


	begin = clock();

	// blur image
	Mat blurred = blur(adaptive_equalized);

	end = clock();

	time_spent = (double) (end - begin) / CLOCKS_PER_SEC;

	//LOGE("Blur =  %.5f . \n", time_spent * 1000);


	adaptive_equalized.release();

	begin = clock();
	//OTSU image (converts gray to binary)
	Mat adaptive_threshold = adaptiveThreshold(blurred);

	//namedWindow( "Display window", WINDOW_AUTOSIZE );// Create a window for display.
	//imshow( "Display window", adaptive_threshold );

	imwrite(file + "greyImage.jpg", gray_image);
	imwrite(file + "otsuImage.jpg", adaptive_threshold);

	end = clock();

	time_spent = (double) (end - begin) / CLOCKS_PER_SEC;

	begin = clock();

	begin = clock();

	int offset = avoidBlobOffset(adaptive_threshold, centerRadius);

	//cout << "offset " << offsetBlobStart[0] << "\n";

	vector<int> correctedPixels;

	getCorrectedPixelsOffset(adaptive_threshold, centerRadius, offset, correctedPixels);
	imwrite(file + "line_image.jpg", correctedPixels);

	//getCorrectedPixelsCenterOffset(adaptive_threshold, centerRadius, offsetBlobStart, correctedPixels );


	//adaptive_threshold.release();


	vector<int> detectedBits;


	decodeBits(correctedPixels, detectedBits);

	//detectedBits.push_back(0);
	end = clock();


	time_spent = (double) (end - begin) / CLOCKS_PER_SEC;

	//LOGE("Decode =  %.5f . \n", time_spent * 1000);

	//prepare result
	jint fill[detectedBits.size()];

	for(int i=0; i<detectedBits.size(); i++){
		fill[i] = detectedBits[i];
	}



	jintArray result;
	result = env->NewIntArray(detectedBits.size());
	env->SetIntArrayRegion(result, 0, detectedBits.size(), fill);



	env->ReleaseByteArrayElements(NV21FrameData, pNV21FrameData, JNI_ABORT);

	//LOGE("DONE");
	//fout.close();
	return result;


}


void decodeBits(vector<int>& inputPixels, vector<int>& detectedBits){


	//find peaks and falls
	std::vector<int> peaks;
	std::vector<int> falls;

	int dif, nextDif;

	for (int i = 0; i < inputPixels.size()-1; i++){

		if(inputPixels[i] == 0 && inputPixels[i+1] == 255){
			peaks.push_back(i);
			//printf("On: %d \n", i);

		}
		else if(inputPixels[i] == 255 && inputPixels[i+1] == 0 && peaks.size() !=0){
			falls.push_back(i);
			//printf("Off: %d \n", i);
		}

	}


	if(peaks.size() > falls.size())
		peaks.erase(peaks.begin() + peaks.size() - 1);

	int j = 1;
	int firstPreamble=0;
	int secondPreamble=0;

	for (int i = 0; i < peaks.size(); i++){

		dif = falls[i] - peaks[i];

		if ( dif >= 2.5 *16 ){ //freq = 3000 -> 2.5*8
			if( j == 1 ){
				if(i == 0)
					continue;
				firstPreamble = i;
				j++;
			}
			else{

				//nextDif = falls[i+1] - peaks[i+1];
				//if(nextDif < 20 ){

					//if(i == firstPreamble + 1)
						//continue;



					secondPreamble = i;
					break;
				//}
			}

		}

	}

	if(secondPreamble == 0)
		return;

	//cout << "first " << firstPreamble << " second " << secondPreamble<< "\n" ;
	fout<< "first " << firstPreamble << " second " << secondPreamble<< "\n" ;

	j = 1;
	int addZero = 0;


	while(true){

		dif = peaks[firstPreamble +j] - falls[firstPreamble + j -1];

		if(dif > 1.3 * 16 ){ //freq = 3000 -> 2.5*8
			if(j == 1 || firstPreamble + j == secondPreamble){
				//cout << "0" << "\n";
				fout << "0" << "\n";
				detectedBits.push_back(0);
			}
			else{
				detectedBits.push_back(0);
				detectedBits.push_back(0);
				fout<<"0/0"<<"\n";
				//cout << "0" << "\n";
				//cout << "/0" << "\n";
			}
		}
		else{
			if((j != 1 && firstPreamble + j != secondPreamble) || addZero == 1){
				//cout << "0" << "\n";
				fout << "0" << "\n";
				detectedBits.push_back(0);
			}
		}


		if (firstPreamble + j == secondPreamble)
			break;

		dif = falls[firstPreamble +j] - peaks[firstPreamble + j];

		if(dif > 1.5 * 16 ){ //freq = 3000 -> 2.5*8
			detectedBits.push_back(1);
			detectedBits.push_back(1);
			if(firstPreamble + j +1 == secondPreamble)
				addZero = 1;
			fout<<"1/1"<<"\n";
			//cout << "1" << "\n";
			//cout << "/1" << "\n";
		}
		else{
			fout << "1" << "\n";
			//cout << "1" << "\n";
			detectedBits.push_back(1);

		}


		j++;

	}
}

void getCorrectedPixelsCenterOffset(Mat input, int centerRadius[3], vector<int>& offsetBlobStart, vector<int>& correctPixels ){

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

	for(int i = top ; i< bottom ; i++){

		if( (i >= offsetBlobStart[1] + top ) && (i <= offsetBlobStart[1] + top + offsetBlobStart[0])  ){
			correctPixels.push_back(input.at<unsigned char>(i, centerRadius[0] - offsetBlobStart[0]) );
		}
		else{
			correctPixels.push_back(input.at<unsigned char>(i, centerRadius[0]));
		}

	}

}



void avoidBlobCenterOffset(vector<int>& inputPixels, vector<int>& offsetBlobStart){


	//find peaks and falls
	std::vector<int> peaks;
	std::vector<int> falls;

	for (int i = 0; i < inputPixels.size()-1; i++){

		if(inputPixels[i] == 0 && inputPixels[i+1] == 255){
			peaks.push_back(i);
			//printf("On: %d \n", i);

		}
		else if(inputPixels[i] == 255 && inputPixels[i+1] == 0 && peaks.size() !=0){
			falls.push_back(i);
			//printf("Off: %d \n", i);
		}

	}

	int blobStart = 0;
	int blobEnd = 0;

	for (int i = 0; i < falls.size(); i++){

		int dif = falls[i] - peaks[i];

		if (dif > 56){ //freq = 3000 -> dif > 28

			if(blobStart == 0){
				blobStart = peaks[i];
			}

		}
		else if (dif < 40 && blobStart != 0 ){
			blobEnd = falls[i-1];
			break;
		}

	}

	offsetBlobStart.push_back(blobEnd - blobStart);
	offsetBlobStart.push_back(blobStart);

}

int avoidBlobOffset(Mat input, int centerRadius[3]){

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

	while(1){

		vector<int> pixels;

		for(int i = top ; i< bottom ; i++){

			pixels.push_back(input.at<unsigned char>(i, column));
		}


		//find peaks and falls
		std::vector<int> peaks;
		std::vector<int> falls;

		for (int i = 0; i < pixels.size()-1; i++){

			if(pixels[i] == 0 && pixels[i+1] == 255){
				peaks.push_back(i);
				//printf("On: %d \n", i);
				//fflush(stdout);

			}
			else if(pixels[i] == 255 && pixels[i+1] == 0 && peaks.size() !=0){
				falls.push_back(i);
				//printf("Off: %d \n", i);
				//fflush(stdout);
			}
		}

		int noBlob = 0;

		for (int i = 0; i < falls.size(); i++){

			int dif = falls[i] - peaks[i];

			if (dif > 3 * 16){ //freq = 3000 -> 3*8
				column = column - 1;
				noBlob = 1;
			}
		}

		if(noBlob == 0)
			break;
	}

	return centerRadius[0] - column;
}
void getCorrectedPixelsOffset(Mat input, int centerRadius[3], int offset, vector<int>& correctPixels ){

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

	for(int i = top ; i< bottom ; i++){

		correctPixels.push_back(input.at<unsigned char>(i, centerRadius[0] - offset) );

	}
}



int roundUp(int numToRound, int multiple)
{
   return ((numToRound + multiple - 1) / multiple) * multiple;
}

float pow2roundup (int x)
{
    if (x < 0)
        return 0;
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x+1;
}

}


void detectBits(vector<int>& inputPixels, vector<int>& detectedBits){

	//find peaks and falls
	std::vector<int> peaks;
	std::vector<int> falls;

	for (int i = 0; i < inputPixels.size()-1; i++){

		if(inputPixels[i] == 0 && inputPixels[i+1] == 1){
			peaks.push_back(i);

		}
		else if(inputPixels[i] == 1 && inputPixels[i+1] == 0 && peaks.size() !=0){
			falls.push_back(i);
		}

	}


	if(peaks.size() == 0 || falls.size() == 0){

		return;
	}

	//remove very short pixel preambles
	for (int i = 0; i < falls.size(); i++) {
		//printf("Dif = %d \n", falls[i]- peaks[i]);
		if (falls[i] - peaks[i] < 4) {
			falls.erase(falls.begin() + i);
			peaks.erase(peaks.begin() + i);
			//printf("Erased \n");
			//update index cause item was removed
			i = i - 1;
			if (i < 0)
				i = 0;
		}

	}

	int preambleDif = 0;

	//find preambles
	std::vector<int> preambles;
	for (int i = 0; i < peaks.size() - 2; i++) {
		//printf("%d \n", peaks[i]);

		int firstdif = peaks[i + 1] - peaks[i];

		int seconddif = peaks[i + 2] - peaks[i + 1];

		//if two preambles
		if (seconddif < 1.5 * firstdif) {

			//check if a whole packet is included
			if (i + 4 <= peaks.size() - 1) {

				int thirddif = peaks[i + 3] - peaks[i + 2];
				//avoid confusion with two on bits
				if (thirddif < firstdif)
					continue;

				int fourthdif = peaks[i + 4] - peaks[i + 3];
				if (fourthdif < seconddif)
					continue;

				LOGE("preambe = %d", i);
				preambles.push_back(i);

				//calculate average of preample ON size
				preambleDif = ((falls[i] - peaks[i])
						+ (falls[i + 1] - peaks[i + 1])) / 2;

				//cout << preambleDif << std::endl;

			}
			i = i + 3;
		}
	}



	LOGE("preambleDif = %d", preambleDif);


	for (int i = 0; i < preambles.size(); i++){

		int preamble = preambles[i];
		int firstBitDif = peaks[preamble+3] - peaks[preamble+2];
		int secondBitDif = peaks[preamble+4] - peaks[preamble+3];

		int onPeriodFirstBit = falls[preamble+2] - peaks[preamble+2];
		int onPeriodSecondBit = falls[preamble+3] - peaks[preamble+3];


		//LOGE("first = %d", onPeriodFirstBit);
		//LOGE("second = %d", onPeriodSecondBit);


		//cout << onPeriodFirstBit << std::endl;

		//cout << onPeriodSecondBit << std::endl;
		fout<<"first = "<<onPeriodFirstBit<<" second = "<<onPeriodSecondBit<<"\n";
		float factor = 1.5;

		if (onPeriodFirstBit > factor * preambleDif && onPeriodSecondBit > factor * preambleDif){
			detectedBits.push_back(1);
		}
		else if (onPeriodFirstBit > factor * preambleDif && onPeriodSecondBit < factor * preambleDif){
			detectedBits.push_back(2);
		}
		else if (onPeriodFirstBit < factor * preambleDif && onPeriodSecondBit > factor * preambleDif){
			detectedBits.push_back(3);
		}
		else if (onPeriodFirstBit < factor * preambleDif && onPeriodFirstBit < factor * preambleDif){
			detectedBits.push_back(4);
		}

		/*
		if (onPeriodFirstBit > firstBitDif/2 && onPeriodSecondBit > secondBitDif/2){
			printf("Found 1\n");
		}
		else if (onPeriodFirstBit > firstBitDif/2 && onPeriodSecondBit < secondBitDif/2){
			printf("Found 2\n");
		}
		else if (onPeriodFirstBit < firstBitDif/2 && onPeriodSecondBit > secondBitDif/2){
			printf("Found 3\n");
		}
		else if (onPeriodFirstBit < firstBitDif/2 && onPeriodSecondBit < secondBitDif/2){
			printf("Found 4\n");
		}
		*/


	}

}


void centerToZero(vector<int>& inputPixels, vector<int>& outputPixels){

	for(int i = 0 ; i< inputPixels.size(); i++){

		if(inputPixels[i] == 255)
			outputPixels.push_back(1);
		else
			outputPixels.push_back(0);

	}
}


double averagePixelsForLightCheck (Mat input){

	std::vector<double> averageVertical;

	for(int i = 0 ; i< input.rows ; i++){

		int sum = 0;
		for(int j = 0 ; j< input.cols ; j++){
			sum = sum + input.at<unsigned char>(i, j);
		}

		averageVertical.push_back(sum/input.cols);

	}

	int sum = 0;
	for(int i = 0 ; i< input.rows ; i++){
		sum = sum + averageVertical[i];
	}

	double average = sum / input.rows;

	LOGE("Average %d \n", average);


	return average;
}

double averageBlobPixelsForLight(Mat input, int column, int row ,int radius){

	std::vector<double> averageVertical;


	int top = row - radius;
	int bottom = row + radius;

	if(top < 0){
		top = 0;
	}
	if(bottom > input.rows){
		bottom = input.rows;
	}

	//printf("Start: %d End: %d\n", top, bottom);
	int sum = 0;

	for(int i = top ; i< bottom ; i++){

		sum = sum + input.at<unsigned char>(i, column);

	}


	double average = sum / radius;

	return average;

}



void getMiddlePixels(Mat input, int centerRadius[3], vector<int>& pixels){

	float factor = 1;
	int newRadius = ceil(centerRadius[2] * factor);
	int offset = centerRadius[2];

	int top = centerRadius[1] - newRadius;
	int bottom = centerRadius[1] + newRadius;

	if(top < 0){
		top = 0;
	}
	if(bottom > input.rows){
		bottom = input.rows;
	}

	//printf("Start: %d End: %d\n", top, bottom);

	for(int i = top ; i< bottom ; i++){

		pixels.push_back(input.at<unsigned char>(i, centerRadius[0]));

	}

}

Mat adaptiveThreshold(Mat input){

	Mat adaptive;

	adaptiveThreshold(input, adaptive, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 105, 2);

	return adaptive;

}

Mat blur(Mat input){

	//Blur image
	Mat blurred_image;
	blur(input, blurred_image, Size(3, 3));

	return blurred_image;

}

Mat histogramEqualization(Mat input){

	Mat equalized;
	equalizeHist(input, equalized);

	return equalized;

}


Mat clahe(Mat input){

	Ptr<CLAHE> clahe = createCLAHE();
	clahe->setClipLimit(10);
	clahe->setTilesGridSize(Size(8,8));

	Mat adaptive_equalized;
	clahe->apply(input,adaptive_equalized);


	return adaptive_equalized;
}


void detector(Mat input, vector< vector<int> > &returnMatrix){

  Mat blurred_image;
  blur(input, blurred_image, Size(100, 100));

  /*Mat erode_img;
  Mat element = getStructuringElement(MORPH_RECT, Size(15, 15));
  erode(blurred_image, erode_img, element);*/

  Mat otsu;
  threshold(blurred_image, otsu, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);

  Mat cimg = input;

  vector<vector<Point> > contours;
  vector<Point2i> center;
  vector<int> radius;

  findContours(otsu.clone(), contours, CV_RETR_TREE, CV_CHAIN_APPROX_NONE);

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
  cv::Scalar red(255,255,255);

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

void start_log()
{
	fout.open("/storage/emulated/0/blobtest/text_output.txt", ofstream::out | ofstream::app);
	if (fout.is_open())
	{
		fout << "Start::\n";
		cout << "Output operation successfully performed\n";
	}
	else
		cout << "Error opening file";
}
