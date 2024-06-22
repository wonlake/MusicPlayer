
// MusicPlayerDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "MusicPlayer.h"
#include "MusicPlayerDlg.h"
#include "afxdialogex.h"
#include <atlsimpstr.h>
#include <nlohmann/json.hpp>
#include <fstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMusicPlayerDlg �Ի���


#define WM_SYSTRAYNOTIFY	WM_USER + 100

CMusicPlayerDlg::CMusicPlayerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMusicPlayerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pvecMusicFullPaths = NULL;
	m_iCurSelectedList = 0;
	m_iListPlaying = -1;
}

void CMusicPlayerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ADDMODE, m_cbAddMode);
	DDX_Control(pDX, IDC_MUSICDETAIL, m_lcMusic);
	DDX_Control(pDX, IDC_PLAYMODE, m_cbPlayMode);
	DDX_Control(pDX, IDC_MUSICLIST, m_lcListName);
	DDX_Control(pDX, IDC_SLIDER1, m_scPlayingProcess);
	DDX_Control(pDX, IDC_SLIDER2, m_scVolume);
}

BEGIN_MESSAGE_MAP(CMusicPlayerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_PLAYPAUSE, &CMusicPlayerDlg::OnBnClickedPlaypause)
	ON_CBN_SELCHANGE(IDC_ADDMODE, &CMusicPlayerDlg::OnCbnSelchangeAddmode)
	ON_CBN_SELCHANGE(IDC_PLAYMODE, &CMusicPlayerDlg::OnCbnSelchangePlaymode)
	ON_BN_CLICKED(IDC_ADDLIST, &CMusicPlayerDlg::OnBnClickedAddlist)
	ON_BN_CLICKED(IDC_DELLIST, &CMusicPlayerDlg::OnBnClickedDellist)
	ON_NOTIFY(NM_DBLCLK, IDC_MUSICLIST, &CMusicPlayerDlg::OnNMDblclkMusiclist)
	ON_NOTIFY(NM_CLICK, IDC_MUSICLIST, &CMusicPlayerDlg::OnNMClickMusiclist)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_PREV, &CMusicPlayerDlg::OnBnClickedPrev)
	ON_BN_CLICKED(IDC_NEXT, &CMusicPlayerDlg::OnBnClickedNext)
	ON_NOTIFY(NM_DBLCLK, IDC_MUSICDETAIL, &CMusicPlayerDlg::OnNMDblclkMusicdetail)
	ON_NOTIFY(LVN_KEYDOWN, IDC_MUSICDETAIL, &CMusicPlayerDlg::OnLvnKeydownMusicdetail)
	ON_NOTIFY(TRBN_THUMBPOSCHANGING, IDC_SLIDER2, &CMusicPlayerDlg::OnTRBNThumbPosChangingSlider2)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER2, &CMusicPlayerDlg::OnNMCustomdrawSlider2)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER2, &CMusicPlayerDlg::OnNMReleasedcaptureSlider2)
	ON_MESSAGE(WM_SYSTRAYNOTIFY, &CMusicPlayerDlg::OnSysTrayNotify)
	ON_WM_GETMINMAXINFO()
	ON_WM_NCHITTEST()
	ON_WM_NCLBUTTONUP()
	ON_WM_NCLBUTTONDOWN()
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_MUSICLIST, &CMusicPlayerDlg::OnLvnEndlabeleditMusiclist)
END_MESSAGE_MAP()


// CMusicPlayerDlg ��Ϣ�������

