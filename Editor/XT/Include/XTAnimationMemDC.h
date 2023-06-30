// XTAnimationMemDC.h : header file
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

#if !defined(___XTANIMATIONMEMDC_H__)
#define ___XTANIMATIONMEMDC_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Summary: Enumerated type used to determine animation effect.
enum ANIMATIONTYPE
{
	animateWindowsDefault,	// As defined in the "Display" settings.
	animateRandom,			// Any of the first three in random selection.
	animateUnfold,			// Unfold top to bottom.
	animateSlide,			// Slide in from left.
	animateFade,			// Fade-in.
	animateNone				// No animation.
};

// Input:    rc - Area to display.
//           pDestDC - Destination device context.
//           pSrcDC - Source device context.
//           nType - Enumerated ANIMATIONTYPE to determine animation effect.
//           nSteps - Number of steps to complete animation.
//           nAnimationTime - Amount of time in milliseconds between each step.
// Summary:  Function pointer used to define a custom animation effect.
// See Also: CXTAnimationMemDC::SetCustomAnimation, ANIMATIONTYPE
typedef void (*xtAnimationProc)(CRect rc, CDC* pDestDC, CDC* pSrcDC, int nType, int nSteps, int nAnimationTime);

//////////////////////////////////////////////////////////////////////
// Summary: CXTAnimationMemDC is a CXTMemDC derived class.  This class is a replacement
//			for the commonly used XTMemDC.  By calling Animate(), the contained
//			bitmap is drawn on the screen with a given effect. This effect is equal
//			to the Menu effect in Windows&reg; and Office&trade;.
class _XT_EXT_CLASS CXTAnimationMemDC : public CXTMemDC
{
public: 
	
	// Input:	pDC - A pointer to a CDC object.
	//			rect - An address of a CRect object.
	//			clrColor - An RGB value that represents the current system face color of 
	//			three dimensional display elements.
	// Summary: Constructs a CXTAnimationMemDC object.
    CXTAnimationMemDC(CDC* pDC, const CRect& rect, COLORREF clrColor=GetSysColor(COLOR_3DFACE));

	// Summary: Destroys a CXTAnimationMemDC object, handles cleanup and de-allocation.
    virtual ~CXTAnimationMemDC();

	// Input:	nType - Type of animation to perform.
	//			nSteps - Number of steps to take during animation.
	//			nAnimationTime - Amount of time to rest, in milliseconds, between each step.
	// Summary: This member function performs the animation.
	void Animate(int nType = animateWindowsDefault, int nSteps = 10, int nAnimationTime = 1000);

	// Input:	rc - Bounding rectangle
	//			pDestDC - Pointer to device context you must draw to.
	//			pSrcDC - Device context that contains the bitmap you must take.
	//			nType - Type of animation to perform. For custom animation you must use
	//			numbers greater than 6.
	//			nSteps - Number of steps to take during animation.
	//			nAnimationTime - Amount of time to rest, in milliseconds, between each step.
	// Summary: This member implements default animation effects - Fade, Slide and Unfold.
	//			You can add new animation effects to call SetCustomAnimation member.
	static void DefaultAnimation(CRect rc, CDC* pDestDC, CDC* pSrcDC, int nType, int nSteps, int nAnimationTime);

    // Input:    pCustom - pointer to custom animation function.
    // Example:  <pre>
    //           int CMainFrame::OnCreate()
    //           {
    //               CXTCoolMenu::m_nAnimationType = 10; // our animation type
    //               CXTAnimationMemDC::SetCustomAnimation(CustomAnimation); // custom animation procedure.
    //               ...
    //           }
    //           ...
    //           void CMainFrame::CustomAnimation(CRect rc, CDC* pDestDC, CDC* pSrcDC, int nType, int nSteps, int nAnimationTime)
    //           {
    //               if (nType == 10)
    //               {
    //                   // do custom animation
    //               } 
    //               else
    //               {
    //                   CXTAnimationMemDC::DefaultAnimation(rc, pDestDC, pSrcDC, nType, nSteps, nAnimationTime);
    //               }   
    //           }
    //           </pre>
    // Summary:  Call this member function to setup new Animation effects. 
    //           You must call DefaultAnimation in your function.
    // See Also: xtAnimationProc, ANIMATIONTYPE
	static void SetCustomAnimation(xtAnimationProc pCustom);

private:
	static xtAnimationProc m_pCustomAnimation;
};

#endif // !defined(___XTANIMATIONMEMDC_H__)
