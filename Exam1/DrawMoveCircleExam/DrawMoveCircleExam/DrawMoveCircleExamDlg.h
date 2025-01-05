
// DrawMoveCircleExamDlg.h : header file
//

#pragma once
#include <vector>


// CDrawMoveCircleExamDlg dialog
class CDrawMoveCircleExamDlg : public CDialogEx
{
// Construction
public:
	CDrawMoveCircleExamDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DRAWMOVECIRCLEEXAM_DIALOG };
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

private:
	std::vector<CPoint> m_points; //클릭한 점들
	bool m_isDragging;
	CPoint m_draggedPoint;
	int m_borderThickness;

	CPoint m_largeCircleCenter;
	int m_largeCircleRadius;
	double m_triangleAreaFactor;

public:
	void DrawBuffer(CDC* pDC); //double buffering
	void DrawCircle(CDC* pDC, CPoint point, COLORREF color, int radius, int index);
	void DrawLargeCircle(CDC* pDC);
	void CalculateLargeCircle();
	void DrawCircleInfo(CDC* pDC, int x, int y, CString text, COLORREF textColor, int FontSize, int fontWeight);

public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};
