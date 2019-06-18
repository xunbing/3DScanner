#include "stdafx.h"
#include "gxbUnpackPhase.h"
#include <opencv2\opencv.hpp>
#include <iostream>
#include <fstream>
#include <time.h>
using namespace std;
#include <Windows.h>
//初始化
int gxbUnpackPhase::gxbInit(int w, int h, int step)
{
	W = w;
	H = h;
	nstep = step;
	return 0;
}
//消除背景
int gxbUnpackPhase::gxbRemoveBg(unsigned char *bgImgb, unsigned char *bgImgw, double *PHASE)
{
	cv::Mat *bg = new cv::Mat[2];
	bg[0] = cv::Mat(H, W, CV_8UC1, bgImgb);
	bg[1] = cv::Mat(H, W, CV_8UC1, bgImgw);
	bool *judge = new bool[W*H];

	//黑白相减
	for (int i = 0; i < H; i++)
	{
		for (int j = 0; j < W; j++)
		{
			int ij = i*W + j;
			if (bgImgw[ij] - bgImgb[ij] > 5)
			{
				bgImgw[ij] = bgImgw[ij] - bgImgb[ij];
			}
			else
				bgImgw[ij] = 0;
			//if (bg[1].at<uchar>(i, j) - bg[0].at<uchar>(i, j) > 5)
			//	bg[1].at<uchar>(i, j) = bg[1].at<uchar>(i, j) - bg[0].at<uchar>(i, j);
			//else
			//	bg[1].at<uchar>(i, j) = 0;
		}
	}
	//中值滤波减少椒盐噪声
	cv::medianBlur(bg[1], bg[1], 3);

	cv::Mat bg_copy1 = cv::Mat(H, W, CV_8UC1, bgImgw);
	//二值化
	cv::threshold(bg_copy1, bg_copy1, 0, 255, CV_THRESH_BINARY);

	blur(bg_copy1, bg_copy1, cv::Size(3, 3));
	cv::Mat detected_edges;
	/// 运行Canny算子
	Canny(bg_copy1, detected_edges, 40, 120, 3);

	/// 使用 Canny算子输出边缘作为掩码显示原图像
	cv::Mat dst;
	dst = cv::Scalar::all(0);
	bg_copy1.copyTo(dst, detected_edges);

	cv::Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE,
		cv::Size(2 * 3 + 1, 2 * 3 + 1),
		cv::Point(1, 1));
	///膨胀操作
	dilate(dst, dst, element);
	threshold(dst, dst, 10, 255, CV_THRESH_BINARY);
	for (int i = 0; i < H; i++)
	{
		for (int j = 0; j < W; j++)
		{
			if (dst.data[i*W + j] == 255)
			{
				PHASE[i*W + j] = 0;
			}
		}
	}
	cv::Mat element2 = cv::getStructuringElement(0, cv::Size(3, 3));
	// 腐蚀操作
	cv::erode(bg_copy1, bg_copy1, element2);
	for (int i = 1; i < H; i++)
	{
		for (int j = 1; j < W; j++)
		{
			int weight = 0;
			if (bgImgw[i*W + j] == 255)
			{
				//八连通区域检测
				if (i < H - 1 && j < W - 1)
				{
					if (bgImgw[i*W + j - 1] == 0)
						weight++;
					if (bgImgw[i*W + j + 1] == 0)
						weight++;
					if (bgImgw[(i - 1)*W + j] == 0)
						weight++;
					if (bgImgw[(i + 1)*W + j] == 0)
						weight++;
					if (bgImgw[(i - 1)*W + j - 1] == 0)
						weight++;
					if (bgImgw[(i + 1)*W + j + 1] == 0)
						weight++;
					if (bgImgw[(i - 1)*W + j + 1] == 0)
						weight++;
					if (bgImgw[(i + 1)*W + j - 1] == 0)
						weight++;
				}
				if (weight <= 3)
					judge[i*W + j] = TRUE;
				else
					judge[i*W + j] = FALSE;
			}
			else
			{
				judge[i*W + j] = FALSE;
				//bg[1].at<uchar>(i, j) = 0;
			}
		}
	}
	for (int k = 0; k < 17; k++)
	{
		for (int i = 0; i < H; i++)
		{
			for (int j = 0; j < W; j++)
			{
				if (i < H - 1 && j < W - 1)
				{
					if (!judge[i*W + j])
					{
						PHASE[i*W + j] = 0;
					}
				}
			}
		}
	}
	delete[]judge;
	return 0;
}
//相位滤波
int gxbUnpackPhase::gxbPhaseflitter(double *PHASE)
{
	double *phasej1 = new double[W*H];
	memset(phasej1, 0, W*H);
	for (int i = 1; i < H - 1; i++)
	{
		for (int j = 1; j < W - 1; j++)
		{
			if (fabs(PHASE[i*W + j + 1] - PHASE[i*W + j]) > 6)
			{
				phasej1[i*W + j] = 1;
				phasej1[i*W + j - 1] = 1;
				phasej1[i*W + j + 1] = 1;
				phasej1[i*W + j + 2] = 1;
			}
		}
	}
	for (int i = 1; i < H - 1; i++)
	{
		for (int j = 1; j < W - 1; j++)
		{
			if (phasej1[i*W + j] == 1)
				PHASE[i*W + j] = 0;
		}
	}
	///上下左右八连通滤波
	double *phasej = new double[W*H];
	for (int i = 0; i < W*H; i++)
	{
		phasej[i] = 0;
	}
	double aver = 0;

	for (int i = 1; i < H - 1; i++)
	{
		for (int j = 1; j < W - 1; j++)
		{
			aver = (PHASE[i*W + j] + PHASE[(i - 1)*W + j] + PHASE[(i + 1)*W + j] + PHASE[i*W + j - 1] + PHASE[i*W + j + 1] + PHASE[(i - 1)*W + j - 1] + PHASE[(i - 1)*W + j + 1] + PHASE[(i + 1)*W + j + 1] + PHASE[(i + 1)*W + j - 1]) / 9;
			if (fabs(aver - PHASE[i*W + j]) > 0.1)
				phasej[i*W + j] = 1;
		}
	}

	for (int i = 1; i < H - 1; i++)
	{
		for (int j = 1; j < W - 1; j++)
		{
			if (phasej[i*W + j] == 1)
				PHASE[i*W + j] = 0;
		}
	}
	delete[]phasej;
	return 0;
}
//求包裹相位
int gxbUnpackPhase::gxbGetWrappedPhase(int step, unsigned char **&Arrayofphase, double *PHASE, int size, int index, int row)
{
	//Mat getPhaseImg = Mat(H, W, CV_8UC1, Arrayofphase[1]);
	//imshow("getPhaseimg", getPhaseImg);
	//waitKey(-1);
	double* sinValue = new double[step];
	double* cosValue = new double[step];
	double interMove = 2 * PI / step;

	for (int k = 0; k < step; k++)
	{
		sinValue[k] = -1 * sin((k)*interMove);
		cosValue[k] = cos((k)*interMove);
	}

	for (int i = 0; i < size; i++)
	{
		double deltax = 0;
		double deltay = 0;

		for (int k = 0; k < step; k++)
		{
			deltay += Arrayofphase[k + index][row*W + i] * sinValue[k];
			deltax += Arrayofphase[k + index][row*W + i] * cosValue[k];
		}
		{
			if (deltax == 0 && deltay >= 0)
				PHASE[i] = PI / 2;
			else if (deltax == 0 && deltay < 0)
				PHASE[i] = -PI / 2;
			else
				PHASE[i] = atan2(deltay, deltax);

			//Phase[i] = (Phase[i] + PI)/(2*PI);
		}
	}
	delete[]sinValue;
	delete[]cosValue;
	return 0;
}
//求展开相位
int gxbUnpackPhase::gxbPhaseResult(unsigned char **&Img, double *phase)
{

	double *Phase;
	double *Phase2;
	double *Phase3;
	double *abPhase;
	double *abPhase2;
	double *abPhase22;
	double *abPhase3;

	double *ababPhase;
	double *ababPhase2;
	double *ababPhase22;
	double *ababPhase3;
	//临时存储φ1，φ2和φ2，φ3的周期数
	int *C1;
	int *C2;
	int *C3;
	int *C4;
	C1 = new int[W];
	C2 = new int[W];
	C3 = new int[W];
	C4 = new int[W];
	//相移矩阵5*W的二维数组
	string filel = "myphase/Phase.txt";
	string filer = "myphase/Phase2.txt";
	string file3 = "myphase/Phase3.txt";
	string filecp1 = "myphase/cp_abPhase1.txt";
	string filecp2 = "myphase/cp_abPhase2.txt";
	string filecp22 = "myphase/cp_abPhase22.txt";
	string filecp3 = "myphase/cp_abPhase3.txt";
	ofstream outl, outr, out3, outcp1, outcp2, outcp22, outcp3;
	outl.open(filel.c_str(), ofstream::out);
	outr.open(filer.c_str(), ofstream::out);
	out3.open(file3.c_str(), ofstream::out);

	outcp1.open(filecp1.c_str(), ofstream::out);
	outcp2.open(filecp2.c_str(), ofstream::out);
	outcp22.open(filecp22.c_str(), ofstream::out);
	outcp3.open(filecp3.c_str(), ofstream::out);

	Phase = new double[W];
	Phase2 = new double[W];
	Phase3 = new double[W];
	abPhase = new double[W];
	abPhase2 = new double[W];
	abPhase22 = new double[W];
	abPhase3 = new double[W];
	ababPhase = new double[W];
	ababPhase2 = new double[W];
	ababPhase22 = new double[W];
	ababPhase3 = new double[W];
	//保存下次展开真实相位
	double *cp_abPhase = new double[W];
	double *cp_abPhase2 = new double[W];
	double *cp_abPhase22 = new double[W];
	double *cp_abPhase3 = new double[W];

	for (int i = 0; i < H; i++)
	{
		//求包裹相位
		gxbGetWrappedPhase(nstep, Img, Phase, W, 0, i);
		gxbGetWrappedPhase(nstep, Img, Phase2, W, 5, i);
		gxbGetWrappedPhase(nstep, Img, Phase3, W, 10, i);

		//解次展开相位
		for (int l = 0; l < W; l++)
		{

			double delta = 0;
			double xi1 = 0, xi2 = 0;
			double d1 = 0, d2 = 0;

			if (Phase[l] >= Phase2[l])
			{
				delta = Phase[l] - Phase2[l];
				d1 = p2*delta;
				d2 = p1*delta;
				C1[l] = (int)((p2 / (p2 - p1)*delta - Phase[l]) / (2 * PI));
				C2[l] = (int)((p1 / (p2 - p1)*delta - Phase2[l]) / (2 * PI));

				abPhase[l] = 2 * PI*C1[l] + Phase[l];
				abPhase2[l] = 2 * PI*C2[l] + Phase2[l];
				//if (C1[l] == 12)	C1[l] = 0;
				//if (C2[l] == 11)	C2[l] = 0;

				//if (C1[l - 1] == -1)
				//{
				//	abPhase[l - 1] = (abPhase[l] + abPhase[l - 2]) / 2;
				//}
				//if (C2[l - 1] == -1)
				//{
				//	abPhase2[l - 1] = (abPhase2[l] + abPhase2[l - 2]) / 2;
				//}
				//
				if (d1 - abPhase[l] > 4.8)
				{
					//if (abPhase[l]<abPhase[l - 1])
					{
						abPhase[l] += 2 * PI; C1[l] += 1;
					}
				}
				if (d2 - abPhase2[l] > 4.8)
				{
					//if (abPhase2[l] < abPhase2[l - 1])
					{
						abPhase2[l] += 2 * PI; C2[l] += 1;
					}
				}
				xi1 = abPhase[l] - d1;
				xi2 = abPhase2[l] - d2;
				abPhase[l] -= -p1*xi1 / (p1*p1 + p2*p2);
				abPhase2[l] -= p2*xi2 / (p1*p1 + p2*p2);
				cp_abPhase[l] = abPhase[l];
				cp_abPhase2[l] = abPhase2[l];
			}
			else
			{
				delta = Phase[l] - Phase2[l] + 2 * PI;
				d1 = p2*delta;
				d2 = p1*delta;
				C1[l] = (int)((p2 / (p2 - p1)*delta - Phase[l]) / (2 * PI));
				C2[l] = (int)((p1 / (p2 - p1)*delta - Phase2[l]) / (2 * PI));

				/*if (i == 943)
				outfile123 << C1[l] << endl;*/


				abPhase[l] = 2 * PI*C1[l] + Phase[l];
				abPhase2[l] = 2 * PI*C2[l] + Phase2[l];
				//if (C1[l] == 12)	C1[l] = 0;
				//if (C2[l] == 11)	C2[l] = 0;

				//if (C1[l - 1] == -1)
				//{
				//	abPhase[l - 1] = (abPhase[l] + abPhase[l - 2]) / 2;
				//}
				//if (C2[l - 1] == -1)
				//{
				//	abPhase2[l - 1] = (abPhase2[l] + abPhase2[l - 2]) / 2;
				//}
				if (d1 - abPhase[l] > 4.8)
				{
					//if (abPhase[l]<abPhase[l - 1])
					{
						abPhase[l] += 2 * PI; C1[l] += 1;
					}
				}
				if (d2 - abPhase2[l] > 4.8)
				{
					//if (abPhase2[l] < abPhase2[l - 1])
					{
						abPhase2[l] += 2 * PI; C2[l] += 1;
					}
				}


				xi1 = abPhase[l] - d1;
				xi2 = abPhase2[l] - d2;
				abPhase[l] -= -p1*xi1 / (p1*p1 + p2*p2);
				abPhase2[l] -= p2*xi2 / (p1*p1 + p2*p2);
				cp_abPhase[l] = abPhase[l];
				cp_abPhase2[l] = abPhase2[l];
			}
			if (Phase2[l] >= Phase3[l])
			{
				delta = Phase2[l] - Phase3[l];
				d1 = p3*delta;
				d2 = p2*delta;

				C3[l] = (int)((p3 / (p3 - p2)*delta - Phase2[l]) / (2 * PI));
				C4[l] = (int)((p2 / (p3 - p2)*delta - Phase3[l]) / (2 * PI));

				abPhase22[l] = 2 * PI*C3[l] + Phase2[l];
				abPhase3[l] = 2 * PI*C4[l] + Phase3[l];


				//if (C3[l] == 13)	C3[l] = 0;
				//if (C4[l] == 12)	C4[l] = 0;


				//if (C3[l - 1] == -1)
				//{
				//	abPhase22[l - 1] = (abPhase22[l] + abPhase22[l - 2]) / 2;
				//}
				//if (C4[l - 1] == -1)
				//{
				//	abPhase3[l - 1] = (abPhase3[l] + abPhase3[l - 2]) / 2;
				//}


				if (d1 - abPhase22[l] > 4.8)
				{
					//if (abPhase22[l]<abPhase22[l - 1])
					{
						abPhase22[l] += 2 * PI; C3[l] += 1;
					}
				}
				if (d2 - abPhase3[l] > 4.8)
				{
					//if (abPhase22[l] < abPhase22[l - 1])
					{
						abPhase3[l] += 2 * PI; C4[l] += 1;
					}
				}

				xi1 = abPhase22[l] - d1;
				xi2 = abPhase3[l] - d2;
				abPhase22[l] -= -p2*xi1 / (p3*p3 + p2*p2);
				abPhase3[l] -= p3*xi2 / (p3*p3 + p2*p2);
				cp_abPhase22[l] = abPhase22[l];
				cp_abPhase3[l] = abPhase3[l];
			}
			else
			{
				delta = Phase2[l] - Phase3[l] + 2 * PI;
				d1 = p3*delta;
				d2 = p2*delta;
				C3[l] = (int)((p3 / (p3 - p2)*delta - Phase2[l]) / (2 * PI));
				C4[l] = (int)((p2 / (p3 - p2)*delta - Phase3[l]) / (2 * PI));

				abPhase22[l] = 2 * PI*C3[l] + Phase2[l];
				abPhase3[l] = 2 * PI*C4[l] + Phase3[l];
				//if (C3[l] == 13)	C3[l] = 0;
				//if (C4[l] == 12)	C4[l] = 0;


				//if (C3[l - 1] == -1)
				//{
				//	abPhase22[l - 1] = (abPhase22[l] + abPhase22[l - 2]) / 2;
				//}
				//if (C4[l - 1] == -1)
				//{
				//	abPhase3[l - 1] = (abPhase3[l] + abPhase3[l - 2]) / 2;
				//}



				if (d1 - abPhase22[l] > 4.8)
				{
					//if (abPhase22[l]<abPhase22[l - 1])
					{
						abPhase22[l] += 2 * PI; C3[l] += 1;
					}
				}
				if (d2 - abPhase3[l] > 4.8)
				{
					//if (abPhase22[l] < abPhase22[l - 1])
					{
						abPhase3[l] += 2 * PI; C4[l] += 1;
					}
				}

				xi1 = abPhase22[l] - d1;
				xi2 = abPhase3[l] - d2;
				abPhase22[l] -= -p2*xi1 / (p3*p3 + p2*p2);
				abPhase3[l] -= p3*xi2 / (p3*p3 + p2*p2);
				cp_abPhase22[l] = abPhase22[l];
				cp_abPhase3[l] = abPhase3[l];
			}
		}


		//对特殊点取平均值
		for (int j = 1; j < W - 1; j++)
		{
			if (abPhase2[j] - abPhase2[j - 1] > PI&&abPhase2[j] - abPhase2[j - 1] < 3 * PI)
			{
				abPhase2[j] = (abPhase2[j - 1] + abPhase2[j + 1]) / 2;
			}
			if (abPhase3[j] - abPhase3[j - 1] > PI&&abPhase3[j] - abPhase3[j - 1] < 3 * PI)
			{
				abPhase3[j] = (abPhase3[j - 1] + abPhase3[j + 1]) / 2;
			}
			if (abPhase2[j] - abPhase2[j - 1]<-PI&&abPhase2[j] - abPhase2[j - 1]>-3 * PI)
			{
				abPhase2[j] = (abPhase2[j - 1] + abPhase2[j + 1]) / 2;
			}
			if (abPhase3[j] - abPhase3[j - 1]<-PI&&abPhase3[j] - abPhase3[j - 1]>-3 * PI)
			{
				abPhase3[j] = (abPhase3[j - 1] + abPhase3[j + 1]) / 2;
			}
			cp_abPhase2[j] = abPhase2[j];
			cp_abPhase3[j] = abPhase3[j];
		}
		//解全局相位
		for (int l = 0; l < W; l++)
		{
			double delta = 0;
			//11
			abPhase[l] = (abPhase[l] + PI / 2) / (p2)-PI;
			//12
			abPhase2[l] = (abPhase2[l] + PI / 2) / (p1)-PI;
			//12
			abPhase22[l] = (abPhase22[l] + PI / 2) / (p3)-PI;
			//13
			abPhase3[l] = (abPhase3[l] + PI / 2) / (p2)-PI;
			//
			if (abPhase[l] >= abPhase22[l])
			{
				delta = abPhase[l] - abPhase22[l];
				ababPhase[l] = 2 * PI*(int)((double(p2*p3 / (p3 - p2)) / (p2*p3 / (p3 - p2) - p2*p1 / (p2 - p1)) * delta - abPhase[l]) / (2 * PI)) + abPhase[l];
				ababPhase22[l] = 2 * PI*(int)((double(p2*p1 / (p2 - p1)) / (p2*p3 / (p3 - p2) - p2*p1 / (p2 - p1)) * delta - abPhase22[l]) / (2 * PI)) + abPhase22[l];
				//t1 = (int)((double(p2*p3/(p3-p2)) / (p2*p3/(p3-p2)-p2*p1/(p2-p1)) * delta - abPhase[l]) / (2 * PI));
				//t2 = (int)((double(p2*p1/(p2-p1)) / (p2*p3/(p3-p2)-p2*p1/(p2-p1)) * delta - abPhase22[l]) / (2 * PI));
			}
			else
			{
				delta = abPhase[l] - abPhase22[l] + 2 * PI;
				ababPhase[l] = 2 * PI*(int)((double(p2*p3 / (p3 - p2)) / (p2*p3 / (p3 - p2) - p2*p1 / (p2 - p1)) * delta - abPhase[l]) / (2 * PI)) + abPhase[l];
				ababPhase22[l] = 2 * PI*(int)((double(p2*p1 / (p2 - p1)) / (p2*p3 / (p3 - p2) - p2*p1 / (p2 - p1)) * delta - abPhase22[l]) / (2 * PI)) + abPhase22[l];
				//t1 = (int)((double(p2*p3/(p3-p2)) / (p2*p3/(p3-p2)-p2*p1/(p2-p1)) * delta - abPhase[l]) / (2 * PI));
				//t2 = (int)((double(p2*p1/(p2-p1)) / (p2*p3/(p3-p2)-p2*p1/(p2-p1)) * delta - abPhase22[l]) / (2 * PI));

			}
			if (abPhase2[l] >= abPhase3[l])
			{
				delta = abPhase2[l] - abPhase3[l];
				ababPhase2[l] = 2 * PI*(int)((double(p2*p3 / (p3 - p2)) / (p2*p3 / (p3 - p2) - p2*p1 / (p2 - p1)) * delta - abPhase2[l]) / (2 * PI)) + abPhase2[l];
				ababPhase3[l] = 2 * PI*(int)((double(p2*p1 / (p2 - p1)) / (p2*p3 / (p3 - p2) - p2*p1 / (p2 - p1)) * delta - abPhase3[l]) / (2 * PI)) + abPhase3[l];
				//t1 = (int)((double(p2*p3/(p3-p2)) / (p2*p3/(p3-p2)-p2*p1/(p2-p1)) * delta - abPhase2[l]) / (2 * PI));
				//t2 = (int)((double(p2*p1/(p2-p1)) / (p2*p3/(p3-p2)-p2*p1/(p2-p1)) * delta - abPhase3[l]) / (2 * PI));

			}
			else
			{
				delta = abPhase2[l] - abPhase3[l] + 2 * PI;

				ababPhase2[l] = 2 * PI*(int)((double(p2*p3 / (p3 - p2)) / (p2*p3 / (p3 - p2) - p2*p1 / (p2 - p1)) * delta - abPhase2[l]) / (2 * PI)) + abPhase2[l];
				ababPhase3[l] = 2 * PI*(int)((double(p2*p1 / (p2 - p1)) / (p2*p3 / (p3 - p2) - p2*p1 / (p2 - p1)) * delta - abPhase3[l]) / (2 * PI)) + abPhase3[l];
				//t1 = (int)((double(p2*p3/(p3-p2)) / (p2*p3/(p3-p2)-p2*p1/(p2-p1)) * delta - abPhase2[l]) / (2 * PI));
				//t2 = (int)((double(p2*p1/(p2-p1)) / (p2*p3/(p3-p2)-p2*p1/(p2-p1)) * delta - abPhase3[l]) / (2 * PI));
			}
			//归一化
			/*ababPhase3[l] = (ababPhase3[l] + PI) / (132 * 2 * PI) * (p2*p3/(p3-p2)-p2*p1/(p2-p1));*/

			phase[i*W + l] = ababPhase3[l];
		}
		//保存差值数组用来判断混乱度


		//if (i == 350)
		//{
		//	string name1 = "myphase/diff1.txt";
		//	string name2 = "myphase/diff2.txt";
		//	ofstream out1, out2;
		//	out1.open(name1.c_str(), ofstream::out);
		//	out2.open(name2.c_str(), ofstream::out);
		//	memset(Diff1, 0, W);
		//	memset(Diff2, 0, W);
		//	//for (int p = 0; p < W; p++)
		//	//{
		//	//	//if (cp_abPhase2[p] < 75 && p < W - 1)
		//	//	if ( p < W - 1)
		//	//	{
		//	//		Diff1[p] = cp_abPhase2[p + 1] - cp_abPhase2[p];
		//	//		out1 << Diff1[p] << endl;
		//	//		//if (cp_abPhase2[p + 1] - cp_abPhase2[p] > -6 && cp_abPhase2[p + 1] - cp_abPhase2[p] < 0)
		//	//		if(Diff1[p]>1||Diff1[p]<0)
		//	//		{
		//	//			phase[i*W+p] = -4;
		//	//			cout << 1;
		//	//		}
		//	//	}
		//	//	//if (cp_abPhase3[p] < 82 && p < W - 1)
		//	//	if ( p < W - 1)
		//	//	{
		//	//		Diff2[p] = cp_abPhase3[p + 1] - cp_abPhase3[p];
		//	//		out2 << Diff2[p] << endl;
		//	//		//if (cp_abPhase3[p + 1] - cp_abPhase3[p] > -6 && cp_abPhase3[p + 1] - cp_abPhase3[p] < 0)
		//	//		if (Diff2[p]>1 || Diff2[p]<0)
		//	//		{
		//	//			phase[i*W + p] = -4;
		//	//			cout << 1;
		//	//		}
		//	//	}
		//	//}
		//	//gxbReduceUselessPhase(phase, cp_abPhase2, cp_abPhase3, Diff1, Diff2);
		//	for (int j = 0; j < W; j++)
		//	{
		//		outl << Phase[j] << endl;
		//		outr << Phase2[j] << endl;
		//		out3 << Phase3[j] << endl;
		//		outcp1 << cp_abPhase[j] << endl;
		//		outcp2 << cp_abPhase2[j] << endl;
		//		outcp22 << cp_abPhase22[j] << endl;
		//		outcp3 << cp_abPhase3[j] << endl;
		//	}
		//}

	}


	delete[]C1;
	delete[]C2;
	delete[]C3;
	delete[]C4;
	delete[]Phase;
	delete[]Phase2;
	delete[]Phase3;
	delete[]abPhase;
	delete[]abPhase2;
	delete[]abPhase22;
	delete[]abPhase3;
	delete[]ababPhase;
	delete[]ababPhase2;
	delete[]ababPhase22;
	delete[]ababPhase3;

	//disConstruction(arrayofphase, arrayofphase2, arrayofphase3, tmp, img);
	return 0;
}
//边缘处理
int gxbUnpackPhase::gxbRegionGrow(unsigned char**&pImg, double *Phase)
{
	int *Judge = new int[W*H];
	memset(Judge, 0, W * H);
	double *tmpPhaselr = new double[W*H];
	double *tmpPhaseud = new double[W*H];
	unsigned char *data = new unsigned char[W*H];
	unsigned char *result = new unsigned char[W*H];
	memset(data, 0, W*H);
	for (int i = 1; i < H - 1; i++)
	{
		for (int j = 1; j < W - 1; j++)
		{
			//cout << Phase[i*W + j] << endl;
			tmpPhaselr[i*W + j] = Phase[i*W + j + 1] - Phase[i*W + j];
			if (fabs(tmpPhaselr[i*W + j]) > 5)
				data[i*W + j] = 255;
			/*data[i*W + j] = uchar((tmpPhaselr[i*W + j]+PI) / (132 * 2 * PI)*24 * 255);*/
		}
	}
	for (int i = 1; i < H - 1; i++)
	{
		for (int j = 1; j < W - 1; j++)
		{
			tmpPhaseud[i*W + j] = Phase[i*W + j] - Phase[(i - 1)*W + j];
			if (fabs(tmpPhaseud[i*W + j]) > 5)
				data[i*W + j] = 255;
		}
	}
	cv::Mat showCont = cv::Mat(H, W, CV_8UC1, data);

	cv::threshold(showCont, showCont, 50, 255, 0);
	cv::Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE,
		cv::Size(2 * 1 + 1, 2 * 1 + 1),
		cv::Point(1, 1));
	///膨胀操作
	cv::dilate(showCont, showCont, element);
	unsigned char*Img = showCont.data;

	memset(result, 0, W*H);
	cv::Point2i pt = cv::Point2i(1, 1);
	cv::Point2i ptGrowing;                              //待生长点位置
	int nGrowLable = 0;                             //标记是否生长过
	int nSrcValue = 0;                              //生长起点灰度值
	int nCurValue = 0;                              //当前生长点灰度值

													//生长方向顺序数据
	int DIR[8][2] = { { -1,-1 },{ 0,-1 },{ 1,-1 },{ 1,0 },{ 1,1 },{ 0,1 },{ -1,1 },{ -1,0 } };
	std::vector<cv::Point2i> vcGrowPt;						//生长点栈 
	std::vector<cv::Point2i> vcSize;
	for (int p = 1; p < H - 1; p++)
	{
		for (int k = 1; k < W - 1; k++)
		{
			if (Img[p*W + k] == 255 && result[p*W + k] == 0)//种子点满足要求且未被访问过
			{
				pt = cv::Point2i(k, p);
				vcGrowPt.push_back(pt);//将生长点压入栈中 
				vcSize.push_back(pt);//记录生长区域的面积用于判断

				result[pt.y*W + pt.x] = 255;               //标记生长点  
				nSrcValue = Img[pt.y*W + pt.x];            //记录生长点的灰度值  

				while (!vcGrowPt.empty())                       //生长栈不为空则生长  
				{
					pt = vcGrowPt.back();                       //取出一个生长点  
					vcGrowPt.pop_back();

					//分别对八个方向上的点进行生长  
					for (int i = 0; i < 9; ++i)
					{
						ptGrowing.x = pt.x + DIR[i][0];
						ptGrowing.y = pt.y + DIR[i][1];
						//检查是否是边缘点  
						if (ptGrowing.x < 0 || ptGrowing.y < 0 || ptGrowing.x >(W - 1) || (ptGrowing.y > H - 1))
							continue;

						nGrowLable = result[ptGrowing.y*W + ptGrowing.x];      //当前待生长点的灰度值  

						if (nGrowLable == 0)                    //如果标记点还没有被生长  
						{
							nCurValue = Img[ptGrowing.y*W + ptGrowing.x];
							//cout << "生长点:" << ptGrowing << "  灰度值:" << nCurValue << endl;
							if (nCurValue == 255)                  //在阈值范围内则生长  
							{
								result[ptGrowing.y*W + ptGrowing.x] = 255;     //标记为白色  
								Judge[ptGrowing.y*W + ptGrowing.x] = 1;
								vcGrowPt.push_back(ptGrowing);                  //将下一个生长点压入栈中
								vcSize.push_back(ptGrowing);
							}
						}
					}
				}
				//如果生长区域的像素少于20个，则放弃该边缘，蒙版Mask的相关区域重新归0
				if (vcSize.size() < 30)
				{
					for (int i = 0; i < vcSize.size(); i++)
					{
						Judge[vcSize[i].y*W + vcSize[i].x] = 0;
					}
				}
				vcSize.clear();
			}
			else
				continue;
		}
	}
	for (int i = 0; i < H; i++)
	{
		for (int j = 0; j < W; j++)
		{
			int ij = i*W + j;
			if (Judge[ij] == 1)
			{
				pImg[0][ij] = 255;
			}
			else
			{
				pImg[0][ij] = 0;
			}
		}
	}
	delete[]Judge;
	delete[]data;
	delete[]tmpPhaseud;
	delete[]tmpPhaselr;
	return 0;
}

