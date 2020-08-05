//
//  main.cpp
//  MyProjectOpenCV
//
//  Created by Erik Nuroyan on 6/20/20.
//  Copyright � 2020 Erik Nuroyan. All rights reserved.
//
#include <iostream>
#include <opencv2/opencv.hpp>
#include <climits>
#include <chrono>
#include "ImageGrid.h"

int main(int argc, const char * argv[]) {
	using namespace cv;
	std::string image_path = samples::findFile("img15.jpg"); //forest_bgr.jpg
	Mat forest_bgr = imread(image_path, IMREAD_COLOR);
	ImageGrid i_g = ImageGrid(forest_bgr);
	auto start = std::chrono::high_resolution_clock::now();
	const int size = 50;
	i_g.resize(size);
	auto end = std::chrono::high_resolution_clock::now();
	auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	std::cout << milliseconds / (static_cast<float>(size) * 1e3) << '\n';

	// imshow("Resized", i_g.produce_image());
	// waitKey(0);
	return 0;
}

