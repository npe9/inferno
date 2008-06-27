	.align	2
	.global	FPsave
FPsave:
	stfd	%f0,8(%r3)
	stfd	%f1,16(%r3)
	stfd	%f2,24(%r3)
	stfd	%f3,32(%r3)
	stfd	%f4,40(%r3)
	stfd	%f5,48(%r3)
	stfd	%f6,56(%r3)
	stfd	%f7,64(%r3)
	stfd	%f8,72(%r3)
	stfd	%f9,80(%r3)
	stfd	%f10,88(%r3)
	stfd	%f11,96(%r3)
	stfd	%f12,104(%r3)
	stfd	%f13,112(%r3)
	stfd	%f14,120(%r3)
	stfd	%f15,128(%r3)
	stfd	%f16,136(%r3)
	stfd	%f17,144(%r3)
	stfd	%f18,152(%r3)
	stfd	%f19,160(%r3)
	stfd	%f20,168(%r3)
	stfd	%f21,176(%r3)
	stfd	%f22,184(%r3)
	stfd	%f23,192(%r3)
	stfd	%f24,200(%r3)
	stfd	%f25,208(%r3)
	stfd	%f26,216(%r3)
	stfd	%f27,224(%r3)
	stfd	%f28,232(%r3)
	stfd	%f29,240(%r3)
	stfd	%f30,248(%r3)
	stfd	%f31,256(%r3)
	mffs	%f0
	stfd	%f0,0(%r3)
	/* Restore f0 so that the save procedure doesn't
	 * alter the FP state */
	lfd		%f0,8(%r1)
	/* do we need to adjust stack pointer?) */
	blr

	.align	2
	.global	FPrestore
FPrestore:
	lfd		%f0,0(%r3)
	mtfsf	0xff, %f0
	lfd		%f0,8(%r3)
	lfd		%f1,16(%r3)
	lfd		%f2,24(%r3)
	lfd		%f3,32(%r3)
	lfd		%f4,40(%r3)
	lfd		%f5,48(%r3)
	lfd		%f6,56(%r3)
	lfd		%f7,64(%r3)
	lfd		%f8,72(%r3)
	lfd		%f9,80(%r3)
	lfd		%f10,88(%r3)
	lfd		%f11,96(%r3)
	lfd		%f12,104(%r3)
	lfd		%f13,112(%r3)
	lfd		%f14,120(%r3)
	lfd		%f15,128(%r3)
	lfd		%f16,136(%r3)
	lfd		%f17,144(%r3)
	lfd		%f18,152(%r3)
	lfd		%f19,160(%r3)
	lfd		%f20,168(%r3)
	lfd		%f21,176(%r3)
	lfd		%f22,184(%r3)
	lfd		%f23,192(%r3)
	lfd		%f24,200(%r3)
	lfd		%f25,208(%r3)
	lfd		%f26,216(%r3)
	lfd		%f27,224(%r3)
	lfd		%f28,232(%r3)
	lfd		%f29,240(%r3)
	lfd		%f31,256(%r3)
	blr

	.align	2
	.global	_tas
_tas:
	sync
	li		%r0, 0
	mr		%r4, %r3
	lis		%r5, 0xcafe
	ori		%r5, %r5, 0xbabe
1:
	lwarx	%r3, %r0, %r4
	cmpwi	%r3, 0
	bne		2f
	stwcx.	%r5, %r0, %r4
	bne-	1b
2:
	sync
	blr

/*
 * void
 * executeonnewstack(void *tos, void (*tramp)(void *arg), void *arg)
 */
	.align	2
	.global	executeonnewstack
executeonnewstack:
	mr		%r1,%r3	/* change stacks */
	stwu 	%lr,-16(%r1)	/* save lr to aid the traceback */
	li		%r0,0
	stw 	%r0,20(%r1)
	mr		%r3,%r5
	mtctr 	%r4
	bctrl	/* tramp(arg) */
	br

/*
 * void unlockandexit(int *key)
 *
 * NB: the return status may be garbaged if the stack is reused
 *	between the unlock and the system call, but this should
 *	not matter since no task is waiting for the result
 */
	.align	2
	.globl	unlockandexit
unlockandexit:
	li	%r0,0x0
	stw	%r0,0(%r3)	/* unlock */
	li	%r0,1		/* sys exit; 234 is exit group */
	li	%r3,0		/* exit status */
	sc
	br
