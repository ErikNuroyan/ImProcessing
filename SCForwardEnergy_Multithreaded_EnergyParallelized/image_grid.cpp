#include <iostream>
#include <opencv2/opencv.hpp>
#include "image_grid.h"
#include <climits>
#include <opencv2/videoio.hpp>

ImageGrid::ImageGrid(const cv::Mat & img) {
	this->height = img.rows;
	this->width = img.cols;
	this->n_threads = std::thread::hardware_concurrency();
	Node * current = nullptr;
	Node * prev = nullptr;
	auto n_threads = std::thread::hardware_concurrency();
	this->thread_rows = new Node**[n_threads];

	for (int k = 0; k < n_threads; k++) {
		thread_rows[k] = new Node*[img.cols];
	}

	for (int k = 0; k < n_threads; k++) {
		int offset = k * ceil(img.rows / n_threads);
		int size = MIN(ceil(img.rows / n_threads), img.rows - offset);
		for (int j = 0; j < img.cols; j++) {
			if (k == 0)thread_rows[k][j] = new Node();
			current = thread_rows[k][j];
			prev = current->up;
			for (int i = offset; i < offset + size; i++) {
				current->up = prev;
				if (i != offset + size - 1) {
					current->down = new Node();
				}
				else if (k != n_threads - 1) {
					current->down = (thread_rows[k + 1][j] = new Node());
					thread_rows[k + 1][j]->up = current;
				}
				current->col = img.at<cv::Vec3b>(i, j);
				prev = current;
				current = current->down;
				//std::cout << i << std::endl;
			}
		}
	}
	/*for(int i=0;i<200;i++){
		this->energy();
	}*/
	//
	//this->energy();
	//this->update_energy();
}

