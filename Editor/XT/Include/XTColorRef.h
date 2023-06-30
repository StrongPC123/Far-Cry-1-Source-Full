// XTColorRef.h: interface for the CXTColorRef class.
//
// This file is a part of the Xtreme Toolkit for MFC.
// ©1998-2003 Codejock Software, All Rights Reserved.
//
// This source code can only be used under the terms and conditions 
// outlined in the accompanying license agreement.
//
// support@codejock.com
// http://www.codejock.com
//
//////////////////////////////////////////////////////////////////////

#if !defined(__XTCOLORREF_H__)
#define __XTCOLORREF_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
// Summary: CXTColorRef is a stand alone class.  It is used to create a 
//			CXTColorRef object. This object is used for manipulation of RGB values.
class _XT_EXT_CLASS CXTColorRef
{
public:

	// Summary: Constructs a CXTColorRef object.
	CXTColorRef();

	// Input:	r - An RGB value.
	// Summary: Constructs a CXTColorRef object.
	CXTColorRef(COLORREF r);

	// Input:	r - Specifies the intensity of the red color component. 'r' can range
	//			from 0 to 255. Zero is the minimum color intensity; 255 is the maximum
	//			color intensity.
	//			g - Specifies the intensity of the green color component. 'g' can range
	//			from 0 to 255. Zero is the minimum color intensity; 255 is the maximum
	//			color intensity.
	//			b - Specifies the intensity of the blue color component. 'b' can range
	//			from 0 to 255. Zero is the minimum color intensity; 255 is the maximum
	//			color intensity.
	// Summary: Constructs a CXTColorRef object.
	CXTColorRef(int r, int g, int b);

    // Input:	cr - An RGB value.
	// Returns: A reference to a valid CXTColorRef object.
	// Summary: This assignment operator is used to initialize a CXTColorRef object.
	CXTColorRef& operator = (COLORREF cr);

    // Input:	val - Specifies the intensity of the color component. 'val' can range from
	//			0 to 255. Zero is the minimum color intensity; 255 is the maximum
	//			color intensity.
	// Returns: A CXTColorRef object.
	// Summary: This assignment operator is used to modify a CXTColorRef object.
	CXTColorRef operator + (int val) const;

    // Input:	val - Specifies the intensity of the color component. 'val' can range from
	//			0 to 255. Zero is the minimum color intensity; 255 is the maximum
	//			color intensity.
	// Returns: A CXTColorRef object.
	// Summary: This assignment operator is used to modify a CXTColorRef object.
	CXTColorRef operator - (int val) const;

    // Input:	val - Specifies the intensity of the color component. 'val' can range from
	//			0 to 255. Zero is the minimum color intensity; 255 is the maximum
	//			color intensity.
	// Returns: A CXTColorRef object.
	// Summary: This assignment operator is used to modify a CXTColorRef object.
	CXTColorRef operator * (int val) const;

	// Input:	val - Specifies the intensity of the color component. 'val' can range from
	//			0 to 255. Zero is the minimum color intensity; 255 is the maximum
	//			color intensity.
	// Returns: A CXTColorRef object.
	// Summary: This assignment operator is used to modify a CXTColorRef object.
	CXTColorRef operator / (int val) const;

    // Input:	cr - An RGB value.
	// Returns: A CXTColorRef object.
	// Summary: This assignment operator is used to modify a CXTColorRef object.
	CXTColorRef operator + (COLORREF cr) const;

    // Input:	cr - An RGB value.
	// Returns: A CXTColorRef object.
	// Summary: This assignment operator is used to modify a CXTColorRef object.
	CXTColorRef operator - (COLORREF cr) const;

	// Input:	cr - An RGB value.
	// Returns: A CXTColorRef object.
    // Summary: This assignment operator is used to modify a CXTColorRef object.
	CXTColorRef operator * (COLORREF cr) const;

    // Input:	cr - An RGB value.
	// Returns: A CXTColorRef object.
	// Summary: This assignment operator is used to modify a CXTColorRef object.
	CXTColorRef operator / (COLORREF cr) const;

    // Input:	val - Specifies the intensity of the color component. 'val' can range from
	//			0 to 255. Zero is the minimum color intensity; 255 is the maximum
	//			color intensity.
	// Returns: A reference to a valid CXTColorRef object.
	// Summary: This assignment operator is used to modify a CXTColorRef object.
	CXTColorRef& operator += (int val);

