#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>


using namespace cv;
using namespace std;
int pre_width = 35;

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

    // cout<<"Peaks:: ";
    // for (int i=0; i<=peaks.size()-1; ++i) 
    //     cout << peaks[i] << " ";

    // cout<<"\nFalls:: ";
    // for (int i=0; i<=falls.size()-1; ++i)
    //     cout << falls[i] << " ";
    // cout<<"\n";

    if(peaks.size() > falls.size())
        peaks.erase(peaks.begin() + peaks.size() - 1);      //Removes the last peak if no fall is associated to it.

    // cout<<"Peaks:: ";
    for (int i=0; i<=peaks.size()-1; ++i) 
        cout <<setw(3)<< i <<" :: " << setw(4) << peaks[i] << " , " << setw(4) << falls[i]<<" : "<<setw(4)<<falls[i]-peaks[i]<<endl;

    // cout<<"\nFalls:: ";
    // for (int i=0; i<=falls.size()-1; ++i)
    //     cout << falls[i] << " ";
    // cout<<"\n";

    int j = 1;
    int firstPreamble=0;
    int secondPreamble=0;

    for (unsigned int i = 0; i < peaks.size(); i++)
    {
        dif = falls[i] - peaks[i];
        //cout<<"diff :: "<<dif;
        if ( dif >= pre_width ) //2.5*16=40
        { //freq = 3000 -> 2.5*8
          //  cout<<"\tPreamble detected";
            //WTF!!!  ---  why such a way to get preambles????
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

    cout << "\nfirst " << firstPreamble << " second " << secondPreamble<< "\n" ;

    j = 1;
    int addZero = 0;

    while(true)
    {
        dif = peaks[firstPreamble +j] - falls[firstPreamble + j -1];
        cout<<"diff1:"<<dif<<":"<<j<<"@";
        if(dif > 1.2 * 16 ) // 1.3*16=20.8
        { //freq = 3000 -> 2.5*8
            if(j == 1 || (firstPreamble + j == secondPreamble))
            {
                cout<<"0";
                detectedBits.push_back(0);
            }
            else
            {
                cout << "0";
                cout << "/0";
                detectedBits.push_back(0);
                detectedBits.push_back(0);
            }
        }
        else
        {
            if((j != 1 && firstPreamble + j != secondPreamble) || addZero == 1)
            {
                cout << "0";
                detectedBits.push_back(0);
            }
        }
        if (firstPreamble + j == secondPreamble)
            break;

        dif = falls[firstPreamble +j] - peaks[firstPreamble + j];
        cout<<"diff2:"<<dif<<":"<<j<<"@";
        if(dif > 1.3 * 16 ) // 1.5*16=24
        { //freq = 3000 -> 2.5*8
            detectedBits.push_back(1);
            detectedBits.push_back(1);
            if(firstPreamble + j +1 == secondPreamble)
                addZero = 1;
            cout << "1";
            cout << "/1";
        }
        else
        {
            detectedBits.push_back(1);
            cout<<"1";
        }
        j++;
    }
    cout<<endl;
}


int main( int argc, char** argv )
{
    Mat image;
    ofstream fout,fout2;
    int count=0;
    
    if( argc < 2)
    {
     cout <<" Usage: display_image ImageToLoadAndDisplay" << endl;
     return -1;
    }

    image = imread( argv[1], 0);
    if(argv[2]!=NULL)
        pre_width = atoi(argv[2]);

    // fout.open("linePixelCount.csv");
    // fout2.open("bits.csv");
 
    if ( !image.data )
    {
        cout<<"No image data\n";
        return -1;
    }

    cout<<"Image dimension::"<<image.rows<<"x"<<image.cols<<endl;
    
    std::vector<int> array(image.rows*image.cols);
    if (image.isContinuous())
        array.assign(image.datastart, image.dataend);
    
    for(unsigned int i=0; i<array.size()-1; ++i)
    {
        if(array[i] > 125)
            array[i] = 255;
        else
            array[i] = 0;
    }

    vector<int> decodedBits;
    decodeBits(array,decodedBits);

    if(decodedBits.size() == 0)
    {
        cout<<"Image not decoded properly\n";
        return -1;
    }

    for(unsigned int i=0; i<decodedBits.size(); ++i)
        cout<<decodedBits[i];
    cout<<endl;

    /*cout<<"Image dimension::"<<image.rows<<"x"<<image.cols<<endl;
    transpose(image, image);
    flip(image, image,1);
    cout<<"Image dimension::"<<image.rows<<"x"<<image.cols<<endl;
    imwrite("rotated.jpg",image);*/

    // fout.close();
    // fout2.close();
    // namedWindow("Display Image", WINDOW_AUTOSIZE );
    // imshow("Display Image", image);
    // waitKey(0);
    return 0;
}
