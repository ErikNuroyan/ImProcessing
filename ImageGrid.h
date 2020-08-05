#pragma once

#include <iostream>
#include <opencv2/opencv.hpp>

class ImageGrid{
private:
	struct Node{	
	public:
		Node *up, *down, *left, *right;
		int energy;
		cv::Vec3b col;
		Node(Node *up, Node *down, Node *left, Node *right, const cv::Vec3b & col) {
			this->up = up;
			this->down = down;
			this->left = left;
			this->right = right;
			this->energy = 0;
			this->col = col;
		}
		Node() {
			this->up = nullptr;
			this->down = nullptr;
			this->left = nullptr;
			this->right = nullptr;
		}
		~Node() {
			this->up = nullptr;
			this->down = nullptr;
			this->left = nullptr;
			this->right = nullptr;
		};

	};

	Node * head;
	short width;
	short height;
	Node ** first_row;
	void resize_once();

	inline cv::Vec3b get_coord(const cv::Mat & img, int i, int j) {
		return ((i >= 0 && i < img.rows) && (j >= 0 && j < img.cols)) ? img.at<cv::Vec3b>(i, j) :cv::Vec3b(0,0,0);
	}
	inline int cost_Cmid(const cv::Mat & img, const int i, const int j)
	{
		short gray1 = get_gray(get_coord(img, i + 1, j));
		short gray2 = get_gray(get_coord(img, i - 1, j));
		return abs(gray1 - gray2);
	}
	inline short get_gray(const cv::Vec3b & v) {
		return ((v[0] + v[1] + v[2]) / 3);
	}
	inline int cost_Cmid_grid(const Node & n)
	{
		short gray2 = ((n.up == nullptr) ? 0 : get_gray((n.up)->col));
		short gray1 = ((n.down == nullptr) ? 0 : get_gray((n.down)->col));
		return abs(gray1 - gray2);
	}

public:
	ImageGrid(const cv::Mat & img);
	void print_grid();
	void resize(int n_pixels);
	cv::Mat produce_image();
};