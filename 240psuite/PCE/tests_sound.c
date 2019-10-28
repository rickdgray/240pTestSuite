 /*
 * 240p Test Suite
 * Copyright (C)2014-2019 Artemio Urbina (PC Engine/TurboGrafx-16)
 *
 * This file is part of the 240p Test Suite
 *
 * The 240p Test Suite is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * The 240p Test Suite is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with 240p Test Suite; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 
 
 This version of the suite is compiled with HuC from https://github.com/uli/huc
 
 */

#ifdef CDROM1 

#include "huc.h"
#include "res.h"
#include "font.h"
#include "video.h"
#include "tests.h"
#include "help.h"
#include "tools.h"

char palCD[512];

extern int xres_flags;
extern unsigned char Enabled240p;
extern unsigned char UseDefault;
extern unsigned char EnabledSoft;
extern unsigned char Enabled_C_BW;
#endif

#ifdef CDROM1
void main()
{
	RestoreGlobals();
	
	switch(ToolItem)
	{
		case TOOL_SOUND:
			SoundTest();
			break;
		case TOOL_AUDIOSYNC:
			AudioSyncTest();
			break;
		case TOOL_MDFOURIER:
			MDFourier();
			break;
		case TOOL_MANUAL:
			ManualLagTest();
			break;
	}
	cd_execoverlay(MAIN_OVERLAY);
}
#endif

const unsigned char *psg_ch = 0x800;
const unsigned char *psg_bal = 0x801;
const unsigned char *psg_freqlo = 0x802;
const unsigned char *psg_freqhi = 0x803;
const unsigned char *psg_ctrl = 0x804;
const unsigned char *psg_chbal = 0x805;
const unsigned char *psg_data = 0x806;
const unsigned char *psg_noise = 0x807;
const unsigned char *psg_lfofreq = 0x808;
const unsigned char *psg_lfoctrl = 0x809;

#define PULSE_TRAIN_FREQ 	13
#define PULSE_INTERNAL_FREQ	9
#define PULSE_SKIP_EMU		4


/*
http://ppmck.web.fc2.com/wavetable_js.html
*/

const unsigned char sine1x[32] = {	0x11, 0x14, 0x17, 0x1a, 0x1c, 0x1e, 0x1f, 0x1f,
									0x1f, 0x1f, 0x1e, 0x1c, 0x1a, 0x17, 0x14, 0x11,
									0x0e, 0x0b, 0x08, 0x05, 0x03, 0x01, 0x00, 0x00,
									0x00, 0x00, 0x01, 0x03, 0x05, 0x08, 0x0b, 0x0e };
/*									
const unsigned char sine2x[32] = {	0x13, 0x18, 0x1d, 0x1f, 0x1f, 0x1d, 0x18, 0x13, 
									0x0c, 0x07, 0x02, 0x00, 0x00, 0x02, 0x07, 0x0c, 
									0x13, 0x18,	0x1d, 0x1f, 0x1f, 0x1d, 0x18, 0x13,
									0x0c, 0x07, 0x02, 0x00, 0x00, 0x02, 0x07, 0x0c };
*/
	
const unsigned char sine4x[32] = {	0x16, 0x1e, 0x1e, 0x16, 0x09, 0x01, 0x01, 0x09, 
									0x16, 0x1e, 0x1e, 0x16, 0x09, 0x01, 0x01, 0x09,
									0x16, 0x1e, 0x1e, 0x16, 0x09, 0x01, 0x01, 0x09,
									0x16, 0x1e, 0x1e, 0x16, 0x09, 0x01, 0x01, 0x09 };
