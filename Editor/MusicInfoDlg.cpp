// MusicInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MusicInfoDlg.h"
#include <ISound.h>

// CMusicInfoDlg dialog

IMPLEMENT_DYNAMIC(CMusicInfoDlg, CDialog)
CMusicInfoDlg::CMusicInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMusicInfoDlg::IDD, pParent)
{
}

CMusicInfoDlg::~CMusicInfoDlg()
{
	DestroyWindow();
}

void CMusicInfoDlg::Resize()
{
	CRect rcClient;
	GetClientRect(rcClient);
	if (m_wndPlayingFrame.GetSafeHwnd())
		m_wndPlayingFrame.SetWindowPos(NULL, 0, 0, rcClient.Width()-(m_rcClient.Width()-m_rcPlayingFrame.Width()), rcClient.Height()-(m_rcClient.Height()-m_rcPlayingFrame.Height()), SWP_NOZORDER | SWP_NOMOVE);
	if (m_wndPlaying.GetSafeHwnd())
	{
		m_wndPlaying.SetWindowPos(NULL, 0, 0, rcClient.Width()-(m_rcClient.Width()-m_rcPlaying.Width()), rcClient.Height()-(m_rcClient.Height()-m_rcPlaying.Height()), SWP_NOZORDER | SWP_NOMOVE);
		// Delete all of the columns.
		int nColumnCount=m_wndPlaying.GetHeaderCtrl()->GetItemCount();
		for (int i=0;i<nColumnCount;i++)
		{
			m_wndPlaying.DeleteColumn(0);
		}
		// create columns in PLAYING-listview...
		LVCOLUMN ColumnInfo;
		ColumnInfo.mask=LVCF_TEXT | LVCF_WIDTH;

		CRect rcPlayingClient;
		m_wndPlaying.GetClientRect(rcPlayingClient);

		ColumnInfo.cx=(int)((float)m_rcPlayingClient.Width()*0.2f);
		ColumnInfo.pszText="Layer";
		m_wndPlaying.InsertColumn(0, &ColumnInfo);

		ColumnInfo.cx=(int)((float)m_rcPlayingClient.Width()*0.65f)+(rcPlayingClient.Width()-m_rcPlayingClient.Width());
		ColumnInfo.pszText="Pattern";
		m_wndPlaying.InsertColumn(1, &ColumnInfo);

		ColumnInfo.cx=(int)((float)m_rcPlayingClient.Width()*0.15f);
		ColumnInfo.pszText="Vol";
		m_wndPlaying.InsertColumn(2, &ColumnInfo);
	}
}

void CMusicInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PLAYING, m_wndPlaying);
	DDX_Control(pDX, IDC_STREAMING, m_wndStreaming);
	DDX_Control(pDX, IDC_THEME, m_wndTheme);
	DDX_Control(pDX, IDC_MOOD, m_wndMood);
	DDX_Control(pDX, IDC_PLAYINGFRAME, m_wndPlayingFrame);
}


BEGIN_MESSAGE_MAP(CMusicInfoDlg, CDialog)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_SIZING()
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CMusicInfoDlg message handlers

BOOL CMusicInfoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	GetClientRect(m_rcClient);
	m_wndPlayingFrame.GetWindowRect(m_rcPlayingFrame);
	m_wndPlaying.GetWindowRect(m_rcPlaying);
	m_wndPlaying.GetClientRect(m_rcPlayingClient);

	Resize();

	m_hTimer=SetTimer(1, 250, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CMusicInfoDlg::OnDestroy()
{
	KillTimer(m_hTimer);
	CDialog::OnDestroy();
}

void CMusicInfoDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent==1)
	{
		IMusicSystem *pMusicSystem=GetIEditor()->GetSystem()->GetIMusicSystem();
		if (pMusicSystem && IsWindowVisible())
		{
			SMusicSystemStatus *pStatus=pMusicSystem->GetStatus();
			if (pStatus)
			{
				m_wndPlaying.DeleteAllItems();
				if (pStatus->bPlaying)
					m_wndStreaming.SetWindowText("Yes");
				else
					m_wndStreaming.SetWindowText("No");
				m_wndTheme.SetWindowText(pStatus->sTheme.c_str());
				m_wndMood.SetWindowText(pStatus->sMood.c_str());
				LVITEM ItemInfo;
				ItemInfo.mask=LVIF_TEXT | LVIF_STATE;
				ItemInfo.state=0;
				ItemInfo.stateMask=0xffffffff;
				ItemInfo.iImage=0;
				for (TPatternStatusVecIt It=pStatus->m_vecPlayingPatterns.begin();It!=pStatus->m_vecPlayingPatterns.end();++It)
				{
					SPlayingPatternsStatus &PatternStatus=(*It);
					char sText[16];
					ItemInfo.iItem=m_wndPlaying.GetItemCount();
					ItemInfo.iSubItem=0;
					switch (PatternStatus.nLayer)
					{
						case MUSICLAYER_MAIN:				strcpy(sText, "Main");	break;
						case MUSICLAYER_RHYTHMIC:		strcpy(sText, "Rhyt");	break;
						case MUSICLAYER_INCIDENTAL:	strcpy(sText, "Inci");	break;
						default:										strcpy(sText, "Unkn");	break;
					}
					ItemInfo.pszText=sText;
					int nItem=m_wndPlaying.InsertItem(&ItemInfo);
					m_wndPlaying.SetItemText(nItem, 1, PatternStatus.sName.c_str());
					sprintf(sText, "%03d", PatternStatus.nVolume);
					m_wndPlaying.SetItemText(nItem, 2, sText);
				}
			}
		}
	}
	CDialog::OnTimer(nIDEvent);
}

void CMusicInfoDlg::OnSizing(UINT fwSide, LPRECT pRect)
{
	CDialog::OnSizing(fwSide, pRect);
	Resize();
}

void CMusicInfoDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	Resize();
}
