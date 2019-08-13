#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

bool compareContourAreas ( std::vector<cv::Point> contour1, std::vector<cv::Point> contour2 ) {
    double i = fabs( contourArea(cv::Mat(contour1)) );
    double j = fabs( contourArea(cv::Mat(contour2)) );
    return ( i > j );
}

int main( int argc, char** argv )
{
	if(argc != 2){
		cerr<<"Pass the source file as a command line argument."<<endl;
		exit(-1);
	}

	Mat image = imread(argv[1]);
	int largest_area = 0;
	int largest_contour_index = 0;
	Mat gray, thresh, canny;


    cv::cvtColor(image, gray, COLOR_BGR2GRAY);	//convert color image to grayscale image

    double otsu;
    otsu = threshold(gray, thresh, 50, 255, THRESH_BINARY | THRESH_OTSU);	//apply threshold and return the threshold value to get otsu thresh value

    Canny(gray, canny, otsu, otsu*2, 5, true);	//apply canny to detect the box boundary of the code



    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours( canny, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );	//find contours on detected edges to find the box


    // sort contours
    std::sort(contours.begin(), contours.end(), compareContourAreas);	//assuming the box boundary will be greater than others


    vector<vector<Point> > contours_poly(1);
    //Traverse through all contours to find out the box
    for(size_t i = 0; i < contours.size(); i++){
    	//find the box corners of the contours
    	double peri = arcLength(contours[i], true);
    	approxPolyDP(Mat(contours[i]), contours_poly[0], 0.05*peri, true);
    	if(contours_poly[0].size() == 4)	//check if the contour has 4 curve/polygons
    	{
			largest_contour_index = i;
			break;
		}
    	else
    	{
			cout<<"No 4 point found!"<<endl;
		}

    }

    Mat draw = Mat::zeros(canny.size(), CV_8UC3);
    drawContours(draw, contours, 0, Scalar(255, 255, 255), 0, 8);

    Mat final(256, 256, CV_8UC1);

    Rect boundRect = boundingRect(contours_poly[0]);
	double ar = boundRect.width/(float)boundRect.height;
//	drawContours(draw, contours_poly, 0, Scalar(255, 255, 255), 0, 8);

	if(ar > 0.95 && ar < 1.05)	//we can assume that the image is not single planar image since the black box is a square shaped
	{
		cout<<"Don't need to correct the perspective."<<endl;
		return 0;
	}

	//TODO multipoint perspective correction.


    imshow("Region of Interest", draw);
    imshow("ROI Image", image(boundRect));
    waitKey();
    return 0;
}
;
