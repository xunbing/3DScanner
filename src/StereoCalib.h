#pragma once
#include <iostream>
#include<opencv2\core\core.hpp>
#include<opencv2\highgui\highgui.hpp>
#include<opencv2\imgproc\imgproc.hpp>
#include<opencv2\calib3d\calib3d.hpp>
#include <opencv2/features2d/features2d.hpp>
#include<string>
#include<vector>

#include <string>
#include <algorithm>

#include <iterator>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
using namespace std;
using namespace cv;

class StereoCalibrator
{
private:
	int Height = 1536;
	int Width = 2048;
	double aee = 0;
	double RMSError = 0;
public:
	Mat rmap[2][2];
	Mat cameraMatrix[2], distCoeffs[2];
	Mat R, T, E, F;
	Mat R1, R2, P1, P2, Q;
	int print_help()
	{
		cout <<
			" Given a list of chessboard images, the number of corners (nx, ny)\n"
			" on the chessboards, and a flag: useCalibrated for \n"
			"   calibrated (0) or\n"
			"   uncalibrated \n"
			"     (1: use cvStereoCalibrate(), 2: compute fundamental\n"
			"         matrix separately) stereo. \n"
			" Calibrate the cameras and display the\n"
			" rectified results along with the computed disparity images.   \n" << endl;
		cout << "Usage:\n ./stereo_calib -w=<board_width default=9> -h=<board_height default=6> -s=<square_size default=1.0> <image list XML/YML file default=../data/stereo_calib.xml>\n" << endl;
		return 0;
	}


