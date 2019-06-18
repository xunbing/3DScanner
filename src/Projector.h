#pragma once
#include "stdafx.h"
#include "CameraApi.h"
#include <opencv2\opencv.hpp>
using namespace cv;
double PI = 3.14159265358979;
class Projector
{
private:
	int p1 = 16;
	int p2 = 17;
	int p3 = 18;
public:
	int gxbCreateProjImg(int index, unsigned char*data, int proW)
	{
		int colorRange = 255;
		if (index >= 1 && index <= 5)
		{
			for (int i = 0; i < proW; i++)
			{
				int remainer1 = i % p1;
				int result1 = int((sin((2.0f * PI / p1 * remainer1 + 2.0f * PI*(index - 1) / 5.0)) + 1) / 2.0 * colorRange + 0.5f);
				data[i] = result1;
			}
		}
		else if (index >= 6 && index <= 10)
		{
			for (int i = 0; i < proW; i++)
			{
				int remainer2 = i % p2;
				int result2 = int((sin((2.0f * PI / p2 * remainer2 + 2.0f * PI*(index - 6) / 5.0)) + 1) / 2.0 * colorRange + 0.5f);
				data[i] = result2;
			}
		}
		else if (index >= 11 && index <= 15)
		{
			for (int i = 0; i < proW; i++)
			{
				int remainer3 = i % p3;
				int result3 = int((sin((2.0f * PI / p3 * remainer3 + 2.0f * PI*(index - 11) / 5.0)) + 1) / 2.0 * colorRange + 0.5f);
				data[i] = result3;
			}
		}
		else if (index == 16)
		{
			for (int i = 0; i < proW; i++)
			{
				data[i] = 0;
			}
		}
		else if (index == 17)
		{
			for (int i = 0; i < proW; i++)
			{
				data[i] = colorRange;
			}
		}
		return 0;
	}
	int gxbProAndPic(int proW, int proH, unsigned char*&proImg, unsigned char**&pImgL, unsigned char**&pImgR)
	{

		unsigned char*dataForPro = new unsigned char[proW];
		float timeExpose = 0;
		int timeFlag = 0;
		unsigned char * rgbL = 0;
		unsigned char * rgbR = 0;
		char* wndname = "Proimg";
		for (int i = 0; i < 17; i++)
		{
			gxbCreateProjImg(i + 1, dataForPro, proW);
			for (int k = 0; k < proH; k++)
			{
				for (int j = 0; j < proW; j++)
				{
					proImg[k*proW + j] = dataForPro[j];
				}
			}
			Mat proShow = Mat(proH, proW, CV_8UC1, proImg);
			//imwrite(rasname, proShow);
			namedWindow(wndname, WINDOW_AUTOSIZE);
			setWindowProperty(wndname, CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
			moveWindow(wndname, 1920, 0);
			imshow(wndname, proShow);
			if (i == 0)
				waitKey(400);
			else
				waitKey(300);


		}
		return 0;
	}
};