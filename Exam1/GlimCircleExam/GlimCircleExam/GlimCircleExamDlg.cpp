
// GlimCircleExamDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "GlimCircleExam.h"
#include "GlimCircleExamDlg.h"
#include "afxdialogex.h"
#include <filesystem>
#include <iostream>
#include <random>
#include <map>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/*
//링커 지시자 추가하자, 콘솔 사용하기 위해
#ifdef _DEBUG
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#endif
*/

//for utility funtion //////////////////////////////////////////////////
std::map<UINT, CFont*> g_FontMap;

void SetControlFont(CWnd* pParent, UINT nID, CString strFontName, int nFontSize, COLORREF clrFontColor) {
	CWnd* pControl = pParent->GetDlgItem(nID);
	if (!pControl) return;

	//font setting
	LOGFONT logFont = { 0 };
	logFont.lfHeight = nFontSize;
	_tcscpy_s(logFont.lfFaceName, strFontName);

	CFont* pFont = new CFont;
	pFont->CreateFontIndirect(&logFont);

	//이전 폰트를 삭제하지 않고 맵에 저장하여 관리
	auto it = g_FontMap.find(nID);
	if (it != g_FontMap.end()) {
		//맵에 이전 폰트가 있으면 삭제
		delete it->second;
		it->second = pFont;
	}
	else {
		//맵에 새로운 폰트를 추가
		g_FontMap[nID] = pFont;
	}

	//new font set
	pControl->SetFont(pFont);


	//static control can set color for font
	if (pControl->IsKindOf(RUNTIME_CLASS(CColoredStatic))) {
		((CColoredStatic*)pControl)->SetTextColor(clrFontColor);
	}

	pControl->Invalidate();
	pControl->UpdateWindow();

	return;
}

//컨트롤이 파괴될 때 동적 폰트 객체를 삭제
void DeleteControlFont(CWnd* pParent, UINT nID) {
	auto it = g_FontMap.find(nID);
	if (it != g_FontMap.end()) {
		delete it->second;
		g_FontMap.erase(it);
	}
}

void SetControlText(CWnd* pParent, UINT nID, CString strText) {
	CWnd* pControl = pParent->GetDlgItem(nID);
	if (!pControl) return;
	pControl->SetWindowText(strText);

	pControl->Invalidate(); //다시 그리도록 요청
	pControl->RedrawWindow(); //강제로 다시 그리기
	pControl->UpdateWindow(); //즉시 갱신
}
//////////////////////////////////////////////////////////////////////


//CStatic control subclassing ///////////////////////////////////////
CColoredStatic::CColoredStatic() {
	//기본 텍스트 색상은 검정
	m_textColor = RGB(0, 0, 0);
	m_bkColor = RGB(255, 255, 255);
	m_bkBrush.CreateSolidBrush(m_bkColor); //기본 배경색 브러시 생성
}

CColoredStatic::~CColoredStatic() {
}

void CColoredStatic::SetTextColor(COLORREF color) {
	m_textColor = color; //텍스트 색상 설정
	Invalidate(); //화면 갱신 요청
}

void CColoredStatic::SetBkColor(COLORREF color) {
	m_bkColor = color; //배경색 설정
	m_bkBrush.DeleteObject(); //이전 브러시 삭제
	m_bkBrush.CreateSolidBrush(m_bkColor); //새로운 배경색 브러시 생성
	Invalidate(); //화면 갱신 
}

BEGIN_MESSAGE_MAP(CColoredStatic, CStatic)
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_ERASEBKGND() //배경을 지우는 메시지 처리
END_MESSAGE_MAP()

HBRUSH CColoredStatic::CtlColor(CDC* pDC, UINT nCtlColor) {
	pDC->SetTextColor(m_textColor); //텍스트 색상 설정
	pDC->SetBkMode(TRANSPARENT); //배경 투명 설정
	//pDC->SetBkColor(m_bkColor); //배경색 설정
	return (HBRUSH)::GetStockObject(NULL_BRUSH); //투명 배경
}

BOOL CColoredStatic::OnEraseBkgnd(CDC* pDC) {
	//배경을 지우는 작업을 수행
	CRect rect;
	GetClientRect(&rect);
	pDC->FillSolidRect(&rect, m_bkColor); //배경 색상으로 채우기
	return TRUE;
}
///////////////////////////////////////////////////////////////////////


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


// CGlimCircleExamDlg dialog


