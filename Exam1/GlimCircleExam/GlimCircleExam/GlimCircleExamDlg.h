
// GlimCircleExamDlg.h : header file
//

#pragma once
#include <string>
#include "vosThread.h"

using namespace std;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//#define DRAW_BG_COLOR		128 //gray
#define DRAW_BG_COLOR		100	
#define DRAW_CIRCLE_COLOR	255 //white



//DrawCircleTh Class
class DrawCircleTh : public vosThread {
public:
	DrawCircleTh();
	DrawCircleTh(void* pWnd);
	virtual ~DrawCircleTh();
public:
	void run(LPVOID pParam);
	void SetWndPointer(void* pWnd);
	DrawCircleTh& RefMe() {
		return *this;
	}
	DrawCircleTh* PointerMe() {
		return this;
	}
public:
	void* m_pWnd;
};

//CStatic control subclassing
class CColoredStatic : public CStatic {
public:
	CColoredStatic();
	virtual ~CColoredStatic();

	void SetTextColor(COLORREF color);
	void SetBkColor(COLORREF color);

protected:
	//�ؽ�Ʈ ���� ó��
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC); //��� �����
	DECLARE_MESSAGE_MAP()

private:
	COLORREF m_textColor;
	COLORREF m_bkColor;
	CBrush m_bkBrush;
};


// CGlimCircleExamDlg dialog
class CGlimCircleExamDlg : public CDialogEx
{
// Construction
public:
	CGlimCircleExamDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GLIMCIRCLEEXAM_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	CRect m_workAreaRect; //Drawing �۾� ����
	string m_modulePath; //�������� path
	CImage m_image;

public:
	int m_drawTimes;
	int m_circleX, m_circleY, m_radius;

public:
	CColoredStatic m_curDrawTimesStatic;

public:
	DrawCircleTh m_drawCircleTh;
	bool StartDrawCircleTh(void);

public:
	string GetModulePath();
	bool MakeDirectory(const string& baseDir, string dirName);
	bool DeleteDirectory(const string& path);
	bool ProgramEnvSet();

public:
	//Image ������ �Լ���...
	void DrawCircle(unsigned char* pbits, int pitch, int x_center, int y_center, int radius, CRect& rect);
	void UpdateDisplay();

	//������ Image ���� load�� �Լ���
	void FindCircle(CImage& image, int& x, int& y, int& radius);
	void DrawImage(CDC* pDC);
	void DrawCircleDetails(CDC* pDC, int image_width, int image_height);
	void PerformDoubleBuffering(CDC* pDC, const CRect& rect);
	void DrawBufferedContent(CDC* pDC, const CRect& rect);
	void DrawRadiusLine(CDC* pDC, int image_width, int image_height);
	void DrawRadiusText(CDC* pDC, int image_width, int image_height);
	int GetRadiusAngle(CDC* pDC, int image_width, int image_height);
	

	//Progress bar control�� �Լ���
	void InitProgress(void);
	void ResetProgress(void);
	void UpdateProgress(int value);

	//Text control�� �Լ���
	void InitText(void);
	void ResetText(void);
	void UpdateText(int x, int y, int r);

	//Button control�� �Լ���
	void SetButtonState(int id, BOOL enable);

public:
	//Image ���� �� drawing �۾��� �� client area�� ����ϴ� �Լ�
	CRect CalcWorkClientArea();

public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedBtnDrawcircleStart();
	afx_msg void OnBnClickedBtnDrawcirclePause();
	afx_msg void OnBnClickedBtnDrawcircleResume();
	afx_msg void OnBnClickedBtnDrawcircleStop();
	afx_msg void OnBnClickedBtnImgload();
	afx_msg void OnDestroy();
};




