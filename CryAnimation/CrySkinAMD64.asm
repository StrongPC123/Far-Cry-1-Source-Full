
;****************************************************************************
;*
;* This is the 64bit SSE-version of CrySkinFull.cpp.
;* Rewritten by Ivo Herzeg
;*
;* Entry parameters:
;*
;* rcx      = pAux
;* rdx      = pVertex
;* r8       = pDest
;* r9       = pBone
;* [rSP+60] = pvMin			; After the call but before the push rbp
;* [rSP+68] = pBoneEnd
;*
;****************************************************************************


CRY_SKIN_AUX_INT_SIZE	equ	2		; This must match the define in CrySkinTypes.h

pvMin			= 060h				; After the push rbp
pBoneEnd	= 068h				; "



X00 = 000h
X10 = 004h
X20 = 008h

Y01 = 010h
Y11 = 014h
Y21 = 018h

Z02 = 020h
Z12 = 024h
Z22 = 028h

TransX	= 030h				; After the push rbp
TransY	= 034h				; After the push rbp
TransZ	= 038h				; After the push rbp






_data	SEGMENT
align 16
var6 qword 0,0
var7 qword 0,0
var8 qword 0,0
var9 qword 0,0
var10 qword 0,0
var11 qword 0,0
var12 qword 0,0
var13 qword 0,0
var14 qword 0,0
var15 qword 0,0

_text	SEGMENT
PUBLIC Amd64Skinner


Amd64Skinner	PROC	FRAME

							push		rBP
							push		rSI
							push		rDI

							push		rAX
							push		rBX
							push		rCX
							push		rDX
              
              movdqa  var6,xmm6
              movdqa  var7,xmm7
              movdqa  var8,xmm8
              movdqa  var9,xmm9
              movdqa  var10,xmm10
              movdqa  var11,xmm11
              movdqa  var12,xmm12
              movdqa  var13,xmm13
              movdqa  var14,xmm14
              movdqa  var15,xmm15

							             
; For debug, I will copy the parameters into the same registers which Crytek used in the inline assembler.

							mov rSI, rdx		;parameter 1
							mov	rDX, rcx		;parameter 2
							mov	rDI, r8			;parameter 3
					;		mov qqqqq, r9		;parameter 4  //pointer to the matrix stack


startLoop:
							cmp	r9, pBoneEnd[rSP]				
							jz endLoop

							; load the current matrix; we don't need the move component
							movss		xmm9,  [r9+X00]  
							movss		xmm10, [r9+Y01]
							movss		xmm11, [r9+X10]
							movss		xmm12, [r9+Y11]
							movss		xmm13, [r9+X20]
							movss		xmm14, [r9+Y21]

							; load the counter for the number of non-flipped tangets for this bone
							xor		rCX,rCX
							mov		CX, word ptr [rdx]			; Was	mov CX, word ptr [EDX]
							add		rDX, 2									;		add EDX, 2
							test	eCX, eCX
							jz endLoopRigid

	startLoopRigid:

							; calculate the destination pointer
							mov			rax,	[rSI+0Ch]		
							and			rax,	0FFFFFFh
							add			rax,	rax

							prefetch  [rSI+140h]

							movss  xmm0, [rSI+00h]	;x
							movss  xmm3, xmm0
							movss  xmm1, [rSI+04h]	;y
							movss  xmm4, xmm1
							movss  xmm2, [rSI+08h]	;z
							movss  xmm5, xmm2

							prefetchw [rDI+rax*8+40h]

							movss  xmm6, xmm0
							movss  xmm7, xmm1
							movss  xmm8, xmm2

							mulss  xmm0, xmm13					;x*M20
							mulss  xmm1, xmm14          ;y*M21
							mulss  xmm2, [r9+Z22]      ;z*M22
							addss  xmm0, [r9+TransZ]
							addss  xmm1, xmm2
							addss  xmm0, xmm1


							mulss  xmm3, xmm9           ;x*M00
							mulss  xmm4, xmm10          ;y*M01
							mulss  xmm5, [r9+Z02]      ;z*M02
							addss  xmm3, [r9+TransX]
							addss  xmm4, xmm5
							addss  xmm3, xmm4

							mulss  xmm6, xmm11          ;x*M10
							mulss  xmm7, xmm12          ;y*M11
							mulss  xmm8, [r9+Z12]      ;z*M12
							addss  xmm6, [r9+TransY] 
							addss  xmm7, xmm8
							addss  xmm6, xmm7

							movss  [rDI+rAX*8+08h], xmm0
							movss  [rDI+rAX*8+00h], xmm3
							movss  [rDI+rAX*8+04h], xmm6

							add			rSI,	010h					; rdi+rax*8 (EDI+EAX*8) points to the destination vector now
							dec	eCX
							jnz startLoopRigid
endLoopRigid:



;/////////////////////////////////////////////////////////
;// Smooth-1 loop
;/////////////////////////////////////////////////////////

							movss		xmm9,  [r9+X00]  
							movss		xmm10, [r9+Y01]
							movss		xmm11, [r9+X10]
							movss		xmm12, [r9+Y11]
							movss		xmm13, [r9+X20]
							movss		xmm14, [r9+Y21]


							; load the counter for the number of smooth vertices met for the first time
							xor ECX,ECX
							mov CX, word ptr [rdx]
							add rDX, 2						
							test ECX, ECX
							jz endLoopSmooth1


