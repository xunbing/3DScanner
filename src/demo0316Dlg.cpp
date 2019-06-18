
// demo0316Dlg.cpp : 实现文件
//

#include "stdafx.h"
#include "demo0316.h"
#include "demo0316Dlg.h"
#include "afxdialogex.h"
#include "VtkViewer.h"
#include "CvvImage.h"
#include "Camera.h"
#include "StereoCalib.h"
#include "gxbPhaseMatch.h"
#include "gxbUnpackPhase.h"
#include "Projector.h"
#include <opencv2\opencv.hpp>
using namespace cv;
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

//用于应用程序“相机标定”项的 CCalibDlg 标定对话框
//j为传递图片流的公共下标
static int j = 0;
//IsCalibflag是否标定的标志位，默认为已标定
static bool IsCalibflag = 1;
class CCalibDlg : public CDialogEx
{
public:
	int Width = 2048;
	int Height = 1536;
	int ImgNumforCalib = 23;
	int m_NumOfCalib;
	unsigned char**ArrayImgforCalib = new unsigned char*[ImgNumforCalib * 2];
	CCalibDlg();
	Mat imgLeft, imgRight;
	unsigned char * dataL = new unsigned char[Width*Height * 3];
	unsigned char * dataR = new unsigned char[Width*Height * 3];
	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_CALIB };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
														// 实现
protected:
	DECLARE_MESSAGE_MAP()
public:
	void DrawPicToHDC(IplImage*img, UINT ID);//在picControl控件中显示图片
	afx_msg void OnBnClickedBtnOpencam();
	afx_msg void OnBnClickedBtnCatch();
	afx_msg void OnBnClickedBtnClosecam();
	afx_msg void OnBnClickedBtnCalib();
	
	afx_msg void OnBnClickedBtnSinglecalibleft();
	afx_msg void OnBnClickedBtnSinglecalibright();
	double m_RMS;
	double m_AEE;
};
void CCalibDlg::DrawPicToHDC(IplImage*img, UINT ID)
{
	CDC *pDC = GetDlgItem(ID)->GetDC();
	HDC hDC = pDC->GetSafeHdc();
	CRect rect;
	GetDlgItem(ID)->GetClientRect(&rect);
	CvvImage cimg;
	cimg.CopyOf(img); 
	cimg.DrawToHDC(hDC, &rect); 
	ReleaseDC(pDC);
}
CCalibDlg::CCalibDlg() : CDialogEx(IDD_DIALOG_CALIB)
, m_NumOfCalib(0)
, m_RMS(0)
, m_AEE(0)
{
	
}

void CCalibDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_CAMNUM, m_NumOfCalib);
	DDX_Text(pDX, IDC_EDIT_RMS, m_RMS);
	DDX_Text(pDX, IDC_EDIT_AEE, m_AEE);
}

BOOL CCalibDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。


	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作

	for (int p = 0; p < ImgNumforCalib * 2; p++)
	{
		ArrayImgforCalib[p] = new unsigned char[Width*Height];
	}
									// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

BEGIN_MESSAGE_MAP(CCalibDlg, CDialogEx)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BTN_OPENCAM, &CCalibDlg::OnBnClickedBtnOpencam)
	ON_BN_CLICKED(IDC_BTN_CATCH, &CCalibDlg::OnBnClickedBtnCatch)
	ON_BN_CLICKED(IDC_BTN_CLOSECAM, &CCalibDlg::OnBnClickedBtnClosecam)
	ON_BN_CLICKED(IDC_BTN_CALIB, &CCalibDlg::OnBnClickedBtnCalib)
	ON_BN_CLICKED(IDC_BTN_SINGLECALIBLEFT, &CCalibDlg::OnBnClickedBtnSinglecalibleft)
	ON_BN_CLICKED(IDC_BTN_SINGLECALIBRIGHT, &CCalibDlg::OnBnClickedBtnSinglecalibright)
END_MESSAGE_MAP()

//用于应用程序“显示点云”项的 CCCheckCloud 标定对话框

