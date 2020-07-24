//
//  main.cpp
//  MyProjectOpenCV
//
//  Created by Erik Nuroyan on 6/20/20.
//  Copyright © 2020 Erik Nuroyan. All rights reserved.
//


#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core/cuda.hpp>
#include <climits>


cv::Mat gradient_n(const cv::Mat & img);
int * energyMinSeam(const cv::Mat & grads);
cv::Mat resize(const int * ptr, const cv::Mat & img);

int main(int argc, const char * argv[]) {
    using namespace cv;
    


    //change the directory to where the image is
    //Mat forest=cv::imread ("forest.png",IMREAD_GRAYSCALE);
	//Mat lenochk=cv::imread("lenochk.png", IMREAD_GRAYSCALE);
	

	Mat forest_bgr = imread("forest_bgr.jpg", IMREAD_COLOR);

	//imshow("Colored Forest", forest_bgr);
	//Mat grads = gradient_n(forest_bgr);
	//imshow("Gradient_Colored", gradient_n(forest_bgr));
	
	
	


	
	
	int n = 300;                     //Enter the number of Pixels to be deleted
	int * minSeam=nullptr;
	Mat result=forest_bgr;
	Mat grad;

	
	for (int i = 0; i < n; i++) {
		grad = gradient_n(result);
		minSeam = energyMinSeam(grad);
		result=resize(minSeam, result);
	}
	

	//imshow("Gradient",gradient_n(forest));

	//imshow("Forest", forest);
	
	imshow("Resized", result);

	//imwrite("Resized_Forest.jpg", result);
	
	


	delete minSeam;
    waitKey(0);
    
    
    return 0;
}



//Function for calculating the gradient of the image
//Both for x and y axes


cv::Mat gradient_n(const cv::Mat & img){
      using namespace cv;
    
      
     

   //   //Making the edge detector for x axis
   //   
	  //int arr_x[3][3] = { { -1,-2,-1 } ,
			//			  { 0,0,0 } ,
			//			  { 1,2,1 } };
	  //Mat kernelX = Mat(3, 3, CV_32S, arr_x);
   //   
   //   //Making the edge detector for y axis
	  //int arr_y[3][3] = { { -1,0,1 } ,
			//			  { -2,0,2 } ,
			//			  { -1,0,1 } };
	 

	  //Mat kernelY = Mat(3, 3, CV_32S, arr_y);
	  
      

      Mat dX;
      Mat dY;
	  Mat gradient;

	  Sobel(img, dX, CV_16SC1, 1, 0, 3);
	  Sobel(img, dY, CV_16SC1, 0, 1, 3);

	 
	  gradient = abs(dX) + abs(dY);

	  Mat bgr[3];
	  split(gradient, bgr);
	  Mat grad_final;
	  grad_final = bgr[0] + bgr[1] + bgr[2];

	  
	  grad_final.convertTo(grad_final, CV_32F);
	  

      return grad_final;
}

