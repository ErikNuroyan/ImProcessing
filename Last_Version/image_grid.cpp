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

	//Constructing the first rows of each thread 
	this->thread_rows = new Node**[n_threads];
	for (int k = 0; k < n_threads; k++) {
		thread_rows[k] = new Node*[img.cols];
	}

	for (int n = 0; n < n_threads; n++) {
		for (int w = 0; w < this->width; w++) {
			thread_rows[n][w] = new Node();
		}
	}

	//Filling the grid and linking the the thread portions
	for (int n = 0; n < n_threads; n++) {
		int offset = n * ceil(double(img.rows) / n_threads);
		int size = MIN(ceil(double(img.rows) / n_threads), img.rows - offset);
		for (int j = 0; j < this->width; j++) {
			current = thread_rows[n][j];
			prev = current->up;
			for (int i = offset; i < offset + size; i++) {
				current->up = prev;
				if (i != offset + size - 1) {
					current->down = new Node();
				}
				else if (n != n_threads - 1) {
					thread_rows[n+1][j]->up=current;
					current->down = thread_rows[n + 1][j];
				}
				current->col = img.at<cv::Vec3b>(i, j);
				prev = current;
				current = current->down;
			}
		}
	}
}
ImageGrid::~ImageGrid() {
	//Deallocating memory consumed by the grid
	for (int i = 0; i < this->width; i++) {
		Node * current = thread_rows[0][i];
		Node * next = (current->down);
		while (current != nullptr) {
			delete current;
			current = next;
			if (next != nullptr) {
				next = next->down;
			}
		}
	}
	for (int i = 0; i < n_threads; i++) {
		delete[] thread_rows[i];
	}
	delete[] thread_rows;
}
void ImageGrid::print_grid() {
	//Printing the whole grid(optional function)
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
		for (int j = 0; j < this->width; j++) {
			thread_rows[i][j]->energy = -1;
			thread_rows[i][j]->up->energy = -1;
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
			current->energy = std::min({ c_down, c_mid, c_up }) + current->penalty;
			current = current->down;
		}

		//The characers and their meanings: '1'-"Upper Cell",'2'- "Central Cell", '3'- "Bottom Cell"
		for (int c = 1; c < this->width; c++) {
			prev_current = thread_first_row[c - 1];
			current = thread_first_row[c];

			if (current->up == nullptr) {
				c_mid = cost_Cmid_grid(*current);
				temp = get_gray(prev_current->col);
				c_down = c_mid + abs(temp - get_gray(current->down->col));
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
			else {
				while (prev_current->up->energy == -1) {
					//wait
				}
				c_mid = cost_Cmid_grid(*current);
				temp = get_gray(prev_current->col);
				c_down = c_mid + abs(temp - get_gray(current->down->col));
				c_up = c_mid + abs(temp - get_gray(current->up->col));
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
			current = current->down;
			prev_current = prev_current->down;
			counter = 1;
			while (counter < size - 1) {
				c_mid = cost_Cmid_grid(*current);
				temp = get_gray(prev_current->col);
				c_down = c_mid + abs(temp - get_gray(current->down->col));
				c_up = c_mid + abs(temp - get_gray(current->up->col));


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

				prev_current = prev_current->down;
				current = current->down;
				counter++;
			}
			if (current->down == nullptr) {
				c_mid = cost_Cmid_grid(*current);
				temp = get_gray(prev_current->col);
				c_up = c_mid + abs(temp - get_gray(current->up->col));
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
				while (prev_current->down->energy == -1) {
					//wait
				}
				c_mid = cost_Cmid_grid(*current);
				temp = get_gray(prev_current->col);
				c_down = c_mid + abs(temp - get_gray(current->down->col));
				c_up = c_mid + abs(temp - get_gray(current->up->col));

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

	//Finding the node with minimum energy to start backtracing
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

	//Doing the update for thread starts
	//To make the job for each thread nearly equal
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
void ImageGrid::resize_once_expand(int n) {
	Node * current;
	Node * prev_current;
	for (int i = 0; i < n; i++) {
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

		if (this->height%n_threads == 0) {

			//Update the thread start rows correspondingly
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
		}
		else {
			//Update the thread start rows correspondingly
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
				int x = curr_row % int((ceil(double(this->height) / n_threads)));
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
		}
		this->height = this->height + 1;
	}
}

void ImageGrid::resize(int n_pixels) {
	if (n_pixels < 0) {
		for (int i = 0; i < abs(n_pixels); i++) {
			resize_once_shrink();
		}
		imwrite("est_shrinking.jpg", produce_image());
	}
	else if (n_pixels > 0) {
		resize_once_expand(n_pixels);
		imwrite("Expanded.jpg", produce_image());
	}
	else {
		return;
	}
}