class CCCheckCloud : public CDialogEx
{
public:
	CCCheckCloud();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_CHECK };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

														// 实现
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg void OnBnClickedBtnOpencloud();
	std::vector<Pointf> pointCloud[1];
	int PointSize;
	int Editponitsize;
	CVtkViewer m_vtk2;
	char ln[1000];
	FILE *f;
	int n, L;
	double d1, d2, d3, d4, d5, d6;
	CString Editfilepath;
};

CCCheckCloud::CCCheckCloud() : CDialogEx(IDD_DIALOG_CHECK)
{
}

void CCCheckCloud::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//DDX_Text(pDX, IDC_EDITshowsize, Editponitsize);
	DDX_Control(pDX, IDC_PIC_CHECK, m_vtk2);
	//DDX_Text(pDX, IDC_EDITshowpath, Editfilepath);
}
void CCCheckCloud::OnPaint()
{
	CClientDC dc(this);
	CRect  Rect;
	GetClientRect(&Rect);
	GetDlgItem(IDC_PIC_CHECK)->GetWindowRect(&Rect);
	m_vtk2.ReadPointCloud(pointCloud[0]);
	//m_vtk2.MoveWindow(Rect);
	m_vtk2.ShowWindow(SW_SHOW);

	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;


		//m_vtk2.ReadPointCloud(pointCloud[0]);
		//m_vtk2.MoveWindow(rect);
		//m_vtk2.ShowWindow(SW_SHOW);

		// 绘制图标
	}
	else
	{
		CDialogEx::OnPaint();
	}
}
BEGIN_MESSAGE_MAP(CCCheckCloud, CDialogEx)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BTN_OPENCLOUD, &CCCheckCloud::OnBnClickedBtnOpencloud)
END_MESSAGE_MAP()
// Cdemo0316Dlg 对话框
Cdemo0316Dlg::Cdemo0316Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DEMO0316_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void Cdemo0316Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(Cdemo0316Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_OpenCalibWnd, &Cdemo0316Dlg::OnBnClickedOpencalibwnd)
	ON_BN_CLICKED(IDC_BTN_CHECK, &Cdemo0316Dlg::OnBnClickedBtnCheck)
	ON_BN_CLICKED(IDC_BTN_SCAN, &Cdemo0316Dlg::OnBnClickedBtnScan)
END_MESSAGE_MAP()


// Cdemo0316Dlg 消息处理程序

BOOL Cdemo0316Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void Cdemo0316Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void Cdemo0316Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
HCURSOR Cdemo0316Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
//打开标定对话框
void Cdemo0316Dlg::OnBnClickedOpencalibwnd()
{
	// TODO: 在此添加控件通知处理程序代码
	CCalibDlg ccalibDlg;
	ccalibDlg.DoModal();
}
//打开查看点云对话框
void Cdemo0316Dlg::OnBnClickedBtnCheck()
{
	// TODO: 在此添加控件通知处理程序代码
	CCCheckCloud cccheckDlg;
	cccheckDlg.DoModal();
}
//打开点云文件
void CCCheckCloud::OnBnClickedBtnOpencloud()
{
	// TODO: 在此添加控件通知处理程序代码
	pointCloud[0].clear();
	CString filter;
	filter = "所有文件(*.*)||";
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filter, NULL);
	if (dlg.DoModal() == IDOK)
	{
		CString filepathname;
		filepathname = dlg.GetPathName();
		Editfilepath = filepathname;
		MessageBox(Editfilepath);
	}
	CString fileinfo = Editfilepath;
	int len = WideCharToMultiByte(CP_ACP, 0, fileinfo, -1, NULL, 0, NULL, NULL);
	char *path = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, fileinfo, -1, path, len, NULL, NULL);
	f = fopen(path, "r");
	n = 0;
	Pointf cloud;
	while (1)
	{
		if (NULL == fgets(ln, 1000, f))
			break;
		L = strlen(ln);
		if ('\n' == ln[L - 1])
		{
			n++;
			sscanf(ln, "%lf%lf%lf", &d1, &d2, &d3);
			cloud.x = d1;
			cloud.y = d2;
			cloud.z = d3;
			cloud.r = 255;
			cloud.g = 255;
			cloud.b = 255;
			pointCloud[0].push_back(cloud);
			//cout << d1 << " " << d2 << " " << d3 << " " << d4 << " " << d5 << " " << d6 << endl;
		}
	}
	fclose(f);
	PointSize = pointCloud[0].size();
	SetDlgItemInt(IDC_EDIT_SIZE, pointCloud[0].size());
	//SetDlgItemText(IDC_EDITshowpath, Editfilepath);
	UpdateData();
	Invalidate();
}

