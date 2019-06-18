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
	/*...........
	This part of the code is confidential and not open to the public.
	sorry!!!
	............*/
}
//求展开相位
int gxbUnpackPhase::gxbPhaseResult(unsigned char **&Img, double *phase)
{
	/*...........
	This part of the code is confidential and not open to the public.
	sorry!!!
	............*/
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
