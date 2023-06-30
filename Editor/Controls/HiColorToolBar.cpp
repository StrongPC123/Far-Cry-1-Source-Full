// HiColorToolBar.cpp : implementation file
//

#include "stdafx.h"
#include "HiColorToolBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHiColorToolBar

CHiColorToolBar::CHiColorToolBar()
{
}

CHiColorToolBar::~CHiColorToolBar()
{
}


BEGIN_MESSAGE_MAP(CHiColorToolBar, CToolBar)
	//{{AFX_MSG_MAP(CHiColorToolBar)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHiColorToolBar message handlers

void CHiColorToolBar::AttachToolbarImages(UINT inNormalImageID)
{
	////////////////////////////////////////////////////////////////////////
	// Load the high color toolbar images and attach them to the toolbar
	////////////////////////////////////////////////////////////////////////

	// Make high-color image lists for each of the bitmaps
	MakeToolbarImageList(inNormalImageID, m_ToolbarImages, LTNormal);
	MakeToolbarImageList(inNormalImageID, m_ToolbarImagesDisabled, LTDisabled);
	MakeToolbarImageList(inNormalImageID, m_ToolbarImagesHot, LTHot);

	// Get the toolbar control associated with the CToolbar object
	CToolBarCtrl& barCtrl = GetToolBarCtrl();

	// Attach the image lists to the toolbar control
	barCtrl.SetImageList(&m_ToolbarImages);
	barCtrl.SetDisabledImageList(&m_ToolbarImagesDisabled);
	barCtrl.SetHotImageList(&m_ToolbarImagesHot);
}

void CHiColorToolBar::ReplaceBackgroundColor(CBitmap& ioBM)
{
	////////////////////////////////////////////////////////////////////////
	// Find every pixel of the default background color in the specified
	// bitmap and set each one to the user's button color
	////////////////////////////////////////////////////////////////////////

	// Figure out how many pixels there are in the bitmap
	BITMAP		bmInfo;

	VERIFY (ioBM.GetBitmap (&bmInfo));

	// Add support for additional bit depths if you choose
	VERIFY (bmInfo.bmBitsPixel == 24);
	VERIFY (bmInfo.bmWidthBytes == (bmInfo.bmWidth * 3));

	const UINT numPixels (bmInfo.bmHeight * bmInfo.bmWidth);

	// Get a pointer to the pixels
	DIBSECTION  ds;

	VERIFY (ioBM.GetObject (sizeof (DIBSECTION), &ds) == sizeof (DIBSECTION));

	RGBTRIPLE*		pixels = reinterpret_cast<RGBTRIPLE *> (ds.dsBm.bmBits);
	VERIFY (pixels != NULL);

	// Get the user's preferred button color from the system
	const COLORREF		buttonColor (::GetSysColor (COLOR_BTNFACE));
	const RGBTRIPLE		userBackgroundColor = {
	GetBValue (buttonColor), GetGValue (buttonColor), GetRValue (buttonColor)};

	// Search through the pixels, substituting the user's button
	// color for any pixel that has the magic background color
	for (UINT i = 0; i < numPixels; ++i)
	{
		if (pixels[i].rgbtBlue == kBackgroundColor.rgbtBlue 
			&& pixels[i].rgbtGreen == kBackgroundColor.rgbtGreen 
			&& pixels[i].rgbtRed == kBackgroundColor.rgbtRed)
		{
			pixels [i] = userBackgroundColor;
		}
	}
}

void CHiColorToolBar::MakeDisabled(CBitmap &ioBM)
{
	////////////////////////////////////////////////////////////////////////
	// Give the bitmap a disabled look
	////////////////////////////////////////////////////////////////////////

	// Figure out how many pixels there are in the bitmap
	BITMAP		bmInfo;

	VERIFY (ioBM.GetBitmap (&bmInfo));

	// Add support for additional bit depths if you choose
	VERIFY (bmInfo.bmBitsPixel == 24);
	VERIFY (bmInfo.bmWidthBytes == (bmInfo.bmWidth * 3));

	const UINT numPixels (bmInfo.bmHeight * bmInfo.bmWidth);

	// Get a pointer to the pixels
	DIBSECTION  ds;

	VERIFY (ioBM.GetObject (sizeof (DIBSECTION), &ds) == sizeof (DIBSECTION));

	RGBTRIPLE*		pixels = reinterpret_cast<RGBTRIPLE *> (ds.dsBm.bmBits);
	VERIFY (pixels != NULL);

	// Get the user's preferred button color from the system
	const COLORREF		buttonColor (::GetSysColor (COLOR_BTNFACE));
	const RGBTRIPLE		userBackgroundColor = {
	GetBValue (buttonColor), GetGValue (buttonColor), GetRValue (buttonColor)};

	// Loop trough all pixels
	for (UINT i = 0; i < numPixels; ++i)
	{
		if (pixels[i].rgbtBlue == userBackgroundColor.rgbtBlue 
			&& pixels[i].rgbtGreen == userBackgroundColor.rgbtGreen 
			&& pixels[i].rgbtRed == userBackgroundColor.rgbtRed)
		{
			// Skip pixels that have the background color
			continue;
		}

		// Average out the BGR channels
		pixels[i].rgbtBlue = (pixels[i].rgbtBlue + pixels[i].rgbtGreen + pixels[i].rgbtRed) / 3;
		pixels[i].rgbtRed = pixels[i].rgbtGreen = pixels[i].rgbtBlue;
	}
}