BOOL CMusicPlayerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��
	
	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	m_cbAddMode.AddString( _T("Ŀ¼") );
	m_cbAddMode.AddString( _T("�ļ�") );
	m_cbAddMode.SetCurSel( 0 );

	m_cbPlayMode.AddString( _T("����ѭ��") );
	m_cbPlayMode.AddString( _T("ѭ������") );
	m_cbPlayMode.AddString( _T("˳�򲥷�") );
	m_cbPlayMode.SetCurSel( 1 );

	m_lcListName.InsertColumn( 0, _T("�����б�"), 0, 100 );
	m_lcMusic.InsertColumn( 0, _T("������"), 0, 160 );

	SetTimer( 1000, 100, NULL );
	SetTimer( 1001, 40, NULL );

	m_iItem = -1;
	m_iCurPlayIndex = -1;

	m_pAudioPlayer = new AudioPlayer();

	m_scPlayingProcess.SetRangeMax( 1000 );
	m_scVolume.SetPos( m_scVolume.GetRangeMax() );

	m_iPlayMode = 1;
	m_bPause = FALSE;

	NOTIFYICONDATA nid = { 0 };
	nid.cbSize = sizeof(nid);
	nid.hIcon = LoadIcon( GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_MAINFRAME) );
	nid.hWnd = m_hWnd;
	nid.uCallbackMessage = WM_SYSTRAYNOTIFY;
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | 
		NIF_INFO;
	
	_tcscpy( nid.szInfoTitle, _T("������ʾ") );
	_tcscpy( nid.szInfo, _T("���������ʾ�������Ҽ�����") );
	_tcscpy( nid.szTip, _T("MusicPlayer���ֲ���������ӭʹ��") );

	Shell_NotifyIcon( NIM_ADD, &nid );

	LoadSettings();

	if( m_vecMusicList.size() < 1 )
	{		
		m_vecMusicList.push_back( std::make_pair(_T("Ĭ���б�"), std::vector<std::wstring>()) );
		m_lcListName.InsertItem( 0, m_vecMusicList[0].first.c_str() );
	}
	else
	{
		for( int i = 0; i < m_vecMusicList.size(); ++i )
		{
			m_lcListName.InsertItem( i, m_vecMusicList[i].first.c_str() );
		}
	}
	m_pvecMusicFullPaths = &m_vecMusicList[0].second;
	for( int i = 0; i < m_pvecMusicFullPaths->size(); ++i )
	{
		const TCHAR* p = _tcsrchr( m_pvecMusicFullPaths->at(i).c_str(), _T('\\'));
		if( p != NULL )
			m_lcMusic.InsertItem( i, p + 1 );
		else
			m_lcMusic.InsertItem( i, m_pvecMusicFullPaths->at(i).c_str() );
	}

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CMusicPlayerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CMusicPlayerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CMusicPlayerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMusicPlayerDlg::OnBnClickedPlaypause()
{
	if( m_pvecMusicFullPaths == NULL )
		return;

	m_iListPlaying = m_iCurSelectedList;

	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if( m_pvecMusicFullPaths == NULL )
		return;

	if( m_pvecMusicFullPaths->size() < 1 )
		return;
	
	int iSelectedItem = -1;
	POSITION pos = m_lcMusic.GetFirstSelectedItemPosition();
	while( pos )
	{
		iSelectedItem = m_lcMusic.GetNextSelectedItem(pos);
	}

	if( iSelectedItem == m_iItem &&
		m_iItem > -1 &&
		m_iItem < m_pvecMusicFullPaths->size() )
	{
		m_bPause = TRUE;		
	}
	else
	{
		m_bPause = FALSE;			
	}

	m_iItem = iSelectedItem;
	if( m_iItem > -1 && m_iItem < m_pvecMusicFullPaths->size() )
		m_iCurPlayIndex = m_iItem;
	else
	{
		m_iItem = m_iCurPlayIndex = 0;		
	}

	if (m_pAudioPlayer != NULL && !m_bPause)
	{
		m_pAudioPlayer->LoadMusicFromFile(
			m_pvecMusicFullPaths->at(m_iCurPlayIndex).c_str());
		
		m_lcMusic.SetItemState( m_iCurPlayIndex, LVIS_SELECTED, LVIS_SELECTED );
		
		CString strWinTitle = CString("MusicPlayer - ") + m_pvecMusicFullPaths->at(m_iCurPlayIndex).c_str();
		SetWindowText( strWinTitle );
	}

	CString  strWinText;
	GetDlgItem(IDC_PLAYPAUSE)->GetWindowText( strWinText );
	if( strWinText.Find( _T("����") ) != -1 )
	{
		if( m_bPause )
		{
			m_bPause = FALSE;			
		}
		GetDlgItem(IDC_PLAYPAUSE)->SetWindowText( _T("��ͣ") );
	}
	else
	{
		if( m_bPause )
			GetDlgItem(IDC_PLAYPAUSE)->SetWindowText( _T("����") );
	}
}


