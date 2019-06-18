#include "CameraApi.h"
#include <iostream>
class Camera
{
	int Width = 2048;
	int Height = 1536;
public:
	tSdkCameraDevInfo  pCameraList[2];
	int piNums = 2;
	CameraHandle pCameraHandle[2];
	int OpenCamera()
	{
		std::cout << "初始化sdk(中文模式):" << CameraSdkInit(1) << std::endl;//初始化sdk(中文模式)
		std::cout << "枚举设备:" << CameraEnumerateDevice(pCameraList, &piNums) << std::endl;//枚举设备
		std::cout << "初始化相机:" << CameraInit(&pCameraList[0], -1, -1, &pCameraHandle[0]) << std::endl;//初始化相机
		std::cout << "初始化相机:" << CameraInit(&pCameraList[1], -1, -1, &pCameraHandle[1]) << std::endl;//初始化相机
		std::cout << "左相机进入图像采集模式:" << CameraPlay(pCameraHandle[0]) << std::endl;//左相机进入图像采集模式
		std::cout << "右相机进入图像采集模式:" << CameraPlay(pCameraHandle[1]) << std::endl;//右相机进入图像采集模式
		return 1;
	}
	int CloseCamera()
	{
		CameraUnInit(pCameraHandle[0]); //逆初始化
		CameraUnInit(pCameraHandle[1]);//逆初始化
		return 1;
	}
};