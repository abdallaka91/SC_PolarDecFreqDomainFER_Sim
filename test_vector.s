	.file	"test_vector.cpp"
	.text
#APP
	.globl _ZSt21ios_base_library_initv
#NO_APP
	.section	.text.startup,"ax",@progbits
	.p2align 4
	.globl	main
	.type	main, @function
main:
.LFB9209:
	.cfi_startproc
	endbr64
	leaq	8(%rsp), %r10
	.cfi_def_cfa 10, 0
	andq	$-32, %rsp
	pushq	-8(%r10)
	pushq	%rbp
	movq	%rsp, %rbp
	.cfi_escape 0x10,0x6,0x2,0x76,0
	pushq	%r12
	pushq	%r10
	.cfi_escape 0xf,0x3,0x76,0x70,0x6
	.cfi_escape 0x10,0xc,0x2,0x76,0x78
	pushq	%rbx
	subq	$4096, %rsp
	orq	$0, (%rsp)
	subq	$2200, %rsp
	.cfi_escape 0x10,0x3,0x2,0x76,0x68
	vmovaps	.LC1(%rip), %ymm15
	vmovaps	.LC2(%rip), %ymm14
	vmovaps	.LC3(%rip), %ymm13
	vmovaps	.LC4(%rip), %ymm12
	vmovaps	.LC5(%rip), %ymm11
	vmovaps	.LC6(%rip), %ymm10
	vmovaps	.LC7(%rip), %ymm9
	vmovaps	.LC8(%rip), %ymm8
	vmovaps	.LC9(%rip), %ymm7
	vmovaps	.LC10(%rip), %ymm6
	vmovaps	.LC11(%rip), %ymm5
	vmovaps	.LC12(%rip), %ymm4
	movq	%fs:40, %rax
	movq	%rax, -56(%rbp)
	xorl	%eax, %eax
	vmovaps	.LC13(%rip), %ymm3
	vmovaps	.LC14(%rip), %ymm2
	vmovups	%ymm15, -4224(%rbp)
	vmovaps	.LC15(%rip), %ymm1
	vmovups	%ymm14, -2144(%rbp)
	leaq	-2144(%rbp), %rsi
	leaq	-6304(%rbp), %rax
	vmovups	%ymm13, -4192(%rbp)
	movq	%rsi, %rdi
	movq	%rax, %rdx
	leaq	-4224(%rbp), %rcx
	vmovups	%ymm12, -2112(%rbp)
	vmovups	%ymm11, -4160(%rbp)
	vmovups	%ymm10, -2080(%rbp)
	vmovups	%ymm9, -4128(%rbp)
	vmovups	%ymm8, -2048(%rbp)
	vmovups	%ymm7, -4096(%rbp)
	vmovups	%ymm6, -2016(%rbp)
	vmovups	%ymm5, -4064(%rbp)
	vmovups	%ymm4, -1984(%rbp)
	vmovups	%ymm3, -4032(%rbp)
	vmovups	%ymm2, -1952(%rbp)
	vmovups	%ymm1, -4000(%rbp)
	vmovaps	.LC16(%rip), %ymm0
	vmovups	%ymm15, -3964(%rbp)
	vmovups	%ymm0, -1920(%rbp)
	vmovups	%ymm14, -1884(%rbp)
	vmovups	%ymm13, -3932(%rbp)
	vmovups	%ymm12, -1852(%rbp)
	vmovups	%ymm11, -3900(%rbp)
	vmovups	%ymm10, -1820(%rbp)
	vmovups	%ymm9, -3868(%rbp)
	vmovups	%ymm8, -1788(%rbp)
	vmovups	%ymm7, -3836(%rbp)
	vmovups	%ymm6, -1756(%rbp)
	vmovups	%ymm5, -3804(%rbp)
	vmovups	%ymm4, -1724(%rbp)
	vmovups	%ymm3, -3772(%rbp)
	vmovups	%ymm2, -1692(%rbp)
	vmovups	%ymm1, -3740(%rbp)
	vmovups	%ymm0, -1660(%rbp)
	vmovups	%ymm15, -3704(%rbp)
	vmovups	%ymm14, -1624(%rbp)
	vmovups	%ymm13, -3672(%rbp)
	vmovups	%ymm12, -1592(%rbp)
	vmovups	%ymm11, -3640(%rbp)
	vmovups	%ymm10, -1560(%rbp)
	vmovups	%ymm9, -3608(%rbp)
	vmovups	%ymm8, -1528(%rbp)
	vmovups	%ymm7, -3576(%rbp)
	vmovups	%ymm6, -1496(%rbp)
	vmovups	%ymm5, -3544(%rbp)
	vmovups	%ymm4, -1464(%rbp)
	vmovups	%ymm3, -3512(%rbp)
	vmovups	%ymm2, -1432(%rbp)
	vmovups	%ymm1, -3480(%rbp)
	vmovups	%ymm0, -1400(%rbp)
	vmovups	%ymm15, -3444(%rbp)
	vmovups	%ymm14, -1364(%rbp)
	vmovups	%ymm13, -3412(%rbp)
	vmovups	%ymm12, -1332(%rbp)
	vmovups	%ymm11, -3380(%rbp)
	vmovups	%ymm10, -1300(%rbp)
	vmovups	%ymm9, -3348(%rbp)
	vmovups	%ymm8, -1268(%rbp)
	vmovups	%ymm7, -3316(%rbp)
	vmovups	%ymm6, -1236(%rbp)
	vmovups	%ymm5, -3284(%rbp)
	vmovups	%ymm4, -1204(%rbp)
	vmovups	%ymm3, -3252(%rbp)
	vmovups	%ymm2, -1172(%rbp)
	vmovups	%ymm1, -3220(%rbp)
	vmovups	%ymm0, -1140(%rbp)
	vmovups	%ymm15, -3184(%rbp)
	vmovups	%ymm14, -1104(%rbp)
	vmovups	%ymm13, -3152(%rbp)
	vmovups	%ymm12, -1072(%rbp)
	vmovups	%ymm11, -3120(%rbp)
	vmovups	%ymm10, -1040(%rbp)
	vmovups	%ymm9, -3088(%rbp)
	vmovups	%ymm8, -1008(%rbp)
	vmovups	%ymm7, -3056(%rbp)
	vmovups	%ymm6, -976(%rbp)
	vmovups	%ymm5, -3024(%rbp)
	vmovups	%ymm4, -944(%rbp)
	vmovups	%ymm3, -2992(%rbp)
	vmovups	%ymm2, -912(%rbp)
	vmovups	%ymm1, -2960(%rbp)
	vmovups	%ymm0, -880(%rbp)
	vmovups	%ymm15, -2924(%rbp)
	vmovups	%ymm14, -844(%rbp)
	vmovups	%ymm13, -2892(%rbp)
	vmovups	%ymm12, -812(%rbp)
	vmovups	%ymm11, -2860(%rbp)
	vmovups	%ymm10, -780(%rbp)
	vmovups	%ymm9, -2828(%rbp)
	vmovups	%ymm8, -748(%rbp)
	vmovups	%ymm7, -2796(%rbp)
	vmovups	%ymm6, -716(%rbp)
	vmovups	%ymm5, -2764(%rbp)
	vmovups	%ymm4, -684(%rbp)
	vmovups	%ymm3, -2732(%rbp)
	vmovups	%ymm2, -652(%rbp)
	vmovups	%ymm1, -2700(%rbp)
	vmovups	%ymm0, -620(%rbp)
	vmovups	%ymm15, -2664(%rbp)
	vmovups	%ymm14, -584(%rbp)
	vmovups	%ymm13, -2632(%rbp)
	vmovups	%ymm12, -552(%rbp)
	vmovups	%ymm11, -2600(%rbp)
	vmovups	%ymm10, -520(%rbp)
	vmovups	%ymm9, -2568(%rbp)
	vmovups	%ymm8, -488(%rbp)
	vmovups	%ymm7, -2536(%rbp)
	vmovups	%ymm6, -456(%rbp)
	vmovups	%ymm5, -2504(%rbp)
	vmovups	%ymm4, -424(%rbp)
	vmovups	%ymm3, -2472(%rbp)
	vmovups	%ymm2, -392(%rbp)
	vmovups	%ymm1, -2440(%rbp)
	vmovups	%ymm0, -360(%rbp)
	vmovups	%ymm15, -2404(%rbp)
	vmovups	%ymm14, -324(%rbp)
	vmovups	%ymm13, -2372(%rbp)
	vmovups	%ymm12, -292(%rbp)
	vmovups	%ymm11, -2340(%rbp)
	vmovups	%ymm10, -260(%rbp)
	vmovups	%ymm9, -2308(%rbp)
	vmovups	%ymm8, -228(%rbp)
	vmovups	%ymm7, -2276(%rbp)
	vmovups	%ymm6, -196(%rbp)
	vmovups	%ymm5, -2244(%rbp)
	vmovups	%ymm4, -164(%rbp)
	vmovups	%ymm3, -2212(%rbp)
	vmovups	%ymm2, -132(%rbp)
	vmovups	%ymm1, -2180(%rbp)
	vmovups	%ymm0, -100(%rbp)
	.p2align 4,,10
	.p2align 3
