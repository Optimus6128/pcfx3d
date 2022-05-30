#ifndef MY_ASM_FUNCS_H
#define MY_ASM_FUNCS_H

void king_kram_write_buffer(void* addr, int size);
void king_kram_write_buffer32(void* addr, int size);
void king_kram_write_buffer_memmap(void* addr, int size);
void king_kram_write_buffer_memmap32(void* addr, int size);
void king_kram_write_buffer_bitcopy(void* addr, int size);
void king_kram_write_buffer_bytes(void* addr, int size);

void king_kram_write_line32(void* addr, int size);
void king_kram_write_line32_bytes(void* addr, int size);

#endif
