;******************** (C) COPYRIGHT 2009  STMicroelectronics ********************
;* File Name          : cr4_fft_256_stm32.s
;* Author             : MCD Application Team
;* Version            : V2.0.0
;* Date               : 04/27/2009
;* Description        : Optimized 256-point radix-4 complex FFT for Cortex-M3
;********************************************************************************
;* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
;* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
;* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
;* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
;* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
;* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
;*******************************************************************************/

	THUMB
  	REQUIRE8
    PRESERVE8

	AREA |.text|, CODE, READONLY, ALIGN=2
		        
	EXPORT cr4_fft_256_stm32      
	EXTERN TableFFT


pssK      	RN R0
pssOUT    	RN R0
pssX      	RN R1
pssIN     	RN R1
butternbr 	RN R2
Nbin      	RN R2
index     	RN R3
Ar        	RN R4
Ai        	RN R5
Br        	RN R6
Bi        	RN R7
Cr        	RN R8
Ci        	RN R9
Dr        	RN R10
Di        	RN R11
cntrbitrev 	RN R12
tmp       	RN R12
pssIN2    	RN R14
tmp2      	RN R14

NPT EQU 256

;----------------------------- MACROS ----------------------------------------
	 
	 	MACRO
	 	DEC  $reg
     	SUB  $reg,$reg,#1
     	MEND

	 	MACRO
     	INC  $reg
     	ADD  $reg,$reg,#1
     	MEND


 	 	MACRO
	 	QUAD  $reg
     	MOV  $reg,$reg,LSL#2
     	MEND

