
// DrawMoveCircleExamDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "DrawMoveCircleExam.h"
#include "DrawMoveCircleExamDlg.h"
#include "afxdialogex.h"
#include <cmath>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/*
//링커 지시자 추가하자, 콘솔 사용하기 위해
#ifdef _DEBUG
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#endif
*/


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
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


// CDrawMoveCircleExamDlg dialog



CDrawMoveCircleExamDlg::CDrawMoveCircleExamDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DRAWMOVECIRCLEEXAM_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_isDragging = false;
	m_borderThickness = 2;
	m_largeCircleRadius = 0;
	m_triangleAreaFactor = 0.0F;
}

void CDrawMoveCircleExamDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDrawMoveCircleExamDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()


// CDrawMoveCircleExamDlg message handlers

BOOL CDrawMoveCircleExamDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	SetWindowText(_T("(주)그림 SW 직군 지원 과제 1-1 (작성자 : 박시현)"));

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDrawMoveCircleExamDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDrawMoveCircleExamDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		//CDialogEx::OnPaint();

		//checkMe!!!!
		CPaintDC dc(this);
		DrawBuffer(&dc);

	}
}


void CDrawMoveCircleExamDlg::DrawBuffer(CDC* pDC) {
	CRect rect;
	GetClientRect(&rect);

	//더블 버퍼링 설정
	CDC memDC;
	CBitmap bitmap;
	memDC.CreateCompatibleDC(pDC);
	bitmap.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = memDC.SelectObject(&bitmap);

	//배경 초기화
	memDC.FillSolidRect(&rect, RGB(255, 255, 255));

	//작은원 그리기
	static COLORREF colors[] = { RGB(127, 255, 0), RGB(175, 238, 238), RGB(255, 255, 0) };
	for (size_t i = 0; i < m_points.size(); ++i) {
		DrawCircle(&memDC, m_points[i], colors[i], 15, i);
	}


	//큰 원 그리기
	if (m_points.size() == 3) {
		CalculateLargeCircle();
		if (m_largeCircleRadius > 0) {
			DrawLargeCircle(&memDC);
		}
	}

	CRect updateRect;
	if (!GetUpdateRect(&updateRect, FALSE)) {
		updateRect = rect; //전체복사
	}

	pDC->BitBlt(
		updateRect.left, updateRect.top,
		updateRect.Width(), updateRect.Height(),
		&memDC, updateRect.left, updateRect.top, SRCCOPY
	);

	//원 정보 출력
	if (m_points.size() >= 3) {
		CString s;
		s.Format(_T("A:(%d,%d) B:(%d,%d) C:(%d,%d)"),
			m_points[0].x, m_points[0].y,
			m_points[1].x, m_points[1].y,
			m_points[2].x, m_points[2].y
		);
		DrawCircleInfo(pDC, 10, rect.Height() - 30, s, RGB(25, 25, 112), 20, FW_MEDIUM);
	}

	memDC.SelectObject(pOldBitmap);
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDrawMoveCircleExamDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CDrawMoveCircleExamDlg::DrawCircle(CDC* pDC, CPoint point, COLORREF color, int radius, int index) {
	CBrush brush(color);
	CBrush* pOldBrush = pDC->SelectObject(&brush);
	pDC->Ellipse(point.x - radius, point.y - radius, point.x + radius, point.y + radius);
	pDC->SelectObject(pOldBrush);

	//A, B, C 출력
	static TCHAR* labels[] = { _T("A"), _T("B"), _T("C") };
	if (index >= 0 && index < 3) {
		pDC->SetTextColor(RGB(100, 100, 100));
		pDC->SetBkMode(TRANSPARENT);
		//pDC->TextOut(point.x -5, point.y - 8, labels[index]);

		CFont font;
		font.CreateFont(
			18,                        // Height (픽셀 크기)
			0,                         // Width (0이면 기본 비율)
			0,                         // Escapement
			0,                         // Orientation
			FW_BOLD,                   // 굵기 (FW_BOLD)
			FALSE,                     // Italic
			FALSE,                     // Underline
			FALSE,                     // StrikeOut
			DEFAULT_CHARSET,           // Charset
			OUT_DEFAULT_PRECIS,        // OutPrecision
			CLIP_DEFAULT_PRECIS,       // ClipPrecision
			DEFAULT_QUALITY,           // Quality
			DEFAULT_PITCH | FF_SWISS,  // Pitch and Family
			_T("Arial"));              // Font name

		CFont* pOldFont = pDC->SelectObject(&font);
		pDC->TextOutW(point.x, point.y - 8, labels[index]);
		pDC->SelectObject(pOldFont);
	}
}

void CDrawMoveCircleExamDlg::DrawLargeCircle(CDC* pDC) {
	CRect rect;
	GetClientRect(&rect);

	if (m_largeCircleRadius == 0) {
		fprintf(stderr, "radius == 0\n");
		return;
	}

	int radius = m_largeCircleRadius;

	// 큰 원 그리기
	CPen pen(PS_SOLID, m_borderThickness, RGB(0, 0, 0));
	CPen* pOldPen = pDC->SelectObject(&pen);
	pDC->SelectStockObject(HOLLOW_BRUSH);
	pDC->Ellipse(m_largeCircleCenter.x - radius, m_largeCircleCenter.y - radius, m_largeCircleCenter.x + radius, m_largeCircleCenter.y + radius);
	pDC->SelectObject(pOldPen);
}



void CDrawMoveCircleExamDlg::CalculateLargeCircle() {
	if (m_points.size() != 3)
		return;

	// 외접원의 중심과 반지름 계산
	CPoint a = m_points[0];
	CPoint b = m_points[1];
	CPoint c = m_points[2];

	double D = 2.0 * ((a.x * (b.y - c.y)) + (b.x * (c.y - a.y)) + (c.x * (a.y - b.y)));
	m_triangleAreaFactor = D;

	// 세 점이 정확히 일직선일 경우
	if (D == 0) {
		m_largeCircleRadius = 0; //반지름을 0으로 설정
		fprintf(stdout, "Three points are exactly loacted in a straight line!!!\n");
		return;
	}

	/*
	//마우스 드래그로 도저히 이값을 가져올 수 없네.....
	if (fabs(D) < 0.01F) { 
		fprintf(stderr, "fabs(D) = %.2f < 15000.0\n", abs(D));
		m_largeCircleRadius = 0; //반지름을 0으로 설정
		return;
	}
	*/

	double ux = ((a.x * a.x + a.y * a.y) * (b.y - c.y) + (b.x * b.x + b.y * b.y) * (c.y - a.y) + (c.x * c.x + c.y * c.y) * (a.y - b.y)) / D;
	double uy = ((a.x * a.x + a.y * a.y) * (c.x - b.x) + (b.x * b.x + b.y * b.y) * (a.x - c.x) + (c.x * c.x + c.y * c.y) * (b.x - a.x)) / D;

	// 중심 및 반지름 업데이트
	m_largeCircleCenter = CPoint(static_cast<int>(ux), static_cast<int>(uy));
	m_largeCircleRadius = static_cast<int>(
		sqrt((m_largeCircleCenter.x - a.x) * (m_largeCircleCenter.x - a.x) +
			(m_largeCircleCenter.y - a.y) * (m_largeCircleCenter.y - a.y)));

	/*
	// 결과 디버깅 로그 출력
	fprintf(stderr, "Large Circle: Center=(%d, %d), Radius=%d, fabs(D)=%.4lf\n",
		m_largeCircleCenter.x, m_largeCircleCenter.y, m_largeCircleRadius, fabs(D));
	*/
}

void CDrawMoveCircleExamDlg::DrawCircleInfo(CDC* pDC, int x, int y, CString text, COLORREF textColor, int fontSize, int fontWeight) {
	// 텍스트 색상 설정
	pDC->SetTextColor(textColor);
	pDC->SetBkMode(TRANSPARENT);

	// 폰트 설정
	CFont font;
	BOOL fontCreated = font.CreateFont(
		fontSize,
		0,
		0,
		0,
		fontWeight,
		FALSE,
		FALSE,
		FALSE,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS,
		_T("Arial")
	);

	if (!fontCreated) {
		fprintf(stderr, "Font creation failed!");
		return;
	}

	CFont* pOldFont = pDC->SelectObject(&font);

	pDC->TextOut(x, y, text);

	// 원래 폰트로 되돌리기
	pDC->SelectObject(pOldFont);
}

void CDrawMoveCircleExamDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (m_points.size() < 3) {
		m_points.push_back(point);
	}
	else if (m_points.size() == 3) {
		CRect circleRect(m_points[2].x - 15, m_points[2].y - 15, m_points[2].x + 15, m_points[2].y + 15);
		if (circleRect.PtInRect(point)) {
			m_isDragging = true;
			m_draggedPoint = point;
		}
	}
	Invalidate();

	CDialogEx::OnLButtonDown(nFlags, point);
}


void CDrawMoveCircleExamDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (m_isDragging) {
		m_isDragging = false;
		m_points[2] = m_draggedPoint;

		fprintf(stderr, "Large Circle: Center=(%d, %d), Radius=%d, fabs(D)=%.4lf\n",
			m_largeCircleCenter.x, m_largeCircleCenter.y, m_largeCircleRadius, fabs(m_triangleAreaFactor));

		Invalidate();
		UpdateWindow();
	}

	CDialogEx::OnLButtonUp(nFlags, point);
}


void CDrawMoveCircleExamDlg::OnMouseMove(UINT nFlags, CPoint point) {
	// TODO: Add your message handler code here and/or call default

	/*
	//flickering 현상 발생
	if (m_isDragging) {
		m_draggedPoint = point;
		Invalidate();
	}
	*/

	//마우스가 드래그 되어 움질일때 마우스 위치가 변경된 경우만 처리
	if (m_isDragging) {
		if (m_draggedPoint != point) {
			//이전 영역과 현재 영역 무효화
			CRect oldRect(m_draggedPoint.x - 20, m_draggedPoint.y - 20, m_draggedPoint.x + 20, m_draggedPoint.y + 20);
			CRect newRect(point.x - 20, point.y - 20, point.x + 20, point.y + 20);
			m_draggedPoint = point;
		}

		//큰원 계산과 동기화
		if (m_points.size() == 3) {
			m_points[2] = m_draggedPoint; // 작은 원 위치 업데이트
		}
		UpdateWindow(); //즉시 다시 그리기
	}

	CDialogEx::OnMouseMove(nFlags, point);
}
