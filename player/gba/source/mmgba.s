/****************************************************************************
 *                                                          __              *
 *                ____ ___  ____ __  ______ ___  ____  ____/ /              *
 *               / __ `__ \/ __ `/ |/ / __ `__ \/ __ \/ __  /               *
 *              / / / / / / /_/ />  </ / / / / / /_/ / /_/ /                *
 *             /_/ /_/ /_/\__,_/_/|_/_/ /_/ /_/\____/\__,_/                 *
 *                                                                          *
 *                             GBA Sound Test                               *
 *                                                                          *
 *         Copyright (c) 2008, Mukunda Johnson (mukunda@maxmod.org)         *
 *                                                                          *
 * Permission to use, copy, modify, and/or distribute this software for any *
 * purpose with or without fee is hereby granted, provided that the above   *
 * copyright notice and this permission notice appear in all copies.        *
 *                                                                          *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES *
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF         *
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR  *
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES   *
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN    *
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF  *
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.           *
 ****************************************************************************/

/****************************************************************************
 *
 * DEFINITIONS
 *
 ****************************************************************************/

// Screen Layout

/******************************
 *                  <------------ DMA0: bg gradient
 *                            *
 *     +----------------+     *
 *     |             <----------- BG2: window display
 *     |                |#    *
 *     |  Music:    00<---------- BG0+BG1: text + shadow
 *     |                |#    *
 *.....|  Effect:   00  |#....*
 *.....|                |#<------ BG3: window shadow
 *.....|  Tempo:  100%  |#....*
 *.....|                |#....*
 *.....|  Pitch:  100%  |#....*
 *:::::|                |#::::*
 *:::::|                |#::::*
 *:::::+----------------+#::::*
 *::::::##################::::*
 *::::::::::::::::::::::::::::*
 *############################*
 *############################*
 ******************************/

// Palette layout

/*********************************************************
 | background, 10 colors | text color | shadow | ... |
 ---------------------------------------------------------
 | x | x | window face | window border | ... |
 *******************************************/

.EQU	DISPCNT,	0x000
.EQU	DISPSTAT,	0x004
.EQU	IE,		0x200
.EQU	IME,		0x208
.EQU	WAITCNT,	0x204
.EQU	BG0CNT,		0x008
.EQU	BG0HOFS,	0x010
.EQU	BG0VOFS,	0x012
.EQU	BLDCNT,		0x050
.EQU	COLEY,		0x054
.equ	PALETTE,	0x5000000

.equ	DMA0SAD,	0x40000B0
.equ	KEYINPUT,	0x4000130

.equ	BG0MAP,		0x6001800
.equ	BG1MAP,		0x6002000
.equ	BG2MAP,		0x6002800
.equ	BG3MAP,		0x6003000

.equ	CpuSet,		0x0B
.equ	BitUnPack,	0x10
.equ	VBlankIntrWait,	0x05
.equ	Div,		0x06

.equ	SB_ADDR,	0x4000
.equ	SB_ADDR_RAM,	0x2000000+SB_ADDR
.equ	SB_ADDR_CART,	0x8000000+SB_ADDR
.EQU	MAX_RAMDATA,	(262144 - SB_ADDR)

.equ	TEXT_COLOR,	0x7FFF0000
.equ	WINDOW_COLOR,	0x7FFF5D60


.equ	KEY_A,		1
.equ	KEY_B,		2
.equ	KEY_SELECT,	3
.equ	KEY_START,	4
.equ	KEY_RIGHT,	5
.equ	KEY_LEFT,	6
.equ	KEY_UP,		7
.equ	KEY_DOWN,	8
.equ	KEY_R,		9
.equ	KEY_L,		10

.equ	BUTT_A,		0
.equ	BUTT_B,		1
.equ	BUTT_LEFT,	2
.equ	BUTT_RIGHT,	3

.equ	PRESS_TIMEOUT,	7
.equ	SCROLL_RATE,	4

/****************************************************************************
 *
 * MEMORY
 *
 ****************************************************************************/

.struct 0
v_keys_prev:	.space 2
v_music:	.space 2
v_effect:	.space 2
v_tempo:	.space 2
v_pitch:	.space 2
v_ntracks:	.space 2
v_neffects:	.space 2
v_uflags:	.space 1
v_paused:	.space 1