void ImageGrid::print_grid() {
	Node * current;
	for (int c = 0; c < this->width; c++) {
		current = thread_rows[0][c];
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
		current = thread_rows[0][c];
		while (current != nullptr) {
			result.at<Vec3b>(i, c) = current->col;
			current = current->down;
			i++;
		}
	}
	return result;
}
void ImageGrid::energy() {
	//Filling the upper rows of threads with -1 to check whether energy was updated or not
	for (int i = 1; i < n_threads; i++) {
		for (int j = 0; j<this->width; j++) {
			thread_rows[i][j]->energy = -1;
		}
	}

	//Filling with threads
	auto lambda = [this](Node **thread_first_row, int size) mutable {
		Node * current = thread_first_row[0];
		Node * prev_current = nullptr;
		int c_mid = 0;
		int c_down = 0;
		int c_up = 0;
		int temp = 0;
		int counter = 0;

		for (int i = 0; i < size; i++) {
			c_mid = cost_Cmid_grid(*current);
			c_down = c_mid + (current->down == nullptr ? temp : abs(temp - get_gray(current->down->col)));
			c_up = c_mid + (current->up == nullptr ? temp : abs(temp - get_gray(current->up->col)));
			current->energy = std::min({ c_down, c_mid, c_up })+current->penalty;
			current = current->down;
		}

		//the characers and their meanings: '1'-"Upper Cell",'2'- "Central Cell", '3'- "Bottom Cell"
		for (int c = 1; c < this->width; c++) {
			prev_current = thread_first_row[c - 1];
			current = thread_first_row[c];
			counter = 0;
			while (counter < size) {
				c_mid = cost_Cmid_grid(*current);
				temp = get_gray(prev_current->col);
				c_down = c_mid + (current->down == nullptr ? temp : abs(temp - get_gray(current->down->col)));
				c_up = c_mid + (current->up == nullptr ? temp : abs(temp - get_gray(current->up->col)));
				if (current->up == nullptr) {
					int min = MIN((prev_current->energy) + c_mid, (prev_current->down->energy) + c_down);
					current->energy = current->penalty + min;
					if (min == (prev_current->energy) + c_mid) {
						current->left = prev_current;    //Central  cell
						current->dir = '2';
					}
					else {
						current->left = (prev_current->down); //The Bottom cell
						current->dir = '3';
					}
				}
				else if (current->down == nullptr) {
					int min = MIN((prev_current->energy) + c_mid, (prev_current->up->energy) + c_up);
					current->energy = current->penalty + min;
					if (min == (prev_current->energy) + c_mid) {
						current->left = prev_current;    //Central  cell
						current->dir = '2';
					}
					else {
						current->left = (prev_current->up); //The Top cell
						current->dir = '1';
					}
				}
				else {
					while (prev_current->up->energy == -1 || prev_current->down->energy == -1) {
						//Wait
					}
					int min = MIN(MIN(prev_current->energy + c_mid, prev_current->up->energy + c_up), prev_current->down->energy + c_down);
					current->energy = current->penalty + min;
					if (min == (prev_current->energy) + c_mid) {
						current->left = prev_current;    //Central  cell
						current->dir = '2';
					}
					else if (min == (prev_current->up->energy) + c_up) {
						current->left = (prev_current->up); //The Top cell
						current->dir = '1';
					}
					else {
						current->left = (prev_current->down); //The Bottom cell
						current->dir = '3';
					}
				}
				prev_current = prev_current->down;
				current = current->down;
				counter++;
			}
		}
	};

	std::vector<std::thread> threads;
	for (int i = 0; i < n_threads; i++) {
		int offset = i * ceil(double((this->height)) / n_threads);
		int size = MIN(ceil(double((this->height)) / n_threads), this->height - offset);
		if (i != n_threads - 1) {
			threads.emplace_back(lambda, thread_rows[i], size);
		}
		else {
			lambda(thread_rows[i], size);
		}
	}

	for (std::thread& t : threads) {
		t.join();
	}
}
void ImageGrid::resize_once_shrink() {
	this->energy();
	int curr_row = 0;
	int counter = 1;
	Node * min = thread_rows[0][width - 1];
	Node * current = min->down;

	while (current != nullptr) {
		if ((current->energy) < (min->energy)) {
			min = current;
			curr_row = counter;
		}
		current = current->down;
		counter++;
	}

	counter = 0;
	Node * next = min->left;
	while (min != nullptr) {
		if (min->up != nullptr) {
			(min->up)->down = min->down;
		}
		if (min->down != nullptr)(min->down)->up = min->up;
		min->left = nullptr;

		for (int i = ceil(double(curr_row) / (ceil(double(this->height) / n_threads))); i < n_threads; i++) {
			thread_rows[i][this->width - counter - 1] = thread_rows[i][this->width - counter - 1]->down;
		}

		if (min->dir == '1') {
			curr_row--;
		}
		else if (min->dir == '2') {
			//Do nothing
		}
		else if (min->dir == '3') {
			curr_row++;
		}
		else {
			//Not assigned
		}
		delete min;
		min = next;
		if (next != nullptr) {
			next = next->left;
		}
		counter++;
	}
	this->height = this->height - 1;

	if (this->height % (n_threads) == 0) {
		for (int i = 0; i < n_threads; i++) {
			for (int j = 0; j < this->width; j++) {
				for (int k = 0; k < i; k++) {
					thread_rows[i][j] = thread_rows[i][j]->up;
				}
			}
		}
	}
}
void ImageGrid::resize_once_expand(int n, const cv::Mat & upper, const cv::Mat & lower) {
	//this->energy();
	Node * current;
	Node * prev_current;
	//cv::Mat result = cv::Mat(upper.rows+this->height + n+lower.rows, upper.cols, CV_8UC3);
	//for (int i = 0; i < upper.rows-1; i++) {
	//	for (int j = 0; j < upper.cols; j++) {
	//		result.at<cv::Vec3b>(i, j) = upper.at<cv::Vec3b>(i, j);
	//	}
	//}
	//for (int j = 0; j < upper.cols; j++) {
	//	result.at<cv::Vec3b>(upper.rows - 1, j) = cv::Vec3b(0, 0, 255);
	//}

	for (int i = 0; i < n; i++) {

		if (this->height%n_threads == 0) {
		/*	if (i != 0) {
				this->update_energy();
			}*/
			this->energy();
			int curr_row = 0;
			int counter = 1;
			Node * min = thread_rows[0][width - 1];
			Node * current = min->down;
			while (current != nullptr) {
				if ((current->energy) < (min->energy)) {
					min = current;
					curr_row = counter;
				}
				current = current->down;
				counter++;
			}

			Node * next = min->left;
			Node * prev_down = nullptr;
			int counter_2 = 0;
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
				min->penalty = 2088961;
				min->down->penalty = 2088961;
				int thread_to_be_added_to = curr_row / (ceil(double(this->height) / n_threads));
				int h = 0;
				//Shiftings before the given index(including)
				for (h; h <= thread_to_be_added_to; h++) {
					for (int z = 0; z < h; z++) {
						thread_rows[h][this->width - counter_2 - 1] = thread_rows[h][this->width - counter_2 - 1]->down;
					}
				}
				//Shiftings after the given index
				for (h; h < n_threads; h++) {
					for (int z = 0; z < h - 1; z++) {
						thread_rows[h][this->width - counter_2 - 1] = thread_rows[h][this->width - counter_2 - 1]->down;
					}
				}
				if (min->dir == '1') {
					curr_row--;
				}
				else if (min->dir == '2') {
					//Do nothing
				}
				else if (min->dir == '3') {
					curr_row++;
				}
				else {
					//Not assigned
				}
				min = next;
				if (next != nullptr) {
					next = next->left;
				}
				counter_2++;
			}
			this->height = this->height + 1;
		}
		else {
				this->energy();
				int curr_row = 0;
				int counter = 1;
				Node * min = thread_rows[0][width - 1];
				Node * current = min->down;
				while (current != nullptr) {
					if ((current->energy) < (min->energy)) {
						min = current;
						curr_row = counter;
					}
					current = current->down;
					counter++;
				}
				//current = thread_rows[0][width - 1];
				//for (int z = 0; z <=curr_row; z++) {
				//	if (z == curr_row) {
				//		std::cout << (current == min);
				//	}
				//	current = current->down;
				//}

				Node * next = min->left;
				Node * prev_down = nullptr;
				int counter_2 = 0;
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
					min->penalty = 2088961;
					min->down->penalty = 2088961;
					int x = curr_row%int((ceil(double(this->height) / n_threads)));
					for (int i = (x == 0 ? curr_row / (ceil(double(this->height) / n_threads)) + 1 : ceil(curr_row / (ceil(double(this->height) / n_threads)))); i < n_threads; i++) {
						thread_rows[i][this->width - counter_2 - 1] = thread_rows[i][this->width - counter_2 - 1]->up;
					}
					if (min->dir == '1') {
						curr_row--;
					}
					else if (min->dir == '2') {
						//Do nothing
					}
					else if (min->dir == '3') {
						curr_row++;
					}
					else {
						//Not assigned
					}
					min = next;
					if (next != nullptr) {
						next = next->left;
					}
					counter_2++;
				}
				this->height = this->height + 1;

		}
		//std::cout << i << std::endl;
		
	}




	//cv::Mat intermediate = produce_image();
	//for (int k = upper.rows; k < upper.rows + intermediate.rows; k++) {
	//	for (int z = 0; z < upper.cols; z++) {
	//		result.at<cv::Vec3b>(k, z) = intermediate.at<cv::Vec3b>(k - upper.rows, z);
	//	}
	//}
	//for (int j = 0; j < upper.cols; j++) {
	//	result.at<cv::Vec3b>(upper.rows + intermediate.rows, j) = cv::Vec3b(0, 0, 255);
	//}
	//for (int k = upper.rows + intermediate.rows + 1; k <upper.rows + intermediate.rows + lower.rows; k++) {
	//	for (int z = 0; z < upper.cols; z++) {
	//		result.at<cv::Vec3b>(k, z) = lower.at<cv::Vec3b>(k - upper.rows - intermediate.rows, z);
	//	}
	//}
	imwrite("Ujezzvanq.jpg", produce_image());
	//imwrite("EsElExpand.jpg", result);
}

