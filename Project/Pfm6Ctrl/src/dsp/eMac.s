;
;
; implementacija integratorja energije z DSP instrukcijami ...
;
;
                AREA    |.text|, CODE, READONLY

eMac			PROC
				push		{r4-r6}
				mov			r5,#0				; brisi r5,r6
				mov			r6,#0
				lsl			r2,#16				; tokovni offset v zgornjo polovico r2
__eloop
				ldr			r4,[r0,#0]			; r4 = U/I dma buffer
				ssub16		r4,r4,r2			; odštej tokovni offset
				smlalbt		r5,r6,r4,r4			; mac
				add			r0,r0,#4			; inkr. dma stack pointer
				subs		r1,r1,#1			; dekr. števca
				bne 		__eloop				; loop
				
				mov			r0,r5				; return r1:r0 64 bits
				mov			r1,r6
				pop			{r4-r6}
				bx			lr
                ENDP
				EXPORT		eMac
				END