CGlimCircleExamDlg::CGlimCircleExamDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_GLIMCIRCLEEXAM_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_drawTimes = 1;
	m_modulePath = "";

	m_circleX = 0;
	m_circleY = 0;
	m_radius = 0;
}

void CGlimCircleExamDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//CStatic control ID를 CColoredStatic과 연결해주자
	DDX_Control(pDX, IDC_STATIC_CURDRAWTIMESSTATIC, m_curDrawTimesStatic);
}

BEGIN_MESSAGE_MAP(CGlimCircleExamDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_DRAWCIRCLE_START, &CGlimCircleExamDlg::OnBnClickedBtnDrawcircleStart)
	ON_BN_CLICKED(IDC_BTN_DRAWCIRCLE_PAUSE, &CGlimCircleExamDlg::OnBnClickedBtnDrawcirclePause)
	ON_BN_CLICKED(IDC_BTN_DRAWCIRCLE_RESUME, &CGlimCircleExamDlg::OnBnClickedBtnDrawcircleResume)
	ON_BN_CLICKED(IDC_BTN_DRAWCIRCLE_STOP, &CGlimCircleExamDlg::OnBnClickedBtnDrawcircleStop)
	ON_BN_CLICKED(IDC_BTN_IMGLOAD, &CGlimCircleExamDlg::OnBnClickedBtnImgload)
	ON_WM_DESTROY()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CGlimCircleExamDlg message handlers

