#include <stdlib.h>

#include "main.h"
#include "system.h"
#include "fastking.h"


volatile int __attribute__ ((zda)) zda_timer_count = 0;

/* Declare this "noinline" to ensure that my_timer_irq() is not a leaf. */
__attribute__ ((noinline)) void increment_zda_timer_count (void)
{
	zda_timer_count++;
}

/* Simple test interrupt_handler that is not a leaf. */
/* Because it is not a leaf function, it will use the full IRQ preamble. */
__attribute__ ((interrupt)) void my_timer_irq (void)
{
	eris_timer_ack_irq();

	increment_zda_timer_count();
}

void initTimer()
{
	// The PC-FX firmware leaves a lot of hardware actively generating
	// IRQs when a program starts, and it is only because the V810 has
	// interrupts-disabled that the firmware IRQ handlers are not run.
	//
	// You *must* mask/disable/reset the existing IRQ sources and init
	// new handlers before enabling the V810's interrupts!

	// Disable all interrupts before changing handlers.
	irq_set_mask(0x7F);

	// Replace firmware IRQ handlers for the Timer and HuC6270-A.
	//
	// This liberis function uses the V810's hardware IRQ numbering,
	// see FXGA_GA and FXGABOAD documents for more info ...
	irq_set_raw_handler(0x9, my_timer_irq);

	// Enable Timer interrupt.
	//
	// d6=Timer
	// d5=External
	// d4=KeyPad
	// d3=HuC6270-A
	// d2=HuC6272
	// d1=HuC6270-B
	// d0=HuC6273
	irq_set_mask(0x3F);

	// Reset and start the Timer.
	eris_timer_init();
	eris_timer_set_period(23864); /* approx 1/60th of a second */
	eris_timer_start(1);

	// Hmmm ... this needs to be cleared here for some reason ... there's
	// probably a bug to find somewhere!
	zda_timer_count = 0;

	// Allow all IRQs.
	//
	// This liberis function uses the V810's hardware IRQ numbering,
	// see FXGA_GA and FXGABOAD documents for more info ...
	irq_set_level(8);

	// Enable V810 CPU's interrupt handling.
	irq_enable();
}

int getFps()
{
	static int fps = 0;
	static int prev_sec = 0;
	static int prev_nframe = 0;

	const int curr_sec = zda_timer_count / 60;
	if (curr_sec != prev_sec) {
		fps = nframe - prev_nframe;
		prev_sec = curr_sec;
		prev_nframe = nframe;
	}
	return fps;
}

void vsync()
{
	while(eris_tetsu_get_raster() !=200) {};
}

Screen *initDisplay(int width, int height, int bpp)
{
	int i, kingBgMode = KING_BGMODE_256_PAL;
	u16 microprog[16];
	const int screenSize = (width * height * bpp) >> 3;

	Screen *screen = malloc(sizeof(Screen));
	screen->width = width;
	screen->height = height;
	screen->bpp = bpp;
	screen->data = malloc(screenSize);

	eris_king_init();
	eris_tetsu_init();

	eris_tetsu_set_priorities(0, 0, 1, 0, 0, 0, 0);
	eris_tetsu_set_king_palette(0, 0, 0, 0);
	eris_tetsu_set_rainbow_palette(0);

	if (bpp > 8) kingBgMode = KING_BGMODE_64K;
	eris_king_set_bg_prio(KING_BGPRIO_0, KING_BGPRIO_HIDE, KING_BGPRIO_HIDE, KING_BGPRIO_HIDE, 0);
	eris_king_set_bg_mode(kingBgMode, KING_BGPRIO_HIDE, KING_BGPRIO_HIDE, KING_BGPRIO_HIDE);
	eris_king_set_kram_pages(0, 0, 0, 0);

	for(i = 0; i < 16; i++) {
		microprog[i] = KING_CODE_NOP;
	}
	microprog[0] = KING_CODE_BG0_CG_0;
	microprog[1] = KING_CODE_BG0_CG_0;
	microprog[2] = KING_CODE_BG0_CG_2;
	microprog[3] = KING_CODE_BG0_CG_2;
	if (bpp > 8) {
		microprog[4] = KING_CODE_BG0_CG_4;
		microprog[5] = KING_CODE_BG0_CG_4;
		microprog[6] = KING_CODE_BG0_CG_6;
		microprog[7] = KING_CODE_BG0_CG_6;
	}

	eris_king_disable_microprogram();
	eris_king_write_microprogram(microprog, 0, 16);
	eris_king_enable_microprogram();

	eris_tetsu_set_video_mode(TETSU_LINES_262, 0, TETSU_DOTCLOCK_5MHz, TETSU_COLORS_16, TETSU_COLORS_16, 0, 0, 1, 0, 0, 0, 0);

	eris_king_set_bat_cg_addr(KING_BG0, 0, 0);
	eris_king_set_bat_cg_addr(KING_BG0SUB, 0, 0);
	eris_king_set_scroll(KING_BG0, 0, 0);
	eris_king_set_bg_size(KING_BG0, KING_BGSIZE_256, KING_BGSIZE_256, KING_BGSIZE_256, KING_BGSIZE_256);

	eris_king_set_kram_read(0, 1);
	eris_king_set_kram_write(0, 1);

	for(i = 0; i < screenSize; ++i) {
		eris_king_kram_write(0);
	}
	eris_king_set_kram_write(0, 1);

	return screen;
}

int displayMethod = 0;

/*void writeDisplay(Screen *screen)
{
	const int screenSize = (screen->width * screen->height * screen->bpp) >> 3;

	eris_king_set_kram_write(0, 1);
	
	if (screen->bpp==8) {
		king_kram_write_buffer_bytes(screen->data, screenSize);
	} else {
		switch(displayMethod) {
			case 0:
				king_kram_write_buffer(screen->data, screenSize);
			break;

			case 1:
				king_kram_write_buffer32(screen->data, screenSize);
			break;

			case 2:
				king_kram_write_buffer_memmap(screen->data, screenSize);
			break;

			case 3:
				king_kram_write_buffer_memmap32(screen->data, screenSize);
			break;

			case 4:
				king_kram_write_buffer_bitcopy(screen->data, screenSize);
			break;
		}
	}
}*/
