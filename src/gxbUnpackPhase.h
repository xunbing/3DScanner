#pragma once
class gxbUnpackPhase
{

	int W = 2048;//照片宽度
	int H = 1536;//照片高度
	int nstep = 5;//相移步数
	double PI = 3.14159265358979;
	///针对1280pixel幅面的相机
	//int p1 = 13;
	//int p2 = 14;
	//int p3 = 15;
	///针对2048pixel幅面的相机
	int p1 = 16;
	int p2 = 17;
	int p3 = 18;

public:
	//
	int gxbInit(int w, int h, int step);
	int gxbGetWrappedPhase(int step, unsigned char **&Arrayofphase, double *PHASE, int size, int index, int row);
	int gxbPhaseResult(unsigned char **&Img, double *phase);
	int gxbGetPhase(unsigned char**img, double *phase);
	int gxbPhaseflitter(double *PHASE);
	int gxbRemoveBg(unsigned char *bgImgb, unsigned char *bgImgw, double *PHASE);
	int gxbRegionGrow(unsigned char**&pImg, double *Phase);
	int writemyphase(int index, int w, int h, double *phasel, double *phaser);
	int gxbReduceUselessPhase(double *&finalPhase, double *cp_abPhase1, double *cp_abPhase2, double *&Diff1, double *&Diff2);

};