void CMusicPlayerDlg::OnCbnSelchangeAddmode()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	if( m_pvecMusicFullPaths == NULL )
		return;

	int i = m_cbAddMode.GetCurSel();
	CString strText;
	int len = m_cbAddMode.GetLBTextLen( i );
	m_cbAddMode.GetLBText( i, strText.GetBuffer(len) );

	int iSelectedItem = -1;
	POSITION pos = m_lcListName.GetFirstSelectedItemPosition();
	while( pos )
	{
		iSelectedItem = m_lcListName.GetNextSelectedItem(pos);		
	}

	if( iSelectedItem != -1 && iSelectedItem < m_vecMusicList.size() )
	{
		if( iSelectedItem != m_iCurSelectedList )
		{
			m_pvecMusicFullPaths = &m_vecMusicList[iSelectedItem].second;
			m_iItem = -1;
		}
	}

	if( strText == _T("�ļ�") )
	{
		CFileDialog dlg( TRUE, NULL, NULL, 0, _T("����|*.mp3;*.ogg||") );
		if( dlg.DoModal() == IDOK )
		{
			m_lcMusic.InsertItem( m_lcMusic.GetItemCount(), (LPCTSTR)dlg.GetFileName() );
			
			m_pvecMusicFullPaths->push_back( (LPCTSTR)dlg.GetPathName() );

			POSITION pos = m_lcMusic.GetFirstSelectedItemPosition();			
			while( pos )
			{
				m_iItem = m_lcMusic.GetNextSelectedItem(pos);
			}
		}
	}
	else if( strText == _T("Ŀ¼") )
	{
		static TCHAR szFolderName[MAX_PATH];
		BROWSEINFO bi = { 0 };
		bi.hwndOwner = this->m_hWnd;
		bi.lpszTitle = _T("��ѡ��һ���ļ���");
		bi.pidlRoot = NULL;//ILCreateFromPath( _T("F:\\����") );
		bi.pszDisplayName = szFolderName;
		bi.ulFlags = BIF_NONEWFOLDERBUTTON;

		if( SHGetPathFromIDList( SHBrowseForFolder( &bi ), szFolderName ) )
		{
				CString strFolderName = szFolderName;
				strFolderName += _T("\\");
				CString strPattern = strFolderName + _T("\\*.*");
				
				WIN32_FIND_DATA FindData;
				ZeroMemory( &FindData, sizeof(FindData) );

				HANDLE hFile = FindFirstFile( (LPCTSTR)strPattern, &FindData );

				if( hFile != NULL )
				{
					int len = _tcslen(FindData.cFileName);
					if( len > 5 )
					{
						if( _tcsicmp( FindData.cFileName + len - 4, _T(".mp3") ) ==  0 ||
							_tcsicmp( FindData.cFileName + len - 4, _T(".ogg") ) == 0 )
						{
							CString strFileName = strFolderName + FindData.cFileName;
							m_pvecMusicFullPaths->push_back( (LPCTSTR)strFileName );
							m_lcMusic.InsertItem( m_lcMusic.GetItemCount(), FindData.cFileName );
						}
					}
					while( FindNextFile( hFile, &FindData ) != NULL )
					{
						int len = _tcslen(FindData.cFileName);
						if( len > 5 )
						{
							if( _tcsicmp( FindData.cFileName + len - 4, _T(".mp3") ) ==  0 ||
								_tcsicmp( FindData.cFileName + len - 4, _T(".ogg") ) == 0 )
							{
								CString strFileName = strFolderName + FindData.cFileName;
								m_pvecMusicFullPaths->push_back( (LPCTSTR)strFileName );
								m_lcMusic.InsertItem( m_lcMusic.GetItemCount(), FindData.cFileName );
							}
						}
					}
				}
				FindClose( hFile );
		}

		//CFolderPickerDialog dlg1;
		//if( dlg1.DoModal() == IDOK )
		//{
		//	CString strFolderName = dlg.GetFolderPath() + _T("\\");
		//	CString strPattern = strFolderName + _T("\\*.*");
		//	
		//	WIN32_FIND_DATA FindData;
		//	ZeroMemory( &FindData, sizeof(FindData) );

		//	HANDLE hFile = FindFirstFile( (LPCTSTR)strPattern, &FindData );

		//	if( hFile != NULL )
		//	{
		//		int len = _tcslen(FindData.cFileName);
		//		if( len > 5 )
		//		{
		//			if( _tcsicmp( FindData.cFileName + len - 4, _T(".mp3") ) ==  0 ||
		//				_tcsicmp( FindData.cFileName + len - 4, _T(".ogg") ) == 0 )
		//			{
		//				CString strFileName = strFolderName + FindData.cFileName;
		//				m_pvecMusicFullPaths->push_back( (LPCTSTR)strFileName );
		//				m_lcMusic.InsertItem( m_lcMusic.GetItemCount(), FindData.cFileName );
		//			}
		//		}
		//		while( FindNextFile( hFile, &FindData ) != NULL )
		//		{
		//			int len = _tcslen(FindData.cFileName);
		//			if( len > 5 )
		//			{
		//				if( _tcsicmp( FindData.cFileName + len - 4, _T(".mp3") ) ==  0 ||
		//					_tcsicmp( FindData.cFileName + len - 4, _T(".ogg") ) == 0 )
		//				{
		//					CString strFileName = strFolderName + FindData.cFileName;
		//					m_pvecMusicFullPaths->push_back( (LPCTSTR)strFileName );
		//					m_lcMusic.InsertItem( m_lcMusic.GetItemCount(), FindData.cFileName );
		//				}
		//			}
		//		}
		//	}
		//	FindClose( hFile );
		//}
	}
}


