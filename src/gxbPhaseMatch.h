#pragma once
#include "stdafx.h"
#include <opencv2\opencv.hpp>
#include <iostream>
#include <fstream>
#include <time.h>
using namespace std;
#include <Windows.h>
class gxbPhaseMatch
{

public:
	int phaseMatch(int W, int H, double *&phaseL, double *&phaseR, double *&D)
	{
		cout << "正在进行相位匹配..." << endl;
		for (int i = 0; i < H; i++)
		{
			for (int j = 0; j < W; j++)
			{
				for (int k = 0; k < W; k++)
				{
					if (fabs(phaseL[i*W + j] - phaseR[i*W + k]) < 0.01&&phaseL[i*W + j] != 0 && phaseR[i*W + k] != 0)
					{
						D[i*W + j] = abs(j - k);
						break;
					}
				}
			}
		}
		return 0;
	}
	int createPointCloud(int W, int H, double *&D, char *filename,double focousX,double focousY,double B,Mat Q)
	{
		cout << "正在保存点云.." << endl;
		ofstream out;
		out.open(filename, ofstream::out);
		for (int y = 0; y < H; y++)
		{
			//double qx = q[0][1] * y + q[0][3], qy = q[1][1] * y + q[1][3];
			//double qz = q[2][1] * y + q[2][3], qw = q[3][1] * y + q[3][3];
			for (int x = 0; x < W; x++)
			{

				if (D[y*W + x] > 10)
				{
					double d = D[y*W + x];

					//double Z =focousX*(-1*B) / d;
					//double X = x*Z/ focousX;
					//double Y = y*Z/ focousY;
					double Ww = (-1 * B) / d;
					double X = (x + Q.at<double>(0, 3))/Ww;
					double Y = -1*(y + Q.at<double>(1, 3))/Ww;
					double Z = Q.at<double>(2, 3) / Ww;
					
					out << X << " " << Y << " " << Z << endl;
				}
			}
		}
		return 0;
	}
};