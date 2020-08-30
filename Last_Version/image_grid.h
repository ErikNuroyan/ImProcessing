#pragma once

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>

class ImageGrid {
private:
	struct Node {
	public:
		Node *up, *down, *left;
		volatile long long energy;
		int penalty;
		char dir = ' ';
		cv::Vec3b col;
		Node(Node *up, Node *down, Node *left, const cv::Vec3b & col) {
			this->up = up;
			this->down = down;
			this->left = left;
			this->energy = -1;
			this->col = col;
			this->penalty = 0;
		}
		Node() {
			this->up = nullptr;
			this->down = nullptr;
			this->left = nullptr;
			this->energy = -1;
			this->penalty = 0;
		}
		~Node() {
			 this->up=nullptr;
			 this->down=nullptr;
			 this->left=nullptr;
		};

	};

	int width;
	int height;
	int n_threads;
	Node *** thread_rows;
	void resize_once_shrink_multithreaded();
	void resize_once_shrink();
	void resize_once_expand(int n);
	void energy_multithreaded();
	void energy();
	inline cv::Vec3b get_coord(const cv::Mat & img, int i, int j) {
		return ((i >= 0 && i < img.rows) && (j >= 0 && j < img.cols)) ? img.at<cv::Vec3b>(i, j) : cv::Vec3b(0, 0, 0);
	}
	inline int cost_Cmid(const cv::Mat & img, const int i, const int j)
	{
		int gray1 = get_gray(get_coord(img, i + 1, j));
		int gray2 = get_gray(get_coord(img, i - 1, j));
		return abs(gray1 - gray2);
	}
	inline int get_gray(const cv::Vec3b & v) {
		return ((int(v[0]) + v[1] + v[2]) / 3);
	}
	inline int cost_Cmid_grid(const Node & n)
	{
		int gray2 = ((n.up == nullptr) ? 0 : get_gray((n.up)->col));
		int gray1 = ((n.down == nullptr) ? 0 : get_gray((n.down)->col));
		return abs(gray1 - gray2);
	}

public:
	ImageGrid(const cv::Mat & img);
	~ImageGrid();
	void print_grid();
	void resize(int n_pixels);
	cv::Mat produce_image();
};