void CMusicPlayerDlg::OnCbnSelchangePlaymode()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	if( m_pvecMusicFullPaths == NULL )
		return;

	int i = m_cbPlayMode.GetCurSel();
	CString strText;
	int len = m_cbPlayMode.GetLBTextLen( i );
	m_cbPlayMode.GetLBText( i, strText.GetBuffer(len) );
	
	if( strText == _T("����ѭ��") )
		m_iPlayMode = 0;
	else if( strText == _T("ѭ������") )
		m_iPlayMode = 1;
	else if( strText == _T("˳�򲥷�"))
		m_iPlayMode = 2;
}

void CMusicPlayerDlg::OnBnClickedAddlist()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	if( m_pvecMusicFullPaths == NULL )
		return;

	TCHAR msg[MAX_PATH] = { 0 };
	_stprintf( msg, _T("���б�%d"), m_lcListName.GetItemCount() + 1 );
	m_lcListName.InsertItem( m_lcListName.GetItemCount(), msg );

	m_vecMusicList.push_back( std::make_pair(msg, std::vector<std::wstring>() ));
}


void CMusicPlayerDlg::OnBnClickedDellist()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	if( m_pvecMusicFullPaths == NULL )
		return;

	POSITION pos = m_lcListName.GetFirstSelectedItemPosition();
	std::vector<int> vecSelected;

	while( pos )
	{
		int iItem = m_lcListName.GetNextSelectedItem(pos);
		vecSelected.push_back( iItem );	
	}
	
	for( int i = 0; i < vecSelected.size(); ++i )
	{
		m_lcListName.DeleteItem( vecSelected[i] - i );

		std::vector<std::pair<std::wstring, std::vector<std::wstring>> >::iterator iter =
			m_vecMusicList.begin();
		for( int j = 0; j < vecSelected[i] - i; ++j )
			++iter;

		m_lcMusic.DeleteAllItems();
		if( m_iListPlaying == vecSelected[i] )
		{
			m_pvecMusicFullPaths = &m_vecNull;
			m_iItem = -1;
		}

		if( m_iListPlaying > vecSelected[i] - i )
		{
			m_iCurSelectedList = m_iListPlaying -= 1;
		}
		m_vecMusicList.erase( iter );
		m_pvecMusicFullPaths = &m_vecMusicList[m_iListPlaying].second;
	}
	
	for( int i = 0; i < m_pvecMusicFullPaths->size(); ++i )
	{
		const TCHAR* p = _tcsrchr( m_pvecMusicFullPaths->at(i).c_str(), _T('\\'));
		if( p != NULL )
			m_lcMusic.InsertItem( i, p + 1 );
		else
			m_lcMusic.InsertItem( i, m_pvecMusicFullPaths->at(i).c_str() );
	}
}


