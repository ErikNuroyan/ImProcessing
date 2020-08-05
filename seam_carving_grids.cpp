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
#include <thread>
cv::Mat gradient_n(const cv::Mat & img);
int * energyMinSeam(const cv::Mat & grads);
cv::Mat resize(const int * ptr, const cv::Mat & img);

//inline unsigned char get_coord(const cv::Mat & img, int i, int j) {
//	return ((i >= 0 && i < img.rows) && (j >= 0 && j < img.cols)) ? img.at<unsigned char>(i, j) : 0;
//}
//inline int cost_Cmid(const cv::Mat & img, const int i, const int j)
//{
//	return abs((int)get_coord(img, i + 1, j)-(int)get_coord(img, i - 1, j));	
//}

int main(int argc, const char * argv[]) {
	using namespace cv;

	std::string image_path = samples::findFile("img15.jpg"); //forest_bgr.jpg
	Mat forest_bgr = imread(image_path, IMREAD_COLOR);
	//imshow("Image", forest_bgr);
	//Mat resized;
	//resize(forest_bgr, resized,Size(10,10),INTER_CUBIC);
	//imshow("Resized", resized);

	
	ImageGrid i_g = ImageGrid(forest_bgr);
	//Mat res = i_g.produce_image();
	//std::cout << res.rows << "   " << res.cols << std::endl;
	auto t_start = std::chrono::high_resolution_clock::now();
	for(int i=0;i<25;i++){
		i_g.resize(1);
	}
	//auto t_start = std::chrono::high_resolution_clock::now();
	//i_g.print_grid();
	auto t_end = std::chrono::high_resolution_clock::now();
	std::cout << std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start).count()/static_cast<float>(25 * 1e6) << std::endl;
	//std::cout << forest_bgr.at<Vec3b>(0, 0);

	//int n = 200;                     //Enter the number of Pixels to be deleted
	//int * minSeam = nullptr;
	//Mat result = forest_bgr;
	//Mat grad;
	//
	//for (int i = 0; i < n; i++) {
	//	minSeam = energyMinSeam(result);
	//	result = resize(minSeam, result);
	//}
	//imshow("Resized", result);
	//imshow("Retrieved", i_g.produce_image());
	// waitKey(0);
	//delete minSeam;
	system("pause");
	return 0;
}