    // Input:	val - Specifies the intensity of the color component. 'val' can range from
	//			0 to 255. Zero is the minimum color intensity; 255 is the maximum
	//			color intensity.
	// Returns: A reference to a valid CXTColorRef object.
	// Summary: This assignment operator is used to modify a CXTColorRef object.
	CXTColorRef& operator -= (int val);

    // Input:	val - Specifies the intensity of the color component. 'val' can range from
	//			0 to 255. Zero is the minimum color intensity; 255 is the maximum
	//			color intensity.
	// Returns: A reference to a valid CXTColorRef object.
	// Summary: This assignment operator is used to modify a CXTColorRef object.
	CXTColorRef& operator *= (int val);

    // Input:	val - Specifies the intensity of the color component. 'val' can range from
	//			0 to 255. Zero is the minimum color intensity; 255 is the maximum
	//			color intensity.
	// Returns: A reference to a valid CXTColorRef object.
	// Summary: This assignment operator is used to modify a CXTColorRef object.
	CXTColorRef& operator /= (int val);

    // Input:	cr - An RGB value.
	// Returns: A reference to a valid CXTColorRef object.
	// Summary: This assignment operator is used to modify a CXTColorRef object.
	CXTColorRef& operator += (COLORREF cr);

    // Input:	cr - An RGB value.
	// Returns: A reference to a valid CXTColorRef object.
	// Summary: This assignment operator is used to modify a CXTColorRef object.
	CXTColorRef& operator -= (COLORREF cr);

    // Input:	cr - An RGB value.
	// Returns: A reference to a valid CXTColorRef object.
	// Summary: This assignment operator is used to modify a CXTColorRef object.
	CXTColorRef& operator *= (COLORREF cr);

    // Input:	cr - An RGB value.
	// Returns: A reference to a valid CXTColorRef object.
	// Summary: This assignment operator is used to modify a CXTColorRef object.
	CXTColorRef& operator /= (COLORREF cr);
    
	// Returns: An RGB value.
	// Summary: This operator retrieves the RGB value for the CXTColorRef object.
	operator COLORREF () const;

	//////////////////////////////////////////////////////////////
    // RGB Values
	//////////////////////////////////////////////////////////////

	// Returns: The red component of the color.
	// Summary: This method gets the red component of this object.
	int getRValue() const;
    
	// Returns: The green component of the color.
	// Summary: This method gets the green component of this object.
	int getGValue() const;

	// Returns: The blue component of the color.
	// Summary: This method gets the blue component of this object.
	int getBValue() const;

	// Input:	val - Specifies the intensity of the color component. 'val' can range from
	//			0 to 255. Zero is the minimum color intensity; 255 is the maximum
	//			color intensity.
	// Summary: Call this member function to set the red intensity value for the CXTColorRef
	//			object.
	void setRValue(int val);

	// Input:	val - Specifies the intensity of the color component. 'val' can range from
	//			0 to 255. Zero is the minimum color intensity; 255 is the maximum
	//			color intensity.
	// Summary: Call this member function to set the green intensity value for the
	//			CXTColorRef object.
	void setGValue(int val);

	// Input:	val - Specifies the intensity of the color component. 'val' can range from
	//			0 to 255. Zero is the minimum color intensity; 255 is the maximum
	//			color intensity.
	// Summary: Call this member function to set the blue intensity value for the CXTColorRef
	//			object.
	void setBValue(int val);

	//////////////////////////////////////////////////////////////
	// Misc Information
	//////////////////////////////////////////////////////////////

	// Returns: A double that represents the current color intensity.
	// Summary: This member function will retrieve the overall intensity of the 
	//			CXTColorRef	object. 
	double getIntensity() const;

	//////////////////////////////////////////////////////////////
    // Color Conversion 
	//////////////////////////////////////////////////////////////

	// Input:	h - The variable to receive the hue value.
	//			s - The variable to receive the saturation value.
	//			l - The variable to receive the luminance value.
	// Summary: This function converts the RGB color to an HSL (hue, saturation, 
	//			luminance) trio.
    void toHSL(double& h, double& s, double& l) const;

	// Input:	h - The variable to receive the hue value.
	//			s - The variable to receive the saturation value.
	//			b - The variable to receive the brightness value.
	// Summary: This function converts the RGB color to an HSB (hue, saturation, 
	//			brightness) trio.
    void toHSB(double& h, double& s, double& b) const;