	void
		StereoCalib(unsigned char **&ArrayofImg, int ImgNum, Size boardSize, float squareSize, bool displayCorners, bool useCalibrated, bool showRectified)
	{
		if (ImgNum % 2 != 0)
		{
			cout << "Error: the image list contains odd (non-even) number of elements\n";
			return;
		}

		const int maxScale = 2;
		// ARRAY AND VECTOR STORAGE:
		//imagePoints数组中的两个元素分别用来记录左右图像的角点
		vector<vector<Point2f> > imagePoints[2];
		vector<vector<Point3f> > objectPoints;
		Size imageSize;

		///int i, j, k, nimages = (int)imagelist.size() / 2;
		int i, j, k, nimages = ImgNum / 2;

		imagePoints[0].resize(nimages);
		imagePoints[1].resize(nimages);
		//合法图像列表
		///vector<string> goodImageList;
		vector<int> goodImg;
		//unsigned char**goodImg = new unsigned char*[ImgNum];
		//for (int p = 0; p < ImgNum; p++)
		//{
		//	goodImg[p] = new unsigned char[Width*Height];
		//}
		//第i幅图像,k=0表示左图像，k=1表示右图像,j表示成功匹配的图像对数
		for (i = j = 0; i < nimages; i++)
		{
			for (k = 0; k < 2; k++)
			{
				///const string& filename = imagelist[i * 2 + k];
				//Mat img = imread(filename, 0);
				Mat img = Mat(Height, Width, CV_8UC1, ArrayofImg[i * 2 + k]);
				//imshow("image", img);
				
				if (img.empty())
					break;
				if (imageSize == Size())
					imageSize = img.size();
				else if (img.size() != imageSize)
				{
					cout << "The image " << i * 2 + k << " has the size different from the first image size. Skipping the pair\n";
					break;
				}
				bool found = false;
				//
				vector<Point2f>& corners = imagePoints[k][j];
				for (int scale = 1; scale <= maxScale; scale++)
				{
					Mat timg;
					if (scale == 1)
						timg = img;
					else
						resize(img, timg, Size(), scale, scale);
					found = findChessboardCorners(timg, boardSize, corners);
					//	CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_NORMALIZE_IMAGE);
					if (found)
					{
						if (scale > 1)
						{
							Mat cornersMat(corners);
							cornersMat *= 1. / scale;
						}
						break;
					}
				}
				if (displayCorners)
				{
					///cout << filename << endl;
					Mat cimg, cimg1;
					cvtColor(img, cimg, COLOR_GRAY2BGR);
					drawChessboardCorners(cimg, boardSize, corners, found);
					double sf =1024. / MAX(img.rows, img.cols);
					resize(cimg, cimg1, Size(), sf, sf);
					if (displayCorners)
					{
						///cout << filename << endl;
						Mat cimg, cimg1;
						cvtColor(img, cimg, COLOR_GRAY2BGR);
						drawChessboardCorners(cimg, boardSize, corners, found);
						double sf = 1024. / MAX(img.rows, img.cols);
						resize(cimg, cimg1, Size(), sf, sf);
						namedWindow("corners", 1);
						imshow("corners", cimg1);
						//destroyWindow("corners");
						char c = (char)waitKey(10);
						if (c == 27 || c == 'q' || c == 'Q') //Allow ESC to quit
							exit(-1);
					}
					imshow("corners", cimg1);
					char c = (char)waitKey(10);
					if (c == 27 || c == 'q' || c == 'Q') //Allow ESC to quit
						exit(-1);
				}
				else
					putchar('.');
				if (!found)
					break;
				cornerSubPix(img, corners, Size(11, 11), Size(-1, -1),
					TermCriteria(TermCriteria::COUNT + TermCriteria::EPS,
						30, 0.01));
			}
			if (k == 2)
			{
				goodImg.push_back(i * 2);
				goodImg.push_back(i * 2 + 1);
				///goodImageList.push_back(imagelist[i * 2]);
				///goodImageList.push_back(imagelist[i * 2 + 1]);
				j++;
			}
		}
		destroyWindow("corners");
		cout << j << " pairs have been successfully detected.\n";
		nimages = j;
		if (nimages < 2)
		{
			cout << "Error: too little pairs to run the calibration\n";
			return;
		}

		imagePoints[0].resize(nimages);
		imagePoints[1].resize(nimages);
		objectPoints.resize(nimages);
		//角点世界坐标系赋值
		for (i = 0; i < nimages; i++)
		{
			for (j = 0; j < boardSize.height; j++)
				for (k = 0; k < boardSize.width; k++)
					objectPoints[i].push_back(Point3f(k*squareSize, j*squareSize, 0));
		}

		cout << "Running stereo calibration ...\n";

		

		string cameraL_filename = "cameraL.yml";
		string cameraR_filename = "cameraR.yml";
		if (!cameraL_filename.empty())
		{
			// reading intrinsic parameters
			FileStorage fs(cameraL_filename, FileStorage::READ);
			if (!fs.isOpened())
			{
				printf("Failed to open file %s\n", cameraL_filename.c_str());
				//return -1;
			}


			fs["cameraL_matrix"] >> cameraMatrix[0];
			fs["distortionL_coefficients"] >> distCoeffs[0];
		}
		if (!cameraR_filename.empty())
		{
			// reading intrinsic parameters
			FileStorage fs(cameraR_filename, FileStorage::READ);
			if (!fs.isOpened())
			{
				printf("Failed to open file %s\n", cameraR_filename.c_str());
				//return -1;
			}


			fs["cameraR_matrix"] >> cameraMatrix[1];
			fs["distortionR_coefficients"] >> distCoeffs[1];
		}




		cameraMatrix[0] = initCameraMatrix2D(objectPoints, imagePoints[0], imageSize, 0);
		cameraMatrix[1] = initCameraMatrix2D(objectPoints, imagePoints[1], imageSize, 0);
		//E是本质矩阵，F为基础矩阵
		
		//发下这个distCoeffs和cameraMatrix居然是空的！！！




		double rms = stereoCalibrate(objectPoints, imagePoints[0], imagePoints[1],
			cameraMatrix[0], distCoeffs[0],
			cameraMatrix[1], distCoeffs[1],
			imageSize, R, T, E, F,
			//CALIB_FIX_ASPECT_RATIO +
			//CALIB_ZERO_TANGENT_DIST +
			CALIB_USE_INTRINSIC_GUESS,
			//CALIB_SAME_FOCAL_LENGTH +
			//CALIB_RATIONAL_MODEL +
			//CALIB_FIX_K3 + CALIB_FIX_K4 + CALIB_FIX_K5,
			TermCriteria(TermCriteria::COUNT + TermCriteria::EPS, 100, 1e-5));
		//RMS:均方根值
		cout << "done with RMS error=" << rms << endl;
		RMSError = rms;
		// CALIBRATION QUALITY CHECK
		// because the output fundamental matrix implicitly
		// includes all the output information,
		// we can check the quality of calibration using the
		//Epipolar error: m2^t*F*m1=0
		double err = 0;
		int npoints = 0;
		vector<Vec3f> lines[2];
		for (i = 0; i < nimages; i++)
		{
			//npt表示单幅图像中角点的数目
			int npt = (int)imagePoints[0][i].size();
			Mat imgpt[2];
			for (k = 0; k < 2; k++)
			{
				//imgpt[0]表示左图像中第i幅图像的角点矩阵
				imgpt[k] = Mat(imagePoints[k][i]);
				//对图像中的所有角点进行畸变校正
				//undistortPoints(imgpt[k], imgpt[k], cameraMatrix[k], distCoeffs[k], Mat(), cameraMatrix[k]);
				//为一副图像中的点计算其在另一幅图像中对应的极线
				computeCorrespondEpilines(imgpt[k], k + 1, F, lines[k]);
			}
			for (j = 0; j < npt; j++)
			{
				double errij = fabs(imagePoints[0][i][j].x*lines[1][j][0] +
					imagePoints[0][i][j].y*lines[1][j][1] + lines[1][j][2]) +
					fabs(imagePoints[1][i][j].x*lines[0][j][0] +
						imagePoints[1][i][j].y*lines[0][j][1] + lines[0][j][2]);
				err += errij;
			}
			npoints += npt;
		}
		cout << "average epipolar err = " << err / npoints << endl;
		aee = err / npoints;


		//因为opencv标定不稳定，暂时不考虑畸变
		//distCoeffs[0] = Mat::ones(0, 5, CV_32F);
		//distCoeffs[1] = Mat::ones(0, 5, CV_32F);
		// save intrinsic parameters
		FileStorage fs("intrinsics.yml", FileStorage::WRITE);
		if (fs.isOpened())
		{
			fs << "ML" << cameraMatrix[0] << "DL" << distCoeffs[0] <<
				"MR" << cameraMatrix[1] << "D" << distCoeffs[1];
			fs.release();
		}
		else
			cout << "Error: can not save the intrinsic parameters\n";

		
		Rect validRoi[2];

		


		stereoRectify(cameraMatrix[0], distCoeffs[0],
			cameraMatrix[1], distCoeffs[1],
			imageSize, R, T, R1, R2, P1, P2, Q,
			CALIB_ZERO_DISPARITY,0.8, imageSize, &validRoi[0], &validRoi[1]);

		fs.open("extrinsics.yml", FileStorage::WRITE);
		if (fs.isOpened())
		{
			fs << "R" << R << "T" << T << "R1" << R1 << "R2" << R2 << "P1" << P1 << "P2" << P2 << "Q" << Q;
			fs.release();
		}
		else
			cout << "Error: can not save the extrinsic parameters\n";

		// OpenCV can handle left-right
		// or up-down camera arrangements
		bool isVerticalStereo = fabs(P2.at<double>(1, 3)) > fabs(P2.at<double>(0, 3));

		// COMPUTE AND DISPLAY RECTIFICATION
		if (!showRectified)
			return;

		
		// IF BY CALIBRATED (BOUGUET'S METHOD)
		if (useCalibrated)
		{
			// we already computed everything
		}
		// OR ELSE HARTLEY'S METHOD
		else
			// use intrinsic parameters of each camera, but
			// compute the rectification transformation directly
			// from the fundamental matrix
		{
			vector<Point2f> allimgpt[2];
			for (k = 0; k < 2; k++)
			{
				for (i = 0; i < nimages; i++)
					std::copy(imagePoints[k][i].begin(), imagePoints[k][i].end(), back_inserter(allimgpt[k]));
			}
			F = findFundamentalMat(Mat(allimgpt[0]), Mat(allimgpt[1]), FM_8POINT, 0, 0);
			Mat H1, H2;
			stereoRectifyUncalibrated(Mat(allimgpt[0]), Mat(allimgpt[1]), F, imageSize, H1, H2, 3);

			R1 = cameraMatrix[0].inv()*H1*cameraMatrix[0];
			R2 = cameraMatrix[1].inv()*H2*cameraMatrix[1];
			P1 = cameraMatrix[0];
			P2 = cameraMatrix[1];
		}

		//Precompute maps for cv::remap()
		initUndistortRectifyMap(cameraMatrix[0], distCoeffs[0], R1, P1, imageSize, CV_16SC2, rmap[0][0], rmap[0][1]);
		initUndistortRectifyMap(cameraMatrix[1], distCoeffs[1], R2, P2, imageSize, CV_16SC2, rmap[1][0], rmap[1][1]);

		//for (i = 0; i < nimages; i++)
		//{
		//	for (k = 0; k < 2; k++)
		//	{


		//		Mat img = Mat(Height, Width, CV_8UC1, ArrayofImg[i * 2 + k]), rimg, cimg;
		//		remap(img, rimg, rmap[k][0], rmap[k][1], INTER_LINEAR);
		//		cvtColor(rimg, cimg, COLOR_GRAY2BGR);
		//		Rect vroi(cvRound(validRoi[k].x*1), cvRound(validRoi[k].y*1),
		//			cvRound(validRoi[k].width*1), cvRound(validRoi[k].height*1));
		//		rectangle(cimg, vroi, Scalar(0, 0, 255), 3, 8);
		//		imshow("rimg", rimg);
		//		imshow("cimg", cimg);
		//		waitKey(-1);
		//	}
		//}



		Mat canvas;
		double sf;
		int w, h;
		if (!isVerticalStereo)
		{
			sf = 600. / MAX(imageSize.width, imageSize.height);
			w = cvRound(imageSize.width*sf);
			h = cvRound(imageSize.height*sf);
			canvas.create(h, w * 2, CV_8UC3);
		}
		else
		{
			sf = 300. / MAX(imageSize.width, imageSize.height);
			w = cvRound(imageSize.width*sf);
			h = cvRound(imageSize.height*sf);
			canvas.create(h * 2, w, CV_8UC3);
		}

		for (i = 0; i < nimages; i++)
		{
			for (k = 0; k < 2; k++)
			{
				Mat img = Mat(Height, Width, CV_8UC1, ArrayofImg[i * 2 + k]), rimg, cimg;
				remap(img, rimg, rmap[k][0], rmap[k][1], INTER_LINEAR);
				cvtColor(rimg, cimg, COLOR_GRAY2BGR);
				Mat canvasPart = !isVerticalStereo ? canvas(Rect(w*k, 0, w, h)) : canvas(Rect(0, h*k, w, h));
				resize(cimg, canvasPart, canvasPart.size(), 0, 0, INTER_AREA);
				if (useCalibrated)
				{
					Rect vroi(cvRound(validRoi[k].x*sf), cvRound(validRoi[k].y*sf),
						cvRound(validRoi[k].width*sf), cvRound(validRoi[k].height*sf));
					rectangle(canvasPart, vroi, Scalar(0, 0, 255), 3, 8);
				}
			}

			if (!isVerticalStereo)
				for (j = 0; j < canvas.rows; j += 16)
					line(canvas, Point(0, j), Point(canvas.cols, j), Scalar(0, 255, 0), 1, 8);
			else
				for (j = 0; j < canvas.cols; j += 16)
					line(canvas, Point(j, 0), Point(j, canvas.rows), Scalar(0, 255, 0), 1, 8);
			namedWindow("rectified", 1);
			imshow("rectified", canvas);
			//waitKey(0);
			//system("pause");
			char c = (char)waitKey(1000);
			if (c == 27 || c == 'q' || c == 'Q')
				break;
		}
		//system("pause");
		destroyWindow("rectified");
		destroyAllWindows();
	}
	double getRMSError()
	{
		return RMSError;
	}
	double getAee()
	{
		return aee;
	}
};
