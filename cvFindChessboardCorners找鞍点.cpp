// testOpenCV.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <opencv2\opencv.hpp>
using namespace cv;
#include <fstream>
#include <iostream>
using namespace std;
#pragma comment(lib,"F:\\opencv\\build\\x64\\vc12\\lib\\opencv_world310.lib")

int main()
{
	cout << "Draw Chess OpenCV!" << endl;
	ifstream fin("calibdata.txt"); /* 标定所用图像文件的路径 */
	ofstream fout("caliberation_result.txt");  /* 保存标定结果的文件 */
											   //读取每一幅图像，从中提取出角点，然后对角点进行亚像素精确化   
	cout << "开始提取角点………………";
	string fileName;
	int count = -1;//用于存储角点个数。 
	int corner_row = 8;//interior number of row corners.(this can be countered by fingers.)  
	int corner_col = 6;//interior number of column corners.  
	int corner_n = corner_row*corner_col;
	CvSize pattern_size = cvSize(corner_row, corner_col);
	// CvPoint2D32f* corners=new CvPoint2D32f[corner_n];  
	CvPoint2D32f corners[48];
	while (getline(fin, fileName))
	{
		const char *filename = fileName.c_str();
		IplImage* imgRGB = cvLoadImage(filename);
		IplImage* imgGrey = cvLoadImage(filename, CV_LOAD_IMAGE_GRAYSCALE);
		if (imgGrey == NULL) {//image validation  
			cout << "No valid image input." << endl;
			char c = getchar();
			return 1;
		}

		//-------find chessboard corners--------------  
		
		int corner_count;

		int found = cvFindChessboardCorners(//returning non-zero means sucess.  
			imgGrey,// 8-bit single channel greyscale image.  
			pattern_size,//how many INTERIOR corners in each row and column of the chessboard.  
			corners,//an array where the corner locations can be recorded.  
			&corner_count,// optional, if non-NULL, its a point to an integer where the nuber of corners found can be recorded.  
						  // CV_CALIB_CB_ADAPTIVE_THRESH|CV_CALIB_CB_FILTER_QUADS// check page 382-383.  
			0
		);
		cout << "corner_count = " << corner_count;
		//-------Draw the corner pattern-------  
		cvDrawChessboardCorners(
			imgRGB,
			pattern_size,
			corners,
			corner_count,
			found
		);
		cout << "found=" << found << endl;
		cout << "x=" << corners[1].x;
		cout << ",y=" << corners[1].y << endl;

		cvNamedWindow("Find and Draw ChessBoard", 0);
		cvShowImage("Find and Draw ChessBoard", imgRGB);

		cvWaitKey(0);

		cvReleaseImage(&imgGrey);
		cvReleaseImage(&imgRGB);
		cvDestroyWindow("Find and Draw ChessBoard");
	}
	
	system("pause");


    return 0;
}