//放在一个函数内
int gxbUnpackPhase::gxbGetPhase(unsigned char**img, double *phase)
{

	clock_t start = clock();
	gxbPhaseResult(img, phase);
	clock_t end = clock();
	cout << "解相位耗时" << end - start <<"ms"<< endl;
	gxbPhaseflitter(phase);
	gxbRemoveBg(img[15], img[16], phase);
	gxbRegionGrow(img, phase);


	return 0;
}
//文件写
int gxbUnpackPhase::writemyphase(int index, int w, int h, double *phasel, double *phaser)
{
	cout << "正在把相位写入文件" << endl;
	clock_t start = clock();
	string filel = "myphase/phaseL.txt";
	string filer = "myphase/phaseR.txt";
	ofstream outl, outr;
	outl.open(filel.c_str(), ofstream::out);
	outr.open(filer.c_str(), ofstream::out);
	if (index != -1)
	{
		//for (int i = 0; i < h; i++)
		{
			for (int j = 0; j < w; j++)
			{
				outl << phasel[index*w + j] <<endl;
				outr << phaser[index*w + j] <<endl;
			}
			//outl << endl;
			//outr << endl;
		}
	}

	clock_t end = clock();
	cout << "写文件耗时：" << end - start << "ms" << endl;
	return 0;
}
//进一步减少无效相位
int gxbUnpackPhase::gxbReduceUselessPhase(double *&finalPhase, double *cp_abPhase1, double *cp_abPhase2, double *&Diff1, double *&Diff2)
{

	for (int p = 0; p < W; p++)
	{
		if (cp_abPhase1[p] < 75 && p < W - 1)
		{
			Diff1[p] = cp_abPhase1[p + 1] - cp_abPhase1[p];
			if (cp_abPhase1[p + 1] - cp_abPhase1[p] > -6 && cp_abPhase1[p + 1] - cp_abPhase1[p] < 0)
			{
				finalPhase[p] = -4;
				cout << 1;
			}
		}
		if (cp_abPhase2[p] < 82 && p < W - 1)
		{
			Diff2[p] = cp_abPhase2[p + 1] - cp_abPhase2[p];
			if (cp_abPhase2[p + 1] - cp_abPhase2[p] > -6 && cp_abPhase2[p + 1] - cp_abPhase2[p] < 0)
				finalPhase[p] = -4;
		}
	}

	string name1 = "myphase/diff1.txt";
	string name2 = "myphase/diff2.txt";
	ofstream out1, out2;
	out1.open(name1.c_str(), ofstream::out);
	out2.open(name2.c_str(), ofstream::out);
	for (int j = 0; j < W; j++)
	{
		out1 << Diff1[j] << endl;
		out2 << Diff2[j] << endl;
	}
	return 0;
}