v_sel:		.space 1
v_ptime:	.space 1
v_press:	.space 1
v_butt:		.space 1
v_size:

.equ	UF_MUSIC, 1
.equ	UF_EFFECT, 2
.equ	UF_TEMPO, 4
.equ	UF_PITCH, 8

	.BSS
	.ALIGN

mixing_buffer:
	.space	1408
wave_buffer:
	.space	1408

mod_ch:
	.space	40*32
act_ch:
	.space	28*32
mix_ch:
	.space	24*32

bg_color_data:
	.space 160*2

effect_table:
	.space 2*1000

variables:
	.space v_size

	.TEXT
	.ARM
	.ALIGN
	.global main

//---------------------------------------------------------------------------
main:
//---------------------------------------------------------------------------
	
	mov	r4, #0x4000000		@ registers
	ldr	r0,=intr_main		@ set user IRQ vector
	str	r0, [r4, #-4]		
	mrs	r0, cpsr		@ enable interrupts (CPU)
	bic	r0, r0, #0x80		
	msr	cpsr, r0		
	mov	r0, #0			@ setup communications for BURSTBOOT BACKDOOR
	str	r0, [r4, #0x134]	@ RCNT (normal mode)
	ldr	r0,=0x5080
	str	r0, [r4, #0x128]	@ SIOCNT (normal mode)
	ldr	r0, msg_boot		@ burstboot signal
	str	r0, [r4, #0x120]	@ SIODATA32
	mov	r0, #0x0008
	str	r0, [r4, #0x4]
	mov	r0, #1			@ IME=true
	str	r0, [r4, #0x208]

	mov	r0, #0x81		@ IE=vblank|comms
	str	r0, [r4, #0x200]

	mov	r0, #0x8
	strh	r0, [r4, #DISPSTAT]	@ enable vblank interrupt

	add	r0, pc, #1		@ switch to THUMB
	bx	r0

.THUMB

/****************************************************************************
 * AgbMain()
 *
 * Program entry point
 ****************************************************************************/
AgbMain:
	ldr	r0,=0x4000000+WAITCNT	// setup default ROM timing
	ldr	r1,=0x4014		// 3,1 cycles, enable prefetch
	strh	r1, [r0]		//

	ldr	r1,=SB_ADDR_RAM-4	// setup maxmod
	ldmia	r1!, {r0}		// use RAM soundbank if it fits in memory
	cmp	r0, #0
	bne	.fits_in_ram		//
	ldr	r1,=SB_ADDR_CART	//
.fits_in_ram:				//
	ldr	r0,=sndsystem		//
	str	r1, [r0, #32]		//
	push	{r1}
	bl	mmInit			//

//----------------------------------------
// setup some stuff
//----------------------------------------

	ldr	r7,=variables
	

	pop	{r5}			// r0 = neffects (total)
	ldrh	r0, [r5]		//
	
	ldrh	r1, [r5, #2]		// copy ntracks
	strh	r1, [r7, #v_ntracks]	// 
	
	ldr	r1,=effect_table	// BUILD EFFECT TABLE
					// r0 = neffects
	mov	r2, #0			// r1 = effect_table pointer
					// r2 = effect counter

	mov	r6, r5
	
	add	r5, #12			// r5 = sound effect pointers
	
	cmp	r0, #0
	bne	1f
	mov	r1, #0
	b	.no_effects
1:
	
2:	ldmia	r5!, {r4}		// search for sound
	cmp	r4, #0			//
	beq	1f
	
	add	r4, r6			// get sample pointer
	ldrb	r4, [r4, #6]		//
	
	cmp	r4, #0			// check sp flag
	beq	1f			//
	
	strh	r2, [r1]		// write number to table
	add	r1, #2			//
1:	add	r2, #1
	sub	r0, #1
	bne	2b

	ldr	r0,=effect_table	// save effect count
	sub	r1, r0			//
	lsr	r1, #1			//
.no_effects:
	strh	r1, [r7, #v_neffects]	//

//----------------------------------------
// setup palette
//----------------------------------------

	ldr	r1,=PALETTE		// set window colors//
	ldr	r2,=WINDOW_COLOR	//
	str	r2, [r1, #2*2]		//
	ldr	r0,=TEXT_COLOR		// set text colors
	str	r0, [r1, #0*2]		//

//----------------------------------------
// generate bg gradient
//----------------------------------------

	ldr	r0,=31<<10
	ldr	r1,=1<<10
	ldr	r2,=bg_color_data
	mov	r3, #8
	mov	r4, #20
	
1:	strh	r0, [r2]
	add	r2, #2
	sub	r3, #1
	bne	1b
	sub	r0, r1
	mov	r3, #8
	sub	r4, #1
	bne	1b	

//----------------------------------------
// decompress font
//----------------------------------------
	
	ldr	r0,=fontTiles
	ldr	r1,=0x6000400
	ldr	r2,=unpack_tiles
	swi	BitUnPack
	
//----------------------------------------

	ldr	r0,=windowTiles
	ldr	r1,=0x6001000
	mov	r2, #288/2
	lsl	r2, #1
	swi	CpuSet

//----------------------------------------
// setup controls
//----------------------------------------

	ldr	r0,=bg_controls
	ldr	r1,=0x4000000+BG0CNT
	mov	r2, #12
	swi	CpuSet

//----------------------------------------
// draw window
//----------------------------------------

	ldr	r0,=BG2MAP
	mov	r1, #128

//---------------------------
// top
//---------------------------
	strh	r1, [r0]		// top left
	add	r0, #2
	
	add	r1, #1
	mov	r2, #16
	
1:	strh	r1, [r0]		// top
	add	r0, #2			//
	sub	r2, #1			//
	bne	1b			//

	ldr	r1,=128 + (1<<10)	// top right
	strh	r1, [r0]

//---------------------------
// middle
//---------------------------

	mov	r2, #11

2:	mov	r1, #128+2
	add	r0, #30
	strh	r1, [r0]		// left
	add	r0, #2
	add	r1, #1
	
	mov	r3, #16
1:	strh	r1, [r0]		// middle
	add	r0, #2
	sub	r3, #1
	bne	1b

	ldr	r1,=128+2 + (1<<10)
	strh	r1, [r0]		// right

	sub	r2, #1
	bne	2b

//---------------------------
// bottom
//---------------------------

	ldr	r1,=128 + (1<<11)
	add	r0, #30
	strh	r1, [r0]
	add	r1, #1
	add	r0, #2
	
	mov	r2, #16
1:	strh	r1, [r0]
	add	r0, #2
	sub	r2, #1
	bne	1b

	ldr	r1,=128 + (1<<10) + (1<<11)
	strh	r1, [r0]
	
//----------------------------------------
// copy to shadow
//----------------------------------------

	ldr	r1,=BG2MAP
	ldr	r0,=BG3MAP
	mov	r2, #1
	lsl	r2, #9
	ldr	r4,=0x10001000

1:	ldmia	r1!, {r3}
	add	r3, r4
	stmia	r0!, {r3}
	sub	r2, #1
	bne	1b

//----------------------------------------
// setup blending
//----------------------------------------
// blend bg1+bg3
// 50%,50%

	ldr	r0,=0x4000000+BLDCNT
	ldr	r1,=0x0808354A
	str	r1, [r0]

//----------------------------------------
// setup sprites
//----------------------------------------

	ldr	r0,=fingeTiles
	ldr	r1,=0x6010000
	mov	r2, #32/2
	swi	CpuSet
	
	ldr	r0,=fingePal
	ldr	r1,=0x5000200
	mov	r2, #16/2
	swi	CpuSet

	ldr	r0,=0x7000000		// disable all sprites
	mov	r1, #1			//
	lsl	r1, #9			//
	mov	r2, #128		//
					//
1:	stmia	r0!, {r1,r2}		//
	sub	r2, #1			//
	bne	1b			//

//----------------------------------------
// initialize information
//----------------------------------------


	ldr	r7,=variables

	mov	r0, #1
	lsl	r0, #10
	strh	r0, [r7, #v_tempo]
	strh	r0, [r7, #v_pitch]

	mov	r0, #2
	mov	r1, #2
	adr	r2, str_MUSIC
	bl	textString

	mov	r0, #2
	mov	r1, #4
	adr	r2, str_EFFECT
	bl	textString

	mov	r0, #2
	mov	r1, #6
	adr	r2, str_TEMPO
	bl	textString

	mov	r0, #2
	mov	r1, #8
	adr	r2, str_PITCH
	bl	textString
	
	mov	r1, #0xFF
	strb	r1, [r7, #v_uflags]

	bl	updateInfo

	bl	textUpdateShadow

	
	bl	updateFinge
	
	b	.enable_display
.align
str_MUSIC:
	.string "Music:    --"
.align
str_EFFECT:
	.string "Effect:   --"
.align
str_TEMPO:
	.string "Tempo:  100%"
.align
str_PITCH:
	.string "Pitch:  100%"

.align 2

//----------------------------------------
// enable display
//----------------------------------------
.enable_display:
	swi	VBlankIntrWait

	ldr	r0,=0x4000000+DISPCNT
	ldr	r1,=(1<<8) | (1<<9) | (1<<10) | (1<<11) | (1<<12)
	strh	r1, [r0]

//----------------------------------------
// main loop
//----------------------------------------

main_loop:

	swi	VBlankIntrWait
	
//----------------------------------------
// key input
//----------------------------------------
	
	ldr	r0,=KEYINPUT		// read keys [inversed bits]
	ldrh	r0, [r0]		//
	mvn	r5, r0			// 

	// keydown = 0->1 ((p AND k) XOR k)
	// keyheld = 1->1 (p AND k)
	// keyup   = 1->0 (k AND p) XOR p)

	// A  = 0
	// B   = 1
	// Left = 2
	// Right = 3

	ldrh	r4, [r7, #v_keys_prev]
	strh	r5, [r7, #v_keys_prev]
	
	mov	r6, r4			// r6 = keysdown
	and	r6, r5			//
	eor	r6, r5			//

	lsr	r0, r6, #KEY_UP		// up: move cursor up
	bcc	1f			//
	bl	selUp			//
1:
	lsr	r0, r6, #KEY_DOWN	// down: move cursor down
	bcc	1f			//
	bl	selDown			//
1:

	mov	r0, r6			// do keysdown actions
	ldr	r1,=selPress		//
	bl	keyActions		//

	mov	r0, r4			// do keysheld actions
	and	r0, r5			// (maybe change to just r0=r5 ?)
	ldr	r1,=selHold		//
	bl	keyActions		//
	
	mov	r0, r5			// do keysup actions
	and	r0, r4			//
	eor	r0, r4			//
	ldr	r1,=selRelease		//
	bl	keyActions		//

	bl	updateFinge

	bl	updateInfo

	ldrb	r0, [r7, #v_uflags]
	cmp	r0, #0
	beq	1f

	bl	textUpdateShadow

1:	mov	r0, #0
	strb	r0, [r7, #v_uflags]

	bl	mmFrame
	
	b	main_loop

.pool

/********************************************************************
 * keyActions( keys, function )
 *
 * keys = key bitmask
 * function = function to call
 ********************************************************************/
						.thumb_func
keyActions:
	push	{r4,r5,lr}
	mov	r4, r0
	mov	r5, r1

	lsr	r0, r4, #KEY_A			// test A
	bcc	1f				//
	mov	r0, #BUTT_A			//
	bl	.enter_key			//
1:
	lsr	r0, r4, #KEY_B			// test B
	bcc	1f				//
	mov	r0, #BUTT_B			//
	bl	.enter_key			//
1:
	lsr	r0, r4, #KEY_LEFT		// test Left
	bcc	1f				//
	mov	r0, #BUTT_LEFT			//
	bl	.enter_key			//
1:
	lsr	r0, r4, #KEY_RIGHT		// test right
	bcc	1f				//
	mov	r0, #BUTT_RIGHT			//
	bl	.enter_key			//
1:
	pop	{r4,r5,pc}

.enter_key:
	bx	r5

/********************************************************************
 * selPress(butt)
 * 
 * keysdown event
 ********************************************************************/
						.thumb_func
selPress:
	mov	r1, #1
	strb	r1, [r7, #v_press]
	strb	r0, [r7, #v_butt]
	mov	r1, #0				// action[pos]( button, 0 )
						//
jump_keyclick:
	ldrb	r2, [r7, #v_sel]		//
	lsl	r2, #1				//
	add	r2, pc				//
	mov	pc, r2				//
	//nop					//
	b	click_Music			//
	b	click_Effect			//
	b	click_Tempo			//
	b	click_Pitch			//

/********************************************************************
 * selHold(butt)
 * 
 * keysheld event
 ********************************************************************/
						.thumb_func
selHold:
	ldrb	r1, [r7, #v_press]
	cmp	r1, #0
	beq	1f
	ldrb	r1, [r7, #v_butt]
	cmp	r0, r1
	bne	1f
	mov	r1, #1
	
	b	jump_keyclick
1:	bx	lr

/********************************************************************
 * selRelease(butt)
 * 
 * keysup event
 ********************************************************************/
						.thumb_func
selRelease:

	ldrb	r1, [r7, #v_ptime]
	cmp	r1, #0
	bne	1f
	ldrb	r1, [r7, #v_butt]
	cmp	r0, r1
	bne	1f
	mov	r0, #0
	strb	r0, [r7, #v_press]
1:	bx	lr

/********************************************************************
 * selUp
 *
 * move cursor up
 ********************************************************************/
						.thumb_func
selUp:
	
	ldrb	r0, [r7, #v_sel]		// sel--
	sub	r0, #1				// wrap back to 3
	bpl	.selud_store			//
	mov	r0, #3				//

.selud_store:
	strh	r0, [r7, #v_sel]		//
	mov	r0, #0				// pressed=0
	strb	r0, [r7, #v_press]		//
	bx	lr

/********************************************************************
 * selDown
 *
 * move cursor down
 ********************************************************************/
						.thumb_func
selDown:

	ldrb	r0, [r7, #v_sel]		// sel++
	add	r0, #1				//
	cmp	r0, #3				// wrap back to 0
	ble	.selud_store			//
	mov	r0, #0				//
	b	.selud_store			//

/********************************************************************
 * clickMusicOrEffect(button,held,max,function_a,function_b,flag,value)
 ********************************************************************/
						.thumb_func
clickMusicOrEffect:

	cmp	r2, #0
	bpl	1f
	mov	r0, #0
	strb	r0, [r7, #v_ptime]
	strb	r0, [r7, #v_press]
2:	bx	lr
1:	
	cmp	r1, #1
	beq	2b
	
	mov	r1, #PRESS_TIMEOUT
	strb	r1, [r7, #v_ptime]
	
	cmp	r0, #1
	bgt	.cme_test01
	beq	.cme_b
.cme_a:
	mov	pc, r3
.cme_b:
	mov	pc, r4

.cme_test01:

	ldrh	r1, [r7, r6]

	cmp	r0, #3
	beq	.cme_right
.cme_left:
	sub	r1, #1
	bpl	.cme_set
	mov	r1, r2
	b	.cme_set
.cme_right:
	add	r1, #1
	cmp	r1, r2
	ble	.cme_set
	mov	r1, #0
.cme_set:
	strh	r1, [r7, r6]

	ldrb	r0, [r7, #v_uflags]
	orr	r0, r5
	strb	r0, [r7, #v_uflags]
	
	bx	lr

/********************************************************************
 * click_Music(button,held)
 *
 * click music event
 ********************************************************************/
						.thumb_func
click_Music:
	push	{r4-r6,lr}
	ldrh	r2, [r7, #v_ntracks]
	sub	r2, #1
	adr	r3, music_start
	adr	r4, music_stop
	mov	r5, #UF_MUSIC
	mov	r6, #v_music
	bl	clickMusicOrEffect
	pop	{r4-r6,pc}
.align
music_start:
	mov	r0, #0
	strb	r0, [r7, #v_paused]
	ldrh	r0, [r7, #v_music]
	mov	r1, #0		// <- loop
	ldr	r3,=mmStart
	bx	r3
.align
music_stop:
	mov	r0, #0
	strb	r0, [r7, #v_paused]
	ldr	r0,=mmStop
	bx	r0

/********************************************************************
 * click_Effect(button,held)
 *
 * click effect event
 ********************************************************************/
						.thumb_func
click_Effect:
	push	{r4-r6,lr}
	ldrh	r2, [r7, #v_neffects]
	sub	r2, #1
	adr	r3, effect_start
	ldr	r4,=mmEffectCancelAll
	mov	r5, #UF_EFFECT
	mov	r6, #v_effect
	bl	clickMusicOrEffect
	pop	{r4-r6,pc}
.align
effect_start:
	ldr	r1,=effect_table
	ldrh	r0, [r7, #v_effect]
	lsl	r0, #1
	ldr	r0, [r1, r0]
	ldr	r3,=mmEffect
	bx	r3

/***************************************************************
 * click tempo/pitch
 ***************************************************************/
.macro CLICK_VALUE value, flag, function

	push	{lr}

	cmp	r0, #BUTT_A			// ignore A
	bne	1f				//
	strb	r0, [r7, #v_press]		//
	strb	r0, [r7, #v_ptime]		//
3:	pop	{pc}
1:	
	ldrh	r2, [r7, #\value]
	cmp	r0, #BUTT_LEFT			// test B,left,right

	beq	1f				//
	bgt	2f				//
//------------------------------------------------
// B pressed
//------------------------------------------------
	cmp	r1, #0				// reset value
	bne	3b				//
	mov	r0, #PRESS_TIMEOUT
	strb	r0, [r7, #v_ptime]

	mov	r2, #1				//
	lsl	r2, #10				//
	b	4f				//

//------------------------------------------------
1: // Left pressed
//------------------------------------------------
	sub	r2, #SCROLL_RATE		// scroll down
	lsr	r0, r2, #9			//
	bne	3f				// clip to 512
	mov	r2, #1				//
	lsl	r2, #9				//
	b	3f				//

//------------------------------------------------
2: // Right pressed
//------------------------------------------------
	add	r2, #SCROLL_RATE		// scroll up
	lsr	r0, r2, #11			//
	beq	3f				// clip to 2048
	mov	r2, #1				//
	lsl	r2, #11				//

//------------------------------------------------
3: // store value
//------------------------------------------------

	mov	r0, #0
	strb	r0, [r7, #v_ptime]

4:
	strh	r2, [r7, #\value]
	
	ldrb	r0, [r7, #v_uflags]
	mov	r1, #\flag
	orr	r0, r1
	strb	r0, [r7, #v_uflags]

	mov	r0, r2
	bl	\function

	POP	{pc}
.endm

/********************************************************************
 * click_Tempo(button,held)
 *
 * click tempo event
 ********************************************************************/
						.thumb_func
click_Tempo:
	
	CLICK_VALUE v_tempo, UF_TEMPO, mmSetModuleTempo

/********************************************************************
 * click_Pitch(button,held)
 *
 * click pitch event
 ********************************************************************/
						.thumb_func
click_Pitch:
	
	CLICK_VALUE v_pitch, UF_PITCH, mmSetModulePitch

/********************************************************************
 * textString( x, y, string )
 *
 * render string
 ********************************************************************/
textString:
	
	lsl	r1, #5
	add	r0, r1
	lsl	r0, #1
	ldr	r1,=BG0MAP
	add	r1, r0
	b	2f

1:	strh	r0, [r1]
	add	r1, #2
2:	ldrb	r0, [r2]
	add	r2, #1
	cmp	r0, #0
	bne	1b

.end_string:
	bx	lr

/********************************************************************
 * textUpdateShadow()
 *
 * Update text shadow layer
 ********************************************************************/
textUpdateShadow:

	push	{r4}

	mov	r4, #1
	lsl	r4, #9
	
	ldr	r0,=BG0MAP
	ldr	r1,=BG1MAP

	ldr	r2,=0x10001000
	
1:	ldmia	r0!, {r3}
	add	r3, r2
	stmia	r1!, {r3}
	
	sub	r4, #1
	bne	1b

	pop	{r4}
	bx	lr

/********************************************************************
 * updateFinge
 *
 * Update finge sprite
 ********************************************************************/
						.thumb_func
updateFinge:

	ldrb	r0, [r7, #v_ptime]		// if timeout
	sub	r0, #1				//   timeout--
	bmi	1f				//   pressed=timeout
	strb	r0, [r7, #v_ptime]		//  
	cmp	r0, #0				//
	bne	1f				//
	strb	r0, [r7, #v_press]		//
1:

	ldr	r0,=0x7000000			// y = 48+sel*16
	mov	r1, #48				//
	ldrb	r2, [r7, #v_sel]		//
	lsl	r2, #4				//
	add	r1, r2				//
	strh	r1, [r0]			//

	ldr	r2,=(1<<10)+1			// shadow, +1 y, blend.
	add	r1, r2				//
	strh	r1, [r0, #8]			//
	
	mov	r1, #60				// x = 60 + pressed
	ldrb	r2, [r7, #v_press]		//
	cmp	r2, #0				//
	beq	1f				//
	add	r1, #1				//
1:	strh	r1, [r0, #2]			//
	
	add	r1, #1				// shadow
	strh	r1, [r0, #10]			//
	
	mov	r1, #0				// tile:0	
	strh	r1, [r0, #4]			//

	mov	r1, #1				// shadow
	lsl	r1, #12				//
	strh	r1, [r0, #12]			//

	bx	lr

/****************************************************************************
 * drawNumber(number,target)
 * drawNumberP(number,target)
 *
 * draw 2 to 3 digit number
 * P: convert 0->2048 into 0->100 (percent) 
 ****************************************************************************/
						.thumb_func
drawNumberP:
	mov	r2, #100
	mul	r0, r2
	add	r0, #255
	add	r0, #255
	lsr	r0, #10
						.thumb_func
drawNumber:
	mov	r2, r1
	mov	r1, #10
	swi	Div
	add	r1, #'0'
	strh	r1, [r2, #4]
	
	cmp	r0, #10
	blt	1f
	mov	r1, #10
	swi	Div
	add	r1, #'0'
	strh	r1, [r2, #2]
	add	r0, #'0'
	strh	r0, [r2, #0]
	b	.dnexit
1:	add	r0, #'0'
	strh	r0, [r2, #2]
	mov	r0, #0
	strh	r0, [r2]

.dnexit:
	bx	lr

/****************************************************************************
 * updateInfo()
 *
 * update onscreen information
 ****************************************************************************/
						.thumb_func
updateInfo:
	push	{r4-r7,lr}
	ldrb	r4, [r7, #v_uflags]

	lsr	r4, #1				// test&update Music
	bcc	1f				//
						//
	ldrh	r0, [r7, #v_ntracks]		//
	cmp	r0, #0				//
	beq	1f				//
						//
	ldrh	r0, [r7, #v_music]		//
	ldr	r1,=BG0MAP + 11*2 + 2*32*2	//
	bl	drawNumber			//
						//
1:						//

	lsr	r4, #1				// test&update Effect
	bcc	1f				//
						//
	ldrh	r0, [r7, #v_neffects]		//
	cmp	r0, #0				//
	beq	1f				//
						//
	ldrh	r0, [r7, #v_effect]		//
	ldr	r1,=BG0MAP + 11*2 + 4*32*2	//
	bl	drawNumber			//
						//
1:						//

	lsr	r4, #1				// test&update Tempo
	bcc	1f				//
						//
	ldrh	r0, [r7, #v_tempo]		// convert tempo into percent
	ldr	r1,=BG0MAP + 10*2 + 6*32*2	//
	bl	drawNumberP			//
						//
1:						//

	lsr	r4, #1				// test&update Pitch
	bcc	1f				//
						//
	ldrh	r0, [r7, #v_pitch]		//
	ldr	r1,=BG0MAP + 10*2 + 8*32*2	//
	bl	drawNumberP			//
						//
1:						//

	pop	{r4-r7,pc}

/****************************************************************************
 *
 * Interrupts
 *
 ****************************************************************************/

	.ALIGN
	.ARM

//---------------------------------------------------------------------------
intr_main:
//---------------------------------------------------------------------------

        mov     r0, #0x4000000		@ REGISTERS
        ldr     r1, [r0,#0x200]		@ IE IF
	and	r1, r1, r1, lsr#16	@ and together (occured&enabled)
	add	r2, r0, #0x200		@ acknowledge interrupts 
	strh	r1, [r2, #2]		@ (store to IF)

	ldrh	r2, [r0, #-8]		@ ORR to 0x3007FF8
	orr	r2, r2, r1		@ required for SWI 5
	strh	r2, [r0, #-8]		@ ..

        @and     r1, r3, r3, lsr #16	@ r1 = IE & IF

	tst	r1, #0x80		@ comm interrupt?
	bne	burst_entry

	ldr	r0,=DMA0SAD		@ reset bg DMA
	mov	r1, #0			@
	str	r1, [r0, #0x8]		@
	ldr	r1,=bg_color_data	@
	ldr	r2,=0x5000000		@
	stmia	r0!, {r1,r2}		@
	ldr	r1,=0xA2400001		@
	str	r1, [r0]		@

	ldr	r1,=31<<10		@ initialize first color
	strh	r1, [r2]		@
	
1:	ldr	r0,=mmVBlank		@ otherwise this interrupt is a vblank interrupt
	bx	r0			@ pass control to mmVBlank


/****************************************************************************
 * 
 * Burstboot Backdoor
 *
 ****************************************************************************/

burst_entry:
	ldr	r2,[r0,#0x120]
	ldr	r3, msg_brst	@ check for signal
	cmp	r2,r3
	beq	burst_boot
	bx	lr

@-------------------------------------------------------------------------------------
burst_boot:
@-------------------------------------------------------------------------------------

	mov	r2, #0
	str	r2, [r0, #0xC6]	@ disable hdma
	str	r2, [r0, #0xC4]	@ disable dma1
	str	r2, [r0, #0xD0]	@ disable dma2
	strh	r2, [r0, #0]	@ disable screen
	str	r2, [r0, #0x84]	@ disable sound
	
	@ receive loader from pc
	ldr	r4, msg_okay
	bl	sio_transfer	@ send okay signal, receive transfer length 
	mov	r2, r1		@
	ldr	r3,=__bss_start__	@ transfer program to iwram
	mov	r4, #0		@ crc counter...
.bb_transfer:
	bl	sio_transfer	@ transfer crc, receive word
	str	r1, [r3], #4	@ store word
	add	r4, r4, r1	@ add to crc
	subs	r2, r2, #4	@ count
	bhi	.bb_transfer	@ loop
	bl	sio_transfer	@ send CRC value to master..
	ldr	r3,=__bss_start__	@ 
	bx	r3		@ start transfer program
	
@--------------------------------------------------------------------------------------
sio_transfer:
@--------------------------------------------------------------------------------------

	@ r4 = data
	@ r0,=0x4000000
	@ returns r1=received data
	str	r4, [r0, #0x120]
	ldr	r1, [r0, #0x128]
	orr	r1, r1, #0x80
	str	r1, [r0, #0x128]
.st_wait:
	ldr	r1, [r0, #0x128]
	tst	r1, #0x80
	bne	.st_wait
	ldr	r1, [r0, #0x120]
	bx	lr

@------------------------------------
@ tables
@------------------------------------

.align
msg_boot:
	.byte	'B','O','O','T'

msg_okay:
	.byte	'O','K','A','Y'

msg_brst:
	.byte	'B','R','S','T'

@-----------------------------------
.align

sndsystem:
	.word	5		@ 21khz mixing
	.word	32		@ 32ch
	.word	32
	.word	mod_ch
	.word	act_ch
	.word	mix_ch
	.word	mixing_buffer
	.word	wave_buffer
	.word	0		@ soundbank, change during runtime
.align
unpack_tiles:
	.hword	768		// size
	.byte	1		// src width
	.byte	4		// dest width
	.word	0		// color


.align
bg_controls:
	.hword	0x0300		// cnt0
	.hword	0x0400		// cnt1	
	.hword	0x0500		// cnt2
	.hword	0x0600		// cnt3
	.hword	-56		// hofs0
	.hword	-32		// vofs0
	.hword	-57		// hofs1
	.hword	-33		// vofs1
	.hword	-48		// hofs2
	.hword	-24		// vofs2
	.hword	-54		// hofs3
	.hword	-30		// vofs3

.end
