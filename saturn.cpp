
#include "wl_def.h"
#include "sdl/SDL.h"
#include "sdl/SDL_mixer.h"
#include <stdlib.h>  /* malloc */
//#include <string.h>  /* memcpy */
#include <ctype.h>  /* toupper */
#include <malloc.h>
extern "C" {

//#include "L:\saturn\dev\sms_elf2\cdc\SEGA_CDC.H"
#include	"C:/SaturnOrbit/SGL_302j/INC/sl_def.h"
//#ifdef VBT
 #include "sega_cdc.h"
//#endif

}
//#define VBT

//

#ifdef VBT
int CdUnlock();
#define SYS_CDINIT1(i) \
((**(void(**)(int))0x60002dc)(i))

#define SYS_CDINIT2() \
((**(void(**)(void))0x600029c)())
#endif

extern "C" {
	extern void DMA_ScuInit(void);
	extern void SPR_InitSlaveSH(void);
	extern void DMA_ScuMemCopy(void *dst, void *src, Uint32 cnt);
	extern Uint32 DMA_ScuResult(void);
	extern void memcpywh(char *dst, char *src, short width, short height, short step);// , long srchstep, long srcwstep, char flag);
//	extern void memcpyl(void *dst, void *src, int size);
}
void SCU_DMAWait(void);

extern void CheckWeaponChange (void);
extern void ShapeTest (void);
//#define ACTION_REPLAY 1
#ifndef ACTION_REPLAY
Sint32 file_max;
GfsDirName dir_name[MAX_DIR];
#endif

   /*
#define SDL_FillRect(arg1, dest, arg3)	\
	slBMBoxFill((dest)->x, (dest)->y, (dest)->x + (dest)->w - 1, (dest)->y + (dest)->h - 1, 0)
	 */
void Pal2CRAM( Uint16 *Pal_Data , void *Col_Adr , Uint32 suu );
void InitCD();
void	InitCDBlock(void);
void ChangeDir(char *dirname);
int SDL_InitSubSystem(Uint32 flags);

 //#include <sddrvs.dat>
//extern Uint8 control_status ;
//extern volatile ScanCode	LastScan;
Sint32 GetFileSize(int file_id);
/*
 debug
*/
//Uint32 frame = 0;
//extern Image images[];
static unsigned char vbt_event[13][2];
static int current_event=0;
//int nb_event=0;
static Uint32 count=0;
Uint16 previouscount=0;
Uint16 multip=0;
PCM m_dat[4];

static	CdcPly	playdata;
static	CdcPos	posdata;

//Uint8 *lowram=(Uint8 *)0x00202000;

static Uint16 pad_asign[] = {
	PER_DGT_KU,
	PER_DGT_KD,
	PER_DGT_KR,
	PER_DGT_KL,
	PER_DGT_TA,
	PER_DGT_TB,
	PER_DGT_TC,
	PER_DGT_ST,
	PER_DGT_TX,
	PER_DGT_TY,
	PER_DGT_TZ,
	PER_DGT_TR,
	PER_DGT_TL,
};

static const Sint8	logtbl[] = {
/* 0 */		0, 
/* 1 */		1, 
/* 2 */		2, 2, 
/* 4 */		3, 3, 3, 3, 
/* 8 */		4, 4, 4, 4, 4, 4, 4, 4, 
/* 16 */	5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 
/* 32 */	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 
			6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 
/* 64 */	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
			7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
			7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
			7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
/* 128 */	8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
			8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
			8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
			8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
			8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
			8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
			8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
			8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
	};

/*
 dummy
 */
//SDL_Screen scr;
//SDL_Screen *screen = &scr;