	// Input:	h - The hue of the source color.
	//			s - The saturation of the source color.
	//			l - The luminance of the source color.
	// Summary: This function sets an RGB value that is the equivalent of a specific 
	//			HSL (hue, saturation, luminance) color.
    void setHSL(double h, double s, double l);

	// Input:	h - The hue of the source color.
	//			s - The saturation of the source color.
	//			b - The brightness of the source color.
	// Summary: This function sets an RGB value that is the equivalent of a specific 
	//			HSB (hue, saturation, brightness) color.
    void setHSB(double h, double s, double b);

	// Input:	h - The hue of the source color.
	//			s - The saturation of the source color.
	//			l - The luminance of the source color.
	// Summary: This static function creates a new CXTColorRef object out of an HSL
	//			(hue, saturation, luminance) color trio. Returns a CXTColorRef object.
    static CXTColorRef fromHSL(double h, double s, double l);

	// Input:	h - The hue of the source color.
	//			s - The saturation of the source color.
	//			b - The brightness of the source color.
	// Summary: This static function creates a new CXTColorRef object out of an HSB
	//			(hue, saturation, brightness) color trio. Returns a CXTColorRef object.
    static CXTColorRef fromHSB(double h, double s, double b);

	//////////////////////////////////////////////////////////////////////
    // Color Manipulation
	//////////////////////////////////////////////////////////////////////    

	// Input:	clr - A reference to a valid CXTColorRef object.
	//			opacity - The amount of transparency.
	// Returns: A reference to a valid CXTColorRef object.
	// Summary: Call this member function to blend the CXTColorRef object with the
	//			object specified by 'clr'. 
	CXTColorRef& blend(const CXTColorRef& clr, int opacity);

	// Input:	clr - A reference to a valid CXTColorRef object.
	// Returns: A reference to a valid CXTColorRef object.
	// Summary: Call this member function to add the CXTColorRef object specified by 'clr'. 
	CXTColorRef& add(const CXTColorRef& clr);

	// Input:	clr - A reference to a valid CXTColorRef object.
	// Returns: A reference to a valid CXTColorRef object.
	// Summary: Call this member function to subtract the CXTColorRef object specified
	//			by 'clr'. 
	CXTColorRef& subtract(const CXTColorRef& clr);

	//////////////////////////////////////////////////////////////////////
    // Standard Colors
	////////////////////////////////////////////////////////////////////// 
	
    static const CXTColorRef BLACK;   // RGB value that represents the color black.
    static const CXTColorRef WHITE;   // RGB value that represents the color white.
    static const CXTColorRef GRAY_25; // RGB value that represents the 25% gray color.
    static const CXTColorRef GRAY_50; // RGB value that represents the 50% gray color.
    static const CXTColorRef GRAY_75; // RGB value that represents the 75% gray color.

	//////////////////////////////////////////////////////////////////////
    // Helper Functions
	//////////////////////////////////////////////////////////////////////  

	// Input:	iVal - Specifies the color intensity which is the source of 
	//			the operation.
	//			iAdd - Specifies the intensity to add to the source. It can be 
	//			negative.
	// Summary: This member function adds two color intensities, ensuring that the
	//			sum is in the proper color value range (0-255).  For example, 
	//			addColorValue(200, 100) will return 255 and not 300. 
	//			addColorValue(100, -200) will return 0.
	static inline int addColorValue(int iVal, int iAdd);

	// Input:	iVal - Specifies the color intensity which is the source of the operation.
	//			iMult - Specifies a value by which to multiply the source.
	//			iDiv - Specifies a value by which to divide the source.
	// Summary: This member function manipulates a color value by performing a 
	//			multiplication and then a division. For instance, 
	//			multiplyColorValue(10, 2, 3) will return 10 * 2 / 3 which is 6.  
	//			The function guarantees that the resulting value does not exceeded 255.
	static inline int multiplyColorValue(int iVal, int iMult, int iDiv);

	// Input:	iClrFront - Specifies the intensity of the foreground color component. 
	//			'iClrFront' can range from 0 to 255. Zero is the minimum color intensity;
	//			255 is the maximum color intensity.
	//			iClrBack - Specifies the intensity of the background color component. 
	//			'iClrBack' can range from 0 to 255. Zero is the minimum color intensity; 
	//			255	is the maximum color intensity.
	//			opacity - The amount of transparency.
	// Summary: This function takes a blended color intensity which is (100-opacity)%
	//			* iClrFront and opacity% * iClrBack.
	static inline int BlendColor(int iClrFront, int iClrBack, int opacity);

