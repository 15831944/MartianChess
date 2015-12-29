// MChessServerDlg.h : header file
//

#pragma once


// CMChessServerDlg dialog
class CMChessServerDlg : public CDialog
{
// Construction
public:
	CMChessServerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_MCHESSSERVER_DIALOG };

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
	CString m_ServerName;
	int m_Port;
	CString m_IPAddr;
	CString m_Output;
	afx_msg void OnBnClickedBegin();
	afx_msg void OnBnClickedEnd();
	LRESULT UpdateOutput(WPARAM wParam, LPARAM lParam);
};
