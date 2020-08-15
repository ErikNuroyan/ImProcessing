#include <iostream>
#include <opencv2/opencv.hpp>
#include <climits>
#include <opencv2/videoio.hpp>
#include "ImageGrid.h"
#include <thread>
#include <chrono>

ImageGrid::ImageGrid(const cv::Mat & img) {
	this->height = img.rows;
	this->width = img.cols;
	this->first_row = new Node*[img.cols];
	Node * current = nullptr;
	Node * prev = nullptr;

	//multi-thread
	auto lambda = [&img, current, this, prev](int start, int size) mutable {
		for (int j = start; j < start + size; j++) {
			current = (first_row[j] = new Node());
			for (int i = 0; i < img.rows; i++) {
				current->up = prev;
				if (i != img.rows - 1) {
					current->down = new Node();
				}
				current->col = img.at<cv::Vec3b>(i, j);
				prev = current;
				current = current->down;
			}
			prev = nullptr;
		}
	};
	auto n_threads = std::thread::hardware_concurrency();
	std::vector<std::thread> threads;

	for (int i = 0; i < n_threads; i++) {
		int offset = i * ceil(img.cols / n_threads);
		int size = MIN(ceil(img.cols / n_threads), (img.cols - offset));
		threads.emplace_back(lambda, offset, size);
	}

	for (std::thread & t : threads) {
		t.join();
	}

	//single-thread
	/*for (int j = 0; j < img.cols; j++) {
		current = (first_row[j] = new Node());
		for (int i = 0; i < img.rows; i++) {
			current->up = prev;
			if (i != img.rows - 1) {
				current->down = new Node();
			}
			current->col = img.at<cv::Vec3b>(i, j);
			prev = current;
			current = current->down;
		}
		prev = nullptr;
	}*/
}
void ImageGrid::print_grid() {
	Node * current;
	for (int c = 0; c < this->width; c++) {
		current = first_row[c];
		while (current != nullptr) {
			current = current->down;
		}
	}
}
cv::Mat ImageGrid::produce_image() {
	using namespace cv;
	Mat result = Mat(this->height, this->width, CV_8UC3);
	Node * current;
	int i = 0;
	for (int c = 0; c < this->width; c++) {
		i = 0;
		current = first_row[c];
		while (current != nullptr) {
			result.at<Vec3b>(i, c) = current->col;
			current = current->down;
			i++;
		}
	}
	return result;
}
void ImageGrid::energy() {
	Node * current = first_row[0];
	Node * prev_current;
	int c_mid = 0;
	int c_down = 0;
	int c_up = 0;
	int temp = 0;

	//Filling the first column
	for (int i = 0; i < this->height; i++) {
		c_mid = cost_Cmid_grid(*current);
		c_down = c_mid + (current->down == nullptr ? temp : abs(temp - get_gray(current->down->col)));
		c_up = c_mid + (current->up == nullptr ? temp : abs(temp - get_gray(current->up->col)));
		current->energy = std::min({ c_down, c_mid, c_up });
		current = current->down;
	}
	//Filling the rest of the energy values
	for (int c = 1; c < this->width; c++) {
		prev_current = first_row[c - 1];
		current = first_row[c];
		while (current != nullptr) {
			c_mid = cost_Cmid_grid(*current);
			temp = get_gray(prev_current->col);
			c_down = c_mid + (current->down == nullptr ? temp : abs(temp - get_gray(current->down->col)));
			c_up = c_mid + (current->up == nullptr ? temp : abs(temp - get_gray(current->up->col)));
			if (current->up == nullptr) {
				int min = MIN((prev_current->energy) + c_mid, (prev_current->down->energy) + c_down);
				current->energy = min;
				if (min == (prev_current->energy) + c_mid) {
					current->left = prev_current;    //Central  cell
				}
				else {
					current->left = (prev_current->down); //The Bottom cell
				}
			}
			else if (current->down == nullptr) {
				int min = MIN((prev_current->energy) + c_mid, (prev_current->up->energy) + c_up);
				current->energy = min;
				if (min == (prev_current->energy) + c_mid) {
					current->left = prev_current;    //Central  cell
				}
				else {
					current->left = (prev_current->up); //The Top cell
				}
			}
			else {
				int min = MIN(MIN(prev_current->energy + c_mid, prev_current->up->energy + c_up), prev_current->down->energy + c_down);
				current->energy = min;
				if (min == (prev_current->energy) + c_mid) {
					current->left = prev_current;    //Central  cell
				}
				else if (min == (prev_current->up->energy) + c_up) {
					current->left = (prev_current->up); //The Top cell
				}
				else {
					current->left = (prev_current->down); //The Bottom cell
				}
			}
			prev_current = prev_current->down;
			current = current->down;
		}
	}
}
void ImageGrid::resize_once_shrink() {
	this->energy();
	Node * min = first_row[width - 1];
	Node * current = min->down;
	while (current != nullptr) {
		if ((current->energy) < (min->energy)) {
			min = current;
		}
		current = current->down;
	}

	int counter = 0;
	Node * next = min->left;
	while (min != nullptr) {
		if (min->up != nullptr) {
			(min->up)->down = min->down;
		}
		else {
			first_row[this->width - counter - 1] = min->down;
		}
		if (min->down != nullptr)(min->down)->up = min->up;
		min->left = nullptr;
		delete min;
		min = next;
		if (next != nullptr) {
			next = next->left;
		}
		counter++;
	}
	this->height = this->height - 1;
}
void ImageGrid::update_energy() {

	Node * current = first_row[0];
	Node * prev_current;
	for (int c = 1; c < this->width; c++) {
		prev_current = first_row[c - 1];
		current = first_row[c];
		while (current != nullptr) {
			if (current->up == nullptr) {
				int min = MIN((prev_current->energy), (prev_current->down->energy));
				current->energy = current->energy + min;
				if (min == (prev_current->energy)) {
					current->left = prev_current;    //Central  cell
				}
				else {
					current->left = (prev_current->down); //The Bottom cell
				}
			}
			else if (current->down == nullptr) {
				int min = MIN((prev_current->energy), (prev_current->up->energy));
				current->energy = current->energy + min;
				if (min == (prev_current->energy)) {
					current->left = prev_current;    //Central  cell
				}
				else {
					current->left = (prev_current->up); //The Top cell
				}
			}
			else {
				int min = MIN(MIN(prev_current->energy, prev_current->up->energy), prev_current->down->energy);
				current->energy = current->energy + min;
				if (min == (prev_current->energy)) {
					current->left = prev_current;    //Central  cell
				}
				else if (min == (prev_current->up->energy)) {
					current->left = (prev_current->up); //The Top cell
				}
				else {
					current->left = (prev_current->down); //The Bottom cell
				}
			}
			prev_current = prev_current->down;
			current = current->down;
		}
	}
}
void ImageGrid::resize_once_expand(int n, const cv::Mat & upper, const cv::Mat & lower) {
	this->energy();
	Node * current;
	Node * prev_current;
	cv::Mat result = cv::Mat(upper.rows + this->height + n + lower.rows, upper.cols, CV_8UC3);

	for (int i = 0; i < upper.rows - 1; i++) {
		for (int j = 0; j < upper.cols; j++) {
			result.at<cv::Vec3b>(i, j) = upper.at<cv::Vec3b>(i, j);
		}
	}
	for (int j = 0; j < upper.cols; j++) {
		result.at<cv::Vec3b>(upper.rows - 1, j) = cv::Vec3b(0, 0, 255);
	}

	for (int i = 0; i < n; i++) {
		if (i != 0) {
			this->update_energy();
		}
		Node * min = first_row[width - 1];
		Node * current = min->down;
		while (current != nullptr) {
			if ((current->energy) < (min->energy)) {
				min = current;
			}
			current = current->down;
		}

		Node * next = min->left;
		Node * prev_down = nullptr;
		while (min != nullptr) {
			if (min->down == nullptr) {
				min->down = new Node();
				min->down->up = min;
				min->down->left = min->left;
				min->down->col = min->up->col;
			}
			else {
				prev_down = min->down;
				min->down = new Node();
				min->down->up = min;
				min->down->down = prev_down;
				prev_down->up = min->down;
				min->down->left = min->left;
				min->down->col = (min->up != nullptr ? (min->up->col / 2 + prev_down->col / 2) : prev_down->col);
			}
			min->energy = min->energy + 45000;
			min->down->energy = min->energy + 45000;
			min = next;
			if (next != nullptr) {
				next = next->left;
			}
		}

		this->height = this->height + 1;
	}
	cv::Mat intermediate = produce_image();
	//multi-thread
	auto lambda = [&result, &upper, &lower,&intermediate](int start, int size) {
		for (int i = 0; i < upper.rows - 1; i++) {
			for (int j = start; j < start+size; j++) {
				result.at<cv::Vec3b>(i, j) = upper.at<cv::Vec3b>(i, j);
			}
		}
		for (int j = 0; j < upper.cols; j++) {
			result.at<cv::Vec3b>(upper.rows - 1, j) = cv::Vec3b(0, 0, 255);
		}
		for (int k = upper.rows; k < upper.rows + intermediate.rows; k++) {
			for (int z = start; z < start+size; z++) {
				result.at<cv::Vec3b>(k, z) = intermediate.at<cv::Vec3b>(k - upper.rows, z);
			}
		}
		for (int j = 0; j < upper.cols; j++) {
			result.at<cv::Vec3b>(upper.rows + intermediate.rows, j) = cv::Vec3b(0, 0, 255);
		}
		for (int k = upper.rows + intermediate.rows + 1; k < upper.rows + intermediate.rows + lower.rows; k++) {
			for (int z = start; z < start+size; z++) {
				result.at<cv::Vec3b>(k, z) = lower.at<cv::Vec3b>(k - upper.rows - intermediate.rows, z);
			}
		}
	};
	auto n_threads = std::thread::hardware_concurrency();
	std::vector<std::thread> threads;

	for (int i = 0; i < n_threads; i++) {
		int offset = i * ceil(upper.cols / n_threads);
		int size = MIN(ceil(upper.cols / n_threads), (upper.cols - offset));
		threads.emplace_back(lambda, offset, size);
	}

	for (std::thread & t : threads) {
		t.join();
	}
	
	//single-thread
	/*for (int k = upper.rows; k < upper.rows + intermediate.rows; k++) {
		for (int z = 0; z < upper.cols; z++) {
			result.at<cv::Vec3b>(k, z) = intermediate.at<cv::Vec3b>(k - upper.rows, z);
		}
	}
	for (int j = 0; j < upper.cols; j++) {
		result.at<cv::Vec3b>(upper.rows + intermediate.rows, j) = cv::Vec3b(0, 0, 255);
	}
	for (int k = upper.rows + intermediate.rows + 1; k < upper.rows + intermediate.rows + lower.rows; k++) {
		for (int z = 0; z < upper.cols; z++) {
			result.at<cv::Vec3b>(k, z) = lower.at<cv::Vec3b>(k - upper.rows - intermediate.rows, z);
		}
	}*/
	imwrite("E:/Test Set Videos/Test/test_expansion_2.jpg", result);
}
void ImageGrid::resize(int n_pixels, const cv::Mat & upper, const cv::Mat & lower) {
	if (n_pixels < 0) {
		cv::Mat result = cv::Mat(upper.rows + this->height + n_pixels + lower.rows, upper.cols, CV_8UC3);
		for (int i = 0; i < upper.rows - 1; i++) {
			for (int j = 0; j < upper.cols; j++) {
				result.at<cv::Vec3b>(i, j) = upper.at<cv::Vec3b>(i, j);
			}
		}
		//Red line
		for (int j = 0; j < upper.cols; j++) {
			result.at<cv::Vec3b>(upper.rows - 1, j) = cv::Vec3b(0, 0, 255);
		}
		cv::Mat intermediate;
		for (int i = 0; i < abs(n_pixels); i++) {
			resize_once_shrink();
		}
		intermediate = produce_image();
		for (int k = upper.rows; k < upper.rows + intermediate.rows; k++) {
			for (int z = 0; z < upper.cols; z++) {
				result.at<cv::Vec3b>(k, z) = intermediate.at<cv::Vec3b>(k - upper.rows, z);
			}
		}
		//Red line
		for (int j = 0; j < upper.cols; j++) {
			result.at<cv::Vec3b>(upper.rows + intermediate.rows, j) = cv::Vec3b(0, 0, 255);
		}
		for (int k = upper.rows + intermediate.rows + 1; k < upper.rows + intermediate.rows + lower.rows; k++) {
			for (int z = 0; z < upper.cols; z++) {
				result.at<cv::Vec3b>(k, z) = lower.at<cv::Vec3b>(k - upper.rows - intermediate.rows, z);
			}
		}
		imwrite("E:/Test Set Videos/Test/test_shrinking.jpg", result);
	}
	else if (n_pixels > 0) {
		resize_once_expand(n_pixels, upper, lower);
	}
	else {
		return;
	}
}
