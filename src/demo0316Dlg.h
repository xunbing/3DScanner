
// demo0316Dlg.h : 头文件
//
#include "CameraApi.h"
#pragma once


// Cdemo0316Dlg 对话框
class Cdemo0316Dlg : public CDialogEx
{
// 构造
public:
	Cdemo0316Dlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DEMO0316_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOpencalibwnd();
	afx_msg void OnBnClickedBtnCheck();

	
	int Width = 2048;
	int Height = 1536;
	double *phaseL = new double[Width*Height];
	double *phaseR = new double[Width*Height];
	unsigned char * dataL = new unsigned char[Width*Height * 3];
	unsigned char * dataR = new unsigned char[Width*Height * 3];
	

	afx_msg void OnBnClickedBtnScan();
};