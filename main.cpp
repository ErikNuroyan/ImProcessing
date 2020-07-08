//
//  main.cpp
//  MyProjectOpenCV
//
//  Created by Erik Nuroyan on 6/20/20.
//  Copyright © 2020 Erik Nuroyan. All rights reserved.
//


#include <iostream>
#include <opencv2/opencv.hpp>

cv::Mat gradient(const cv::Mat & img);
int * energyMinSeam(const cv::Mat & grads);
cv::Mat resize(const int * ptr, const cv::Mat & img);

int main(int argc, const char * argv[]) {
    using namespace cv;
    
    //change the directory to where the image is
    Mat forest=imread ("forest.png",IMREAD_GRAYSCALE);
	
	
  
    


	
	
	int n = 200;                     //Enter the number of Pixels to be deleted
	int * minSeam;
	Mat result=forest;
	Mat grad;

	for (int i = 0; i < n; i++) {
		grad = gradient(result);
		minSeam = energyMinSeam(grad);
		result=resize(minSeam, result);
	}



	imshow("Forest", forest);
	imshow("Resized", result);

	imwrite("Resized_Forest.png", result);
	



	//delete minSeam;
    waitKey(0);
    
    
    return 0;
}

//Function for calculating the gradient of the image
//Both for x and y axes

cv::Mat gradient(const cv::Mat & img){
      using namespace cv;
    
      Mat kernelX=Mat::zeros(3,3,CV_8S);
      Mat kernelY=Mat::zeros(3,3,CV_8S);
      //Making the edge detector for x axis
      auto* img_data=kernelX.data;
      size_t step=kernelX.step;
      
      for(int i=0;i<3;i++){
          img_data[i*step]=1;
          img_data[i*step+1]=0;
          img_data[i*step+2]=-1;
          
      }
      
      
      //Making the edge detector for x axis
      img_data=kernelY.data;
      for(int i=0;i<3;i++){
          img_data[i*step]=-1;
          img_data[i*step+1]=0;
          img_data[i*step+2]=1;
          
      }
    
      
      Mat resOfConvX;
      Mat resOfConvY;
      
      filter2D(img, resOfConvX, 8, kernelX);
      filter2D(img, resOfConvY, 8, kernelY);
      
    
    
      
      return resOfConvX+resOfConvY;
}

int * energyMinSeam(const cv::Mat & grads){
    using namespace cv;
    
	  // for keeping the directions
	
    
    //Calculating the minimum seam without extracting the result
    //Result should be extracted by storing the x coordinates
    //which were the minimum and using that vector which has
    //length=grads.cols to extract the minimum seam
    
    auto * grad_data=grads.data;
    int * en_data=new int [grads.rows*grads.cols];
	int * dir_data = new int[grads.rows*grads.cols];
	const int rows = grads.rows;
	const int cols = grads.cols;
    const int step=grads.cols;
	
	

	int * min_seam=new int [cols];   //Will keep indices of rows which comprise the horizontal minimum seam
	
    
    //Initiallizing the first column of energies with corresponding
    //values of grads
	for (int i = 0; i < rows; i++) {
		en_data[i*step] = 0;
		dir_data[i*step] = 0;
	}
	
    
    for(int j=1;j<cols;j++){
        for(int i=0;i<rows;i++){
            //Handling the case when the first row is under consideration
            if(i==0){
				int min = std::min(en_data[i*step + (j - 1)], en_data[(i + 1)*step + (j - 1)]);
				en_data[i*step + j] = min + (int)grad_data[i*step + j];
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
				en_data[i*step + j] = min + (int)grad_data[i*step + j];
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
				en_data[i*step + j] = min + (int)grad_data[i*step + j];
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


	
	
    
    return min_seam;
    
}

cv::Mat resize(const int * ptr, const cv::Mat & img) {

	using namespace cv;

	auto * img_data = img.data;
	size_t step = img.step;

	
	Mat result= Mat::zeros(img.rows-1, img.cols, CV_8U);
	auto * res_data = result.data;
	int rows = result.rows;
	int cols = result.cols;

	for (int j = 0; j < cols; j++) {
		for (int i = 0; i < rows; i++) {
			//Checking if we got to the row of the seam 
			if (i == ptr[j]) {

				for (int k = i; k < rows; k++) {
					res_data[k*step + j] = img_data[(k + 1)*step + j];
				}

				break;
			}
			else {
				res_data[i*step + j] = img_data[i*step + j];
			}
		}
	}
	
	return result;


}