BOOL CGlimCircleExamDlg::OnInitDialog()
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
	if (ProgramEnvSet() == false) {
		fprintf(stderr, "ProgramEnvSet fail\n");
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CGlimCircleExamDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CGlimCircleExamDlg::OnPaint()
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
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CGlimCircleExamDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BOOL CGlimCircleExamDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	
	//ESC 혹은 Return Key 입력시 다이얼로그 종료되지 않게 무시 처리 추가
	if (pMsg->message == WM_KEYDOWN) {
		switch (pMsg->wParam) {
		case VK_ESCAPE:
		case VK_RETURN:
			return TRUE;
		default:
			break;
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

string CGlimCircleExamDlg::GetModulePath() {
	char buffer[MAX_PATH] = { 0, };
	GetModuleFileNameA(NULL, buffer, MAX_PATH);

	string path(buffer);
	size_t pos = path.find_last_of("\\/");
	if (pos != string::npos) {
		path = path.substr(0, pos);
	}
	return path;
}

bool CGlimCircleExamDlg::MakeDirectory(const string& baseDir, string dirName) {
	string dirPath = baseDir + "\\" + dirName;
	if (std::filesystem::create_directory(dirPath)) {
		return true; //디렉토리 생성 성공
	}
	else if (std::filesystem::exists(dirPath)) {
		return true; //이미 존재
	}
	else {
		return false; //생성 실패
	}
}

bool CGlimCircleExamDlg::DeleteDirectory(const string& path) {
	std::filesystem::path dirPath(path);

	//디렉토리가 존재하는지 확인
	if (std::filesystem::exists(dirPath) &&
		std::filesystem::is_directory(dirPath)) {
		try {
			//디렉토리 및 하위 모든 파일 삭제
			std::filesystem::remove_all(dirPath);
			fprintf(stdout, "%s -> deleteion success!\n", path.c_str());
			return true;
		}
		catch (const std::filesystem::filesystem_error& e) {
			fprintf(stderr, "%s -> deletion error! [%s]\n", path.c_str(), const_cast<char*>(e.what()));
			return false;
		}
	}
	else {
		fprintf(stderr, "directory does not exist or it is not a directory!\n");
		return false;
	}
}

//drawing 작업 영역을 계산
CRect CGlimCircleExamDlg::CalcWorkClientArea() {
	//기준이 되는 group box control의 위치를 가져온다
	CWnd* pStandardControl = GetDlgItem(IDC_STATIC_DRAWSETTING);
	if (pStandardControl == nullptr) {
		return CRect(0, 0, 0, 0); //기준이 되는 control을 찾지 못한 경우
	}

	CRect mainDlgRect;
	GetClientRect(&mainDlgRect);

	CRect standardRect;
	pStandardControl->GetWindowRect(&standardRect);
	ScreenToClient(&standardRect);

	CRect workRect;
	workRect.left = standardRect.right + 10;
	workRect.top = standardRect.top;
	workRect.right = mainDlgRect.right - 10;
	workRect.bottom = standardRect.bottom;

	return workRect;
}

bool CGlimCircleExamDlg::ProgramEnvSet() {
	bool bRet = false;

	//제목표시줄 내용 표시
	this->SetWindowText(_T("(주)그림 SW 직군 지원 과제 1 (작성자 : 박시현)"));

	m_workAreaRect = CalcWorkClientArea(); //drawing 영역 획득
	fprintf(stdout, "m_workAreaRect = LT[%d,%d], RB[%d,%d]\n", 
		m_workAreaRect.left, 
		m_workAreaRect.top, 
		m_workAreaRect.right, 
		m_workAreaRect.bottom
	);

	//Image저장을 위한 "Image" 폴더 생성
	m_modulePath = GetModulePath();
	bRet = MakeDirectory(m_modulePath, "Image");

	//drawing time 초기값 설정
	CString strDrawTimes = _T("1");
	GetDlgItem(IDC_EDIT_DRAWCIRCLECNT)->SetWindowText(strDrawTimes);

	//현재 drawing times 표시 관련 설정
	SetControlFont(this, IDC_STATIC_CURDRAWTIMESSTATIC, _T("Arial"), 50, RGB(128, 128, 0));
	SetControlText(this, IDC_STATIC_CURDRAWTIMESSTATIC, _T("0"));

	//button 관련 처리 (enable/disable)
	//Program Load시니까...
	SetButtonState(IDC_BTN_DRAWCIRCLE_START, TRUE);
	SetButtonState(IDC_BTN_DRAWCIRCLE_PAUSE, FALSE);
	SetButtonState(IDC_BTN_DRAWCIRCLE_RESUME, FALSE);
	SetButtonState(IDC_BTN_DRAWCIRCLE_STOP, FALSE);
	SetButtonState(IDC_BTN_IMGLOAD, TRUE);
	
	return bRet;
}


bool CGlimCircleExamDlg::StartDrawCircleTh(void) {
	void* pMainDlg = nullptr;
	pMainDlg = (void*)(this);
	m_drawCircleTh.SetWndPointer(pMainDlg);

	bool start = m_drawCircleTh.start();
	return start;
}

void CGlimCircleExamDlg::DrawCircle( unsigned char* pbits, int pitch, int x_center, int y_center, int radius, CRect& rect) {
	for (int y = -radius; y <= radius; ++y) {
		for (int x = -radius; x <= radius; ++x) {
			//circle 내부의 pixel만 처리하자
			if ((x * x) + (y * y) <= (radius * radius)) {
				int x_pixel = x_center + x;
				int y_pixel = y_center + y;

				//white color로 설정
				//pbits[y_pixel * pitch + x_pixel] = DRAW_CIRCLE_COLOR;

				//유효한 좌표인지 확인
				if (x_pixel > 0 &&
					x_pixel < rect.Width() &&
					y_pixel > 0 &&
					y_pixel < rect.Height()) {
					//white color로 설정
					pbits[y_pixel * pitch + x_pixel] = DRAW_CIRCLE_COLOR;
				}
			}
		}
	}

	return;
}


void CGlimCircleExamDlg::UpdateDisplay() {
	CClientDC dc(this);
	//m_image.Draw(dc, m_workAreaRect.left, m_workAreaRect.top);

	//더블 버퍼링을 위한 메모리 DC 생성하자.
	CBitmap bufferBitmap;
	CDC memoryDC;
	memoryDC.CreateCompatibleDC(&dc);
	bufferBitmap.CreateCompatibleBitmap(&dc, m_workAreaRect.Width(), m_workAreaRect.Height());
	CBitmap* pOldBitmap = memoryDC.SelectObject(&bufferBitmap);

	//배경 지우기
	memoryDC.FillSolidRect(m_workAreaRect, RGB(255, 255, 255));

	//이미지를 메모리 DC에 그리기
	m_image.Draw(memoryDC, 0, 0);

	//메모리 DC의 내용을 실제 DC로 복사
	dc.BitBlt(
		m_workAreaRect.left,
		m_workAreaRect.top,
		m_workAreaRect.Width(),
		m_workAreaRect.Height(),
		&memoryDC,
		0,
		0,
		SRCCOPY
	);

	//이전 비트맵 복원
	memoryDC.SelectObject(pOldBitmap);

	return;
}


void CGlimCircleExamDlg::FindCircle(CImage& image, int& x, int& y, int& radius) {
	int width = image.GetWidth();
	int height = image.GetHeight();

	int white_value = DRAW_CIRCLE_COLOR;
	std::vector<std::pair<int, int>> white_pixels;

	unsigned char* buffer = (unsigned char*)image.GetBits();
	int pitch = image.GetPitch();

	//모든 pixel을 search하면서 white color pixel의 좌표값을 vector에 저장
	for (int j = 0; j < height; ++j) {
		for (int i = 0; i < width; ++i) {
			//get a current BGR value
			unsigned char* pixel = buffer + j * pitch + i * 3;
			//Let's check if the pixel is white...
			if (pixel[0] == white_value &&
				pixel[1] == white_value &&
				pixel[2] == white_value) {
				white_pixels.push_back({ i, j });
			}
		}
	}

	//If there are no white pixels, set the center coordinate and
	//radius to -1 and return...
	if (white_pixels.empty()) {
		x = y = radius = -1;
		return;
	}

	//Let's compute the sum of the x, y coordinates of the white pixels
	int sum_x = 0, sum_y = 0;
	for (int idx = 0; idx < white_pixels.size(); ++idx) {
		sum_x += white_pixels[idx].first;
		sum_y += white_pixels[idx].second;
	}

	//Calcuate the center coordinates
	x = sum_x / static_cast<int>(white_pixels.size());
	y = sum_y / static_cast<int>(white_pixels.size());

	//Let's find max_dist using the equation of the circle...
	int max_dist_squared = 0; //r^2값
	for (int idx = 0; idx < white_pixels.size(); ++idx) {
		int dx = white_pixels[idx].first - x;
		int dy = white_pixels[idx].second - y;
		int dist_squared = dx * dx + dy * dy; //거리의 제곱값 계산
		if (dist_squared > max_dist_squared) {
			max_dist_squared = dist_squared;
		}
	}

	//반지름 값을 구하자.
	radius = static_cast<int>(std::sqrt(max_dist_squared));

	fprintf(stdout, "[w:%d],[h:%d],[p:%d],[c:%d,%d],[r:%d]\n",
		width, height, pitch, x, y, radius);

	return;
}

void CGlimCircleExamDlg::DrawImage(CDC* pDC) {
	if (m_image.IsNull()) {
		fprintf(stderr, "DrawImage() : m_image is null!\n");
		return;
	}

	int image_width = m_image.GetWidth();
	int image_height = m_image.GetHeight();

	// Create memory DC and select image
	CDC memDC;
	memDC.CreateCompatibleDC(pDC);

	CBitmap bitmap;
	bitmap.Attach(m_image);
	CBitmap* pOldBitmap = memDC.SelectObject(&bitmap);

	if (!pOldBitmap) {
		fprintf(stderr, "DrawImage() : Failed to select bitmap!\n");
		return;
	}

	// Draw the image
	pDC->BitBlt(0, 0, image_width, image_height, &memDC, 0, 0, SRCCOPY);

	// Restore and clean up
	memDC.SelectObject(pOldBitmap);
}

void CGlimCircleExamDlg::DrawCircleDetails(CDC* pDC, int image_width, int image_height) {
	if (m_circleX == -1 || m_circleY == -1 || m_radius <= 0) {
		return;
	}

	//원 중앙 좌표 draw
	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(RGB(0, 0, 0));
	CBrush brush(RGB(0, 0, 0));
	CBrush* pOldBrush = pDC->SelectObject(&brush);
	pDC->Ellipse(m_circleX - 3, m_circleY - 3, m_circleX + 3, m_circleY + 3);
	pDC->SelectObject(pOldBrush);

	//원점을 지나는 x, y축 직선 그려주자.
	CPen dashedPen(PS_DOT, 1, RGB(0, 0, 255));
	CPen* pOldPen = pDC->SelectObject(&dashedPen);
	pDC->MoveTo(0, m_circleY);
	pDC->LineTo(image_width, m_circleY);
	pDC->MoveTo(m_circleX, 0);
	pDC->LineTo(m_circleX, image_height);
	pDC->SelectObject(pOldPen);

	//반지름 그려주자
	DrawRadiusLine(pDC, image_width, image_height);

	//반지름 text 써주자
	DrawRadiusText(pDC, image_width, image_height);
}

void CGlimCircleExamDlg::DrawRadiusLine(CDC* pDC, int image_width, int image_height) {
	double radian = GetRadiusAngle(pDC, image_width, image_height) * M_PI / 180.0;
	int endX = m_circleX + static_cast<int>(m_radius * cos(radian));
	int endY = m_circleY - static_cast<int>(m_radius * sin(radian));

	CPen pen(PS_SOLID, 2, RGB(255, 0, 0));
	CPen* pOldPen = pDC->SelectObject(&pen);
	pDC->MoveTo(m_circleX, m_circleY);
	pDC->LineTo(endX, endY);
	pDC->SelectObject(pOldPen);
}

void CGlimCircleExamDlg::DrawRadiusText(CDC* pDC, int image_width, int image_height) {
	CString radiusText;
	radiusText.Format(_T("Radius = %d (%d, %d)"), m_radius, m_circleX, m_circleY);

	CSize textSize = pDC->GetTextExtent(radiusText);

	int angle = GetRadiusAngle(pDC, image_width, image_height);
	double radian = angle * M_PI / 180.0;
	int textX = m_circleX + static_cast<int>((m_radius + 10) * cos(radian));
	int textY = m_circleY - static_cast<int>((m_radius + 10) * sin(radian));

	if (textX + textSize.cx > image_width) 
		textX = image_width - textSize.cx - 15;
	if (textY + textSize.cy > image_height) 
		textY = image_height - textSize.cy - 15;

	if ((angle == 135) || (angle == 225)) {
		textX -= textSize.cx;
	}

	if (textX < 0) 
		textX = 10;
	if (textY < 0) 
		textY = 10;

	//텍스트 색상 변경
	COLORREF oldColor = pDC->SetTextColor(RGB(184, 134, 11));
	pDC->TextOut(textX, textY, radiusText);
	pDC->SetTextColor(oldColor);

	fprintf(stdout, "angle=%d, m_radius=%d, m_circleX=%d, m_circleY=%d, textX=%d, textY=%d\n",
		angle, m_radius, m_circleX, m_circleY, textX, textY);
	
	UpdateText(m_circleX, m_circleY, m_radius);
}

int CGlimCircleExamDlg::GetRadiusAngle(CDC* pDC, int image_width, int image_height) {
	int x; //원 중심 x
	int y; //원 중심 y
	int tsw; //text width
	int tsh; //text height
	int w; //image width
	int h; //image height
	int r; //radius

	x = m_circleX;
	y = m_circleY;
	CString radiusText;
	radiusText.Format(_T("radius = %d (%d, %d)"), m_radius, m_circleX, m_circleY);
	CSize textSize = pDC->GetTextExtent(radiusText);
	tsw = textSize.cx;
	tsh = textSize.cy;
	w = image_width;
	h = image_height;
	r = m_radius;

	if ((x - r - tsw < 0) && (y - r - tsh < 0)) {
		return 315;
		//return 270;
	}
	else if ((x + r + tsw > w) && (y - r - tsh)) {
		return 225;
		//return 270;
	}
	else if ((x - r - tsw < 0) && (y + r + tsh > h)) {
		return 45;
		//return 90;
	}
	else if ((x + r + tsw > w) && (y + r + tsh > h)) {
		return 135;
		//return 90;
	}
	else {
		if ((x < w / 2) && (y < h / 2)) {
			return 315;
		}
		else if ((x > w / 2) && (y < h / 2)) {
			return 225;
		}
		else if ((x < w / 2) && (y > h / 2)) {
			return 45;
		}
		else if ((x > w / 2) && (y > h / 2)) {
			return 135;
		}
		else {
			return 45;
		}
	}
}


void CGlimCircleExamDlg::PerformDoubleBuffering(CDC* pDC, const CRect& rect) {
	//더블 버퍼링을 위한 메모리 DC 생성
	CBitmap bufferBitmap;
	CDC memoryDC;
	memoryDC.CreateCompatibleDC(pDC);
	bufferBitmap.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = memoryDC.SelectObject(&bufferBitmap);

	//배경 지우기
	memoryDC.FillSolidRect(rect, RGB(255, 255, 255));

	//내용물 그려주고... 
	DrawBufferedContent(&memoryDC, rect);

	//화면 출력
	pDC->BitBlt(rect.left, rect.top, rect.Width(), rect.Height(), &memoryDC, 0, 0, SRCCOPY);

	// Restore and clean up
	memoryDC.SelectObject(pOldBitmap);
}

void CGlimCircleExamDlg::DrawBufferedContent(CDC* pDC, const CRect& rect) {
	if (!m_image.IsNull()) {
		DrawImage(pDC);
	}
	DrawCircleDetails(pDC, m_image.GetWidth(), m_image.GetHeight());
}

void CGlimCircleExamDlg::InitProgress() {
	CProgressCtrl* pProgress = (CProgressCtrl*)GetDlgItem(IDC_PROGRESS_DRAWCIRCLE);
	if (pProgress) {
		pProgress->SetRange(0, m_drawTimes);
		pProgress->SetPos(0);
	}
	return;
}

void CGlimCircleExamDlg::ResetProgress() {
	CProgressCtrl* pProgress = (CProgressCtrl*)GetDlgItem(IDC_PROGRESS_DRAWCIRCLE);
	if (pProgress) {
		pProgress->SetPos(0);
	}
	return;
}

void CGlimCircleExamDlg::UpdateProgress(int value) {
	CProgressCtrl* pProgress = (CProgressCtrl*)GetDlgItem(IDC_PROGRESS_DRAWCIRCLE);
	if (pProgress) {
		pProgress->SetPos(value);
	}
}

void CGlimCircleExamDlg::InitText(void) {
	CWnd* pTextControl = GetDlgItem(IDC_STATIC_CENTERCOORDIVALUE);
	if (pTextControl) {
		pTextControl->SetWindowText(_T("0, 0"));
	}
	pTextControl = GetDlgItem(IDC_STATIC_RADIUSVALUE);
	if (pTextControl) {
		pTextControl->SetWindowText(_T("0"));
	}
}

void CGlimCircleExamDlg::ResetText(void) {
	CWnd* pTextControl = GetDlgItem(IDC_STATIC_CENTERCOORDIVALUE);
	if (pTextControl) {
		pTextControl->SetWindowText(_T("0, 0"));
	}
	pTextControl = GetDlgItem(IDC_STATIC_RADIUSVALUE);
	if (pTextControl) {
		pTextControl->SetWindowText(_T("0"));
	}
}

void CGlimCircleExamDlg::UpdateText(int x, int y, int r) {
	CWnd* pTextControl = GetDlgItem(IDC_STATIC_CENTERCOORDIVALUE);
	if (pTextControl) {
		CString s; 
		s.Format(_T("%d, %d"), x, y);
		pTextControl->SetWindowText(s);
	}
	pTextControl = GetDlgItem(IDC_STATIC_RADIUSVALUE);
	if (pTextControl) {
		CString s;
		s.Format(_T("%d"), r);
		pTextControl->SetWindowText(s);
	}
}

void CGlimCircleExamDlg::SetButtonState(int id, BOOL enable) {
	CWnd* pBtnControl = GetDlgItem(id);
	if (pBtnControl) {
		pBtnControl->EnableWindow(enable);
	}
}

void CGlimCircleExamDlg::OnBnClickedBtnDrawcircleStart()
{
	// TODO: Add your control notification handler code here
	
	if (m_drawCircleTh.isRunning() == true) {
		AfxMessageBox(_T("Thread is already running!"));
		return;
	}

	CString strDrawTimes;
	GetDlgItem(IDC_EDIT_DRAWCIRCLECNT)->GetWindowText(strDrawTimes);

	//입력값 유효성 검사 (숫자형, min/max값)
	if (_stscanf_s(strDrawTimes, _T("%d"), &m_drawTimes) == 1) {
		//정수값이 입력됨
		if (m_drawTimes <= 0 || m_drawTimes > 1000) {
			AfxMessageBox(_T("1 ~ 1000 사이의 값을 입력해주세요."));
			return;
		}
	}
	else {
		//변환 실패 - 문자가 입력됨
		AfxMessageBox(_T("문자가 아니라 정수값을 입력해주세요."));
		return;
	}

	string strSavePath = m_modulePath + "\\Image";
	if (DeleteDirectory(strSavePath) == false) {
		AfxMessageBox(_T("Image Directory delete error!"));
		return;
	}
	if (MakeDirectory(m_modulePath, "Image") == false) {
		AfxMessageBox(_T("Image Directory make error!"));
		return;
	}

	bool ret = StartDrawCircleTh();
	if (ret == true) {
		InitProgress();
		InitText();
		SetButtonState(IDC_BTN_DRAWCIRCLE_START, FALSE);
		SetButtonState(IDC_BTN_DRAWCIRCLE_PAUSE, TRUE); 
		SetButtonState(IDC_BTN_DRAWCIRCLE_RESUME, FALSE);
		SetButtonState(IDC_BTN_DRAWCIRCLE_STOP, TRUE);
		SetButtonState(IDC_BTN_IMGLOAD, FALSE);
		fprintf(stdout, "StartDrawCircleTh() start success!\n");
		return;
	}
	else {
		fprintf(stderr, "StartDrawCircleTh() start fail!\n");
		AfxMessageBox(_T("StartDrawCircleTh() start fail!"));
		return;
	}

	return;
}

void CGlimCircleExamDlg::OnBnClickedBtnDrawcirclePause()
{
	// TODO: Add your control notification handler code here
	bool bret = m_drawCircleTh.pause(m_drawCircleTh.getHandle());
	if (bret) {
		fprintf(stdout, "Thread pause!\n");
		SetButtonState(IDC_BTN_DRAWCIRCLE_START, FALSE);
		SetButtonState(IDC_BTN_DRAWCIRCLE_PAUSE, FALSE); 
		SetButtonState(IDC_BTN_DRAWCIRCLE_RESUME, TRUE);
		SetButtonState(IDC_BTN_DRAWCIRCLE_STOP, TRUE);
		SetButtonState(IDC_BTN_IMGLOAD, FALSE);
	}
	else {
		fprintf(stderr, "Thread pause fail!\n");
	}
}


void CGlimCircleExamDlg::OnBnClickedBtnDrawcircleResume()
{
	// TODO: Add your control notification handler code here
	bool bret = m_drawCircleTh.resume(m_drawCircleTh.getHandle());
	if (bret) {
		fprintf(stdout, "Thread resume!\n");
		SetButtonState(IDC_BTN_DRAWCIRCLE_START, FALSE);
		SetButtonState(IDC_BTN_DRAWCIRCLE_PAUSE, TRUE); 
		SetButtonState(IDC_BTN_DRAWCIRCLE_RESUME, FALSE);
		SetButtonState(IDC_BTN_DRAWCIRCLE_STOP, TRUE);
		SetButtonState(IDC_BTN_IMGLOAD, FALSE);
	}
	else {
		fprintf(stderr, "Thread resume fail!\n");
	}
}


void CGlimCircleExamDlg::OnBnClickedBtnDrawcircleStop()
{
	// TODO: Add your control notification handler code here
	bool bret = m_drawCircleTh.stop();
	if (bret) {
		if (m_drawCircleTh.isRunning() == false) {
			fprintf(stdout, "Thread stop!\n");
			SetButtonState(IDC_BTN_DRAWCIRCLE_START, TRUE);
			SetButtonState(IDC_BTN_DRAWCIRCLE_PAUSE, FALSE); 
			SetButtonState(IDC_BTN_DRAWCIRCLE_RESUME, FALSE);
			SetButtonState(IDC_BTN_DRAWCIRCLE_STOP, FALSE);
			SetButtonState(IDC_BTN_IMGLOAD, TRUE);
		}
	}
	else {
		fprintf(stderr, "Thread stop fail!\n");
	}
}


void CGlimCircleExamDlg::OnBnClickedBtnImgload()
{
	// TODO: Add your control notification handler code here

	if (m_image != nullptr) {
		m_image.Destroy();
	}

	CFileDialog dlg(TRUE, _T(".jpg"), NULL, OFN_FILEMUSTEXIST, _T("JPEG Files (*.jpg)|*.jpg|All Files (*.*)|*.*||"));

	string initialDirPath = m_modulePath.append("\\Image");
	dlg.m_ofn.lpstrInitialDir = CA2T(initialDirPath.c_str());

	if (dlg.DoModal() == IDOK) {
		CString file_path = dlg.GetPathName();
		HRESULT hr = m_image.Load(file_path);
		if (FAILED(hr)) {
			AfxMessageBox(_T("File loading fail!"));
			return;
		}
		
		FindCircle(m_image, m_circleX, m_circleY, m_radius);
		CClientDC dc(this);
		PerformDoubleBuffering(&dc, m_workAreaRect);
	}

	return;
}

//DrawCircleTh Class //////////////////////////////////////////////////////////////////////////////////////////////////
DrawCircleTh::DrawCircleTh() {
	m_pWnd = nullptr;
}

DrawCircleTh::DrawCircleTh(void* pWnd) {
	m_pWnd = pWnd;
}

DrawCircleTh::~DrawCircleTh() {
}

void DrawCircleTh::SetWndPointer(void* pWnd) {
	m_pWnd = pWnd;
}

void DrawCircleTh::run(LPVOID pParam) {
	fprintf(stdout, "DrawCircleTh::run() is called!\n");
	CGlimCircleExamDlg* pdlg = static_cast<CGlimCircleExamDlg*>(m_pWnd);

	if (pdlg->m_image != nullptr) {
		pdlg->m_image.Destroy();
	}

	int nBPP = 8; //number of bit per pixel
	//256 color image 생성
	pdlg->m_image.Create(pdlg->m_workAreaRect.Width(), pdlg->m_workAreaRect.Height(), nBPP);

	//8bit만 palette 사용
	if (nBPP == 8) {
		//palette 초기화
		RGBQUAD palette[256] = { 0, };
		for (int i = 0; i < 256; ++i) {
			palette[i].rgbRed = palette[i].rgbGreen = palette[i].rgbBlue = i;
		}
		pdlg->m_image.SetColorTable(0, 256, palette);
	}

	//배경을 gray color로 채우자
	unsigned char* pbits = static_cast<unsigned char*>(pdlg->m_image.GetBits());
	int pitch = pdlg->m_image.GetPitch();
	for (int y = 0; y < pdlg->m_workAreaRect.Height(); ++y) {
		for (int x = 0; x < pdlg->m_workAreaRect.Width(); ++x) {
			pbits[y * pitch + x] = DRAW_BG_COLOR; 
		}
	}

	std::mt19937 rng(static_cast<unsigned int>(time(nullptr)));
	std::uniform_int_distribution<int> dist(10, 90);
	std::uniform_int_distribution<int> x_coord(100, pdlg->m_workAreaRect.Width() - 100);
	std::uniform_int_distribution<int> y_coord(100, pdlg->m_workAreaRect.Height() - 100);

	CString fname = _T("");
	CString full_path = _T("");

	for (int i = 0; i < pdlg->m_drawTimes && _isRunning; ++i) {
		const int radius = dist(rng);
		const int x_center = x_coord(rng);
		const int y_center = y_coord(rng);

		//image draw
		pdlg->DrawCircle(pbits, pitch, x_center, y_center, radius, pdlg->m_workAreaRect);
		//drawing area update
		pdlg->UpdateDisplay();

		//image save
		fname.Format(_T("Circle_%d.jpg"), i + 1);
		full_path = CA2T(pdlg->m_modulePath.c_str()) + _T("\\Image\\") + fname;
		pdlg->m_image.Save(full_path, Gdiplus::ImageFormatJPEG);

		//image clear with gray color
		if (i != (pdlg->m_drawTimes - 1)) {
			for (int y = 0; y < pdlg->m_workAreaRect.Height(); ++y) {
				for (int x = 0; x < pdlg->m_workAreaRect.Width(); ++x) {
					pbits[y * pitch + x] = DRAW_BG_COLOR; 
				}
			}
		}
		
		//0base로 시작해서 +1 해주자
		pdlg->UpdateProgress(i + 1);
		pdlg->UpdateText(x_center, y_center, radius);
		CString s;
		s.Format(_T("%d"), i + 1);
		SetControlText(pdlg, IDC_STATIC_CURDRAWTIMESSTATIC, s);
		
		//_isRunning을 check하기 위해 짧은 sleep 하자.
		//1초를 0.1초씩 분활해서 sleep하면서 스레드 stop check...
		//thread pause/stop 반응도 향상을 위해...
		for (int j = 0; j < 10 && _isRunning; ++j) {
			Sleep(100);
		}
	}

	//외부에서 stop 신호가 들어와야만 _isRunning == false이다.
	if (!_isRunning) {
		pdlg->m_image.Destroy();
		fprintf(stdout, "DrawCircleTh::run() has endend by stop command.\n");
		return;
	}

	//이 경우는 그냥 정상적으로 작업을 다 완료했을 때...
	pdlg->SetButtonState(IDC_BTN_DRAWCIRCLE_START, TRUE);
	pdlg->SetButtonState(IDC_BTN_DRAWCIRCLE_PAUSE, FALSE);
	pdlg->SetButtonState(IDC_BTN_DRAWCIRCLE_RESUME, FALSE);
	pdlg->SetButtonState(IDC_BTN_DRAWCIRCLE_STOP, FALSE);
	pdlg->SetButtonState(IDC_BTN_IMGLOAD, TRUE);
	//이 순간에도 _isRunning == true 이다.
	fprintf(stdout, "DrawCircleTh::run() has endend.\n");

	return;
}

void CGlimCircleExamDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: Add your message handler code here
	DeleteControlFont(this, IDC_STATIC_CURDRAWTIMESSTATIC);
}


