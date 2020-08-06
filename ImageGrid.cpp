#include <iostream>
#include <opencv2/opencv.hpp>
#include "ImageGrid.h"

ImageGrid::ImageGrid(const cv::Mat & img) {
	this->height = img.rows;
	this->width = img.cols;
	this->first_row = new Node*[img.cols];
	Node * current = nullptr;
	Node * prev = nullptr;
	
	for (int j = 0; j < img.cols; j++) {
		current = (first_row[j] = new Node());
		for (int i = 0; i < img.rows; i++) {
			current->up = prev;
			if(i!=img.rows-1){
				current->down = new Node();
			}
			if (j == 0) {
				int c_mid = 0;
				int c_down = 0;
				int c_up = 0;
				int temp = 0;
				c_mid = cost_Cmid(img, i, 0);
				c_down = c_mid + get_gray(get_coord(img, i + 1, 0));
				c_up = c_mid + get_gray(get_coord(img, i - 1, 0));
				current->energy = std::min({ c_down, c_mid, c_up });
			}
			current->col=img.at<cv::Vec3b>(i, j);
			prev = current;
			current = current->down;
		}
		prev = nullptr;
	}
}
void ImageGrid::print_grid() {
	Node * current;
	for (int c = 0; c < this->width; c++) {
		current = first_row[c];
		while (current != nullptr) {
			//std::cout << current->col << std::endl;
			current = current->down;
		}
	}
}
cv::Mat ImageGrid::produce_image() {
	using namespace cv;
	Mat result = Mat(this->height,this->width, CV_8UC3);
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
void ImageGrid::resize_once() {
	Node * current;
	Node * prev_current;
	int c_mid = 0;
	int c_down = 0;
	int c_up = 0;
	int temp = 0;

	for (int c = 1; c < this->width; c++) {
		prev_current = first_row[c - 1];
		current = first_row[c];
		while (current != nullptr) {
			//std::cout << ((*current).col)<<"  ";
			c_mid =  cost_Cmid_grid(*current);
			temp = get_gray(prev_current->col);
			c_down = c_mid + (current->down == nullptr ?temp: abs(temp - get_gray(current->down->col)));
			c_up = c_mid + (current->up == nullptr ?temp : abs(temp - get_gray(current->up->col)));
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
				int min = MIN(MIN(prev_current->energy + c_mid,  prev_current->up->energy + c_up), prev_current->down->energy + c_down);
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
	Node * min=first_row[width-1];
	current = min->down;
	while (current != nullptr) {
		if ((current->energy) < (min->energy)) {
			min = current;
		}
		current = current->down;
	}
	int counter = 0;
	Node * next=min->left;
	//Node *min_2 = min;
	//Node * next_2 = min->left;

	//while (min_2!= nullptr) {
	//	min_2->col = cv::Vec3b(0, 0, 255);
	//	min_2 = next_2;
	//	if (next_2 != nullptr) {
	//		next_2 = next_2->left;
	//	}
	//}
	//

	while (min!= nullptr) {
		if (min->up != nullptr) {
			(min->up)->down = min->down;
		}
		else {
			first_row[this->width - counter - 1] =min->down;
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
void ImageGrid::resize(int n_pixels) {
	for(int i=0;i<n_pixels;i++){
		resize_once();
	}
}