/*
const unsigned char sqstep[32] = { 	0x18, 0x1C, 0x1C, 0x1C,	0x00, 0x1C, 0x04, 0x1C,
									0x08, 0x1C, 0x0C, 0x1C,	0x10, 0x1C, 0x14, 0x1C,
									0x18, 0x1C, 0x1C, 0x1C,	0x00, 0x1C, 0x00, 0x1C,
									0x00, 0x1C, 0x00, 0x1C,	0x04, 0x1C, 0x08, 0x1C };				
			
const unsigned char square[32] = {	0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 
									0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 
									0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
									0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
*/
/*			
const unsigned char tiangle[32] = {	0x01, 0x03, 0x05, 0x07, 0x09, 0x0b, 0x0d, 0x0f, 
									0x11, 0x13, 0x15, 0x17, 0x19, 0x1b, 0x1d, 0x1f,
									0x1f, 0x1d, 0x1b, 0x19, 0x17, 0x15, 0x13, 0x11,
									0x0f, 0x0d, 0x0b, 0x09, 0x07, 0x05, 0x03, 0x01 };
			
const unsigned char saw[32] = {		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
									0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
									0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
									0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f };
			
const unsigned char sinesaw[32] = {	0x10, 0x13, 0x16, 0x18, 0x1B, 0x1D, 0x1E, 0x1F,
									0x1F, 0x1F, 0x1E, 0x1D, 0x1B, 0x18, 0x16, 0x13,
									0x01, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E,
									0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E };
*/

void LoadWave(unsigned char chan, unsigned char *wave)
{	
	__sei();
	*psg_ch = chan;
	*psg_ctrl = 0;
	for(i = 0; i < 32; i++)
	{
		*psg_data = *wave;
		wave++;
	}
	*psg_bal = 0xff;
	*psg_noise = 0;
	__cli();
}

void PlayLeft(unsigned char chan)
{
	__sei();
	*psg_ch = chan;
	*psg_chbal = 0xf0;
	*psg_ctrl = 0x9f;
	__cli();
}

void PlayRight(unsigned char chan)
{
	__sei();
	*psg_ch = chan;
	*psg_chbal = 0x0f;
	*psg_ctrl = 0x9f;
	__cli();
}


void PlayCenter(unsigned char chan)
{
	__sei();
	*psg_ch = chan;
	*psg_chbal = 0xff;
	*psg_ctrl = 0x9f;
	__cli();
}

void StopAudio(unsigned char chan)
{
	__sei();
	*psg_ch = chan;
	*psg_ctrl = 0;
	__cli();
}

void StopAllAudio()
{
	for(i = 0; i < 6; i++)
		StopAudio(i);
}


/*

                       3580000
 12 bit value = ---------------------
                 32 x frequency (Hz)
				 
12 as value -> 9322.91Hz
13 as value -> 8605.76Hz

2044 as value -> 54.73Hz
4    as value -> 27968.75Hz
*/


void SetWaveFreq(unsigned char chan, unsigned int freq)
{
	__sei();
	*psg_ch = chan;
	*psg_freqlo = freq & 0xff;
	*psg_freqhi = freq >> 8;
	__cli();
}

void SetNoiseFreq(unsigned int chan, unsigned int freq)
{
	__sei();
	*psg_ch = chan;
	*psg_noise = 0x80 | (freq & 0x1F)^0x1F;
	__cli();
}

void StopNoise(unsigned int chan)
{
	__sei();
	*psg_ch = chan;
	*psg_noise = 0;
	*psg_ctrl = 0;
	__cli();
}

void ExecutePulseTrain(unsigned int chann)
{
	//Sync
	
	SetWaveFreq(chann, PULSE_TRAIN_FREQ);
	for(i = 0; i < 10; i++)
	{
		PlayCenter(chann);
		vsync();
		StopAudio(chann);
		vsync();
	}
}

void ExecuteSilence()
{
	//Silence
	for(i = 0; i < 20; i++)
		vsync();
}

void PlayRampChannel(int chann)
{
	//54Hz to 22375Hz
	PlayCenter(chann);
	for(i = 2044; i > 4; i-=6)
	{
		SetWaveFreq(chann, i);
		vsync();
	}
	StopAudio(chann);
}

/*
void PlayBothRampChannel(int chann1, int chann2)
{
	//54Hz to 22375Hz
	PlayLeft(chann1);
	PlayRight(chann2);
	for(i = 2044; i > 4; i-=6)
	{
		SetWaveFreq(chann1, i);
		SetWaveFreq(chann2, i);
		vsync();
	}
	StopAudio(chann1);
	StopAudio(chann2);
}
*/