.L2:
	vmovups	(%rcx), %ymm1
	vmulps	(%rsi), %ymm1, %ymm0
	addq	$260, %rcx
	movb	$1, 256(%rdx)
	vmovups	-228(%rcx), %ymm2
	vmovups	-196(%rcx), %ymm3
	addq	$260, %rsi
	addq	$260, %rdx
	vmovups	-164(%rcx), %ymm4
	vmovups	-132(%rcx), %ymm5
	vmovups	-100(%rcx), %ymm6
	vmovups	-68(%rcx), %ymm7
	vmovups	-36(%rcx), %ymm1
	vmovups	%ymm0, -260(%rdx)
	vmulps	-228(%rsi), %ymm2, %ymm0
	vmovups	%ymm0, -228(%rdx)
	vmulps	-196(%rsi), %ymm3, %ymm0
	vmovups	%ymm0, -196(%rdx)
	vmulps	-164(%rsi), %ymm4, %ymm0
	vmovups	%ymm0, -164(%rdx)
	vmulps	-132(%rsi), %ymm5, %ymm0
	vmovups	%ymm0, -132(%rdx)
	vmulps	-100(%rsi), %ymm6, %ymm0
	vmovups	%ymm0, -100(%rdx)
	vmulps	-68(%rsi), %ymm7, %ymm0
	vmovups	%ymm0, -68(%rdx)
	vmulps	-36(%rsi), %ymm1, %ymm0
	vmovups	%ymm0, -36(%rdx)
	cmpq	%rdi, %rcx
	jne	.L2
	leaq	2080(%rax), %rdx
	vxorps	%xmm0, %xmm0, %xmm0
	.p2align 4,,10
	.p2align 3
