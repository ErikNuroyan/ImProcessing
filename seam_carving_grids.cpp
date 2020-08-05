//
//  main.cpp
//  MyProjectOpenCV
//
//  Created by Erik Nuroyan on 6/20/20.
//  Copyright © 2020 Erik Nuroyan. All rights reserved.
//
#include <iostream>
#include <opencv2/opencv.hpp>
#include <climits>
#include <chrono>
#include "ImageGrid.h"

int main(int argc, const char * argv[]) {
	using namespace cv;
	std::string image_path = samples::findFile("forest_bgr.jpg"); //forest_bgr.jpg
	Mat forest_bgr = imread(image_path, IMREAD_COLOR);
	ImageGrid i_g = ImageGrid(forest_bgr);
	i_g.resize(150);
	imshow("Resized", i_g.produce_image());
	waitKey(0);
	return 0;
}