	// Input:	iVal - Specifies the intensity of the color component. 'iVal' can range 
	//			from 0 to 255. Zero is the minimum color intensity; 255 is the maximum
	//			color intensity.
	// Summary: This function takes in a color intensity and ensures that it is between
	//			0-255.  Anything out of range will be adjusted to the closest number
	//			in range (i.e. -100 will become 0, 300 will become 255).
	static int fixColorValue(int iVal);

	// Summary: This function will refresh the display element colors defined in the 
	//			CXTColorRef object.
	static void RefreshColors();

	// BULLETED LIST:

	// Input:	nIndex - Specifies the display element whose color is to be retrieved. 
	//			This parameter can be one of the following values:
	//			[ul]
    //			[li]<b>XPCOLOR_BASE</b> An RGB value that represents the XP base color.[/li]
    //			[li]<b>XPCOLOR_BARFACE</b> An RGB value that represents the XP toolbar 
	//			background color.[/li]
    //			[li]<b>XPCOLOR_HIGHLIGHT</b> An RGB value that represents the XP menu item 
	//			selected color.[/li]
    //			[li]<b>XPCOLOR_HIGHLIGHT_BORDER</b> An RGB value that represents the XP menu
	//			item selected border color.[/li]
    //			[li]<b>XPCOLOR_HIGHLIGHT_PUSHED</b> An RGB value that represents the XP menu
	//			item pushed color.[/li]
    //			[li]<b>XPCOLOR_ICONSHADDOW</b> An RGB value that represents the XP menu item 
	//			icon shadow.[/li]
    //			[li]<b>XPCOLOR_GRAYTEXT</b> An RGB value that represents the XP menu item 
	//			disabled text color.[/li]
    //			[li]<b>XPCOLOR_HIGHLIGHT_CHECKED</b> An RGB value that represents the XP menu
	//			item checked color.[/li]
    //			[li]<b>XPCOLOR_HIGHLIGHT_CHECKED_BORDER</b> An RGB value that represents the 
	//			XP menu item checked border color.[/li]
    //			[li]<b>XPCOLOR_GRIPPER</b> An RGB value that represents the XP toolbar gripper 
	//			color.[/li]
    //			[li]<b>XPCOLOR_SEPARATOR</b> An RGB value that represents the XP toolbar 
	//			separator color.[/li]
    //			[li]<b>XPCOLOR_DISABLED</b> An RGB value that represents the XP menu icon 
	//			disabled color.[/li]
    //			[li]<b>XPCOLOR_MENUTEXT_BACK</b> An RGB value that represents the XP menu 
	//			item text background color.[/li]
    //			[li]<b>XPCOLOR_MENU_EXPANDED</b> An RGB value that represents the XP hidden 
	//			menu commands background color.[/li]
    //			[li]<b>XPCOLOR_MENU_BORDER</b> An RGB value that represents the XP menu 
	//			border color.[/li]
    //			[li]<b>XPCOLOR_MENUTEXT</b> An RGB value that represents the XP menu item 
	//			text color.[/li]
    //			[li]<b>XPCOLOR_HIGHLIGHT_TEXT</b> An RGB value that represents the XP menu
	//			item selected text color.[/li]
    //			[li]<b>XPCOLOR_BARTEXT</b> An RGB value that represents the XP toolbar text
	//			color.[/li]
    //			[li]<b>XPCOLOR_BARTEXT_PUSHED</b> An RGB value that represents the XP toolbar
	//			pushed text color.[/li]
    //			[li]<b>XPCOLOR_TAB_INACTIVE_BACK</b> An RGB value that represents the XP 
	//			inactive tab background color.[/li]
    //			[li]<b>XPCOLOR_TAB_INACTIVE_TEXT</b> An RGB value that represents the XP 
	//			inactive tab text color.[/li]
	//			[/ul]
	// Returns: The red, green, blue (RGB) color value of the given element.
	// Summary: This function retrieves the current color of the specified xp display 
	//			element. Display elements are the parts of a window and the display that 
	//			appear on the system display screen.  
	static COLORREF GetColor(int nIndex);

protected:

    COLORREF m_ColorRef; // Current RGB value for this object.

	// Input:	temp1 - Temp value.
	//			temp2 - Temp value.
	//			temp3 - Temp value.
	// Returns: A double that represents either an r, g, or b COLORREF value.
	// Summary: This member function is called by setHSL() to convert hue.
	static double HuetoRGB(double temp1, double temp2, double temp3);

