//
//  Source.cpp
//  CSC249 Final Project
//
//  Luis Nova
//  Yufei Du
//
#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;
using namespace std;

Mat src_gray;
Mat dst, detected_edges, ELAImage, overlay;

int edgeThresh = 1;
int lowThreshold = 1;
int ratio = 3;
int kernel_size = 3;
char* window_name = "Edge Map";
bool show = false;

/**
* @function CannyThreshold
* @brief Trackbar callback - Canny thresholds input with a ratio 1:3
*/
//Cite: http://docs.opencv.org/2.4/doc/tutorials/imgproc/imgtrans/canny_detector/canny_detector.html
void CannyThreshold(int, void*)
{
	/// Reduce noise with a kernel 8x8
	blur(src_gray, detected_edges, Size(32, 32));

	/// Canny detector
	Canny(detected_edges, detected_edges, lowThreshold, lowThreshold*ratio, kernel_size);

	/// Using Canny's output as a mask, we display our result
	dst = Scalar::all(0);
	overlay.copyTo(dst, detected_edges);
}

int main(int argc, const char * argv[]) {
	cout << "OpenCV Version " << CV_VERSION << endl;

	Mat image, imageCompressed;
	//If there is only one argument, then read file from Images directory
	//and output to Images/Output directory
	//and display results
	String arg = argv[1];
	String filename, outputFile;
	if (argc == 2) {
		filename = "Images/" + arg;
		outputFile = "Images/Output/" + arg;
		show = true;
	}
	//Silent mode: when there are more than one arguments, read file directly from argument
	//and output to Output directory directly
	else {
		filename = arg;
		outputFile = "Output/" + arg;
	}
	//Set threshold for conversion to binary image
	int threshold = 45;
	//Read image
	image = imread(filename, CV_LOAD_IMAGE_UNCHANGED);
	if (image.empty()) {
		cout << "ERROR: Image Could Not Be Loaded" << endl;
	}

	else {
		//Cite for ELA algorithm: https://stackoverflow.com/questions/21464229/error-level-analysis-in-image
		
		//Quality level
		int quality = 95;
		vector<int> parameters;
		parameters.push_back(CV_IMWRITE_JPEG_QUALITY);
		parameters.push_back(quality);
		imwrite("Images/temp.jpg", image, parameters);
		//Store and re-read image to re-compress
		imageCompressed = imread("Images/temp.jpg", CV_LOAD_IMAGE_UNCHANGED);

		//ELAImage: matrix to store Error Level
		ELAImage = Mat::zeros(image.size(), CV_8UC3);
		//overlay: matrix to store thresholded binary error level
		overlay = Mat::zeros(image.size(), CV_8UC3);

		for (int row = 0; row < image.rows; row++) {

			const uchar* ptr_image = image.ptr<uchar>(row);
			const uchar* ptr_imageCompressed = imageCompressed.ptr<uchar>(row);
			uchar* ptr_elaImage = ELAImage.ptr<uchar>(row);
			uchar* ptr_overlay = overlay.ptr<uchar>(row);

			for (int column = 0; column < image.cols; column++) {
				//magnify values 7 times
				ptr_elaImage[0] = abs(ptr_image[0] - ptr_imageCompressed[0])*7;
				ptr_elaImage[1] = abs(ptr_image[1] - ptr_imageCompressed[1])*7;
				ptr_elaImage[2] = abs(ptr_image[2] - ptr_imageCompressed[2])*7;

				//If the sum of Error Levels are over threshold (45), set the corresponding pixel of overlay matrix to white (255,255,255)
				if (ptr_elaImage[0] + ptr_elaImage[1] + ptr_elaImage[2] >= threshold) {
					ptr_overlay[0] = 255;
					ptr_overlay[1] = 255;
					ptr_overlay[2] = 255;
					//If the program is not running in silent mode, print the error level
					if (show) {
						printf("%d %d %d\n", ptr_elaImage[0], ptr_elaImage[1], ptr_elaImage[2]);
					}
				}
				//iterate
				ptr_image += 3;
				ptr_imageCompressed += 3;
				ptr_elaImage += 3;
				ptr_overlay += 3;
			}
		}

		imwrite("Images/temp2.jpg", ELAImage);
		
		//Canny detection
		//Cite: http://docs.opencv.org/2.4/doc/tutorials/imgproc/imgtrans/canny_detector/canny_detector.html
		cvtColor(overlay, src_gray, CV_BGR2GRAY);
		CannyThreshold(0, 0);

		//Find contours based on Canny results
		//Cite: http://docs.opencv.org/2.4/doc/tutorials/imgproc/shapedescriptors/find_contours/find_contours.html
		vector<vector<Point>> contours;
		vector<Vec4i> hierarchy;

		findContours(detected_edges, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
		//Mat drawing = Mat::zeros(dst.size(), CV_8UC3);
		for (int i = 0; i< contours.size(); i++)
		{
			Scalar color = Scalar(0, 255, 255);
			drawContours(image, contours, i, color, 2, 8, hierarchy, 0, Point());
		}

		/// Show in a window
		if (show) {
			namedWindow("Contours", CV_WINDOW_AUTOSIZE);
			imshow("Contours", image);
			waitKey(0);
		}
		imwrite(outputFile, image);
	}

	return 0;
}