.L3:
	vaddss	(%rax), %xmm0, %xmm0
	addq	$260, %rax
	vaddss	-256(%rax), %xmm0, %xmm0
	vaddss	-252(%rax), %xmm0, %xmm0
	vaddss	-248(%rax), %xmm0, %xmm0
	vaddss	-244(%rax), %xmm0, %xmm0
	vaddss	-240(%rax), %xmm0, %xmm0
	vaddss	-236(%rax), %xmm0, %xmm0
	vaddss	-232(%rax), %xmm0, %xmm0
	vaddss	-228(%rax), %xmm0, %xmm0
	vaddss	-224(%rax), %xmm0, %xmm0
	vaddss	-220(%rax), %xmm0, %xmm0
	vaddss	-216(%rax), %xmm0, %xmm0
	vaddss	-212(%rax), %xmm0, %xmm0
	vaddss	-208(%rax), %xmm0, %xmm0
	vaddss	-204(%rax), %xmm0, %xmm0
	vaddss	-200(%rax), %xmm0, %xmm0
	vaddss	-196(%rax), %xmm0, %xmm0
	vaddss	-192(%rax), %xmm0, %xmm0
	vaddss	-188(%rax), %xmm0, %xmm0
	vaddss	-184(%rax), %xmm0, %xmm0
	vaddss	-180(%rax), %xmm0, %xmm0
	vaddss	-176(%rax), %xmm0, %xmm0
	vaddss	-172(%rax), %xmm0, %xmm0
	vaddss	-168(%rax), %xmm0, %xmm0
	vaddss	-164(%rax), %xmm0, %xmm0
	vaddss	-160(%rax), %xmm0, %xmm0
	vaddss	-156(%rax), %xmm0, %xmm0
	vaddss	-152(%rax), %xmm0, %xmm0
	vaddss	-148(%rax), %xmm0, %xmm0
	vaddss	-144(%rax), %xmm0, %xmm0
	vaddss	-140(%rax), %xmm0, %xmm0
	vaddss	-136(%rax), %xmm0, %xmm0
	vaddss	-132(%rax), %xmm0, %xmm0
	vaddss	-128(%rax), %xmm0, %xmm0
	vaddss	-124(%rax), %xmm0, %xmm0
	vaddss	-120(%rax), %xmm0, %xmm0
	vaddss	-116(%rax), %xmm0, %xmm0
	vaddss	-112(%rax), %xmm0, %xmm0
	vaddss	-108(%rax), %xmm0, %xmm0
	vaddss	-104(%rax), %xmm0, %xmm0
	vaddss	-100(%rax), %xmm0, %xmm0
	vaddss	-96(%rax), %xmm0, %xmm0
	vaddss	-92(%rax), %xmm0, %xmm0
	vaddss	-88(%rax), %xmm0, %xmm0
	vaddss	-84(%rax), %xmm0, %xmm0
	vaddss	-80(%rax), %xmm0, %xmm0
	vaddss	-76(%rax), %xmm0, %xmm0
	vaddss	-72(%rax), %xmm0, %xmm0
	vaddss	-68(%rax), %xmm0, %xmm0
	vaddss	-64(%rax), %xmm0, %xmm0
	vaddss	-60(%rax), %xmm0, %xmm0
	vaddss	-56(%rax), %xmm0, %xmm0
	vaddss	-52(%rax), %xmm0, %xmm0
	vaddss	-48(%rax), %xmm0, %xmm0
	vaddss	-44(%rax), %xmm0, %xmm0
	vaddss	-40(%rax), %xmm0, %xmm0
	vaddss	-36(%rax), %xmm0, %xmm0
	vaddss	-32(%rax), %xmm0, %xmm0
	vaddss	-28(%rax), %xmm0, %xmm0
	vaddss	-24(%rax), %xmm0, %xmm0
	vaddss	-20(%rax), %xmm0, %xmm0
	vaddss	-16(%rax), %xmm0, %xmm0
	vaddss	-12(%rax), %xmm0, %xmm0
	vaddss	-8(%rax), %xmm0, %xmm0
	cmpq	%rdx, %rax
	jne	.L3
	leaq	_ZSt4cout(%rip), %rdi
	vcvtss2sd	%xmm0, %xmm0, %xmm0
	vzeroupper
	call	_ZNSo9_M_insertIdEERSoT_@PLT
	movq	%rax, %rbx
	movq	(%rax), %rax
	movq	-24(%rax), %rax
	movq	240(%rbx,%rax), %r12
	testq	%r12, %r12
	je	.L16
	cmpb	$0, 56(%r12)
	je	.L7
	movzbl	67(%r12), %eax