;sXi = *(PssX+1); sXr = *PssX; PssX += offset; PssX= R1

	  	MACRO
	  	LDR2Q  $sXr,$sXi, $PssX, $offset
      	LDRSH $sXi, [$PssX, #2]
      	LDRSH $sXr, [$PssX]
      	ADD $PssX, $PssX, $offset
      	MEND

;!! Same macro, to be used when passing negative offset value !!
		MACRO
		LDR2Qm  $sXr, $sXi, $PssX, $offset
        LDRSH $sXi, [$PssX, #2]
        LDRSH $sXr, [$PssX]
        SUB $PssX, $PssX, $offset
        MEND

;(PssX+1)= sXi;  *PssX=sXr; PssX += offset;
		MACRO
		STR2Q  $sXr, $sXi, $PssX, $offset
        STRH  $sXi, [$PssX, #2]
        STRH  $sXr, [$PssX]
        ADD $PssX, $PssX, $offset
        MEND

; YY = Cplx_conjugate_mul(Y,K)
;  Y = YYr + i*YYi
; use the following trick
;  K = (Kr-Ki) + i*Ki
		MACRO
		CXMUL_V7  $YYr, $YYi, $Yr, $Yi, $Kr, $Ki,$tmp,$tmp2
        SUB  $tmp2, $Yi, $Yr         ; sYi-sYr
        MUL  $tmp, $tmp2, $Ki        ; (sYi-sYr)*sKi
        ADD  $tmp2, $Kr, $Ki, LSL#1  ; (sKr+sKi)
        MLA  $YYi, $Yi, $Kr, $tmp     ; lYYi = sYi*sKr-sYr*sKi
        MLA  $YYr, $Yr, $tmp2, $tmp   ; lYYr = sYr*sKr+sYi*sKi
        MEND

; Four point complex Fast Fourier Transform		
		MACRO
		CXADDA4  $s
        ; (C,D) = (C+D, C-D)
        ADD     Cr, Cr, Dr
        ADD     Ci, Ci, Di
        SUB     Dr, Cr, Dr, LSL#1
        SUB     Di, Ci, Di, LSL#1
        ; (A,B) = (A+(B>>s), A-(B>>s))/4
        MOV     Ar, Ar, ASR#2
        MOV     Ai, Ai, ASR#2
        ADD     Ar, Ar, Br, ASR#(2+$s)
        ADD     Ai, Ai, Bi, ASR#(2+$s)
        SUB     Br, Ar, Br, ASR#(1+$s)
        SUB     Bi, Ai, Bi, ASR#(1+$s)
        ; (A,C) = (A+(C>>s)/4, A-(C>>s)/4)
        ADD     Ar, Ar, Cr, ASR#(2+$s)
        ADD     Ai, Ai, Ci, ASR#(2+$s)
        SUB     Cr, Ar, Cr, ASR#(1+$s)
        SUB     Ci, Ai, Ci, ASR#(1+$s)
        ; (B,D) = (B-i*(D>>s)/4, B+i*(D>>s)/4)
        ADD     Br, Br, Di, ASR#(2+$s)
        SUB     Bi, Bi, Dr, ASR#(2+$s)
        SUB     Di, Br, Di, ASR#(1+$s)
        ADD     Dr, Bi, Dr, ASR#(1+$s)
        MEND

		
		MACRO
		BUTFLY4ZERO_OPT  $pIN,$offset, $pOUT
        LDRSH Ai, [$pIN, #2]
		LDRSH Ar, [$pIN]
	    ADD   $pIN, #NPT
        LDRSH Ci, [$pIN, #2]
        LDRSH Cr, [$pIN]
		ADD   $pIN, #NPT
        LDRSH Bi, [$pIN, #2]
        LDRSH Br, [$pIN]
		ADD   $pIN, #NPT
        LDRSH Di, [$pIN, #2]
        LDRSH Dr, [$pIN]
		ADD   $pIN, #NPT

        ; (C,D) = (C+D, C-D)
        ADD     Cr, Cr, Dr
        ADD     Ci, Ci, Di
        SUB     Dr, Cr, Dr, LSL#1  ; trick
        SUB     Di, Ci, Di, LSL#1  ;trick
        ; (A,B) = (A+B)/4, (A-B)/4
        MOV     Ar, Ar, ASR#2
        MOV     Ai, Ai, ASR#2
        ADD     Ar, Ar, Br, ASR#2
        ADD     Ai, Ai, Bi, ASR#2
        SUB     Br, Ar, Br, ASR#1
        SUB     Bi, Ai, Bi, ASR#1
        ; (A,C) = (A+C)/4, (A-C)/4
        ADD     Ar, Ar, Cr, ASR#2
        ADD     Ai, Ai, Ci, ASR#2
        SUB     Cr, Ar, Cr, ASR#1
        SUB     Ci, Ai, Ci, ASR#1
        ; (B,D) = (B-i*D)/4, (B+i*D)/4
        ADD     Br, Br, Di, ASR#2
        SUB     Bi, Bi, Dr, ASR#2
        SUB     Di, Br, Di, ASR#1
        ADD     Dr, Bi, Dr, ASR#1
        ;
        STRH    Ai, [$pOUT, #2]
        STRH    Ar, [$pOUT], #4
        STRH    Bi, [$pOUT, #2]
        STRH    Br, [$pOUT], #4
        STRH    Ci, [$pOUT, #2]
        STRH    Cr, [$pOUT], #4
        STRH    Dr, [$pOUT, #2]  ; inversion here
        STRH    Di, [$pOUT], #4
        MEND

		MACRO
		BUTFLY4_V7   $pssDin,$offset,$pssDout,$qformat,$pssK
        LDR2Qm   Ar,Ai,$pssDin, $offset;-$offset
        LDR2Q    Dr,Di,$pssK, #4
        ; format CXMUL_V7 YYr, YYi, Yr, Yi, Kr, Ki,tmp,tmp2
        CXMUL_V7 Dr,Di,Ar,Ai,Dr,Di,tmp,tmp2
        LDR2Qm   Ar,Ai,$pssDin,$offset;-$offset
        LDR2Q    Cr,Ci,$pssK,#4
        CXMUL_V7 Cr,Ci,Ar,Ai,Cr,Ci,tmp,tmp2
        LDR2Qm    Ar,Ai, $pssDin, $offset;-$offset
        LDR2Q    Br,Bi, $pssK, #4
        CXMUL_V7  Br,Bi,Ar,Ai,Br,Bi,tmp,tmp2
        LDR2Q    Ar,Ai, $pssDin, #0
        CXADDA4  $qformat
        STRH    Ai, [$pssDout, #2]
        STRH    Ar, [$pssDout]
        ADD 	$pssDout, $pssDout, $offset
        STRH    Bi, [$pssDout, #2]
        STRH    Br, [$pssDout]
        ADD     $pssDout, $pssDout, $offset
        STRH    Ci, [$pssDout, #2]
        STRH    Cr, [$pssDout]
        ADD     $pssDout, $pssDout, $offset
        STRH    Dr, [$pssDout, #2]  ; inversion here
        STRH    Di, [$pssDout], #4
        MEND

;------------------- 			CODE 			--------------------------------
;===============================================================================
;*******************************************************************************
;* Function Name  : cr4_fft_256_stm32
;* Description    : complex radix-4 256 points FFT
;* Input          : - R0 = pssOUT: Output array .
;*                  - R1 = pssIN: Input array 
;*                  - R2 = Nbin: =256 number of points, this optimized FFT function  
;*                    can only convert 256 points.
;* Output         : None 
;* Return         : None
;*******************************************************************************
cr4_fft_256_stm32

        STMFD   SP!, {R4-R11, LR}
        
        MOV cntrbitrev, #0
        MOV index,#0
        
preloop_v7
        ADD     pssIN2, pssIN, cntrbitrev, LSR#24 ;256-pts
        BUTFLY4ZERO_OPT pssIN2,Nbin,pssOUT
        INC index
        RBIT cntrbitrev,index
        CMP index,#64  ;256-pts
        BNE  preloop_v7


        SUB     pssX, pssOUT, Nbin, LSL#2
        MOV     index, #16
        MOVS    butternbr, Nbin, LSR#4   ;dual use of register 
        
;------------------------------------------------------------------------------
;   The FFT coefficients table can be stored into Flash or RAM. 
;   The following two lines of code allow selecting the method for coefficients 
;   storage. 
;   In the case of choosing coefficients in RAM, you have to:
;   1. Include the file table_fft.h, which is a part of the DSP library, 
;      in your main file.
;   2. Decomment the line LDR.W pssK, =TableFFT and comment the line 
;      ADRL    pssK, TableFFT_V7
;   3. Comment all the TableFFT_V7 data.
;------------------------------------------------------------------------------
        ADRL    pssK, TableFFT_V7    ; Coeff in Flash 
        ;LDR.W pssK, =TableFFT      ; Coeff in RAM 

;................................
passloop_v7
        STMFD   SP!, {pssX,butternbr}
        ADD     tmp, index, index, LSL#1
        ADD     pssX, pssX, tmp
        SUB     butternbr, butternbr, #1<<16
;................
grouploop_v7
        ADD     butternbr,butternbr,index,LSL#(16-2)
;.......
butterloop_v7
        BUTFLY4_V7  pssX,index,pssX,14,pssK
        SUBS        butternbr,butternbr, #1<<16
        BGE     butterloop_v7
;.......
        ADD     tmp, index, index, LSL#1
        ADD     pssX, pssX, tmp
        DEC     butternbr
        MOVS    tmp2, butternbr, LSL#16
        IT      NE
        SUBNE   pssK, pssK, tmp
        BNE     grouploop_v7
;................
        LDMFD   sp!, {pssX, butternbr}
        QUAD    index
        MOVS    butternbr, butternbr, LSR#2    ; loop nbr /= radix 
        BNE     passloop_v7
;................................
       LDMFD   SP!, {R4-R11, PC}

;=============================================================================

TableFFT_V7
        ;N=16
        DCW 0x4000,0x0000, 0x4000,0x0000, 0x4000,0x0000
        DCW 0xdd5d,0x3b21, 0x22a3,0x187e, 0x0000,0x2d41
        DCW 0xa57e,0x2d41, 0x0000,0x2d41, 0xc000,0x4000
        DCW 0xdd5d,0xe782, 0xdd5d,0x3b21, 0xa57e,0x2d41
        ; N=64
        DCW 0x4000,0x0000, 0x4000,0x0000, 0x4000,0x0000
        DCW 0x2aaa,0x1294, 0x396b,0x0646, 0x3249,0x0c7c
        DCW 0x11a8,0x238e, 0x3249,0x0c7c, 0x22a3,0x187e
        DCW 0xf721,0x3179, 0x2aaa,0x1294, 0x11a8,0x238e
        DCW 0xdd5d,0x3b21, 0x22a3,0x187e, 0x0000,0x2d41
        DCW 0xc695,0x3fb1, 0x1a46,0x1e2b, 0xee58,0x3537
        DCW 0xb4be,0x3ec5, 0x11a8,0x238e, 0xdd5d,0x3b21
        DCW 0xa963,0x3871, 0x08df,0x289a, 0xcdb7,0x3ec5
        DCW 0xa57e,0x2d41, 0x0000,0x2d41, 0xc000,0x4000
        DCW 0xa963,0x1e2b, 0xf721,0x3179, 0xb4be,0x3ec5
        DCW 0xb4be,0x0c7c, 0xee58,0x3537, 0xac61,0x3b21
        DCW 0xc695,0xf9ba, 0xe5ba,0x3871, 0xa73b,0x3537
        DCW 0xdd5d,0xe782, 0xdd5d,0x3b21, 0xa57e,0x2d41
        DCW 0xf721,0xd766, 0xd556,0x3d3f, 0xa73b,0x238e
        DCW 0x11a8,0xcac9, 0xcdb7,0x3ec5, 0xac61,0x187e
        DCW 0x2aaa,0xc2c1, 0xc695,0x3fb1, 0xb4be,0x0c7c
        ; N=256
        DCW 0x4000,0x0000, 0x4000,0x0000, 0x4000,0x0000
        DCW 0x3b1e,0x04b5, 0x3e69,0x0192, 0x3cc8,0x0324
        DCW 0x35eb,0x0964, 0x3cc8,0x0324, 0x396b,0x0646
        DCW 0x306c,0x0e06, 0x3b1e,0x04b5, 0x35eb,0x0964
        DCW 0x2aaa,0x1294, 0x396b,0x0646, 0x3249,0x0c7c
        DCW 0x24ae,0x1709, 0x37af,0x07d6, 0x2e88,0x0f8d
        DCW 0x1e7e,0x1b5d, 0x35eb,0x0964, 0x2aaa,0x1294
        DCW 0x1824,0x1f8c, 0x341e,0x0af1, 0x26b3,0x1590
        DCW 0x11a8,0x238e, 0x3249,0x0c7c, 0x22a3,0x187e
        DCW 0x0b14,0x2760, 0x306c,0x0e06, 0x1e7e,0x1b5d
        DCW 0x0471,0x2afb, 0x2e88,0x0f8d, 0x1a46,0x1e2b
        DCW 0xfdc7,0x2e5a, 0x2c9d,0x1112, 0x15fe,0x20e7
        DCW 0xf721,0x3179, 0x2aaa,0x1294, 0x11a8,0x238e
        DCW 0xf087,0x3453, 0x28b2,0x1413, 0x0d48,0x2620
        DCW 0xea02,0x36e5, 0x26b3,0x1590, 0x08df,0x289a
        DCW 0xe39c,0x392b, 0x24ae,0x1709, 0x0471,0x2afb
        DCW 0xdd5d,0x3b21, 0x22a3,0x187e, 0x0000,0x2d41
        DCW 0xd74e,0x3cc5, 0x2093,0x19ef, 0xfb8f,0x2f6c
        DCW 0xd178,0x3e15, 0x1e7e,0x1b5d, 0xf721,0x3179
        DCW 0xcbe2,0x3f0f, 0x1c64,0x1cc6, 0xf2b8,0x3368
        DCW 0xc695,0x3fb1, 0x1a46,0x1e2b, 0xee58,0x3537
        DCW 0xc197,0x3ffb, 0x1824,0x1f8c, 0xea02,0x36e5
        DCW 0xbcf0,0x3fec, 0x15fe,0x20e7, 0xe5ba,0x3871
        DCW 0xb8a6,0x3f85, 0x13d5,0x223d, 0xe182,0x39db
        DCW 0xb4be,0x3ec5, 0x11a8,0x238e, 0xdd5d,0x3b21
        DCW 0xb140,0x3daf, 0x0f79,0x24da, 0xd94d,0x3c42
        DCW 0xae2e,0x3c42, 0x0d48,0x2620, 0xd556,0x3d3f
        DCW 0xab8e,0x3a82, 0x0b14,0x2760, 0xd178,0x3e15
        DCW 0xa963,0x3871, 0x08df,0x289a, 0xcdb7,0x3ec5
        DCW 0xa7b1,0x3612, 0x06a9,0x29ce, 0xca15,0x3f4f
        DCW 0xa678,0x3368, 0x0471,0x2afb, 0xc695,0x3fb1
        DCW 0xa5bc,0x3076, 0x0239,0x2c21, 0xc338,0x3fec
        DCW 0xa57e,0x2d41, 0x0000,0x2d41, 0xc000,0x4000
        DCW 0xa5bc,0x29ce, 0xfdc7,0x2e5a, 0xbcf0,0x3fec
        DCW 0xa678,0x2620, 0xfb8f,0x2f6c, 0xba09,0x3fb1
        DCW 0xa7b1,0x223d, 0xf957,0x3076, 0xb74d,0x3f4f
        DCW 0xa963,0x1e2b, 0xf721,0x3179, 0xb4be,0x3ec5
        DCW 0xab8e,0x19ef, 0xf4ec,0x3274, 0xb25e,0x3e15
        DCW 0xae2e,0x1590, 0xf2b8,0x3368, 0xb02d,0x3d3f
        DCW 0xb140,0x1112, 0xf087,0x3453, 0xae2e,0x3c42
        DCW 0xb4be,0x0c7c, 0xee58,0x3537, 0xac61,0x3b21
        DCW 0xb8a6,0x07d6, 0xec2b,0x3612, 0xaac8,0x39db
        DCW 0xbcf0,0x0324, 0xea02,0x36e5, 0xa963,0x3871
        DCW 0xc197,0xfe6e, 0xe7dc,0x37b0, 0xa834,0x36e5
        DCW 0xc695,0xf9ba, 0xe5ba,0x3871, 0xa73b,0x3537
        DCW 0xcbe2,0xf50f, 0xe39c,0x392b, 0xa678,0x3368
        DCW 0xd178,0xf073, 0xe182,0x39db, 0xa5ed,0x3179
        DCW 0xd74e,0xebed, 0xdf6d,0x3a82, 0xa599,0x2f6c
        DCW 0xdd5d,0xe782, 0xdd5d,0x3b21, 0xa57e,0x2d41
        DCW 0xe39c,0xe33a, 0xdb52,0x3bb6, 0xa599,0x2afb
        DCW 0xea02,0xdf19, 0xd94d,0x3c42, 0xa5ed,0x289a
        DCW 0xf087,0xdb26, 0xd74e,0x3cc5, 0xa678,0x2620
        DCW 0xf721,0xd766, 0xd556,0x3d3f, 0xa73b,0x238e
        DCW 0xfdc7,0xd3df, 0xd363,0x3daf, 0xa834,0x20e7
        DCW 0x0471,0xd094, 0xd178,0x3e15, 0xa963,0x1e2b
        DCW 0x0b14,0xcd8c, 0xcf94,0x3e72, 0xaac8,0x1b5d
        DCW 0x11a8,0xcac9, 0xcdb7,0x3ec5, 0xac61,0x187e
        DCW 0x1824,0xc850, 0xcbe2,0x3f0f, 0xae2e,0x1590
        DCW 0x1e7e,0xc625, 0xca15,0x3f4f, 0xb02d,0x1294
        DCW 0x24ae,0xc44a, 0xc851,0x3f85, 0xb25e,0x0f8d
        DCW 0x2aaa,0xc2c1, 0xc695,0x3fb1, 0xb4be,0x0c7c
        DCW 0x306c,0xc18e, 0xc4e2,0x3fd4, 0xb74d,0x0964
        DCW 0x35eb,0xc0b1, 0xc338,0x3fec, 0xba09,0x0646
        DCW 0x3b1e,0xc02c, 0xc197,0x3ffb, 0xbcf0,0x0324
        
       END
;******************* (C) COPYRIGHT 2009  STMicroelectronics *****END OF FILE****
