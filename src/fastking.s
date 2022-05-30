.global _king_kram_write_buffer
.global _king_kram_write_buffer32
.global _king_kram_write_buffer_memmap
.global _king_kram_write_buffer_memmap32
.global _king_kram_write_buffer_bitcopy
.global _king_kram_write_buffer_bytes

.global _king_kram_write_line32
.global _king_kram_write_line32_bytes

.macro	set_rrg	reg
	out.h	\reg, 0x600[r0]
.endm

.macro	set_reg reg, tmp
	movea	\reg, r0, \tmp
	set_rrg	\tmp
.endm


_king_kram_write_buffer:
	set_reg	0xE, r10
	add r6,r7
1:
	ld.w	0[r6],r8
	out.h	r8, 0x604[r0]
	shr 16, r8
	out.h	r8, 0x604[r0]

	ld.w	4[r6],r8
	out.h	r8, 0x604[r0]
	shr 16, r8
	out.h	r8, 0x604[r0]

	ld.w	8[r6],r8
	out.h	r8, 0x604[r0]
	shr 16, r8
	out.h	r8, 0x604[r0]

	ld.w	12[r6],r8
	out.h	r8, 0x604[r0]
	shr 16, r8
	out.h	r8, 0x604[r0]

	addi 16,r6,r6

	cmp	r7,r6
	bl	1b
	jmp	[lp]
	
_king_kram_write_buffer32:
	set_reg	0xE, r10
	add r6,r7
1:
	ld.w	0[r6],r8
	out.w	r8, 0x604[r0]

	ld.w	4[r6],r8
	out.w	r8, 0x604[r0]

	ld.w	8[r6],r8
	out.w	r8, 0x604[r0]

	ld.w	12[r6],r8
	out.w	r8, 0x604[r0]

	addi 16,r6,r6

	cmp	r7,r6
	bl	1b
	jmp	[lp]

_king_kram_write_buffer_memmap:
	set_reg	0xE, r10
	add r6,r7
	movhi 0xBC00,r0,r9
1:
	ld.w	0[r6],r8
	st.h	r8,0[r9]
	shr 16, r8
	st.h	r8,0[r9]

	ld.w	4[r6],r8
	st.h	r8,0[r9]
	shr 16, r8
	st.h	r8,0[r9]

	ld.w	8[r6],r8
	st.h	r8,0[r9]
	shr 16, r8
	st.h	r8,0[r9]

	ld.w	12[r6],r8
	st.h	r8,0[r9]
	shr 16, r8
	st.h	r8,0[r9]

	addi 16,r6,r6

	cmp	r7,r6
	bl	1b
	jmp	[lp]


_king_kram_write_buffer_memmap32:
	set_reg	0xE, r10
	add r6,r7
	movhi 0xBC00,r0,r9
1:
	ld.w	0[r6],r8
	st.w	r8,0[r9]

	ld.w	4[r6],r8
	st.w	r8,0[r9]

	ld.w	8[r6],r8
	st.w	r8,0[r9]

	ld.w	12[r6],r8
	st.w	r8,0[r9]

	addi 16,r6,r6

	cmp	r7,r6
	bl	1b
	jmp	[lp]


_king_kram_write_buffer_bitcopy:
	addi -32, sp, sp
	st.w r26, 0[sp]
	st.w r27, 4[sp]
	st.w r28, 8[sp]
	st.w r29, 12[sp]

	set_reg	0xE, r10
	movhi 0xBC00,r0,r29

	mov r6,r30
	shl 3,r7
	mov r7,r28
	mov r0,r26
	mov r0,r27
	movbsu

	ld.w 0[sp], r26
	ld.w 4[sp], r27
	ld.w 8[sp], r28
	ld.w 12[sp], r29
	addi 32, sp, sp

	jmp	[lp]


_king_kram_write_buffer_bytes:
	set_reg	0xE, r10
	add r6,r7
1:
	ld.w	0[r6],r11
	mov r11,r13
	shr 8,r13
	mov r11,r15
	andi 255,r13,r14
	shl 8,r15
	andi 65280,r13,r13
	shr 24,r11
	or r15,r14
	or r11,r13
	out.h	r14, 0x604[r0]
	out.h	r13, 0x604[r0]

	ld.w	4[r6],r11
	mov r11,r13
	shr 8,r13
	mov r11,r15
	andi 255,r13,r14
	shl 8,r15
	andi 65280,r13,r13
	shr 24,r11
	or r15,r14
	or r11,r13
	out.h	r14, 0x604[r0]
	out.h	r13, 0x604[r0]

	ld.w	8[r6],r11
	mov r11,r13
	shr 8,r13
	mov r11,r15
	andi 255,r13,r14
	shl 8,r15
	andi 65280,r13,r13
	shr 24,r11
	or r15,r14
	or r11,r13
	out.h	r14, 0x604[r0]
	out.h	r13, 0x604[r0]

	ld.w	12[r6],r11
	mov r11,r13
	shr 8,r13
	mov r11,r15
	andi 255,r13,r14
	shl 8,r15
	andi 65280,r13,r13
	shr 24,r11
	or r15,r14
	or r11,r13
	out.h	r14, 0x604[r0]
	out.h	r13, 0x604[r0]

	addi 16,r6,r6

	cmp	r7,r6
	bl	1b
	jmp	[lp]


_king_kram_write_line32:
	set_reg	0xE, r10
	add r6,r7
1:
	ld.w	0[r6],r8
	out.w	r8, 0x604[r0]

	addi 4,r6,r6
	cmp	r7,r6
	bl	1b
	jmp	[lp]

_king_kram_write_line32_bytes:
	set_reg	0xE, r10
	add r6,r7
1:
	ld.w	0[r6],r11
	mov r11,r13
	shr 8,r13
	mov r11,r15
	andi 255,r13,r14
	shl 8,r15
	andi 65280,r13,r13
	shr 24,r11
	or r15,r14
	or r11,r13
	out.h	r14, 0x604[r0]
	out.h	r13, 0x604[r0]

	addi 4,r6,r6
	cmp	r7,r6
	bl	1b
	jmp	[lp]