//int * energyMinSeam(const cv::Mat & img) {
//	using namespace cv;
//
//  //Calculating the minimum seam without extracting the result
//  //Result should be extracted by storing the x coordinates
//  //which were the minimum and using that vector which has
//  //length=grads.cols to extract the minimum seam
//
//	int * en_data = new int[img.rows*img.cols];
//	int * dir_data = new int[img.rows*img.cols];
//	int rows = img.rows;
//	int cols = img.cols;
//	const int step = img.cols;
//
//	Mat gray;
//	cvtColor(img, gray, COLOR_BGR2GRAY);
//
//	int * min_seam = new int[cols];   //Will keep indices of rows which comprise the horizontal minimum seam
//
//	int c_mid=0;
//	int c_down=0;
//	int c_up=0;
//	int temp=0;
//	for (int i = 0; i < rows; i++) {
//		c_mid = cost_Cmid(gray, i, 0);
//		c_down = c_mid + abs(temp - (int)get_coord(gray, i + 1, 0));
//		c_up = c_mid + abs(temp - (int)get_coord(gray, i - 1, 0));
//		en_data[i*cols] = std::min({ c_down, c_mid, c_up}); 
//		dir_data[i*cols] = 0;
//	}
//
//	for (int j = 1; j < cols; j++) {
//		for (int i = 0; i < rows; i++) {
//			c_mid = cost_Cmid(gray, i, j);
//			temp = (int)get_coord(gray, i, j - 1);
//			c_down = c_mid+ abs(temp- (int)get_coord(gray, i + 1, j));
//			c_up = c_mid+ abs(temp - (int)get_coord(gray, i - 1, j));
//
//			//Handling the case when the first row is under consideration
//			if (i == 0) {
//				int min = std::min(en_data[i*step + (j - 1)]+c_mid, en_data[(i + 1)*step + (j - 1)]+c_down);
//				en_data[i*step + j] = min;
//				if (min == en_data[i*step + (j - 1)] + c_mid) {
//					dir_data[i*step + j] = 2;    //Central  cell
//				}
//				else {
//					dir_data[i*step + j] = 3;    //The Bottom cell
//				}
//			}
//			//Handling the case when the last row is under consideration
//			else if (i == rows - 1) {
//
//				int min = std::min(en_data[i*step + (j - 1)]+c_mid, en_data[(i - 1)*step + (j - 1)] + c_up);
//				en_data[i*step + j] = min;
//				if (min == en_data[i*step + (j - 1)]+c_mid) {
//					dir_data[i*step + j] = 2;    //Central  cell
//				}
//				else {
//					dir_data[i*step + j] = 1;    //The Top cell
//				}
//			}
//			//Handling the case when rows are in between of the previous 2 ones
//			else {
//				int min = std::min(en_data[i*step + (j - 1)]+c_mid, en_data[(i - 1)*step + (j - 1)]+c_up);
//				min = std::min(min, en_data[(i + 1)*step + (j - 1)]+c_down);
//				en_data[i*step + j] = min;
//				if (min == en_data[i*step + (j - 1)]+c_mid) {
//					dir_data[i*step + j] = 2;    //Central  cell
//				}
//				else if (min == en_data[(i - 1)*step + (j - 1)]+c_up) {
//					dir_data[i*step + j] = 1;    //The Top cell
//				}
//				else {
//					dir_data[i*step + j] = 3;    //The bottom cell
//				}
//			}
//		}
//	}
//	
//	int ind = 0;
//	int min = en_data[cols - 1];
//	for (int i = 1; i < rows; i++) {
//		if (en_data[i*step + (cols - 1)] < min) {
//			min = en_data[i*step + (cols - 1)];
//			ind = i;
//		}
//	}
//
//	for (int i = cols - 1; i >= 0; i--) {
//		min_seam[i] = ind;
//		if (dir_data[ind*step + i] == 1) {
//			ind--;
//		}
//		else  if (dir_data[ind*step + i] == 2) {
//			//Do nothing 
//		}
//		else  if (dir_data[ind*step + i] == 3) {
//			ind++;
//		}
//		else {                                  // case 0 will not do anything here
//			break;
//		}
//	}
//
//	delete[] en_data;
//	delete[] dir_data;
//
//	return min_seam;
//}

cv::Mat resize(const int * ptr, const cv::Mat & img) {
	using namespace cv;
	auto * img_data = img.data;
	size_t step = img.step;
	Mat result = Mat::zeros(img.rows - 1, img.cols, CV_8UC3);
	auto * res_data = result.data;
	int rows = result.rows;
	int cols = result.cols;
	auto colomnApply = [&img, &result, ptr, cols, rows, step, img_data, res_data](int offset, int size) {
		for (int j = offset; j < offset + size; j++) {
			for (int i = 0; i < rows; i++) {
				if (i == ptr[j]) {
					for (int k = i; k < rows; k++) {
						//res_data[k*step + j] = img_data[(k + 1)*step + j];
						result.at<Vec3b>(k, j)[0] = img.at<Vec3b>(k + 1, j)[0];
						result.at<Vec3b>(k, j)[1] = img.at<Vec3b>(k + 1, j)[1];
						result.at<Vec3b>(k, j)[2] = img.at<Vec3b>(k + 1, j)[2];
					}

					break;
				}
				else {
					//res_data[i*step + j] = img_data[i*step + j];
					result.at<Vec3b>(i, j)[0] = img.at<Vec3b>(i, j)[0];
					result.at<Vec3b>(i, j)[1] = img.at<Vec3b>(i, j)[1];
					result.at<Vec3b>(i, j)[2] = img.at<Vec3b>(i, j)[2];
				}
			}
		}
	};

	std::vector<std::thread> threads;
	int numThreads = std::thread::hardware_concurrency();
	//std::cout << "num threads: " << numThreads << std::endl;
	for (int j = 0; j < numThreads; j++) {
		//threads.push_back(std::thread(colomnApply, j));
		int offset = j * static_cast<double>(cols) / numThreads;
		int size = std::min<double>(static_cast<double>(cols) / numThreads, static_cast<double>(cols - offset));
		if (j != numThreads - 1) {
			threads.emplace_back(colomnApply, offset, size);
		}
		else {
			colomnApply(offset, size);
		}
	}
	for (auto& thread : threads) {
		thread.join();
	}

	return result;
}
