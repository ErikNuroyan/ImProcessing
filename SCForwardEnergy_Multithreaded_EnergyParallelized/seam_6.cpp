//
//  main.cpp
//  MyProjectOpenCV
//
//  Created by Erik Nuroyan on 6/20/20.
//  Copyright � 2020 Erik Nuroyan. All rights reserved.
//
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <climits>
#include <chrono>
#include "image_grid.h"

int main(int argc, const char * argv[]) {
	using namespace cv;
	//Doing all by hand

	//ImageGrid i_g = ImageGrid(img);
	//i_g.resize(-200);
	auto start = std::chrono::high_resolution_clock::now();
	std::string image_path = samples::findFile("forest_bgr.jpg"); //C:/Users/Erik/Desktop/haha/openCVProject1/openCVProject1/Test Set/8345_489_img80.jpg
	Mat img = imread(image_path, IMREAD_COLOR);
	Mat upper = Mat();// img(Rect(0, 0, img.cols, 1649));
	Mat cropped = Mat();// img(Rect(0, 1649, img.cols, 300));
	Mat lower = Mat();// img(Rect(0, 1949, img.cols, img.rows - 1949));
	ImageGrid i_g = ImageGrid(img);
	//imwrite("azganun.jpg", i_g.produce_image());
	i_g.resize(-300, upper, lower);
	auto end = std::chrono::high_resolution_clock::now();
	auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	std::cout << milliseconds / (static_cast<float>(1) * 1e3) << '\n';

	//VideoWriter output = VideoWriter(("E:/Test Set Videos/8345_489_img80.avi"), VideoWriter::fourcc('M', 'J', 'P', 'G'), 10, Size(img.cols, img.rows), true);
	//i_g.resize(-75, upper, lower);

	//Getting the image dataset
	//Cropping the image 
	//Resizing the image (for reduction use negative values and positive ones for expansion)
	//Producing the image output
	//Mat img, upper, cropped, lower;
	//std::vector<std::string> paths;
	//VideoWriter output;
	//cv::glob("C:/Users/Erik/Desktop/haha/openCVProject1/openCVProject1/Test Set/*.jpg", paths);
	//for (String &s : paths) {
	//	img = imread(s, IMREAD_COLOR);
	//	s = s.substr(s.find("\\") + 1, s.length());
	//	int start, height, size;
	//	int first_underscore = -1;
	//	for (int i = 0; i < s.length(); i++) {
	//		if (s[i] == '_' && (first_underscore == -1)) {
	//			start = std::stoi(s.substr(0, i));
	//			first_underscore = i;
	//		}
	//		else if (s[i] == '_' && (first_underscore != -1)) {
	//			height = std::stoi(s.substr(first_underscore + 1, i));
	//			size = -(height / 4);
	//		}
	//	}
	//	s = s.substr(0, s.length() - 4);

	//	upper = img(Rect(0, 0, img.cols, start-1));
	//	cropped = img(Rect(0, start-1, img.cols, height));
	//	lower = img(Rect(0, start-1+height, img.cols, img.rows-(start - 1 + height)));
	//	ImageGrid i_g = ImageGrid(cropped);
	//	i_g.resize(size, upper, lower);
	//}

	//auto start = std::chrono::high_resolution_clock::now();
	//auto end = std::chrono::high_resolution_clock::now();
	//auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	//std::cout << milliseconds / (static_cast<float>(size) * 1e3) << '\n';
	//cv::waitKey(0);
	system("pause");

	return 0;
}


