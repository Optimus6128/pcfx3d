.global _mem_cpy
.global _mem_set

_mem_cpy:
	mov r6,r29
	mov r7,r30
	shl 3,r8
	mov r8,r28
	mov 0,r26
	mov 0,r27
	movbsu

	jmp	[lp]


_mem_set:
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

	jmp	[lp]