.L8:
	movsbl	%al, %esi
	movq	%rbx, %rdi
	call	_ZNSo3putEc@PLT
	movq	%rax, %rdi
	call	_ZNSo5flushEv@PLT
	movq	-56(%rbp), %rax
	subq	%fs:40, %rax
	jne	.L14
	addq	$6296, %rsp
	xorl	%eax, %eax
	popq	%rbx
	popq	%r10
	.cfi_remember_state
	.cfi_def_cfa 10, 0
	popq	%r12
	popq	%rbp
	leaq	-8(%r10), %rsp
	.cfi_def_cfa 7, 8
	ret
.L7:
	.cfi_restore_state
	movq	%r12, %rdi
	call	_ZNKSt5ctypeIcE13_M_widen_initEv@PLT
	movq	(%r12), %rax
	movl	$10, %esi
	movq	%r12, %rdi
	call	*48(%rax)
	jmp	.L8
.L16:
	movq	-56(%rbp), %rax
	subq	%fs:40, %rax
	jne	.L14
	call	_ZSt16__throw_bad_castv@PLT
.L14:
	call	__stack_chk_fail@PLT
	.cfi_endproc
.LFE9209:
	.size	main, .-main
	.section	.rodata.cst32,"aM",@progbits,32
	.align 32
.LC1:
	.long	1065353216
	.long	1073741824
	.long	1077936128
	.long	1082130432
	.long	1084227584
	.long	1086324736
	.long	1088421888
	.long	1090519040
	.align 32