#define SetupToneCommand(command) SetWaveFreq(0, PULSE_INTERNAL_FREQ); PlayCenter(0); vsync(); StopAudio(0); command;

void MDFourierExecute()
{
	StopAllAudio();
	
#ifdef CDROM
	if(cd_status(0) != 0)
	  cd_pause();
	  
	if(ad_stat())
		ad_stop();
#endif

	vsync();
	ExecutePulseTrain(0);
	ExecuteSilence();
	
	PlayRampChannel(0);  // sine 1x
	PlayRampChannel(1);  // sine 4x
	
	PlayCenter(4);
	SetNoiseFreq(4, 0);
	for(i = 0; i < 200; i++)	
		vsync();
	StopNoise(4);
	
	ExecuteSilence();
	ExecutePulseTrain(0);
	
#ifdef CDROM
	
	// Wait PAUSE
	for(i = 0; i < 4; i++)	
		vsync();
		
	// ADPCM
	//
	SetupToneCommand(ad_play(0, 64000, 15, 0));
	
	//Wait for ADPCM to end
	for(i = 0; i < 280; i++)	
		vsync();
	SetupToneCommand(vsync());

	clock_reset();
	// CD-DA	
	x2 = clock_tt();
	SetupToneCommand(cd_playtrk(4, 5, CDPLAY_NORMAL));
	//cd_playtrk(4, 5, CDPLAY_NORMAL);
	x3 = clock_tt();
	
	put_string("Play", 4, 14);
	put_number(x3-x2, 4, 4, 15);
	
	//Wait for the cd audio track to end
	for(i = 0; i < 640; i++)	
		vsync();

	SetupToneCommand(cd_pause());
	
#endif

}

void MDFourier()
{
 	end = 0;
	redraw = 1;
	refresh = 1;
	
#ifdef CDROM
	ad_reset();
	
	ad_trans(ADPCM_SWEEP_OVL, 0, 32, 0);
#endif
	LoadWave(0, sine1x);
	LoadWave(1, sine4x);
	  
	/* 	Some emulators and FPGA implementations
		have issues with the first frame of the
		first sound that is played back
		Do so before the tests.
	*/
	
	SetWaveFreq(0, PULSE_SKIP_EMU);
	PlayCenter(0);
	vsync();
	StopAudio(0);
	
    while(!end)
    {   
		vsync();
		
        if(redraw)
        {
			RedrawBG();
         
            put_string("MDFourier", 16, 4);
			refresh = 1;
            redraw = 0;
			disp_on();
        }
		
		if(refresh)
		{
			set_font_pal(14);
            put_string("Start recording and press I", 6, 12);
			set_font_pal(13);
			put_string("Press START for HELP", 10, 26);
		}

        controller = joytrg(0);
		
		if (controller & JOY_RUN)
		{
			showHelp(MDFOURIER_HELP);
			redraw = 1;
		}
		
		if (controller & JOY_II)
			end = 1;
		
		if (controller & JOY_I)
		{
			set_font_pal(15);
            put_string("Please wait while recording", 6, 12);
			MDFourierExecute();
			//refresh = 1;
		}
    }
}

