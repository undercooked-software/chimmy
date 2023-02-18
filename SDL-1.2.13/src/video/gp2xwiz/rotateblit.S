@ Screen rotate routine for GP2X Wiz

.global rotateblitfast8 @ void* src,void* dst,unsigned int w,unsigned int h
rotateblitfast8:
	stmfd sp!,{r4-r12,lr}

	mov r6,#240 @ dstskip
	mul r5,r6,r2
	mov r4,#1280 @ srcskip
	sub r4,r4,r2
	mov lr,r2
	add r5,r5,#4

rotateloopfast8:
	ldr r9,[r0,#960]
	ldr r8,[r0,#640]
	ldr r7,[r0,#320]
	ldr r6,[r0],#4

	mov r10,r8,lsl #24
	mov r11,r9,lsl #24
	orr r11,r11,r10,lsr #8
	mov r10,r6,lsl #24
	mov r12,r7,lsl #24
	orr r11,r11,r12,lsr #16
	orr r11,r11,r10,lsr #24
	str r11,[r1],#-240

	mov r10,r8,lsr #8
	mov r10,r10,lsl #24
	mov r11,r9,lsr #8
	mov r11,r11,lsl #24
	orr r11,r11,r10,lsr #8
	mov r10,r6,lsr #8
	mov r10,r10,lsl #24
	mov r12,r7,lsr #8
	mov r12,r12,lsl #24
	orr r11,r11,r12,lsr #16
	orr r11,r11,r10,lsr #24
	str r11,[r1],#-240

	mov r10,r6,lsl #8
	mov r10,r10,lsr #24
	mov r11,r7,lsl #8
	mov r11,r11,lsr #24
	orr r10,r10,r11,lsl #8
	mov r11,r8,lsl #8
	mov r11,r11,lsr #24
	mov r12,r9,lsl #8
	mov r12,r12,lsr #24
	orr r10,r10,r11,lsl #16
	orr r10,r10,r12,lsl #24
	str r10,[r1],#-240

	mov r6,r6,lsr #24
	mov r7,r7,lsr #24
	orr r6,r6,r7,lsl #8
	mov r8,r8,lsr #24
	mov r9,r9,lsr #24
	orr r6,r6,r8,lsl #16
	orr r6,r6,r9,lsl #24
	str r6,[r1],#-240

	subs lr,lr,#4
	bne rotateloopfast8

	subs r3,r3,#4
	add r0,r0,r4
	add r1,r1,r5
	mov lr,r2
	bne rotateloopfast8

	ldmfd sp!,{r4-r12,pc}


.global rotateblitslow8
rotateblitslow8: @ void* src,void* dst,unsigned int w,unsigned int h
	stmfd sp!,{r4-r7,lr}

	mov r7,#240 @ dstskip
	mul r5,r7,r2
	mov r4,#320 @ srcskip
	sub r4,r4,r2
	mov r6,r2
	add r5,r5,#1

rotateloopslow8:
	ldrb r7,[r0],#1
	strb r7,[r1],#-240

	subs r6,r6,#1
	bne rotateloopslow8

	subs r3,r3,#1
	add r0,r0,r4
	add r1,r1,r5
	mov r6,r2
	bne rotateloopslow8

	ldmfd sp!,{r4-r7,pc}


.global rotateblitfast16_2x2 @ void* src,void* dst,unsigned int w,unsigned int h
rotateblitfast16_2x2:
	stmfd sp!,{r4-r10,lr}

	mov r6,#480 @ dstskip
	mul r5,r6,r2
	mov r4,#1280 @ srcskip
	mov r7,r2,lsl #1
	sub r4,r4,r7
	mov r6,r2
	add r5,r5,#4

rotateloop16_2x2:
	ldr r8,[r0,#640]
	ldr r7,[r0],#4

	mov r10,r8,lsl #16
	mov r9,r7,lsl #16
	orr r9,r10,r9,lsr #16
	str r9,[r1],#-480

	mov r8,r8,lsr #16
	mov r7,r7,lsr #16
	orr r8,r7,r8,lsl #16
	str r8,[r1],#-480

	subs r6,r6,#2
	bne rotateloop16_2x2

	subs r3,r3,#2
	add r0,r0,r4
	add r1,r1,r5
	mov r6,r2
	bne rotateloop16_2x2

	ldmfd sp!,{r4-r10,pc}


.global rotateblitfast16_4x2 @ void* src,void* dst,unsigned int w,unsigned int h
rotateblitfast16_4x2:
	stmfd sp!,{r4-r10,lr}

	mov r6,#480 @ dstskip
	mul r5,r6,r2
	mov r4,#1280 @ srcskip
	mov r7,r2,lsl #1
	sub r4,r4,r7
	mov r6,r2
	add r5,r5,#4

rotateloop16_4x2:
	ldr r8,[r0,#640]
	ldr r7,[r0],#4

	mov r10,r8,lsl #16
	mov r9,r7,lsl #16
	orr r9,r10,r9,lsr #16
	str r9,[r1],#-480

	mov r8,r8,lsr #16
	mov r7,r7,lsr #16
	orr r8,r7,r8,lsl #16
	str r8,[r1],#-480

	ldr r8,[r0,#640]
	ldr r7,[r0],#4

	mov r10,r8,lsl #16
	mov r9,r7,lsl #16
	orr r9,r10,r9,lsr #16
	str r9,[r1],#-480

	mov r8,r8,lsr #16
	mov r7,r7,lsr #16
	orr r8,r7,r8,lsl #16
	str r8,[r1],#-480

	subs r6,r6,#4
	bne rotateloop16_4x2

	subs r3,r3,#2
	add r0,r0,r4
	add r1,r1,r5
	mov r6,r2
	bne rotateloop16_4x2

	ldmfd sp!,{r4-r10,pc}


.global rotateblitfast16_2x4 @ void* src,void* dst,unsigned int w,unsigned int h
rotateblitfast16_2x4:
	stmfd sp!,{r4-r12,lr}

	mov r6,#480 @ dstskip
	mul r5,r6,r2
	mov r4,#2560 @ srcskip
	mov r7,r2,lsl #1
	sub r4,r4,r7
	mov lr,r2
	add r5,r5,#8

rotateloop16_2x4:
	ldr r9,[r0,#1920]
	ldr r8,[r0,#1280]
	ldr r7,[r0,#640]
	ldr r6,[r0],#4

	mov r12,r9,lsl #16
	mov r11,r8,lsl #16
	orr r11,r12,r11,lsr #16
	mov r12,r7,lsl #16
	mov r10,r6,lsl #16
	orr r10,r12,r10,lsr #16
	stmia r1,{r10,r11}
	sub r1,r1,#480
	
	mov r9,r9,lsr #16
	mov r8,r8,lsr #16
	orr r8,r8,r9,lsl #16
	mov r7,r7,lsr #16
	mov r6,r6,lsr #16
	orr r7,r6,r7,lsl #16
	stmia r1,{r7,r8}
	sub r1,r1,#480

	subs lr,lr,#2
	bne rotateloop16_2x4

	subs r3,r3,#4
	add r0,r0,r4
	add r1,r1,r5
	mov lr,r2
	bne rotateloop16_2x4

	ldmfd sp!,{r4-r12,pc}


.global rotateblitfast16_4x4 @ void* src,void* dst,unsigned int w,unsigned int h
rotateblitfast16_4x4:
	stmfd sp!,{r4-r12,lr}

	mov r6,#480 @ dstskip
	mul r5,r6,r2
	mov r4,#2560 @ srcskip
	mov r7,r2,lsl #1
	sub r4,r4,r7
	mov lr,r2
	add r5,r5,#8

rotateloop16_4x4:
	ldr r9,[r0,#1920]
	ldr r8,[r0,#1280]
	ldr r7,[r0,#640]
	ldr r6,[r0],#4

	mov r12,r9,lsl #16
	mov r11,r8,lsl #16
	orr r11,r12,r11,lsr #16
	mov r12,r7,lsl #16
	mov r10,r6,lsl #16
	orr r10,r12,r10,lsr #16
	stmia r1,{r10,r11}
	sub r1,r1,#480
	
	mov r9,r9,lsr #16
	mov r8,r8,lsr #16
	orr r8,r8,r9,lsl #16
	mov r7,r7,lsr #16
	mov r6,r6,lsr #16
	orr r7,r6,r7,lsl #16
	stmia r1,{r7,r8}
	sub r1,r1,#480

	ldr r9,[r0,#1920]
	ldr r8,[r0,#1280]
	ldr r7,[r0,#640]
	ldr r6,[r0],#4

	mov r12,r9,lsl #16
	mov r11,r8,lsl #16
	orr r11,r12,r11,lsr #16
	mov r12,r7,lsl #16
	mov r10,r6,lsl #16
	orr r10,r12,r10,lsr #16
	stmia r1,{r10,r11}
	sub r1,r1,#480
	
	mov r9,r9,lsr #16
	mov r8,r8,lsr #16
	orr r8,r8,r9,lsl #16
	mov r7,r7,lsr #16
	mov r6,r6,lsr #16
	orr r7,r6,r7,lsl #16
	stmia r1,{r7,r8}
	sub r1,r1,#480

	subs lr,lr,#4
	bne rotateloop16_4x4

	subs r3,r3,#4
	add r0,r0,r4
	add r1,r1,r5
	mov lr,r2
	bne rotateloop16_4x4

	ldmfd sp!,{r4-r12,pc}


.global rotateblitslow16 @ void* src,void* dst,unsigned int w,unsigned int h
rotateblitslow16: 
	stmfd sp!,{r4-r7,lr}

	mov r6,#480 @ dstskip
	mul r5,r6,r2
	mov r4,#320 @ srcskip
	sub r4,r4,r2
	mov r4,r4,lsl #1
	mov r6,r2
	add r5,r5,#2

rotateloopslow16:
	ldrh r7,[r0],#2
	strh r7,[r1]
	sub r1,r1,#480

	subs r6,r6,#1
	bne rotateloopslow16

	subs r3,r3,#1
	add r0,r0,r4
	add r1,r1,r5
	mov r6,r2
	bne rotateloopslow16

	ldmfd sp!,{r4-r7,pc}


.global rotateblitfast32_2x1 @ void* src,void* dst,unsigned int w,unsigned int h
rotateblitfast32_2x1: 
	stmfd sp!,{r4-r7,lr}

	mov r6,#960 @ dstskip
	mul r5,r6,r2
	mov r4,#2560 @ srcskip
	mov r7,r2,lsl #2
	sub r4,r4,r7
	mov lr,r2
	add r5,r5,#8

rotateloopfast32_2x1:
	ldr r7,[r0,#1280]
	ldr r6,[r0],#4
	stmia r1,{r6,r7}
	sub r1,r1,#960

	subs lr,lr,#1
	bne rotateloopfast32_2x1

	subs r3,r3,#2
	add r0,r0,r4
	add r1,r1,r5
	mov lr,r2
	bne rotateloopfast32_2x1

	ldmfd sp!,{r4-r7,pc}


.global rotateblitfast32_4x1 @ void* src,void* dst,unsigned int w,unsigned int h
rotateblitfast32_4x1: 
	stmfd sp!,{r4-r9,lr}

	mov r6,#960 @ dstskip
	mul r5,r6,r2
	mov r4,#5120 @ srcskip
	mov r7,r2,lsl #2
	sub r4,r4,r7
	mov lr,r2
	add r5,r5,#16

rotateloopfast32_4x1:
	ldr r9,[r0,#3840]
	ldr r8,[r0,#2560]
	ldr r7,[r0,#1280]
	ldr r6,[r0],#4
	stmia r1,{r6-r9}
	sub r1,r1,#960

	subs lr,lr,#1
	bne rotateloopfast32_4x1

	subs r3,r3,#4
	add r0,r0,r4
	add r1,r1,r5
	mov lr,r2
	bne rotateloopfast32_4x1

	ldmfd sp!,{r4-r9,pc}


.global rotateblitslow32 @ void* src,void* dst,unsigned int w,unsigned int h
rotateblitslow32: 
	stmfd sp!,{r4-r7,lr}

	mov r6,#960 @ dstskip
	mul r5,r6,r2
	mov r4,#320 @ srcskip
	sub r4,r4,r2
	mov r4,r4,lsl #2
	mov r6,r2
	add r5,r5,#4

rotateloopslow32:
	ldr r7,[r0],#4
	str r7,[r1]
	sub r1,r1,#960

	subs r6,r6,#1
	bne rotateloopslow32

	subs r3,r3,#1
	add r0,r0,r4
	add r1,r1,r5
	mov r6,r2
	bne rotateloopslow32

	ldmfd sp!,{r4-r7,pc}