void CHiColorToolBar::MakeToolbarImageList(UINT inBitmapID, CImageList& outImageList, LoadType lt)
{
	////////////////////////////////////////////////////////////////////////
	// Create an image list for the specified BMP resource
	////////////////////////////////////////////////////////////////////////

	CBitmap bm;
	DWORD dwStyleParam = 0;

	// If we use CBitmap::LoadBitmap() to load the bitmap, the colors
	// will be reduced to the bit depth of the main screen and we won't
	// be able to access the pixels directly. To avoid those problems,
	// we'll load the bitmap as a DIBSection instead and attach the
	// DIBSection to the CBitmap.
	VERIFY(bm.Attach (::LoadImage (::AfxFindResourceHandle(
		MAKEINTRESOURCE (inBitmapID), RT_BITMAP),
		MAKEINTRESOURCE (inBitmapID), IMAGE_BITMAP, 0, 0,
		(LR_DEFAULTSIZE | LR_CREATEDIBSECTION) | dwStyleParam)));

	// Replace the specified color in the bitmap with the user's
	// button color
	ReplaceBackgroundColor(bm);

	// Disable it when requested
	if (lt == LTDisabled)
		MakeDisabled(bm);

	// Create a 24 bit image list with the same dimensions and number
	// of buttons as the toolbar
	VERIFY(outImageList.Create (
	kImageWidth, kImageHeight, kToolBarBitDepth, kNumImages, 0));

	// Attach the bitmap to the image list
	VERIFY(outImageList.Add (&bm, RGB (0, 0, 0)) != -1);
}

void CHiColorToolBar::AddTextStrings()
{
	////////////////////////////////////////////////////////////////////////
	// Add text descriptions to a tool bar
	////////////////////////////////////////////////////////////////////////

	// Set the text for each button
	CToolBarCtrl& bar = GetToolBarCtrl();

	// Remove the string map in case we are loading another toolbar into this control
	if (m_pStringMap)
	{
		delete m_pStringMap;
		m_pStringMap = NULL;
	}
	
	int	nIndex = 0;
	TBBUTTON tb;

	for (nIndex = bar.GetButtonCount() - 1; nIndex >= 0; nIndex--)
	{
		ZeroMemory(&tb, sizeof(TBBUTTON));
		bar.GetButton(nIndex, &tb);

		// Do we have a separator?
		if ((tb.fsStyle & TBSTYLE_SEP) == TBSTYLE_SEP)
			continue;

		// Have we got a valid command id?
		if (tb.idCommand == 0)
			continue;

		// Get the resource string if there is one.
		CString strText;
		LPCTSTR lpszButtonText = NULL;
		CString	strButtonText(_T(""));
		_TCHAR	seps[] = _T("\n");

		strText.LoadString(tb.idCommand);

		if (!strText.IsEmpty())
		{
			lpszButtonText = _tcstok((LPTSTR) (LPCTSTR) strText, seps);

			while(lpszButtonText)
			{
				strButtonText = lpszButtonText;
				lpszButtonText = _tcstok(NULL, seps);
			}
		}

		if (!strButtonText.IsEmpty())
			SetButtonText(nIndex, strButtonText);
	}

	// Resize the buttons so that the text will fit.
	CRect rc(0, 0, 0, 0);
	CSize sizeMax(0, 0);

	for (nIndex = bar.GetButtonCount() - 1; nIndex >= 0; nIndex--)
	{
		bar.GetItemRect(nIndex, rc);

		rc.NormalizeRect();
		sizeMax.cx = __max(rc.Size().cx, sizeMax.cx);
		sizeMax.cy = __max(rc.Size().cy, sizeMax.cy);
	}

	SetSizes(sizeMax, CSize(32, 32));
	
	// Set the minimum button size
	SendMessage(TB_SETBUTTONWIDTH, 0, (LPARAM) (DWORD) MAKELONG(85, 128));
}

bool CHiColorToolBar::CreateAll(UINT nIDRessource, UINT inNormalImageID, CWnd *pwndParent)
{
	////////////////////////////////////////////////////////////////////////
	// Load all data required for the full toolbar functionality
	////////////////////////////////////////////////////////////////////////

	CLogFile::WriteLine("Creating new high color toolbar...");

	// Create the toolbar itself
	if (!CreateEx(pwndParent, TBSTYLE_FLAT | TBSTYLE_WRAPABLE, WS_CHILD | CBRS_TOP
		| CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC))
	{
		return false;
	}

	// Load the buttons
	if (!LoadToolBar(nIDRessource))
		return false;

	// Attach the 24bpp images
	AttachToolbarImages(inNormalImageID);

	// Read the text strings out of the ressource and set them
	AddTextStrings();

	// Set maximum number of rows
	GetToolBarCtrl().SetRows(99, TRUE, NULL);

	// Indent the first button a few pixels so that the toolbar is centered
	GetToolBarCtrl().SetIndent(4);

	return true;
}