void CMusicPlayerDlg::OnNMDblclkMusiclist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	*pResult = 0;

	if( m_pvecMusicFullPaths == NULL )
		return;

	POINT cp; 
	LVHITTESTINFO lhtInfo;
	ZeroMemory( &lhtInfo, sizeof(lhtInfo) );
	GetCursorPos( &cp ); 
	m_lcListName.ScreenToClient( &cp );  //m_TREE��CTreeCtrl�ؼ�������
	lhtInfo.pt = cp;
	m_lcListName.HitTest( &lhtInfo );
	int iItem = lhtInfo.iItem; //��ȡ��ǰ����Ҽ�������λ���µ�item
	if( iItem > -1 )
		m_lcListName.EditLabel( iItem );
}


void CMusicPlayerDlg::OnNMClickMusiclist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	*pResult = 0;

	if( m_pvecMusicFullPaths == NULL )
		return;

	POINT cp; 
	LVHITTESTINFO lhtInfo;
	ZeroMemory( &lhtInfo, sizeof(lhtInfo) );
	GetCursorPos( &cp ); 
	m_lcListName.ScreenToClient( &cp );  //m_TREE��CTreeCtrl�ؼ�������
	lhtInfo.pt = cp;
	m_lcListName.HitTest( &lhtInfo );
	int iItem = lhtInfo.iItem; //��ȡ��ǰ����Ҽ�������λ���µ�item

	if( iItem > -1 )
	{
		if( iItem < m_vecMusicList.size() )
		{
			m_lcMusic.DeleteAllItems();
			m_pvecMusicFullPaths = &m_vecMusicList[iItem].second;
			m_iCurSelectedList = iItem;

			for( int i = 0; i < m_pvecMusicFullPaths->size(); ++i )
			{
				const TCHAR* p = _tcsrchr( m_pvecMusicFullPaths->at(i).c_str(), _T('\\'));
				if( p != NULL )
					m_lcMusic.InsertItem( i, p + 1 );
				else
					m_lcMusic.InsertItem( i, m_pvecMusicFullPaths->at(i).c_str() );
			}
		}
	}
}


void CMusicPlayerDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	if( m_pvecMusicFullPaths == NULL )
		return;

	switch( nIDEvent )
	{
	case 1000:
		{
			if( m_pAudioPlayer != NULL && !m_bPause )
			{
				VARIANT_BOOL bEnd = VARIANT_TRUE;
				m_pAudioPlayer->get_End(&bEnd);
				if( bEnd == VARIANT_FALSE )
				{
					m_pAudioPlayer->Play();
				}
				else
				{
					if( m_pvecMusicFullPaths->size() < 1 )
						break;

					if( m_iPlayMode == 0 )
					{
						m_pAudioPlayer->Replay();
					}

					else if( m_iPlayMode == 1 )
					{
						int iItem = m_iItem;
						if( m_iItem > -1 && m_iItem < m_pvecMusicFullPaths->size() )
						{
							iItem = (m_iItem + 1) % m_pvecMusicFullPaths->size();
						}
						else
							iItem = 0;

						m_lcMusic.SetItemState( iItem, LVIS_SELECTED, LVIS_SELECTED );
						OnBnClickedPlaypause();
					}

					else if( m_iPlayMode == 2 )
					{
						if( m_iItem > -1 && m_iItem < m_pvecMusicFullPaths->size() - 1 )
						{
							m_lcMusic.SetItemState( m_iItem + 1, LVIS_SELECTED, LVIS_SELECTED );
							OnBnClickedPlaypause();
						}						
					}
				}
			}
			if( m_bPause )
				m_pAudioPlayer->Pause();

			break;
		}

	case 1001:
		{
				 if (m_pAudioPlayer != NULL)
			{
				float fPlayingProcess = 0.0f;
				m_pAudioPlayer->get_PlayingProcess(&fPlayingProcess);
				int iMax = m_scPlayingProcess.GetRangeMax();
				m_scPlayingProcess.SetPos( iMax * fPlayingProcess );
			}
			else
			{
				m_scPlayingProcess.SetPos( 0 );					
			}
			break;
		}
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CMusicPlayerDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: �ڴ˴������Ϣ����������
	KillTimer( 1000 );
	KillTimer( 1001 );

	if (m_pAudioPlayer)
	{
		delete m_pAudioPlayer;
		m_pAudioPlayer = NULL;
	}

	NOTIFYICONDATA nid = { 0 };
	nid.hWnd = m_hWnd;
	Shell_NotifyIcon( NIM_DELETE, &nid );

	SaveSettings();
}


void CMusicPlayerDlg::OnBnClickedPrev()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	if( m_pvecMusicFullPaths == NULL )
		return;

	POSITION pos = m_lcListName.GetFirstSelectedItemPosition();
	int iSelectedItem = -1;

	while( pos )
	{
		iSelectedItem = m_lcListName.GetNextSelectedItem(pos);
	}

	if( iSelectedItem != -1 && iSelectedItem < m_vecMusicList.size() )
	{
		if( iSelectedItem != m_iListPlaying )
		{
			m_iCurSelectedList = iSelectedItem;
			m_pvecMusicFullPaths = &m_vecMusicList[m_iCurSelectedList].second;
			m_iItem = 1;
		}
	}

	if( m_pvecMusicFullPaths->size() < 1 )
		return;

	if( m_iItem <= 0 )
		m_lcMusic.SetItemState( m_pvecMusicFullPaths->size() - 1, LVIS_SELECTED, LVIS_SELECTED );
	else
		m_lcMusic.SetItemState( m_iItem - 1, LVIS_SELECTED, LVIS_SELECTED );

	OnBnClickedPlaypause();
}


void CMusicPlayerDlg::OnBnClickedNext()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	if( m_pvecMusicFullPaths == NULL )
		return;

	POSITION pos = m_lcListName.GetFirstSelectedItemPosition();
	int iSelectedItem = -1;

	while( pos )
	{
		iSelectedItem = m_lcListName.GetNextSelectedItem(pos);
	}

	if( iSelectedItem != -1 && iSelectedItem < m_vecMusicList.size() )
	{
		if( iSelectedItem != m_iListPlaying )
		{
			m_iCurSelectedList = iSelectedItem;
			m_pvecMusicFullPaths = &m_vecMusicList[m_iCurSelectedList].second;
			m_iItem = -1;
		}
	}

	if( m_pvecMusicFullPaths->size() < 1 )
		return;

	if( m_iItem >= m_pvecMusicFullPaths->size() - 1 )
		m_lcMusic.SetItemState( 0, LVIS_SELECTED, LVIS_SELECTED );
	else
		m_lcMusic.SetItemState( m_iItem + 1, LVIS_SELECTED, LVIS_SELECTED );

	
	OnBnClickedPlaypause();
}
	