void SoundTest()
{
	option = 1;

#ifdef CDROM
	// type is x2 for CDROM
	x2 = 0;
#endif

	end = 0;
	redraw = 1;
	refresh = 0;
	i = 0;
	
#ifdef CDROM
	ad_reset();
	
	ad_trans(ADPCM_VOICE_OVL, 0, 5, 0);
#endif

	LoadWave(0, sine1x);
	SetWaveFreq(0, 112);
	
    while(!end)
    {   
		vsync();
        if(redraw)
		{
			RedrawBG();
			put_string("Sound Test", 15, 4);
			
            redraw = 0;
			refresh = 1;
			disp_on();
		}
		
		if(refresh)
		{
#ifndef CDROM
			set_font_pal(option == 0 ? 15 : 14);
            put_string("Left", 9, 14);
            set_font_pal(option == 1 ? 15 : 14);
            put_string("Center", 17, 16);
            set_font_pal(option == 2 ? 15 : 14);
            put_string("Right", 26, 14);
#else

			if(x2 == 0)
				set_font_pal(12);
			else
				set_font_pal(14);
			put_string("PSG", 18, 8);
			if(x2 == 1)
				set_font_pal(12);
			else
				set_font_pal(14);
			put_string("CDDA", 18, 18);
			if(x2 == 2)
				set_font_pal(12);
			else
				set_font_pal(14);
			put_string("ADPCM", 18, 20);
			
			if(x2 == 0)
			{
				set_font_pal(option == 0 ? 15 : 14);
				put_string("Left", 9, 10);
				set_font_pal(option == 1 ? 15 : 14);
				put_string("Center", 17, 12);
				set_font_pal(option == 2 ? 15 : 14);
				put_string("Right", 26, 10);
			}
			else
			{
				set_font_pal(14);
				put_string("Left", 9, 10);
				put_string("Center", 17, 12);
				put_string("Right", 26, 10);
			}
#endif
			refresh = 0;
		}

        controller = joytrg(0);
		
		if (controller & JOY_RUN)
		{
			StopAudio(0);
			showHelp(SOUND_HELP);
			redraw = 1;
		}
        
		if (controller & JOY_II)
			end = 1;
			
		if (controller & JOY_I)
		{
	#ifdef CDROM
			if(x2 == 0)
			{
	#endif
				switch(option)
				{
					case 0:
						PlayLeft(0);
						break;
					case 1:
						PlayCenter(0);
						break;
					case 2:
						PlayRight(0);
						break;
				}
				i = 20;
#ifdef CDROM
			}
	
			if(x2 == 1)
			{
				if(cd_status(0) == 0)
					cd_playtrk(3, 4, CDPLAY_NORMAL);
				else
				  cd_pause();
			}
			
			if(x2 == 2)
			{
				if(ad_stat())
					ad_stop();
				ad_play(0, 9595, 14, 0);
			}
	#endif
		}
		
	#ifdef CDROM
		if(x2 == 0)
		{
	#endif
		if (controller & JOY_LEFT)
		{
			option --;
			refresh = 1;
		}
			
		if (controller & JOY_RIGHT)
		{
			option ++;
			refresh = 1;
		}
	
		if(option < 0)
			option = 0;
		if(option > 2)
			option = 2;
			
#ifdef CDROM
		}
#endif
	
#ifdef CDROM		
		if (controller & JOY_UP)
		{
			x2 --;
			refresh = 1;
		}
			
		if (controller & JOY_DOWN)
		{
			x2 ++;
			refresh = 1;
		}
	
		if(x2 < 0)
			x2 = 2;
		if(x2 > 2)
			x2 = 0;
#endif
			
		if(i)
			i--;
			
		if(i == 1)
			StopAudio(0);
    }
	StopAllAudio(0);
	
#ifdef CDROM
	if(cd_status(0) != 0)
		cd_pause();
	if(ad_stat())
		ad_stop();
#endif
}


