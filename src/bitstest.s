.global _mem_cpy
.global _mem_set

_mem_cpy:
	addi -32, sp, sp
	st.w r26, 0[sp]
	st.w r27, 4[sp]
	st.w r28, 8[sp]
	st.w r29, 12[sp]

	mov r6,r29
	mov r7,r30
	shl 3,r8
	mov r8,r28
	mov 0,r26
	mov 0,r27
	movbsu

	ld.w 0[sp], r26
	ld.w 4[sp], r27
	ld.w 8[sp], r28
	ld.w 12[sp], r29
	addi 32, sp, sp

	jmp	[lp]


_mem_set:
	addi -32, sp, sp
	st.w r26, 0[sp]
	st.w r27, 4[sp]
	st.w r28, 8[sp]
	st.w r29, 12[sp]

	mov r6,r30
	mov r6,r29
	add 4,r29

	mov r7,r11
	shl 8,r11
	or r7,r11
	mov r11,r12
	shl 16,r12
	or r11,r12
	st.w r12, 0[r30]

	add -4,r8
	shl 3,r8
	mov r8,r28
	mov 0,r26
	mov 0,r27
	movbsu

	ld.w 0[sp], r26
	ld.w 4[sp], r27
	ld.w 8[sp], r28
	ld.w 12[sp], r29
	addi 32, sp, sp

	jmp	[lp]