	// Input:	r - Specifies the intensity of the red color component. 'r' can range
	//			from 0 to 255. Zero is the minimum color intensity; 255 is the maximum
	//			color intensity.
	//			g - Specifies the intensity of the green color component. 'g' can range
	//			from 0 to 255. Zero is the minimum color intensity; 255 is the maximum
	//			color intensity.
	//			b - Specifies the intensity of the blue color component. 'b' can range
	//			from 0 to 255. Zero is the minimum color intensity; 255 is the maximum
	//			color intensity.
	// Returns: An RGB value.
	// Summary: This function takes in a color intensity and ensures that it is between
	//			0-255.  Anything out of range will be adjusted to the closest number
	//			in range (i.e. -100 will become 0, 300 will become 255). 
	static COLORREF SafeRGB(int r, int g, int b);

//////////////////////////////////////////////////////////////////////
// XP Color Functions
//////////////////////////////////////////////////////////////////////  

private:
	static COLORREF m_arrColor[50];

	static void RefreshSysColors();
	static void RefreshXPColors();
	
	static float ColorWidth(int nLength, int nWidth);
	static float ColorDelta(COLORREF clrA, COLORREF clrB);
	static float Length(COLORREF clrA, COLORREF clrB);
	static BOOL LongColor(COLORREF clrMain, COLORREF clrSub, BOOL bCalcLength, float fDistance);
	static COLORREF MixColor(COLORREF clrMain, COLORREF clrSub, float fDistance);
	static COLORREF AdjustColor(COLORREF clrMain, COLORREF clrSub, float fDistance);
	static COLORREF LightColor(COLORREF clrLight, COLORREF clrDark, int nDelta);
	
	static double GetRDelta(COLORREF clr) { return GetRValue(clr) * (195.0/255.0); }; 
	static double GetGDelta(COLORREF clr) { return GetGValue(clr) * (390.0/255.0); }; 
	static double GetBDelta(COLORREF clr) { return GetBValue(clr) * (65.0/255.0); }; 
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE CXTColorRef::CXTColorRef() {
} 
AFX_INLINE CXTColorRef::CXTColorRef(COLORREF r) {
	*this = r;
}
AFX_INLINE CXTColorRef::CXTColorRef(int r, int g, int b) {
	*this = RGB(r, g, b);
}
AFX_INLINE CXTColorRef& CXTColorRef::operator = (COLORREF cr) {
	m_ColorRef = cr; return *this; 
}
AFX_INLINE CXTColorRef::operator COLORREF () const {
	return m_ColorRef;
}
AFX_INLINE int CXTColorRef::getRValue() const {
	return GetRValue(m_ColorRef);
}
AFX_INLINE int CXTColorRef::getGValue() const {
	return GetGValue(m_ColorRef);
}
AFX_INLINE int CXTColorRef::getBValue() const {
	return GetBValue(m_ColorRef);
}
AFX_INLINE void CXTColorRef::setRValue(int val) {
	m_ColorRef = RGB(val, getGValue(), getBValue());
}
AFX_INLINE void CXTColorRef::setGValue(int val) {
	m_ColorRef = RGB(getRValue(), val, getBValue());
}
AFX_INLINE void CXTColorRef::setBValue(int val) {
	m_ColorRef = RGB(getRValue(), getGValue(), val);
}
AFX_INLINE double CXTColorRef::getIntensity() const {
	return	((299 * getRValue()) + (587 * getGValue()) + (114 * getBValue())) / 1000.0;
}
AFX_INLINE int CXTColorRef::addColorValue(int iVal, int iAdd) {
	return fixColorValue(iVal + iAdd);
}
AFX_INLINE int CXTColorRef::multiplyColorValue(int iVal, int iMult, int iDiv) {
	return fixColorValue(iVal * iMult / iDiv);
}
AFX_INLINE int CXTColorRef::BlendColor(int iClrFront, int iClrBack, int opacity) {
	return fixColorValue(((iClrFront * opacity) + (iClrBack * (100 - opacity))) / 100);
}
AFX_INLINE int CXTColorRef::fixColorValue(int iVal) {
	if (iVal > 255) return 255; if (iVal < 0)   return 0; return iVal;
}

//////////////////////////////////////////////////////////////////////

#endif // !defined(__XTCOLORREF_H__)