void AudioSyncTest()
{
	int status = -1;
	int acc = -1;
	
	y = 160;
	end = 0;
	redraw = 1;
	refresh = 0;
	
	LoadWave(0, sine1x);
	SetWaveFreq(0, 112);
    while(!end)
    {   
		vsync();
        if(redraw)
        {
			disp_off();
			ResetVideo();
			setupFont();

			SetFontColors(13, RGB(2, 2, 2), RGB(0, 6, 0), 0);
			
			for(x = 0; x < 16; x++)
				set_color(x, 0);
				
			set_color(2, RGB(7, 7, 7));
				
#ifndef CDROM1
			set_map_data(audiosync_map, 40, 32);
			set_tile_data(audiosync_bg);
			load_tile(0x1000);
			load_map(0, 0, 0, 0, 40, 32);
#else
			set_screen_size(SCR_SIZE_64x32); 
			cd_loadvram(GPHX_OVERLAY, OFS_audiosync_DATA_bin, 0x1000, SIZE_audiosync_DATA_bin);
			cd_loadvram(GPHX_OVERLAY, OFS_audiosync_BAT_bin, 0x0000, SIZE_audiosync_BAT_bin);
#endif

			init_satb();
			set_color_rgb(256, 0, 0, 0); 
			set_color_rgb(257, 7, 7, 7); 
#ifndef CDROM1		
			load_vram(0x5000, LED_sp, 0x100);
#else
			cd_loadvram(GPHX_OVERLAY, OFS_LEDsprites_tile_bin, 0x5000, SIZE_LEDsprites_tile_bin);
#endif
			spr_make(0, 160, y, 0x5000+0x40, FLIP_MAS|SIZE_MAS, NO_FLIP|SZ_16x16, 0, 1);
			satb_update();
			
			Center224in240();
			
            redraw = 0;
			disp_on();
		}

        controller = joytrg(0);
        
		if (controller & JOY_II)
			end = 1;
			
		if (controller & JOY_I)
		{
			refresh = !refresh;
			if(!refresh)
				status = 121;
			else
				y = 160;
		}
		
		if(refresh && status == -1)
		{
			status = 0;
			acc = -1;
		}
		
		if(status > -1)
		{
			status++;
			if(status <= 120)
			{
				y += acc;
				spr_set(0);				
				spr_x(160);
				spr_y(y);			
				satb_update();
			}
		}

		if(status >= 20 && status <= 120)
		{
			switch (status)
			{
			case 20:
				break;
			case 40:
				set_color(3, RGB(7, 7, 7));
				break;
			case 60:
				acc = 1;
				set_color(4, RGB(7, 7, 7));
				break;
			case 80:
				set_color(5, RGB(7, 7, 7));
				break;
			case 100:
				set_color(6, RGB(7, 7, 7));
				break;
			case 120:
				set_color(7, RGB(7, 7, 7));
				break;
			}
		}
		
		if(status == 120)
		{
			PlayCenter(0);
			set_color(0, RGB(7, 7, 7));
		}

		if(status == 122)
		{
			set_color(0, 0);
			
			for(x = 3; x < 8; x++)
				set_color(x, 0);

			StopAudio(0);
			status = -1;
		}
		
		if (controller & JOY_RUN)
		{
			StopAudio(0);
			showHelp(AUDIOSYNC_HELP);
			redraw = 1;
			refresh = 0;
			status = 121;
			y = 160;
		}
		
    }
	StopAudio(0);
}


void RedrawManualLagTest()
{
	ResetVideo();
	setupFont();
	SetFontColors(13, 0, RGB(0, 7, 0), 0);
	SetFontColors(14, 0, RGB(7, 7, 7), 0);
	SetFontColors(15, 0, RGB(7, 0, 0), 0);

#ifndef CDROM1			
	set_map_data(fs_map, 64, 32);
	set_tile_data(white_bg);
	load_tile(0x1000);
	load_map(0, 0, 0, 0, 64, 32);
	set_color_rgb(1, 0, 0, 0);   
#else
#endif
	ManualLagTestSprites();
	ManualLagTestText();
}

void ManualLagTestSprites()
{
	init_satb();
	set_color_rgb(256, 0, 0, 0); 
	set_color_rgb(257, 7, 7, 7); 
	set_color_rgb(273, 7, 0, 0); 
	set_color_rgb(289, 0, 7, 0);

#ifndef CDROM1		
	load_vram(0x5000, lagspr_sp, 0x100);
#else
	cd_loadvram(GPHX_OVERLAY, OFS_lagspr_tile_bin, 0x5000, SIZE_lagspr_tile_bin);
#endif
	spr_make(0, x, 300, 0x5000, FLIP_MAS|SIZE_MAS, NO_FLIP|SZ_32x32, 0, 1);
	spr_make(1, x, 300, 0x5000, FLIP_MAS|SIZE_MAS, NO_FLIP|SZ_32x32, 0, 1);
	spr_make(2, x,  96, 0x5000, FLIP_MAS|SIZE_MAS, NO_FLIP|SZ_32x32, 0, 2);
	
	satb_update();
}