void CMusicPlayerDlg::OnNMDblclkMusicdetail(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	*pResult = 0;

	if( m_pvecMusicFullPaths == NULL )
		return;

	POINT cp; 
	LVHITTESTINFO lhtInfo;
	ZeroMemory( &lhtInfo, sizeof(lhtInfo) );
	GetCursorPos( &cp ); 
	m_lcMusic.ScreenToClient( &cp );  //m_TREE��CTreeCtrl�ؼ�������
	lhtInfo.pt = cp;
	m_lcMusic.HitTest( &lhtInfo );
	int iItem = lhtInfo.iItem; //��ȡ��ǰ����Ҽ�������λ���µ�item
	if( iItem > -1 )
	{
		POSITION pos = m_lcListName.GetFirstSelectedItemPosition();
		int iSelectedItem = -1;

		while( pos )
		{
			iSelectedItem = m_lcListName.GetNextSelectedItem(pos);
		}

		if( iSelectedItem != -1 && iSelectedItem < m_vecMusicList.size() )
		{
			if( iSelectedItem != m_iListPlaying )
			{
				m_iCurSelectedList = iSelectedItem;
				m_pvecMusicFullPaths = &m_vecMusicList[m_iCurSelectedList].second;
				m_iItem = -1;
			}
		}

		OnBnClickedPlaypause();
	}

}


void CMusicPlayerDlg::OnLvnKeydownMusicdetail(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	*pResult = 0;

	if( m_pvecMusicFullPaths == NULL )
		return;

	if( pLVKeyDow->wVKey != VK_DELETE )
		return;

	int iSelectedItem = -1;
	POSITION pos = m_lcMusic.GetFirstSelectedItemPosition();
	while( pos )
	{
		iSelectedItem = m_lcMusic.GetNextSelectedItem(pos);
	}

	if( iSelectedItem > -1 )
	{
		std::vector<std::wstring>::iterator iter = 
			m_pvecMusicFullPaths->begin();
		int counter = 0;
		while( ++counter <= iSelectedItem )
		{
			++iter;
		}
		m_lcMusic.DeleteItem( iSelectedItem );
		m_pvecMusicFullPaths->erase( iter );
		m_iItem = -1;

		if( iSelectedItem > -1 && iSelectedItem < m_pvecMusicFullPaths->size() )
			m_lcMusic.SetItemState( iSelectedItem, LVIS_SELECTED, LVIS_SELECTED );
		else
		{
			iSelectedItem -= 1;
			if( iSelectedItem > -1 && iSelectedItem < m_pvecMusicFullPaths->size() )
				m_lcMusic.SetItemState( iSelectedItem, LVIS_SELECTED, LVIS_SELECTED );			
		}
	}
}


void CMusicPlayerDlg::OnTRBNThumbPosChangingSlider2(NMHDR *pNMHDR, LRESULT *pResult)
{
	// �˹���Ҫ�� Windows Vista ����߰汾��
	// _WIN32_WINNT ���ű��� >= 0x0600��
	NMTRBTHUMBPOSCHANGING *pNMTPC = reinterpret_cast<NMTRBTHUMBPOSCHANGING *>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	*pResult = 0;

	if( m_pvecMusicFullPaths == NULL )
		return;

	int iPos = m_scVolume.GetPos();
	int iMaxPos = m_scVolume.GetRangeMax();
	int iMinPos = m_scVolume.GetRangeMin();

	float fVolumn = (float)iPos / (float)iMaxPos;
	if( m_pAudioPlayer != NULL )
	{
		m_pAudioPlayer->put_Volume(fVolumn);
	}
}


void CMusicPlayerDlg::OnNMCustomdrawSlider2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	*pResult = 0;

	if( m_pvecMusicFullPaths == NULL )
		return;

	OnTRBNThumbPosChangingSlider2( pNMHDR, pResult );
}


void CMusicPlayerDlg::OnNMReleasedcaptureSlider2(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	*pResult = 0;

}

LRESULT CMusicPlayerDlg::OnSysTrayNotify( WPARAM wParam, LPARAM lParam )
{
	switch( lParam )
	{
	case WM_LBUTTONUP:
		{
			if( !IsWindowVisible() )
			{
				ShowWindow( SW_SHOW );				
			}
			if( IsIconic() )
				ShowWindow( SW_NORMAL );
			SetForegroundWindow();

			break;
		}
	case WM_RBUTTONDOWN:
		{
			if( IsWindowVisible() )
			{
				ShowWindow( SW_HIDE );		
			}
			break;
		}
	}
	return 0;
}

void CMusicPlayerDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	
	CDialogEx::OnGetMinMaxInfo(lpMMI);
}


LRESULT CMusicPlayerDlg::OnNcHitTest(CPoint point)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	
	LRESULT lResult = CDialogEx::OnNcHitTest(point);
	switch( lResult )
	{
	case HTMINBUTTON:
		{
			if( IsWindowVisible() )
				ShowWindow( SW_HIDE );
			break;
		}
	}
	return lResult;
}


void CMusicPlayerDlg::OnNcLButtonUp(UINT nHitTest, CPoint point)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	CDialogEx::OnNcLButtonUp(nHitTest, point);
}


void CMusicPlayerDlg::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	if( nHitTest == HTMINBUTTON )
		ShowWindow( SW_HIDE );

	CDialogEx::OnNcLButtonDown(nHitTest, point);
}


void CMusicPlayerDlg::OnLvnEndlabeleditMusiclist(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	*pResult = 0;

	m_lcListName.SetItem( &pDispInfo->item );
	if( pDispInfo->item.iItem < m_vecMusicList.size() &&
		pDispInfo->item.iItem > -1 )
	{
		if( pDispInfo->item.pszText )
			m_vecMusicList[pDispInfo->item.iItem].first = pDispInfo->item.pszText;
	}

}

void CMusicPlayerDlg::LoadSettings()
{
	std::ifstream f("D:\\Projects\\bin\\setting.json");
	if( !f.is_open())
		return;

	auto doc = nlohmann::json::parse(f); 

	int iEncoding = CP_UTF8;

	for(auto& item : doc.items())
	{
		const std::string& lstName = item.value()["name"];
		if (lstName.empty())
			continue;

		auto len = MultiByteToWideChar(iEncoding, 0, lstName.c_str(), -1, NULL, 0);
		auto pwName = std::make_shared<wchar_t[]>(len);
		MultiByteToWideChar(iEncoding, 0, lstName.c_str(), -1, pwName.get(), len);
		m_vecMusicList.push_back(std::make_pair(pwName.get(), std::vector<std::wstring>()));

		auto& refMusicNames = m_vecMusicList[m_vecMusicList.size() - 1].second;

		for (auto& music : item.value()["list"])
		{
			const std::string& path = music;
			len = MultiByteToWideChar(iEncoding, 0, path.c_str(), -1, NULL, 0);
			auto pwMusicName = std::make_shared<wchar_t[]>(len);
			MultiByteToWideChar(iEncoding, 0, path.c_str(), -1, pwMusicName.get(), len);

			refMusicNames.push_back(pwMusicName.get());
		}
	}
}

void CMusicPlayerDlg::SaveSettings()
{
	nlohmann::json doc;

	int iEncoding = CP_UTF8;

	for( int i = 0; i < m_vecMusicList.size(); ++i )
	{
		int len = WideCharToMultiByte( iEncoding, 0, m_vecMusicList[i].first.c_str(), -1, NULL, 0, NULL, NULL );
		auto plistName = std::make_shared<char[]>(len);
		WideCharToMultiByte( iEncoding, 0, m_vecMusicList[i].first.c_str(), -1, plistName.get(), len, NULL, NULL );

		nlohmann::json lst;

		for( int j = 0; j < m_vecMusicList[i].second.size(); ++j )
		{
			int len = WideCharToMultiByte( iEncoding, 0, m_vecMusicList[i].second[j].c_str(), -1, NULL, 0, NULL, NULL );
			auto pName = std::make_shared<char[]>(len);
			WideCharToMultiByte( iEncoding, 0, m_vecMusicList[i].second[j].c_str(), -1, pName.get(), len, NULL, NULL);

			lst.push_back(pName.get()); 
		}
		nlohmann::json musicList;
		musicList["name"] = plistName.get();
		musicList["list"] = lst;
		doc.push_back(musicList);
	}

	std::ofstream of("setting.json"); 

	std::string&& s = std::move(doc.dump(4));
	of.write(s.data(), s.size());
}
