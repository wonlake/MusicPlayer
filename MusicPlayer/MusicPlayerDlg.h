
// MusicPlayerDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"

#include <string>
#include <list>
#include <vector>
#include <map>
#include "AudioPlayer.h"

// CMusicPlayerDlg 对话框
class CMusicPlayerDlg : public CDialogEx
{
// 构造
public:
	CMusicPlayerDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_MUSICPLAYER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;
	std::vector<std::pair<std::wstring, std::vector<std::wstring>> >		m_vecMusicList;
	std::vector<std::wstring>*	m_pvecMusicFullPaths;
	std::vector<std::wstring>	m_vecNull;

	int							m_iItem;
	int							m_iCurPlayIndex;
	int							m_iCurSelectedList;
	int							m_iListPlaying;

	int							m_iPlayMode;
	BOOL						m_bPause;

	AudioPlayer*				m_pAudioPlayer;

	void LoadSettings();
	void SaveSettings();

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnSysTrayNotify( WPARAM wParam, LPARAM lParam );
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedPlaypause();
	CComboBox m_cbAddMode;
	afx_msg void OnCbnSelchangeAddmode();
	CListCtrl m_lcMusic;
	afx_msg void OnCbnSelchangePlaymode();
	CComboBox m_cbPlayMode;
	afx_msg void OnBnClickedAddlist();
	CListCtrl m_lcListName;
	afx_msg void OnBnClickedDellist();
	afx_msg void OnNMDblclkMusiclist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickMusiclist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	CSliderCtrl m_scPlayingProcess;
	afx_msg void OnBnClickedPrev();
	afx_msg void OnBnClickedNext();
	afx_msg void OnNMDblclkMusicdetail(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnKeydownMusicdetail(NMHDR *pNMHDR, LRESULT *pResult);
	CSliderCtrl m_scVolume;
	afx_msg void OnTRBNThumbPosChangingSlider2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSlider2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMReleasedcaptureSlider2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnNcLButtonUp(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnLvnEndlabeleditMusiclist(NMHDR *pNMHDR, LRESULT *pResult);
};