void ManualLagTestText()
{
	set_font_pal(13);
	put_string("Press \"I\" when the sprite is aligned", 2, 21);
	put_string("with the background.", 3, 22);
	put_string("Negative values mean you pressed \"I\"", 2, 23);
	put_string("before they intersected.", 3, 24);
	put_string("SELECT toggles horizontal and vertical movement.", 2, 25);
	put_string("movement.", 3, 26);
	put_string("D-pad up toggles audio feedback.", 2, 27);
	put_string("D-pad down toggles rhythmic timing.", 2, 28);
		
	Center224in240();
}

void ManualLagTestResults()
{
	int total = 0;
	int totalms = 0;
	int val = 0;
	
	redraw = 1;
	end = 0;
	x = 0;
	
	while(!end)
	{   	
		vsync();

		if(redraw)
		{
			ManualLagTestResultsBack();
			
			set_font_pal(14);
			for(x2 = 0; x2 < 10; x2++)
			{
				val = clicks[x2];
				if(val != 0xFF)
				{
					put_number(val, 2, 10, 8+x2); 
					if(val >= 0)
					{
						total += val;
						x ++;
					}
				}
			}
			
			set_font_pal(15);
			put_string("+", 8, 14);
			put_string("----", 8, 18);
			
			totalms = total/x;
			
			set_font_pal(14);
			put_number(total, 7, 5, 19);
			put_string("/", 12, 19);
			put_number(x, 2, 13, 19);
			set_font_pal(15);
			put_string("=", 15, 19);
			set_font_pal(14);
			put_number(totalms, 2, 16, 19);
			put_string("frames", 19, 19);
			if(totalms == 1)
				put_string(" ", 24, 19);
			totalms = total/x*16;
			put_number(totalms, 2, 16, 20);
			put_string("milliseconds", 19, 20);
			
			set_font_pal(13);
			put_string("Keep in mind that a frame is", 6, 21);
			put_string("16.67 milliseconds.", 6, 22);
			
			if(total == 10)
			{
				x = 1;
				for(x2 = 0; x2 < 10; x2++)
				{
					if(clicks[x2] != 1)
						x = 0;
				}
				if(x)
					put_string("Smells like turbo...", 14, 13);
			}
			if(total < 5)
				put_string("EXCELLENT REFLEXES!", 14, 13);
			if(total == 0)
				put_string("INCREDIBLE REFLEXES!", 14, 13);
			
			redraw = 0;
			disp_on();
		}
		
		controller = joytrg(0);
		
		if (controller & JOY_II)
			end = 1;
	}
}

void ManualLagTestResultsBack()
{
	RedrawBG();
	SetFontColors(13, RGB(3, 3, 3), RGB(0, 7, 0), 0);
}