int * energyMinSeam(const cv::Mat & grads){
    using namespace cv;
    
	  // for keeping the directions
	
    
    //Calculating the minimum seam without extracting the result
    //Result should be extracted by storing the x coordinates
    //which were the minimum and using that vector which has
    //length=grads.cols to extract the minimum seam
    
   
    int * en_data=new int [grads.rows*grads.cols];
	int * dir_data = new int[grads.rows*grads.cols];
	const int rows = grads.rows;
	const int cols = grads.cols;
    const int step=grads.cols;
	
	

	int * min_seam=new int [cols];   //Will keep indices of rows which comprise the horizontal minimum seam
	
    
    //Initiallizing the first column of energies with corresponding
    //values of grads
	for (int i = 0; i < rows; i++) {
		en_data[i*step] = grads.at<int>(i, 0);
		dir_data[i*step] = 0;
	}
	
    
    for(int j=1;j<cols;j++){
        for(int i=0;i<rows;i++){
            //Handling the case when the first row is under consideration
            if(i==0){
				int min = std::min(en_data[i*step + (j - 1)], en_data[(i + 1)*step + (j - 1)]);
				en_data[i*step + j] = min + grads.at<int>(i, j);
				if (min == en_data[i*step + (j - 1)]) {
					dir_data[i*step + j] = 2;    //Central  cell
				}
				else {
					dir_data[i*step + j] = 3;    //The Bottom cell
				}
            }
            //Handling the case when the last row is under consideration
            else if(i==rows-1){
				int min = std::min(en_data[i*step + (j - 1)], en_data[(i - 1)*step + (j - 1)]);
				en_data[i*step + j] = min + grads.at<int>(i, j);
				if (min == en_data[i*step + (j - 1)]) {
					dir_data[i*step + j] = 2;    //Central  cell
				}
				else {
					dir_data[i*step + j] = 1;    //The Top cell
				}
            }
            //Handling the case when rows are in between of the previous 2 ones
            else{
				int min = std::min(en_data[i*step + (j - 1)], en_data[(i - 1)*step + (j - 1)]);
				min = std::min(min, en_data[(i + 1)*step + (j - 1)]);
				en_data[i*step + j] = min + grads.at<int>(i, j);
				if (min == en_data[i*step + (j - 1)]) {
					dir_data[i*step + j] = 2;    //Central  cell
				}
				else if(min == en_data[(i-1)*step + (j - 1)]){
					dir_data[i*step + j] = 1;    //The Top cell
				}
				else {
					dir_data[i*step + j] = 3;    //The bottom cell
				}

            }
			
			

        }
		
    }
	

	//Finding the  index of the row (minimum) from which we should start backtracing

	int ind = 0;
	int min = en_data[cols - 1];

	for (int i = 1; i < rows; i++) {
		if (en_data[i*step + (cols - 1)] < min) {
			min = en_data[i*step + (cols - 1)];
			ind = i;
		}
	}

	

	for (int i = cols-1; i>=0; i--) {
		min_seam[i] = ind;
		
		if (dir_data[ind*step + i] == 1) {
			ind--;
		}
		else  if (dir_data[ind*step + i] == 2) {
			//Do nothing 
		}
		else  if (dir_data[ind*step + i] == 3) {
			ind++;
		}
		else {                                  // case 0 will not do anything here
			break;
		}
	}


	
	delete [] en_data;
	delete [] dir_data; 
    
    return min_seam;
    
}

cv::Mat resize(const int * ptr, const cv::Mat & img) {

	using namespace cv;

	auto * img_data = img.data;
	size_t step = img.step;

	
	Mat result= Mat::zeros(img.rows-1, img.cols, CV_8UC3);
	auto * res_data = result.data;
	int rows = result.rows;
	int cols = result.cols;
	auto colomnApply = [&img, &result,ptr, cols, rows, step, img_data, res_data](int offset ,int size) {
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
					result.at<Vec3b>(i, j)[0] = img.at<Vec3b>(i ,j)[0];
					result.at<Vec3b>(i, j)[1] = img.at<Vec3b>(i, j)[1];
					result.at<Vec3b>(i, j)[2] = img.at<Vec3b>(i, j)[2];
				}
			}
		}
	};

	std::vector<std::thread> threads;
	int numThreads = std::thread::hardware_concurrency();
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
	for (auto& thread: threads) {
		thread.join();
	}

	
	//for (int j = 0; j < cols; j++) {
	//	for (int i = 0; i < rows; i++) {
	//		Checking if we got to the row of the seam 
	//		if (i == ptr[j]) {

	//			for (int k = i; k < rows; k++) {
	//				res_data[k*step + j] = img_data[(k + 1)*step + j];
	//			}

	//			break;
	//		}
	//		else {
	//			res_data[i*step + j] = img_data[i*step + j];
	//		}
	//	}
	//}
	
	return result;


}