//--------------------------------------------------------------------------------------------------------------------------------------
 SDL_Surface * SDL_SetVideoMode  (int width, int height, int bpp, Uint32 flags)
{
	unsigned char tv_mode;
	SDL_Surface *screen;
	screen = (SDL_Surface*)malloc(sizeof(SDL_Surface));
	CHECKMALLOCRESULT(screen);
	if(width==320)
	{
		tv_mode = TV_320x224;
		if(height==240) tv_mode = TV_320x240; 
	}

	if(width==352)
	{
		tv_mode = TV_352x224;
		if(height==240) tv_mode = TV_352x240; 
	}

	if(width==640)
	{
		tv_mode = TV_640x224;
		if(height==240) tv_mode = TV_640x240; 
	}
 
	slInitSystem(tv_mode, (TEXTURE*)NULL, 1);

	if(bpp==8)
	{
		slColRAMMode(CRM16_1024);
		slCharNbg1(COL_TYPE_256 , CHAR_SIZE_1x1);
	}
	else
	{
		slColRAMMode(CRM32_1024);
		slCharNbg1(COL_TYPE_32768 , CHAR_SIZE_1x1);
	}

	slScrPosNbg1(toFIXED(0) , toFIXED(0));
    slInitBitMap(bmNBG1, BM_512x256, (void *)NBG1_CEL_ADR);
    slBMPaletteNbg1(1);

//	slScrPosNbg0(toFIXED(0) , toFIXED(0));
//    slInitBitMap(bmNBG0, BM_512x256, (void *)NBG0_CEL_ADR);
//    slBMPaletteNbg0(1);
    // screen coordinates like in SDL
    slBitMapBase(0, 0);
    slScrAutoDisp(NBG0ON | NBG1ON);
	screen->pixels = (unsigned char*)malloc(sizeof(unsigned char)*width*height);
	CHECKMALLOCRESULT(screen->pixels);
	screen->pitch = width;
	//slTVOn();
//	memset((void *)NBG1_CEL_ADR,0x2222,0x20000);
	return screen;
}
//--------------------------------------------------------------------------------------------------------------------------------------
int SDL_SetColors(SDL_Surface *surface, 	SDL_Color *colors, int firstcolor, int ncolors)
{
	Uint16	palo[256];

	//palo = (Uint16*)malloc(sizeof(Uint16)*256);
	CHECKMALLOCRESULT(palo);
	unsigned int i;

	for(i=0;i<ncolors;i++)
	{
		palo[i] = RGB(colors[i].r>>3,colors[i].g>>3,colors[i].b>>3);
	}
	Pal2CRAM(palo , (void *)NBG1_COL_ADR , ncolors);
//	Pal2CRAM(palo , (void *)NBG0_COL_ADR , ncolors);
	//free(palo);
	return 1;
	//return 0;
}
//--------------------------------------------------------------------------------------------------------------------------------------
int SDL_SetColorKey	(SDL_Surface *surface, Uint32 flag, Uint32 key)
{
	return 0;
}
//--------------------------------------------------------------------------------------------------------------------------------------
static unsigned char initcddone=0;
int SDL_Init(Uint32 flags)
{
	//lowram=(Uint8 *)0x00202000;
#ifndef ACTION_REPLAY
	if(initcddone==0)
	{
		InitCD();
		InitCDBlock();
		initcddone=1;
	}
#endif
	DMA_ScuInit();
	SPR_InitSlaveSH();
	SDL_InitSubSystem(flags);
	return 0;
}
//--------------------------------------------------------------------------------------------------------------------------------------
int SDL_InitSubSystem(Uint32 flags)
{
	if(flags &= SDL_INIT_AUDIO)
	{
		char sound_map[] =  {0xff,0xff};//,0xff,0xff,0xff,0xff,0xff,0xff,0xff};

#ifdef ACTION_REPLAY
		slInitSound(sddrvstsk , sizeof(sddrvstsk) , (Uint8 *)sound_map , sizeof(sound_map)) ;
#else
#define	SDDRV_NAME	"SDDRVS.TSK"
#define	SDDRV_SIZE	26610 //0x7000
#define	SDDRV_ADDR	0x00202000//0x6080000
unsigned char *tutu;
		//slPrint("driver loaded",slLocate(1,1));
		tutu = (unsigned char *)malloc(SDDRV_SIZE);
		GFS_Load(GFS_NameToId((Sint8*)SDDRV_NAME),0,(void *) tutu,SDDRV_SIZE);
		slInitSound(tutu , SDDRV_SIZE , (Uint8 *)sound_map , sizeof(sound_map)) ;
		free(tutu);		
		
  		//slInitSound(sddrvstsk , sizeof(sddrvstsk) , (Uint8 *)sound_map , sizeof(sound_map)) ;

#endif
	}

	if(flags &= SDL_INIT_TIMER)
	{
		TIM_FRT_INIT(TIM_CKS_32);
		TIM_FRT_SET_16(0);
	}

	return 0;
}
//--------------------------------------------------------------------------------------------------------------------------------------
Uint8 SDL_EventState(Uint8 type, int state)
{
	return 0;
}
//--------------------------------------------------------------------------------------------------------------------------------------
/*
void SDL_WarpMouse(Uint16 x, Uint16 y)
{

}
//--------------------------------------------------------------------------------------------------------------------------------------
int SDL_JoystickNumHats(SDL_Joystick *joystick)
 {
	return 0;
}
//--------------------------------------------------------------------------------------------------------------------------------------
int SDL_NumJoysticks(void)
{
	return 0;
}
//--------------------------------------------------------------------------------------------------------------------------------------
int SDL_JoystickNumButtons(SDL_Joystick *joystick)
 {
	return 0;
}
//--------------------------------------------------------------------------------------------------------------------------------------
SDL_Joystick * SDL_JoystickOpen(int device_index)
 {

}
//--------------------------------------------------------------------------------------------------------------------------------------
void SDL_JoystickUpdate(void)
 {

}
//--------------------------------------------------------------------------------------------------------------------------------------
void SDL_JoystickClose(SDL_Joystick *joystick)
 {

}
*/
//--------------------------------------------------------------------------------------------------------------------------------------
SDLMod SDL_GetModState(void)
 {

}
//--------------------------------------------------------------------------------------------------------------------------------------
SDL_GrabMode SDL_WM_GrabInput(SDL_GrabMode mode)
 {

}
//--------------------------------------------------------------------------------------------------------------------------------------
/*
Uint8 SDL_JoystickGetButton(SDL_Joystick *joystick, int button)
 {
	return 0;
}
//--------------------------------------------------------------------------------------------------------------------------------------
Uint8 SDL_JoystickGetHat(SDL_Joystick *joystick, int hat)
 {
	return 0;
}
//--------------------------------------------------------------------------------------------------------------------------------------
Sint16 SDL_JoystickGetAxis(SDL_Joystick *joystick, int axis)
 {
	return 0;
}
//--------------------------------------------------------------------------------------------------------------------------------------
Uint8 SDL_GetMouseState(int *x, int *y)
{
	return 0;
}
*/
//--------------------------------------------------------------------------------------------------------------------------------------
Uint32 SDL_MapRGB (SDL_PixelFormat *format, Uint8 r, Uint8 g, Uint8 b)
{
	
	return 0;
}
//--------------------------------------------------------------------------------------------------------------------------------------
int SDL_LockSurface(SDL_Surface *surface)
{
	return 0;
}
//--------------------------------------------------------------------------------------------------------------------------------------
void SDL_UnlockSurface(SDL_Surface *surface)
{
// si on n'utilise pas de dma decommenter
//	slBMPut(0, 0, screenWidth-1, screenHeight-1, (Sint8*)surface->pixels); // vbt : 16-18fps
//memcpywh((char *)NBG1_CEL_ADR, (char *)surface->pixels, 320, 240, 512 - 320);//, 1, 0, 0); // vbt : 16-18fps
 // vbt : boucle lent 16-18fps
/*	unsigned int i,j;
	unsigned char *src = (unsigned char *)surface->pixels;
	unsigned char *dst = (unsigned char *)NBG1_CEL_ADR;
	unsigned int width = surface->w;
	unsigned int height = surface->h;

	for (i = 0;i < height ;i++ ) 
	{
		for (j = 0;j < width ;j++ ) 
		{
			*dst++ = *src;
			src +=1;
		}
		dst += (512-width);
	}
*/


	unsigned int i; // vbt : le plus rapide
	for (i = 0; i < screenHeight; i++) 
	{
//		DMA_ScuMemCopy((unsigned char*)(NBG1_CEL_ADR + (i<<9)), (unsigned char*)(surface->pixels + (i * screenWidth)), screenWidth); // vbt 20-22fps
//		SCU_DMAWait();
		memcpyl((unsigned long*)(NBG1_CEL_ADR + (i<<9)), (unsigned long*)(surface->pixels + (i * screenWidth)), screenWidth); // vbt : 22-24fps
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------
int SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained)
{
	Sint32		oct, shift_freq, fns;
	Uint8 i;
	Sint8 err=0;
	memcpy(obtained,desired,sizeof(SDL_AudioSpec));

	for (i=0; i<4; i++)
	{
		m_dat[i].mode = 0;

		if(desired->format==AUDIO_U8 || desired->format==AUDIO_S8)  m_dat[i].mode|=_PCM8Bit;
		if(desired->format==AUDIO_U16LSB || desired->format==AUDIO_S16LSB)  m_dat[i].mode|=_PCM16Bit;
		if(desired->channels==1) m_dat[i].mode|=_Mono;
		if(desired->channels==2) m_dat[i].mode|=_Stereo;

		oct			= PCM_CALC_OCT(desired->freq);
		shift_freq = PCM_CALC_SHIFT_FREQ(oct);
		fns			= PCM_CALC_FNS(desired->freq, shift_freq);

		m_dat[i].pitch =PCM_SET_PITCH_WORD(oct, fns);
		m_dat[i].level = 127;
		m_dat[i].channel = i*2;
		//m_dat[i].efselectR = i;
		/*
		if(!slPCMParmChange(&m_dat[i]))
		{
			err--;
		}	 */
		slSynch();
	}
	return err;
}
//--------------------------------------------------------------------------------------------------------------------------------------
int SDL_mutexP(SDL_mutex *mutex)
{
	return 0;
}
//--------------------------------------------------------------------------------------------------------------------------------------
int SDL_mutexV(SDL_mutex *mutex)
{
	return 0;
}
//--------------------------------------------------------------------------------------------------------------------------------------
void SDL_CloseAudio(void)
{
//	slPCMOff(&m_dat);
}
//--------------------------------------------------------------------------------------------------------------------------------------
void SDL_PauseAudio(int pause_on)
{
	Uint8 i;
	for (i=0;i<4;i++)
		slPCMOff(&m_dat[i]);
}
//--------------------------------------------------------------------------------------------------------------------------------------
SDL_mutex * SDL_CreateMutex(void)
{

}
//--------------------------------------------------------------------------------------------------------------------------------------
void SDL_FreeWAV(Uint8 *audio_buf)
{
	//free(audio_buf);
}
//--------------------------------------------------------------------------------------------------------------------------------------
SDL_AudioSpec * SDL_LoadWAV_RW(SDL_RWops *src, int freesrc, SDL_AudioSpec *spec, Uint8 **audio_buf, Uint32 *audio_len)
{
#ifndef ACTION_REPLAY
	Uint8 i, fileId;
	long fileSize;
    char directory[15];
	char filename[15];
	unsigned char *mem_buf;

	if(strlen(src->hidden.stdio.name)>5)
	{
		for(i=0;i<strlen(src->hidden.stdio.name);i++)
			if (src->hidden.stdio.name[i]=='/')	break;

		if(i<strlen(src->hidden.stdio.name) )
		{
			strncpy(directory,src->hidden.stdio.name,i);
			 directory[i]='\0';

			strcpy(filename,&src->hidden.stdio.name[i+1]);
		  
			i=0;
			while (directory[i])
			{
				directory[i]= toupper(directory[i]);
				i++;
			}			 
			ChangeDir(directory);

			//slPrint(directory,slLocate(1,1));
			//slPrint(filename,slLocate(1,2));
		}
	}
	else
		strcpy(filename,src->hidden.stdio.name);
	i=0;
	while (filename[i])
	{
		filename[i]= toupper(filename[i]);
		i++;
	}	  

	fileId = GFS_NameToId((Sint8*)filename);
	fileSize = GetFileSize(fileId);

	 if (fileSize<80000)
		mem_buf = (unsigned char*)malloc(fileSize)	;
	else if (fileSize<90000 && fileSize>=80000)
		mem_buf =  (Uint8 *)(0x00202000)	;
	else
		   mem_buf = (Uint8 *)(0x00232000);

	GFS_Load(fileId, 0, mem_buf, fileSize);
	audio_buf[0] = mem_buf;
	*audio_len = fileSize;
	if (fileSize<0x900)
	*audio_len = 0x900;
	ChangeDir(NULL);
		return spec;
#else

	 if(strcmp(src->hidden.stdio.name,"sounds/tune5.wav")==0)
	{
		audio_buf[0] = tune0;
		*audio_len = sizeof(tune0);
	}	  
#endif
  
	return spec;
}
//--------------------------------------------------------------------------------------------------------------------------------------
int SDL_UpperBlit (SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect)
{
	//slBMPut((dest) == NULL ? 0 : ((SDL_Rect *)dest)->x, (dest) == NULL ? 0 : ((SDL_Rect *)dest)->y, ((dest) == NULL ? 0 : ((SDL_Rect *)dest)->x) + images[img].w - 1, ((dest) == NULL ? 0 : ((SDL_Rect *)dest)->y) + images[img].h - 1, images[img].data);
  
if((srcrect)!=NULL)
	for( Sint16 i=0;i<srcrect->h;i++)
	{
//		Uint8*s = (Uint8*)src->pixels + ((i + srcrect->y) * src->pitch) + srcrect->x; 
//		Uint8*d = (Uint8*)dst->pixels + ((i + dstrect->y) * dst->pitch) + dstrect->x; 
//		memcpy(d,s,srcrect->w);
		memcpyl((unsigned long*)(dst->pixels + ((i + dstrect->y) * dst->pitch) + dstrect->x),(unsigned long*)(src->pixels + ((i + srcrect->y) * src->pitch) + srcrect->x),srcrect->w);
//		DMA_ScuMemCopy(d, s, srcrect->w);
//		SCU_DMAWait();
	}	
	
	//srcrect->h=SWAP_BYTES_16(srcrect->h);
	//srcrect->w=SWAP_BYTES_16(srcrect->w);
}
//--------------------------------------------------------------------------------------------------------------------------------------
void SDL_UpdateRects (SDL_Surface *screen, int numrects, SDL_Rect *rects)
{
//	slBMPut(0, 0, 320-1, 240-1, (Sint8*)screen->pixels);
//	   xxxxx
}
//--------------------------------------------------------------------------------------------------------------------------------------
void SDL_UpdateRect (SDL_Surface *screen, Sint32 x, Sint32 y, Uint32 w, Uint32 h)
{

}
//--------------------------------------------------------------------------------------------------------------------------------------

int SDL_FillRect (SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color)
{
/*
if((dst)!=NULL)		
	if((dstrect)!=NULL)
	slBMBoxFill((dst)->x, (dst)->y, (dst)->x + (dstrect)->w - 1, (dst)->y + (dstrect)->h - 1, color);
*/

if((dst)!=NULL)		
	if((dstrect)!=NULL)
	{
//		slBMBoxFill(dstrect->x, dstrect->y, dstrect->x + dstrect->w - 1, dstrect->y + dstrect->h - 1, color);

	for( Sint16 i=0;i<dstrect->h;i++)
		{
			//Uint8*s = (Uint8*)src->pixels + ((i + srcrect->y) * src->pitch) + srcrect->x; 
			Uint8*d = (Uint8*)dst->pixels + ((i + dstrect->y) * dst->pitch) + dstrect->x; 
//			memset(d,color,dstrect->w);
//
//		Uint8*d = (Uint8*)VDP2_VRAM_B1 + ((i + dstrect->y) * dst->pitch) + dstrect->x; 
		memset(d,color,dstrect->w);
//		DMA_ScuMemCopy(d, color, dstrect->w);
//		DMA_ScuResult();
//		SCU_DMAWait();
		}
	}
	else
	{
//		slBMBoxFill(dstrect->x, dstrect->y, dstrect->x + dstrect->w - 1, dstrect->y + dstrect->h - 1, color);

	   memset(dst->pixels,color,screenWidth*screenHeight);
//		DMA_ScuMemCopy((Uint8*)VDP2_VRAM_B1, color, dstrect->w);
//		DMA_ScuResult();
//		SCU_DMAWait();
	}

	return 0;
}

/*
  slBMBoxFill((dest)->x, (dest)->y, (dest)->x + (dest)->w - 1, (dest)->y + (dest)->h - 1, 0)
*/
//--------------------------------------------------------------------------------------------------------------------------------------
SDL_Surface * SDL_CreateRGBSurface(Uint32 flags, int width, int height, int depth, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask)
{
	SDL_Surface *screen;
	screen = (SDL_Surface*)malloc(sizeof(SDL_Surface));
	screen->pixels = (unsigned char*)malloc(sizeof(unsigned char)*width*height);
	CHECKMALLOCRESULT(screen->pixels);
	//screen->pixels = (unsigned char*)malloc(sizeof(unsigned char)*width*height);
	screen->pitch = width;
	screen->w     =	width;
	screen->h     =	height;
	return screen;
}
//--------------------------------------------------------------------------------------------------------------------------------------
void SDL_Quit(void)
{
	SYS_Exit(0);
}
//--------------------------------------------------------------------------------------------------------------------------------------
int SDL_SetPalette(SDL_Surface *surface, int flags, SDL_Color *colors, int firstcolor, int ncolors)
{
	return 0;
}
//--------------------------------------------------------------------------------------------------------------------------------------
char * SDL_GetError(void)
{

}
//--------------------------------------------------------------------------------------------------------------------------------------
const SDL_VideoInfo * SDL_GetVideoInfo(void)
{
		 SDL_VideoInfo *vidInfo;
		 vidInfo = (SDL_VideoInfo *)malloc(sizeof(SDL_VideoInfo));
		 vidInfo->vfmt = (SDL_PixelFormat*)malloc(sizeof(SDL_PixelFormat));
		 vidInfo->vfmt->BitsPerPixel = 8;
		 return vidInfo;
}
//--------------------------------------------------------------------------------------------------------------------------------------
SDL_Rect ** SDL_ListModes(SDL_PixelFormat *format, Uint32 flags)
{
	return (SDL_Rect **)-1;
}
//--------------------------------------------------------------------------------------------------------------------------------------
void SDL_WM_SetCaption(const char *title, const char *icon)
{

}
//--------------------------------------------------------------------------------------------------------------------------------------
void SDL_WM_GetCaption(char **title, char **icon)
{

}
//--------------------------------------------------------------------------------------------------------------------------------------
SDL_Surface * SDL_CreateRGBSurfaceFrom(void *pixels,
			int width, int height, int depth, int pitch,
			Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask)
{

}
//--------------------------------------------------------------------------------------------------------------------------------------
void SDL_WM_SetIcon(SDL_Surface *icon, Uint8 *mask)
{

}
/*
<vbt> :) looks easy but hard for me
<MartinMan> hmm
<MartinMan> ah
<vbt> for me at first poll I can add multiple events
<MartinMan> no, you just need to return 1 at a time
<vbt> then return the first event not executed
<MartinMan> First, make a list of all supported buttons/directions
<MartinMan> then you assign a number (starting from 0) to each of them
<MartinMan> then when poll() is called, you increase the number until you find a changed button/direction
<MartinMan> and if no more left, return false
<MartinMan> of course you need to store and update a joypad state variable to look for changed buttons/directions
*/



// boucle si UP+RIGHT ou UP+LEFT

/*
sc_1, sc_2, sc_3, sc_4
*/

int SDL_PollEvent(SDL_Event *event)
{
	Uint16 push = 0, data = 0;
	Uint8 i,found=0;
 				 //LastScan=0;
	//event->type = SDL_NOEVENT;
	//event->key.keysym.sym = SDLK_FIRST;	  

	if(Per_Connect1) {
		push = ~Smpc_Peripheral[0].push;
		data = ~Smpc_Peripheral[0].data;
	}
/*	if(Per_Connect2) {
		push |= ~Smpc_Peripheral[15].push;
		data |= ~Smpc_Peripheral[15].data;
	}		*/
	/*
	 quit?
	*/
			
	if(data & PER_DGT_ST && data & PER_DGT_TA && data & PER_DGT_TB && data & PER_DGT_TC) 
	{
		event->type = SDL_QUIT;
		return 1;
	}

	for (i=0;i <13; i++)
	{
		if(data & pad_asign[i])
		{
			if(vbt_event[i][0]!=SDL_KEYDOWN)
			{
				vbt_event[i][0]=SDL_KEYDOWN;
				vbt_event[i][1]=1;
			}
		}
		else/* if(push & pad_asign[i])		 */
		{
			
			if(vbt_event[i][0]==SDL_KEYDOWN)
			{
				vbt_event[i][0]=SDL_KEYUP;
				vbt_event[i][1]=1;
				 //control_status =0;
				 //LastScan=0;
			}/*		  
			else	 if(vbt_event[i][0]==SDL_KEYUP)
			{
				vbt_event[i][0]=0;
				vbt_event[i][1]=0;
				 //LastScan=0;
			}			*/
			else
			{
			  	vbt_event[i][1]=0;				 
				vbt_event[i][0]=0;
				 //LastScan=0;
			}
		}
	}	  
	for (i=0;i <13; i++)
	{
		if (vbt_event[i][1]==1)
		{
			//slPrintHex(i,slLocate(3,21));
			current_event=i;
			found=1;
			break;
		}
	}

/*
	PER_DGT_KU,
	PER_DGT_KD,
	PER_DGT_KR,
	PER_DGT_KL,
	PER_DGT_TA,
	PER_DGT_TB,
	PER_DGT_TC,
	*/

	
	if(data & pad_asign[0])
		if(! (data & pad_asign[2]))
		Keyboard[sc_RightArrow]=0;//vbt_event[2][1]=0;

	if(data & pad_asign[0])
		if(! (data & pad_asign[3]))
		Keyboard[sc_LeftArrow]=0;//vbt_event[3][1]=0;

	if(data & pad_asign[1])
		if(! (data & pad_asign[2]))
		Keyboard[sc_RightArrow]=0;//vbt_event[2][1]=0;

	if(data & pad_asign[1])
		if(! (data & pad_asign[3]))
		Keyboard[sc_LeftArrow]=0;//vbt_event[3][1]=0;

	if(data & pad_asign[2])
		Keyboard[sc_RightArrow]=1;

	if(data & pad_asign[3])
		Keyboard[sc_LeftArrow]=1;
	//IN_ClearKeysDown();

	if(found)
	{
		/*char toto[500];
		sprintf(toto,"evt %d action %d key %d",vbt_event[current_event][0],vbt_event[current_event][1],event->key.keysym.sym );
				slPrint(toto,slLocate(3,25));	 */
		event->type = 	 vbt_event[current_event][0];
		if(vbt_event[current_event][0]==SDL_KEYUP)
		{
			vbt_event[current_event][0] = SDL_NOEVENT;
			vbt_event[i][1]=0;
		}
		//event->type = SDL_KEYDOWN;

		switch(current_event)
		{
			case 4:/*PER_DGT_TA: */
			event->key.keysym.sym = SDLK_KP_ENTER;
			break;
			
			case 5:/*PER_DGT_TB: */
			event->key.keysym.sym = SDLK_SPACE;
			break;	
			
			case 6:/*PER_DGT_TC: */
			event->key.keysym.sym = SDLK_RCTRL;
			break;	

			case 8:/*PER_DGT_TX: */
			event->key.keysym.sym = SDLK_RALT;
			break;
			
			case 9:/*PER_DGT_TY: */
			event->key.keysym.sym = SDLK_RSHIFT;
			break;	

			case 10:/*PER_DGT_TZ: */
			event->key.keysym.sym = SDLK_RCTRL;
			break;	

			case 11:/*PER_DGT_TL: */
			//event->key.keysym.sym = 
			buttonstate[bt_prevweapon] = true;
			CheckWeaponChange ();
			break;	

			case 12:/*PER_DGT_TR: */
			//event->key.keysym.sym = 
			buttonstate[bt_nextweapon] = true;
			CheckWeaponChange ();
			break;	

			case 7:/*PER_DGT_ST: */
				//slPrint("gros connard",slLocate(3,24));
			event->key.keysym.sym = SDLK_ESCAPE;
			break;	

			case 3:/*PER_DGT_KL: */
				event->key.keysym.sym = SDLK_KP4;
			break;	

			case 2:/*PER_DGT_KR: */
				event->key.keysym.sym = SDLK_KP6;
			break;	

			case 1:/*PER_DGT_KD: */
				//slPrint("gros ggggg",slLocate(3,20));
			event->key.keysym.sym = SDLK_KP2;
			break;	

			case 0:/*PER_DGT_KU: */
				//slPrint("gros ggggg",slLocate(3,20));
			event->key.keysym.sym = SDLK_KP8;
			break;	

			default:
				//slPrint("pas trouv�",slLocate(3,20));
				//event->key.keysym.sym =999;
				//event->type = SDL_NOEVENT;
				event->key.keysym.sym = SDLK_LAST;//SDLK_FIRST;	  
				break;
		}
	//slSynch();
	//while(1);
		return 1;
	}
	//else
	//{
	IN_ClearKeysDown();
			event->type = SDL_KEYUP;
			//vbt_event[current_event][0] = SDL_NOEVENT;
			//vbt_event[i][1]=0;
			//event->key.keysym.sym = SDLK_LAST;	
	//}
	for (i=0;i <13; i++)
	{
	//	if (vbt_event[i][1]==1)
		{
				vbt_event[i][0]=0;
				vbt_event[i][1]=0;
		}
	}
	//control_status =0;
	//LastScan=0;
	//event->type = SDL_KEYUP;
	//event->key.keysym.sym = SDLK_LAST;	  
	//slSynch();
	return 0;
}
//--------------------------------------------------------------------------------------------------------------------------------------
int SDL_WaitEvent(SDL_Event *event)
{
	//event->type = SDL_NOEVENT;
#if 0
	if (event->type== SDL_KEYUP) {
		event->type = SDL_NOEVENT;
		event->key.keysym.sym = SDLK_FIRST;	
		return 1;
	}
 
	do {	
		 /*
		 take pressed buttons of both pads, to enable two player
		 */
		Uint16 push = 0, data = 0;
		if(Per_Connect1) {
			push = ~Smpc_Peripheral[0].push;
			data = ~Smpc_Peripheral[0].data;
		}
		if(Per_Connect2) {
			push |= ~Smpc_Peripheral[15].push;
			data |= ~Smpc_Peripheral[15].data;
		}
		/*
		 quit?
		*/
		if(data & PER_DGT_ST && data & PER_DGT_TA && data & PER_DGT_TB && data & PER_DGT_TC) {
			event->type = SDL_QUIT;
			return 1;
		}		
		/*
			move cursor
		*/
/*		if(data & PER_DGT_ST) {
			event->type = SDL_KEYDOWN;
			event->key.keysym.sym = SDLK_SPACE;
			return;
		}

		if(!(data & PER_DGT_ST) 
			&& event->key.keysym.sym == SDLK_SPACE
			&& event->type                     == SDL_KEYDOWN) {
			event->type = SDL_KEYUP;
			return;
		}
  */
		if(data & PER_DGT_TA) {
			event->type = SDL_KEYDOWN;
			event->key.keysym.sym = SDLK_KP_ENTER;
			return 1;
		}
		
		if(!(data & PER_DGT_TA) 
			&& event->key.keysym.sym == SDLK_KP_ENTER
			&& event->type                     == SDL_KEYDOWN) {
			event->type = SDL_KEYUP;
			return 1;
		}

		if(data & PER_DGT_TB) {
			event->type = SDL_KEYDOWN;
			event->key.keysym.sym = SDLK_e;
			return 1;
		}		
		
		if(!(data & PER_DGT_TB) 
			&& event->key.keysym.sym == SDLK_e
			&& event->type                     == SDL_KEYDOWN) {
			event->type = SDL_KEYUP;
			return 1;
		}

		if(data & PER_DGT_TC) {
			event->type = SDL_KEYDOWN;
			event->key.keysym.sym = SDLK_p;
			return 1;
		}
		
		if(!(data & PER_DGT_TC) 
			&& event->key.keysym.sym == SDLK_p
			&& event->type                     == SDL_KEYDOWN) {
			event->type = SDL_KEYUP;
			return 1;
		}

		if(data & PER_DGT_KL) {
			event->type = SDL_KEYDOWN;
			event->key.keysym.sym = SDLK_z;
			return 1;
		}
		
		if(!(data & PER_DGT_KL) 
			&& event->key.keysym.sym == SDLK_z
			&& event->type                     == SDL_KEYDOWN) {
			event->type = SDL_KEYUP;
			return 1;
		}

		if(data & PER_DGT_KR) {
			event->type = SDL_KEYDOWN;
			event->key.keysym.sym = SDLK_x;
			return 1;
		}
		
		if(!(data & PER_DGT_KR) 
			&& event->key.keysym.sym == SDLK_x
			&& event->type                     == SDL_KEYDOWN) {
			event->type = SDL_KEYUP;
			return 1;
		}

		if(data & PER_DGT_KU) {
			event->type = SDL_KEYDOWN;
			event->key.keysym.sym = SDLK_o;
			return 1;
		}
		
		if(!(data & PER_DGT_KU) 
			&& event->key.keysym.sym == SDLK_o
			&& event->type                     == SDL_KEYDOWN) {
			event->type = SDL_KEYUP;
			return 1;
		}

		if(data & PER_DGT_KD) {
			event->type = SDL_KEYDOWN;
			event->key.keysym.sym = SDLK_k;
			return 1;
		}
		
		if(!(data & PER_DGT_KD) 
			&& event->key.keysym.sym == SDLK_k
			&& event->type                     == SDL_KEYDOWN) {
			event->type = SDL_KEYUP;
			return 1;
		}

		//slSynch();
		
	} while(event->type == SDL_NOEVENT);
#endif
}
//--------------------------------------------------------------------------------------------------------------------------------------
void SDL_DestroyMutex(SDL_mutex *mutex)
{

}
//--------------------------------------------------------------------------------------------------------------------------------------
void Mix_FreeChunk(Mix_Chunk *chunk)
{

}
//--------------------------------------------------------------------------------------------------------------------------------------
Mix_Chunk *Mix_LoadWAV_RW(SDL_RWops *src, int freesrc)
{

}
//--------------------------------------------------------------------------------------------------------------------------------------
int Mix_PlayChannel (int channel, Mix_Chunk *chunk, int loops)
{
	unsigned int i = 0;
	//slPrintHex(chunk->alen,slLocate(2,10));
	//slPrintHex(chunk->alen,slLocate(2,11));
//	slPCMOn(sounds[chunk].pcm, sounds[chunk].data, sounds[chunk].size);
	//slPCMOff(&m_dat[0]);
	//slSynch();
	if(chunk->alen>0 && chunk->alen <100000)
	for(i=0;i<4;i++)
	{
		if(!slPCMStat(&m_dat[i]))
		{
			//slPCMOff(&m_dat[i]);
			//slPCMParmChange(&m_dat[i]);
			//slSndFlush() ;
			//slSynch();
			slPCMOn(&m_dat[i],chunk->abuf,chunk->alen);
			slSynch();
				break;
		}		 
	}
/*
		slPCMOn(&m_dat[0],chunk->abuf,chunk->alen);
	*/
	return 0;
}
//--------------------------------------------------------------------------------------------------------------------------------------
int Mix_OpenAudio(int frequency, Uint16 format, int channels, int chunksize)
{
	return 0;
}
//--------------------------------------------------------------------------------------------------------------------------------------
void Mix_ChannelFinished(void (*channel_finished)(int channel))
{

}
//--------------------------------------------------------------------------------------------------------------------------------------
int Mix_SetPanning(int channel, Uint8 left, Uint8 right)
{
	return 0;
}
//--------------------------------------------------------------------------------------------------------------------------------------
int Mix_GroupAvailable(int tag)
{
	return 0;
}
//--------------------------------------------------------------------------------------------------------------------------------------
int Mix_ReserveChannels(int num)
{
	return 0;
}
//--------------------------------------------------------------------------------------------------------------------------------------
int Mix_GroupOldest(int tag)
{
	return 0;
}
//--------------------------------------------------------------------------------------------------------------------------------------
int Mix_GroupChannels(int from, int to, int tag)
{
	return 0;
}
//--------------------------------------------------------------------------------------------------------------------------------------
void Mix_HookMusic(void (*mix_func)(void *udata, Uint8 *stream, int len),void *arg)
{

}
//--------------------------------------------------------------------------------------------------------------------------------------
char *Mix_GetError()
{

}
//--------------------------------------------------------------------------------------------------------------------------------------
int Mix_HaltChannel(int channel)
{
	return 0;
}
//--------------------------------------------------------------------------------------------------------------------------------------
int SDL_SaveBMP_RW (SDL_Surface *surface, SDL_RWops *dst, int freedst)
{
	return 0;
}
//--------------------------------------------------------------------------------------------------------------------------------------
SDL_RWops *SDL_RWFromMem(void *mem, int size)
{

}
//--------------------------------------------------------------------------------------------------------------------------------------
SDL_RWops *SDL_RWFromFile(const char *file, const char *mode)
{

}
//--------------------------------------------------------------------------------------------------------------------------------------
Uint32 SDL_GetTicks(void)
{
	previouscount += TIM_FRT_CNT_TO_MCR(TIM_FRT_GET_16()) / 1000; 
	TIM_FRT_SET_16(0); 
    //set_imask(imask);
	//
	return previouscount;
}


//--------------------------------------------------------------------------------------------------------------------------------------
void SDL_Delay(long delay)
{
/*	Uint16 cnt_delay;
	Uint16 count1 = TIM_FRT_GET_16();
	cnt_delay= TIM_FRT_MCR_TO_CNT(delay);
	TIM_FRT_DELAY_16(cnt_delay);			   
	TIM_FRT_SET_16(count1);			 */
}
//--------------------------------------------------------------------------------------------------------------------------------------
/*
void ss_main()
{
	//InitCD();
}			  */
//--------------------------------------------------------------------------------------------------------------------------------------
void Pal2CRAM( Uint16 *Pal_Data , void *Col_Adr , Uint32 suu )
{
	Uint16 i;
	Uint16 *VRAM;

	VRAM = (Uint16 *)Col_Adr;

	for( i = 0; i < suu; i++ )
		*(VRAM++) = *(Pal_Data++);
}
//--------------------------------------------------------------------------------------------------------------------------------------
#ifndef ACTION_REPLAY
void InitCD()
{
	Uint32 lib_work[GFS_WORK_SIZE(MAX_OPEN) / sizeof(Uint32)];
	GfsDirTbl dirtbl; 

#ifndef ACTION_REPLAY

#ifdef VBT
CdUnlock();
#endif

//	Sint32 ret;
 
	CDC_CdInit(0x00,0x00,0x05,0x0f);
    GFS_DIRTBL_TYPE(&dirtbl) = GFS_DIR_NAME;
    GFS_DIRTBL_DIRNAME(&dirtbl) = dir_name;
    GFS_DIRTBL_NDIR(&dirtbl) = MAX_DIR;
    GFS_Init(MAX_OPEN, lib_work, &dirtbl);
#endif	    
}
//--------------------------------------------------------------------------------------------------------------------------------------
void ChangeDir(char *dirname)
{
#ifndef ACTION_REPLAY	
    Sint32 fid;
	GfsDirTbl dirtbl; 
	
    fid = GFS_NameToId((Sint8 *)dirname);

	GFS_DIRTBL_TYPE(&dirtbl) = GFS_DIR_NAME;
	GFS_DIRTBL_DIRNAME(&dirtbl) = dir_name;
	GFS_DIRTBL_NDIR(&dirtbl) = MAX_DIR;

	GFS_LoadDir(fid,&dirtbl);
//	GFS_SetDir(&dirtbl) ;
	for (;;) {
		if (GFS_SetDir(&dirtbl) == 0) {
			break;
		}				 
	}	 
#endif	
}
//--------------------------------------------------------------------------------------------------------------------------------------
/*
int LoadFile(char *name, Uint8 *location, Uint32 size)
{
#ifndef ACTION_REPLAY	
	Sint32 		fid, i;

	for (i = 0; i < 10; i++) 
	{
		fid = GFS_NameToId(name);
		if (fid >= 0) {
			GFS_Load(fid, 0, location, size);
			return 0;
		}
	}
	return -1;
#else
    return 0;
#endif	
}
*/
//--------------------------------------------------------------------------------------------------------------------------------------
Sint32 GetFileSize(int file_id)
{
#ifndef ACTION_REPLAY	
	GfsHn gfs;
    //Sint32 sctsize, nsct; 
	Sint32	lastsize;
	//Sint32 lastsize;
    
    gfs = GFS_Open(file_id);
 
    //GFS_GetFileSize(gfs, &sctsize, &nsct, &lastsize);
	/*
    GFS_Close(gfs);
//	return sctsize*nsct;
	if(sctsize*(nsct-1)<0)
		return lastsize;
	else
		return (sctsize*(nsct-1) + lastsize);
	*/  
	GFS_GetFileInfo(gfs,(Sint32*)&file_id,(Sint32*)NULL,&lastsize,NULL);
    GFS_Close(gfs);
	return lastsize;	  
#else
    return 300;
#endif	
}

//--------------------------------------------------------------------------------------------------------------------------------------
void	InitCDBlock( void ){
    
//    InitSoundDriver();
    
    CDC_PLY_STYPE(&playdata) = CDC_PTYPE_TNO;	/* set by track number.*/
    CDC_PLY_STNO( &playdata) = 2;		/* start track number. */
    CDC_PLY_SIDX( &playdata) = 1;		/* start index number. */
    CDC_PLY_ETYPE(&playdata) = CDC_PTYPE_TNO;	/* set by track number.*/
    CDC_PLY_ETNO( &playdata) = 10;		/* start track number. */
    CDC_PLY_EIDX( &playdata) = 99;		/* start index number. */
    CDC_PLY_PMODE(&playdata) = CDC_PTYPE_NOCHG;//CDC_PM_DFL + 30;	/* Play Mode. */ // lecture en boucle
//    CDC_PLY_PMODE(&playdata) = CDC_PTYPE_NOCHG;//CDC_PM_DFL+30;//CDC_PM_DFL ;	/* Play Mode. */ // lecture unique
}
//--------------------------------------------------------------------------------------------------------------------------------------
/* Stop Music. */
void	satStopMusic( void ){
    CDC_POS_PTYPE( &posdata ) = CDC_PTYPE_DFL;	/* Stop Music. */
}
//--------------------------------------------------------------------------------------------------------------------------------------
void	satPlayMusic( Uint8 track ){
   track -= (STARTMUSIC-2);
	unsigned char *tno = (unsigned char *) 0x6000CCD;
	if (tno[0] >30)
	{
		tno[0]=0;
	}
  
//	char toto[50];
//	sprintf(toto,"start track **%d\n%d\n**new start  : %d**",tno[0],track,track+tno[0])  ;

//	FNT_Print256_2bpp((volatile Uint8 *)SCL_VDP2_VRAM_B1,(Uint8 *)toto,84,100,2,8,9,0);

//    CDC_PLY_PMODE(&playdata) = CDC_PTYPE_NOCHG;//CDC_PM_DFL + 30;	/* Play Mode. */
	CDC_POS_PTYPE( &posdata ) = CDC_PTYPE_TNO;
    CDC_PLY_STNO( &playdata ) = (Uint8) (track + tno[0]);
    CDC_PLY_ETNO( &playdata ) = (Uint8) (track + tno[0]);
    CDC_CdPlay(&playdata);
	slCDDAOn(127,127,0,0);
	slSndVolume(127);
}
//--------------------------------------------------------------------------------------------------------------------------------------
Uint16 SWAP_BYTES_16(Uint16 a) {
         return ((a >> 8) & 0x00FF) + ((a << 8) & 0xFF00);
 }

Uint32 SWAP_BYTES_32(Uint32 a) {
//return ((a >> 24) & 0x000000FF) |
//((a >>  8) & 0x0000FF00) |
//((a <<  8) & 0x00FF0000) |
//((a << 24) & 0xFF000000);
	Uint32 v;
	asm("mov %0,r1"::"r"(a):"r1");
	asm("swap.b r1,r1");
	asm("swap.w r1,r1");
	asm("swap.b r1,r1");
	asm("mov r1,%0":"=g"(v));
	return v;



}

#ifdef VBT



int CdUnlock (void)
{

Sint32 ret;
CdcStat stat;
volatile int delay;
unsigned int vbt=0;
SYS_CDINIT1(3);

SYS_CDINIT2();

do {

for(delay = 100000; delay; delay--);

ret = CDC_GetCurStat(&stat);
} while ((ret != 0) || (CDC_STAT_STATUS(&stat) == 0xff));


return (int) CDC_STAT_STATUS(&stat);

}



#endif
#endif

void SCU_DMAWait(void) {
	Uint32 res;

	while((res = DMA_ScuResult()) == 2);
}