Camera Cam;
StereoCalibrator scb;
//打开相机
void CCalibDlg::OnBnClickedBtnOpencam()
{
	// TODO: 在此添加控件通知处理程序代码
	Cam.OpenCamera();
	MessageBox(L"相机已打开");

}
//拍照
void CCalibDlg::OnBnClickedBtnCatch()
{

	char picNamel[50];
	char picNamer[50];
	if (j < ImgNumforCalib*2 - 1)
	{
		dataL = CameraGetImageBufferEx(Cam.pCameraHandle[0], &Width, &Height, 2000);
		dataR = CameraGetImageBufferEx(Cam.pCameraHandle[1], &Width, &Height, 2000);
		imgLeft = Mat(Height, Width, CV_8UC3, dataL);
		imgRight = Mat(Height, Width, CV_8UC3, dataR);
		cvtColor(imgLeft, imgLeft, CV_RGB2GRAY);
		cvtColor(imgRight, imgRight, CV_RGB2GRAY);
		IplImage imgTmpL = imgLeft, imgTmpR = imgRight;
		IplImage *IplimgLeft = cvCloneImage(&imgTmpL);
		IplImage *IplimgRight = cvCloneImage(&imgTmpR);
		DrawPicToHDC(IplimgLeft, IDC_PIC_CALIB_LEFT);
		DrawPicToHDC(IplimgRight, IDC_PIC_CALIB_RIGHT);
		sprintf(picNamel, "./board/left%d.bmp", j/2);
		sprintf(picNamer, "./board/right%d.bmp", j/2);
		imwrite(picNamel, imgLeft);
		imwrite(picNamer, imgRight);
		memcpy(ArrayImgforCalib[j], imgLeft.data, Height*Width);
		memcpy(ArrayImgforCalib[j + 1], imgRight.data, Height*Width);
		j += 2;
		m_NumOfCalib = j/2;
		SetDlgItemInt(IDC_EDIT_CAMNUM, m_NumOfCalib);
		UpdateData();
	}
	else
	{
		MessageBox(L"已达要求抓拍数");
	}
	
}
//关闭相机
void CCalibDlg::OnBnClickedBtnClosecam()
{
	// TODO: 在此添加控件通知处理程序代码
	Cam.CloseCamera();
	MessageBox(L"相机已关闭");
}
//双目标定
void CCalibDlg::OnBnClickedBtnCalib()
{
	// TODO: 在此添加控件通知处理程序代码
	
	Size boardSize;

	boardSize.width = 10;
	boardSize.height = 8;
	float squareSize = 10.;
	scb.StereoCalib(ArrayImgforCalib, ImgNumforCalib * 2, boardSize, squareSize, true, true, true);
	m_AEE = scb.getAee();
	m_RMS = scb.getRMSError();
	IsCalibflag = 0;
	UpdateData(FALSE);
	Cam.CloseCamera();
	MessageBox(L"标定已完成");
	//while (1)
	//{
	//	dataL = CameraGetImageBufferEx(Cam.pCameraHandle[0], &Width, &Height, 2000);
	//	dataR = CameraGetImageBufferEx(Cam.pCameraHandle[1], &Width, &Height, 2000);
	//	imgLeft = Mat(Height, Width, CV_8UC3, dataL);
	//	imgRight = Mat(Height, Width, CV_8UC3, dataR);
	//	cvtColor(imgLeft, imgLeft, CV_RGB2GRAY);
	//	cvtColor(imgRight, imgRight, CV_RGB2GRAY);
	//	remap(imgLeft, imgLeft, scb.rmap[0][0], scb.rmap[0][1], INTER_LINEAR);
	//	remap(imgRight, imgRight, scb.rmap[1][0], scb.rmap[1][1], INTER_LINEAR);
	//	IplImage imgTmpL = imgLeft, imgTmpR = imgRight;
	//	IplImage *IplimgLeft = cvCloneImage(&imgTmpL);
	//	IplImage *IplimgRight = cvCloneImage(&imgTmpR);
	//	DrawPicToHDC(IplimgLeft, IDC_PIC_CALIB_LEFT);
	//	DrawPicToHDC(IplimgRight, IDC_PIC_CALIB_RIGHT);
	//	char c = (char)waitKey(1000);
	//	if (c == 27 || c == 'q' || c == 'Q')

	//	{
	//		Cam.CloseCamera();
	//		MessageBox(L"相机已关闭");
	//		break;
	//	}
	//}
}
//扫描
void Cdemo0316Dlg::OnBnClickedBtnScan()
{
	// TODO: 在此添加控件通知处理程序代码
	Size imageSize = Size(2048, 1536);
	Cam.OpenCamera();
	Mat cameraMatrix[2], distCoeffs[2];
	Mat R1, R2, P1, P2, Q, T;
	if (IsCalibflag == 1)
	{
		string intrinsics_filename = "intrinsics.yml";
		string extrinsics_filename = "extrinsics.yml";
		if (!intrinsics_filename.empty())
		{
			// reading intrinsic parameters
			FileStorage fs(intrinsics_filename, FileStorage::READ);
			if (!fs.isOpened())
			{
				printf("Failed to open file %s\n", intrinsics_filename.c_str());
				//return -1;
			}


			fs["ML"] >> cameraMatrix[0];
			fs["MR"] >> cameraMatrix[1];
			fs["DL"] >> distCoeffs[0];
			fs["D"] >> distCoeffs[1];
			distCoeffs[0] = Mat::ones(0, 5, CV_32F);
			distCoeffs[1] = Mat::ones(0, 5, CV_32F);
		}
		if (!extrinsics_filename.empty())
		{
			// reading intrinsic parameters
			FileStorage fs(extrinsics_filename, FileStorage::READ);
			if (!fs.isOpened())
			{
				printf("Failed to open file %s\n", extrinsics_filename.c_str());
				//return -1;
			}


			fs["R1"] >> R1;
			fs["P1"] >> P1;
			fs["P2"] >> P2;
			fs["R2"] >> R2;
			fs["T"] >> T;
			fs["Q"] >> Q;
		}
		initUndistortRectifyMap(cameraMatrix[0], distCoeffs[0], R1, P1, imageSize, CV_16SC2, scb.rmap[0][0], scb.rmap[0][1]);
		initUndistortRectifyMap(cameraMatrix[1], distCoeffs[1], R2, P2, imageSize, CV_16SC2, scb.rmap[1][0], scb.rmap[1][1]);

	}

	Projector Pro;
	Mat imgLeft, imgRight;
	gxbUnpackPhase Unphase;
	int Width = 2048;
	int Height = 1536;
	int proW = 1280;
	int proH = 960;
	unsigned char *ProImg = new unsigned char[Width*Height];
	unsigned char **pImgL = new unsigned char*[17];
	unsigned char **pImgR = new unsigned char*[17];
	char *picNamel = new char[20];
	char *picNamer = new char[20];

	for (int i = 0; i < 17; i++)
	{
		pImgL[i] = new unsigned char[Width*Height];
		pImgR[i] = new unsigned char[Width*Height];
	}
	unsigned char*dataForPro = new unsigned char[proW];
	float timeExpose = 0;
	int timeFlag = 0;
	unsigned char * rgbL = 0;
	unsigned char * rgbR = 0;
	char* wndname = "Proimg";
	for (int i = 0; i < 17; i++)
	{
		Pro.gxbCreateProjImg(i + 1, dataForPro, proW);
		for (int k = 0; k < proH; k++)
		{
			for (int j = 0; j < proW; j++)
			{
				ProImg[k*proW + j] = dataForPro[j];
			}
		}
		Mat proShow = Mat(proH, proW, CV_8UC1, ProImg);
		//imwrite(rasname, proShow);
		namedWindow(wndname, WINDOW_AUTOSIZE);
		setWindowProperty(wndname, CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
		moveWindow(wndname, 1920, 0);
		imshow(wndname, proShow);
		if (i == 0)
			waitKey(800);
		else
			waitKey(800);
		dataL = CameraGetImageBufferEx(Cam.pCameraHandle[0], &Width, &Height, 2000);
		dataR = CameraGetImageBufferEx(Cam.pCameraHandle[1], &Width, &Height, 2000);

		imgLeft = Mat(Height, Width, CV_8UC3, dataL);
		imgRight = Mat(Height, Width, CV_8UC3, dataR);
		cvtColor(imgLeft, imgLeft, CV_RGB2GRAY);
		cvtColor(imgRight, imgRight, CV_RGB2GRAY);
		if(i==1)
		imwrite("imgLeft.bmp", imgLeft);
		//双目校准
		remap(imgLeft, imgLeft, scb.rmap[0][0], scb.rmap[0][1], INTER_LINEAR);
		remap(imgRight, imgRight, scb.rmap[1][0], scb.rmap[1][1], INTER_LINEAR);
		sprintf(picNamel, "./picture/L%d.bmp", i);
		sprintf(picNamer, "./picture/R%d.bmp", i);
		imwrite(picNamel, imgLeft);
		imwrite(picNamer, imgRight);
		memcpy(pImgL[i], imgLeft.data, Height*Width);
		memcpy(pImgR[i], imgRight.data, Height*Width);

		//IplImage imgTmpL = imgLeft, imgTmpR = imgRight;
		//IplImage *IplimgLeft = cvCloneImage(&imgTmpL);
		//IplImage *IplimgRight = cvCloneImage(&imgTmpR);
		//IplImage *img_left_Change, *img_right_Change;
		//img_left_Change = cvCloneImage(IplimgLeft);
		//img_right_Change = cvCloneImage(IplimgRight);
		//memcpy_s(pImgL[i], img_left_Change->width * img_left_Change->height, img_left_Change->imageData, img_left_Change->width * img_left_Change->height);
		//memcpy_s(pImgR[i], img_right_Change->width * img_right_Change->height, img_right_Change->imageData, img_right_Change->width * img_right_Change->height);

		imgLeft = Mat(Height, Width, CV_8UC1, pImgL[i]);
		imgRight = Mat(Height, Width, CV_8UC1, pImgR[i]);
		resize(imgLeft, imgLeft, Size(640, 512));
		namedWindow("imgLeft", WINDOW_AUTOSIZE);
		setWindowProperty("imgLeft", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
		moveWindow("imgLeft", 320, 284);
		imshow("imgLeft", imgLeft);
		resize(imgRight, imgRight, Size(640, 512));
		namedWindow("imgRight", WINDOW_AUTOSIZE);
		setWindowProperty("imgRight", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
		moveWindow("imgRight", 960, 284);
		imshow("imgRight", imgRight);
		waitKey(300);

	}
	destroyAllWindows();
	Unphase.gxbInit(Width, Height, 5);
	Unphase.gxbGetPhase(pImgL, phaseL);
	Unphase.gxbGetPhase(pImgR, phaseR);
	Unphase.writemyphase(600, Width, Height, phaseL, phaseR);
	gxbPhaseMatch PM;
	double *D = new double[Width*Height];
	memset(D, 0, Width*Height);
	PM.phaseMatch(Width, Height, phaseL, phaseR, D);
	Mat XYZ;
	Mat shichaImg = Mat::zeros(Height, Width, CV_8UC1);

	cout << "正在生成视差图..." << endl;
	for (int k = 0; k < Height; k++)
	{
		for (int j = 0; j < Width; j++)
		{
			//shichaImg.data[k*Width + j] = (int)((D[k*Width + j]) / 2048.0 * 255);
			shichaImg.data[k*Width + j] = (int)(D[k*Width + j]);
		}
	}
	//reprojectImageTo3D(shichaImg, XYZ, Q, true, -1);
	//
	//ofstream out;
	//out.open("cloud.txt", ofstream::out);
	//for (int y = 10; y < Height; y++)
	//{
	//	//double qx = q[0][1] * y + q[0][3], qy = q[1][1] * y + q[1][3];
	//	//double qz = q[2][1] * y + q[2][3], qw = q[3][1] * y + q[3][3];
	//	for (int x = 0; x < Width; x++)
	//	{

	//		
	//		{
	//			double Z = XYZ.at<cv::Vec3f>(y,x)[2];
	//			double X = XYZ.at<cv::Vec3f>(y, x)[0];
	//			double Y = XYZ.at<cv::Vec3f>(y, x)[1];;
	//			if(Z!=INFINITY&&Y != -INFINITY&&X != -INFINITY&&Z != -INFINITY&&Y != INFINITY&&X != INFINITY)
	//				out << X << " " << Y << " " << Z << endl;
	//		}
	//	}
	//}


	if(IsCalibflag==1)
		PM.createPointCloud(Width, Height, D, "cloud.txt",cameraMatrix[0].at<double>(0,0), cameraMatrix[0].at<double>(1, 1), T.at<double>(0, 0),Q);
	else
		PM.createPointCloud(Width, Height, D, "cloud.txt", scb.cameraMatrix[0].at<double>(0, 0), scb.cameraMatrix[0].at<double>(1, 1), scb.T.at<double>(0, 0),scb.Q);

	resize(shichaImg, shichaImg, Size(Width / 2, Height / 2));
	imshow("shicha", shichaImg);
	//imshow("XYZ",XYZ);
	imwrite("shicha.bmp", shichaImg);
	waitKey(-1);
	Cam.CloseCamera();
}

//标定左相机
#include "CameraCalibrator.h"
void CCalibDlg::OnBnClickedBtnSinglecalibleft()
{
	// TODO: 在此添加控件通知处理程序代码
	CameraCalibrator Cc;
	unsigned char **ArrayImgforCalib_left = new unsigned char*[ImgNumforCalib];
	for (int p = 0; p < ImgNumforCalib; p++)
	{
		ArrayImgforCalib_left[p] = new unsigned char[Width*Height];
	}
	for (int i = 0; i < ImgNumforCalib*2; i = i + 2)
	{
		memcpy(ArrayImgforCalib_left[i / 2], ArrayImgforCalib[i],Height*Width);
	}
	cv::Size boardSize(10, 8);
	Cc.addChessboardPoints(ArrayImgforCalib_left, Height, Width, ImgNumforCalib , boardSize);
	Cc.calibrate(Size(Height, Width));

	cv::Mat cameraMatrix = Cc.getCameraMatrix();
	cv::Mat disCoeffs = Cc.getDistCoeffs();
	FileStorage fs("cameraL.yml", FileStorage::WRITE);
	if (fs.isOpened())
	{
		fs << "cameraL_matrix" << cameraMatrix << "distortionL_coefficients" << disCoeffs;
		fs.release();
	}
	MessageBox(L"左相机标定完成，结果保存至cameraL.yml");
}

//标定右相机
void CCalibDlg::OnBnClickedBtnSinglecalibright()
{
	// TODO: 在此添加控件通知处理程序代码
	CameraCalibrator Cc;
	unsigned char **ArrayImgforCalib_right = new unsigned char*[ImgNumforCalib];
	for (int p = 0; p < ImgNumforCalib; p++)
	{
		ArrayImgforCalib_right[p] = new unsigned char[Width*Height];
	}
	for (int i = 0; i < ImgNumforCalib*2; i = i + 2)
	{
		memcpy(ArrayImgforCalib_right[i / 2], ArrayImgforCalib[i], Height*Width);
	}
	cv::Size boardSize(10, 8);
	Cc.addChessboardPoints(ArrayImgforCalib_right, Height, Width, ImgNumforCalib, boardSize);
	Cc.calibrate(Size(Height, Width));

	cv::Mat cameraMatrix = Cc.getCameraMatrix();
	cv::Mat disCoeffs = Cc.getDistCoeffs();
	FileStorage fs("cameraR.yml", FileStorage::WRITE);
	if (fs.isOpened())
	{
		fs << "cameraR_matrix" << cameraMatrix << "distortionR_coefficients" << disCoeffs;
		fs.release();
	}
	MessageBox(L"右相机标定完成，结果保存至cameraR.yml");
}