startLoopSmooth1:

							; calculate the destination pointer
							xor		EAX,EAX
							mov		AX, word ptr [rdx]
							add		rDX, 2						
							shl		rAX,1					


							prefetch  [rSI+140h]

							movss  xmm0, [rSI+00h]	;x
							movss  xmm3, xmm0
							movss  xmm1, [rSI+04h]	;y
							movss  xmm4, xmm1
							movss  xmm2, [rSI+08h]	;z
							movss  xmm5, xmm2
							movss  xmm15, [rSI+0ch]	;w

							prefetchw [rDI+rax*8+40h]

							movss  xmm6, xmm0
							movss  xmm7, xmm1
							movss  xmm8, xmm2

							mulss  xmm0, xmm13					;x*M20
							mulss  xmm1, xmm14          ;y*M21
							mulss  xmm2, [r9+Z22]				;z*M22
							addss  xmm0, [r9+TransZ]
							addss  xmm1, xmm2
							addss  xmm0, xmm1

							mulss  xmm3, xmm9           ;x*M00
							mulss  xmm4, xmm10          ;y*M01
							mulss  xmm5, [r9+Z02]				;z*M02
							addss  xmm3, [r9+TransX]
							addss  xmm4, xmm5
							addss  xmm3, xmm4

							mulss  xmm6, xmm11          ;x*M10
							mulss  xmm7, xmm12          ;y*M11
							mulss  xmm8, [r9+Z12]				;z*M12
							addss  xmm6, [r9+TransY] 
							addss  xmm7, xmm8
							addss  xmm6, xmm7

							mulss  xmm0, xmm15          ;Z*weight
							mulss  xmm3, xmm15          ;X*weight
							mulss  xmm6, xmm15          ;Y*weight

							add		rSI, 010h				
							dec		eCX
							
							movss  [rDI+rAX*8+08h], xmm0
							movss  [rDI+rAX*8+00h], xmm3
							movss  [rDI+rAX*8+04h], xmm6

							jnz		startLoopSmooth1
endLoopSmooth1:




;//////////////////////////////////////////////////////////////////
;//		       Smooth-2 loop
;//////////////////////////////////////////////////////////////////

							movss		xmm9,  [r9+X00]  
							movss		xmm10, [r9+Y01]
							movss		xmm11, [r9+X10]
							movss		xmm12, [r9+Y11]
							movss		xmm13, [r9+X20]
							movss		xmm14, [r9+Y21]


						;// load the counter for the number of smooth vertices met for the second time
							xor ECX,ECX
							mov CX, word ptr [rdx]
							add rdx, 2						
							test ECX, ECX
							jz endLoopSmooth2

	startLoopSmooth2:

							; calculate the destination pointer
							xor			EAX,EAX
							mov			ax, word ptr [rdx]
							add			rdx, 2	
							shl			rax, 4						

							prefetch  [rSI+140h]

							movss  xmm0, [rSI+00h]	;x
							movss  xmm3, xmm0
							movss  xmm1, [rSI+04h]	;y
							movss  xmm4, xmm1
							movss  xmm2, [rSI+08h]	;z
							movss  xmm5, xmm2
							movss  xmm15, [rSI+0ch]	;w

							prefetchw [rDI+rAX+40h]

							movss  xmm6, xmm0
							movss  xmm7, xmm1
							movss  xmm8, xmm2

							mulss  xmm0, xmm13					;x*M20
							mulss  xmm1, xmm14          ;y*M21
							mulss  xmm2, [r9+Z22]				;z*M22
							addss  xmm0, [r9+TransZ]
							addss  xmm1, xmm2
							addss  xmm0, xmm1

							mulss  xmm3, xmm9           ;x*M00
							mulss  xmm4, xmm10          ;y*M01
							mulss  xmm5, [r9+Z02]				;z*M02
							addss  xmm3, [r9+TransX]
							addss  xmm4, xmm5
							addss  xmm3, xmm4

							mulss  xmm6, xmm11          ;x*M10
							mulss  xmm7, xmm12          ;y*M11
							mulss  xmm8, [r9+Z12]				;z*M12
							addss  xmm6, [r9+TransY] 
							addss  xmm7, xmm8
							addss  xmm6, xmm7

							mulss  xmm0, xmm15          ;Z*weight
							mulss  xmm3, xmm15          ;X*weight
							mulss  xmm6, xmm15          ;Y*weight

							add		rSI, 010h				
							dec		eCX

							addss  xmm0,[rDI+rAX+08h]
							addss  xmm3,[rDI+rAX+00h]
							addss  xmm6,[rDI+rAX+04h]
							
							movss  [rDI+rAX+08h], xmm0
							movss  [rDI+rAX+00h], xmm3
							movss  [rDI+rAX+04h], xmm6

							jnz			startLoopSmooth2
		
				endLoopSmooth2:

							add			r9, 040h
							jmp startLoop
				endLoop:


              movdqa  xmm6, var6
              movdqa  xmm7, var7
              movdqa  xmm8, var8
              movdqa  xmm9, var9
              movdqa  xmm10, var10
              movdqa  xmm11, var11
              movdqa  xmm12, var12
              movdqa  xmm13, var13
              movdqa  xmm14, var14
              movdqa  xmm15, var15
	
							pop		rDX
							pop		rCX
							pop		rBX
							pop		rAX
							pop		rDI
							pop		rSI
							pop		rBP
							ret

Amd64Skinner	ENDP

_text	ENDS

.endprolog

END