void ImageGrid::resize(int n_pixels, const cv::Mat & upper, const cv::Mat & lower) {
	if (n_pixels < 0) {
		//cv::Mat result = cv::Mat(upper.rows + this->height + n_pixels + lower.rows, upper.cols, CV_8UC3);
		//for (int i = 0; i < upper.rows - 1; i++) {
		//	for (int j = 0; j < upper.cols; j++) {
		//		result.at<cv::Vec3b>(i, j) = upper.at<cv::Vec3b>(i, j);
		//	}
		//}
		////Red line
		//for (int j = 0; j < upper.cols; j++) {
		//	result.at<cv::Vec3b>(upper.rows - 1, j) = cv::Vec3b(0, 0, 255);
		//}
		//cv::Mat intermediate;
		for (int i = 0; i < abs(n_pixels); i++) {
			resize_once_shrink();
		}
		//intermediate = produce_image();
		//for (int k = upper.rows; k < upper.rows + intermediate.rows; k++) {
		//	for (int z = 0; z < upper.cols; z++) {
		//		result.at<cv::Vec3b>(k, z) = intermediate.at<cv::Vec3b>(k - upper.rows, z);
		//	}
		//}
		//Red line
		//for (int j = 0; j < upper.cols; j++) {
		//	result.at<cv::Vec3b>(upper.rows + intermediate.rows, j) = cv::Vec3b(0, 0, 255);
		//}
		//for (int k = upper.rows + intermediate.rows + 1; k < upper.rows + intermediate.rows + lower.rows; k++) {
		//	for (int z = 0; z < upper.cols; z++) {
		//		result.at<cv::Vec3b>(k, z) = lower.at<cv::Vec3b>(k - upper.rows - intermediate.rows, z);
		//	}
		//}
		imwrite("est_shrinking.jpg",produce_image());
	}
	else if (n_pixels > 0) {
		resize_once_expand(n_pixels, upper, lower);
		//Node n = Node();
		//std::cout << (sizeof(n))<<std::endl;
		//system("pause");
	}
	else {
		return;
	}
}