void ManualLagTest()
{
	int pos = 0;

	end = 0;

#ifndef CDROM1			
	showHelp(MANUALLAG_HELP);
#endif
	
	x = 0;
	y = 0;
	x2 = 0;
	y2 = 0;
	
	speed = 1;
	
	variation = 1;
	change = 1;
	audio = 1;
	view = 0;
	vary = 0;
	
	refresh = 0;
	redraw = 1;
	srand(clock_tt());
	
	for(x2 = 0; x2 < 10; x2++)
		clicks[x2] = 0xFF;
	
	x = 144;
	y = 60;
	x2 = 108;
	y2 = 96;
	
	LoadWave(0, sine1x);
	
    while(!end)
    {   
		vsync();
		
        if(redraw)
        {
			RedrawManualLagTest();
            redraw = 0;
			refresh = 1;
			disp_on();
        }
		
		if(refresh)
		{
			RefreshManualLagTest();
			refresh = 0;
		}
		
		if(audio) // n more that one frame with audio
			StopAudio(0);
				
		if(y == 96) // remove full screen flash
			set_color_rgb(1, 0, 0, 0);
		
		controller = joytrg(0);
		
		if (controller & JOY_I)
		{
			if(change)
			{
				clicks[pos] = (y - 96) *speed;
				
				if(audio && clicks[pos] != 0)
				{
					SetWaveFreq(0, 224);
					PlayCenter(0);
				}
	
				if(clicks[pos] >= 0)
				{
					change = 0;
					pos ++;
				}
		
				if(pos > 9)
					end = 1;
			}
		}
		
		if (controller & JOY_RUN)
		{
			showHelp(MANUALLAG_HELP);
			redraw = 1;
		}
        
		if (controller & JOY_II)
			end = 1;
			
		if (controller & JOY_SEL)
		{
			view ++;
			if(view > 2)
				view = 0;
			if(view == 0)
			{
				spr_set(1);
				spr_y(300);
			}
			if(view == 1)
			{
				spr_set(0);
				spr_y(300);
			}
		}
		
		if (controller & JOY_UP)
		{
			audio = !audio;
			refresh = 1;
		}
		
		if (controller & JOY_DOWN)
		{
			variation = !variation;
			refresh = 1;
			if(!variation)
				vary = 0;
		}
		
		ManualLagTestClickRefresh();
		
		if(y > 132 + vary)
		{
			speed = -1;
			change = 1;
			if(variation)
			{
				if(random(2))
					vary = random(7);
				else
					vary = -1 * random(7);
			}
		}

		if(y < 60 + vary)
		{
			speed = 1;
			change = 1;
			if(variation)
			{
				if(random(2))
					vary = random(7);
				else
					vary = -1 * random(7);
			}
		}
		
		y += speed;
		x2 += speed;
		
		if(view == 0 || view == 2)
		{
			spr_set(0);
			spr_x(x);
			spr_y(y);
		}
		
		if(view == 1 || view == 2)
		{
			spr_set(1);
			spr_x(x2);
			spr_y(y2);
		}
		
		if(y == 96)
		{			
			if(audio)
			{
				SetWaveFreq(0, 112);
				PlayCenter(0);
			}
			
			spr_set(0);
			spr_pal(1);
			
			spr_set(1);
			spr_pal(1);
			set_color_rgb(1, 7, 7, 7);   
		}
		else
		{
			if(y == 97 || y == 95) // one pixel off
			{
				//StopAudio(0);
				
				spr_set(0);
				spr_pal(2);
			
				spr_set(1);
				spr_pal(2);
			}

			if(y == 98 || y == 94) // two pixels off
			{
				spr_set(0);
				spr_pal(0);
			
				spr_set(1);
				spr_pal(0);
			}
		}
		
		satb_update();
    }
	
	StopAudio(0);
	
	if(pos > 9)
		ManualLagTestResults();
}

void RefreshManualLagTest()
{
	set_font_pal(13);
	put_string("Audio:", 25, 2);
	if(audio)
		put_string("on ", 32, 2); 
	else
		put_string("off", 32, 2);
		
	put_string("Timing:", 24, 3);
	if(variation)
		put_string("random  ", 32, 3); 
	else
		put_string("rhythmic", 32, 3);
}

void ManualLagTestClickRefresh()
{
	for(x3 = 0; x3 < 10; x3++)
	{
		if(clicks[x3] != 0xFF)
		{
			set_font_pal(14);
			put_string("Offset  :", 2, 2+x3);
			put_number(x3+1, 2, 8, 2+x3); 
			
			if(clicks[x3] >= 0)
			{
				if(clicks[x3] == 0)
					set_font_pal(13);
				else
					set_font_pal(14);
			}
			else
				set_font_pal(15);
			
			put_number(clicks[x3], 2, 11, 2+x3); 
			put_string("frames", 14, 2+x3);
			if(clicks[x3] == 1 || clicks[x3] == -1)
				put_string(" ", 19, 2+x3);
		}
	}
}