.LC2:
	.long	1073741824
	.long	1077936128
	.long	1082130432
	.long	1084227584
	.long	1086324736
	.long	1088421888
	.long	1090519040
	.long	1091567616
	.align 32
.LC3:
	.long	1091567616
	.long	1092616192
	.long	1093664768
	.long	1094713344
	.long	1095761920
	.long	1096810496
	.long	1097859072
	.long	1098907648
	.align 32
.LC4:
	.long	1092616192
	.long	1093664768
	.long	1094713344
	.long	1095761920
	.long	1096810496
	.long	1097859072
	.long	1098907648
	.long	1099431936
	.align 32
.LC5:
	.long	1099431936
	.long	1099956224
	.long	1100480512
	.long	1101004800
	.long	1101529088
	.long	1102053376
	.long	1102577664
	.long	1103101952
	.align 32
.LC6:
	.long	1099956224
	.long	1100480512
	.long	1101004800
	.long	1101529088
	.long	1102053376
	.long	1102577664
	.long	1103101952
	.long	1103626240
	.align 32
.LC7:
	.long	1103626240
	.long	1104150528
	.long	1104674816
	.long	1105199104
	.long	1105723392
	.long	1106247680
	.long	1106771968
	.long	1107296256
	.align 32
.LC8:
	.long	1104150528
	.long	1104674816
	.long	1105199104
	.long	1105723392
	.long	1106247680
	.long	1106771968
	.long	1107296256
	.long	1107558400
	.align 32
.LC9:
	.long	1107558400
	.long	1107820544
	.long	1108082688
	.long	1108344832
	.long	1108606976
	.long	1108869120
	.long	1109131264
	.long	1109393408
	.align 32
.LC10:
	.long	1107820544
	.long	1108082688
	.long	1108344832
	.long	1108606976
	.long	1108869120
	.long	1109131264
	.long	1109393408
	.long	1109655552
	.align 32
.LC11:
	.long	1109655552
	.long	1109917696
	.long	1110179840
	.long	1110441984
	.long	1110704128
	.long	1110966272
	.long	1111228416
	.long	1111490560
	.align 32
.LC12:
	.long	1109917696
	.long	1110179840
	.long	1110441984
	.long	1110704128
	.long	1110966272
	.long	1111228416
	.long	1111490560
	.long	1111752704
	.align 32
.LC13:
	.long	1111752704
	.long	1112014848
	.long	1112276992
	.long	1112539136
	.long	1112801280
	.long	1113063424
	.long	1113325568
	.long	1113587712
	.align 32
.LC14:
	.long	1112014848
	.long	1112276992
	.long	1112539136
	.long	1112801280
	.long	1113063424
	.long	1113325568
	.long	1113587712
	.long	1113849856
	.align 32
.LC15:
	.long	1113849856
	.long	1114112000
	.long	1114374144
	.long	1114636288
	.long	1114898432
	.long	1115160576
	.long	1115422720
	.long	1115684864
	.align 32
.LC16:
	.long	1114112000
	.long	1114374144
	.long	1114636288
	.long	1114898432
	.long	1115160576
	.long	1115422720
	.long	1115684864
	.long	1115815936
	.ident	"GCC: (Ubuntu 13.3.0-6ubuntu2~24.04) 13.3.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	1f - 0f
	.long	4f - 1f
	.long	5
0:
	.string	"GNU"
1:
	.align 8
	.long	0xc0000002
	.long	3f - 2f
2:
	.long	0x3
3:
	.align 8
4:
