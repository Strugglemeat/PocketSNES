#include <stdio.h>
#include <dirent.h>
#include <SDL.h>
#include <sys/time.h>

#include "sal.h"
/*
#ifdef GCW_JOYSTICK
#include "menu.h"
extern struct MENU_OPTIONS *mMenuOptions;
#endif
*/
#define PALETTE_BUFFER_LENGTH	256*2*4
#define SNES_WIDTH  256
#define SNES_HEIGHT 239

SDL_Surface *mScreen = NULL;
static u32 mPaletteBuffer[PALETTE_BUFFER_LENGTH];
static u32 *mPaletteCurr=(u32*)&mPaletteBuffer[0];
static u32 *mPaletteEnd=(u32*)&mPaletteBuffer[PALETTE_BUFFER_LENGTH];

s32 mCpuSpeedLookup[1]={0};

#include <sal_common.h>

static u32 inputHeld = 0;
static int key_repeat_enabled = 0;


static u32 sal_Input(int held)
{
	SDL_Event event;

	if (!SDL_PollEvent(&event)) {
		if (held)
			return inputHeld;
		return 0;
	}

	u32 extraKeys = 0;

	inputHeld = 0;

	u8 *keystate = SDL_GetKeyState(NULL);

	if ( keystate[SDLK_LCTRL] )		inputHeld |= SAL_INPUT_B;
	if ( keystate[SDLK_LALT] )		inputHeld |= SAL_INPUT_Y;
	if ( keystate[SDLK_SPACE] )		inputHeld |= SAL_INPUT_A;
	if ( keystate[SDLK_LSHIFT] )	inputHeld |= SAL_INPUT_X;
	if ( keystate[SDLK_e] )		inputHeld |= SAL_INPUT_L;
	if ( keystate[SDLK_t] )	inputHeld |= SAL_INPUT_R;
	if ( keystate[SDLK_RETURN] )	inputHeld |= SAL_INPUT_START;
	if ( keystate[SDLK_RCTRL] )		inputHeld |= SAL_INPUT_SELECT;
	if ( keystate[SDLK_UP] )		inputHeld |= SAL_INPUT_UP;
	if ( keystate[SDLK_DOWN] )		inputHeld |= SAL_INPUT_DOWN;
	if ( keystate[SDLK_LEFT] )		inputHeld |= SAL_INPUT_LEFT;
	if ( keystate[SDLK_RIGHT] )		inputHeld |= SAL_INPUT_RIGHT;
	if ( keystate[SDLK_ESCAPE] )		inputHeld |= SAL_INPUT_MENU;//menu
	if ( keystate[SDLK_BACKSPACE] )		inputHeld |= SAL_INPUT_MENU;//R2
	if ( keystate[SDLK_TAB] )		inputHeld |= SAL_INPUT_MENU;//L2 - doesn't seem to work

	mInputRepeat = inputHeld;
	return inputHeld | extraKeys;
}

void sal_EnableKeyRepeat() {
	if (!key_repeat_enabled) {
		SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
		key_repeat_enabled = 1;
	}
}

void sal_DisableKeyRepeat() {
	if (key_repeat_enabled) {
		SDL_EnableKeyRepeat(0, 0);
		key_repeat_enabled = 0;
	}
}

u32 sal_InputPollRepeat()
{
	sal_EnableKeyRepeat();
	return sal_Input(0);
}

u32 sal_InputPoll()
{
	sal_DisableKeyRepeat();
	return sal_Input(1);
}

const char* sal_DirectoryGetTemp(void)
{
	return "/tmp";
}

void sal_CpuSpeedSet(u32 mhz)
{
}

u32 sal_CpuSpeedNext(u32 currSpeed)
{
	u32 newSpeed=currSpeed+1;
	if(newSpeed > 500) newSpeed = 500;
	return newSpeed;
}

u32 sal_CpuSpeedPrevious(u32 currSpeed)
{
	u32 newSpeed=currSpeed-1;
	if(newSpeed > 500) newSpeed = 0;
	return newSpeed;
}

u32 sal_CpuSpeedNextFast(u32 currSpeed)
{
	u32 newSpeed=currSpeed+10;
	if(newSpeed > 500) newSpeed = 500;
	return newSpeed;
}

u32 sal_CpuSpeedPreviousFast(u32 currSpeed)
{
	u32 newSpeed=currSpeed-10;
	if(newSpeed > 500) newSpeed = 0;
	return newSpeed;
}

s32 sal_Init(void)
{
	setenv("SDL_NOMOUSE", "1", 1);
	if( SDL_Init( SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_JOYSTICK ) == -1 )
	{
		return SAL_ERROR;
	}
	sal_TimerInit(60);

	memset(mInputRepeatTimer,0,sizeof(mInputRepeatTimer));

	sal_EnableKeyRepeat();


	return SAL_OK;
}

u32 sal_VideoInit(u32 bpp)
{
	SDL_ShowCursor(0);
	
	mBpp=bpp;

	//Set up the screen
	mScreen = SDL_SetVideoMode(SAL_SCREEN_WIDTH, SAL_SCREEN_HEIGHT, bpp, SDL_HWSURFACE | 
#ifdef SDL_TRIPLEBUF
	SDL_TRIPLEBUF
#else
	SDL_DOUBLEBUF
#endif
	);

	//If there was an error in setting up the screen
	if( mScreen == NULL )
	{
	sal_LastErrorSet("SDL_SetVideoMode failed");
	return SAL_ERROR;
	}

	return SAL_OK;
}

u32 sal_VideoGetWidth()
{
	return mScreen->w;
}

u32 sal_VideoGetHeight()
{
	return mScreen->h;
}

u32 sal_VideoGetPitch()
{
	return mScreen->pitch;
}

void sal_VideoEnterGame(u32 fullscreenOption, u32 pal, u32 refreshRate)
{
#ifdef GCW_ZERO
	/* Copied from C++ headers which we can't include in C */
	unsigned int Width = 256 /* SNES_WIDTH */,
	             Height = pal ? 239 /* SNES_HEIGHT_EXTENDED */ : 224 /* SNES_HEIGHT */;
	if (fullscreenOption != 3)
	{
		Width = SAL_SCREEN_WIDTH;
		Height = SAL_SCREEN_HEIGHT;
	}
	if (SDL_MUSTLOCK(mScreen)) SDL_UnlockSurface(mScreen);
	mScreen = SDL_SetVideoMode(Width, Height, mBpp, SDL_HWSURFACE |
#ifdef SDL_TRIPLEBUF
		SDL_TRIPLEBUF
#else
		SDL_DOUBLEBUF
#endif
		);
	mRefreshRate = refreshRate;
	if (SDL_MUSTLOCK(mScreen)) SDL_LockSurface(mScreen);
#endif
}

void sal_VideoSetPAL(u32 fullscreenOption, u32 pal)
{
	if (fullscreenOption == 3) /* hardware scaling */
	{
		sal_VideoEnterGame(fullscreenOption, pal, mRefreshRate);
	}
}

void sal_VideoExitGame()
{
#ifdef GCW_ZERO
	if (SDL_MUSTLOCK(mScreen)) SDL_UnlockSurface(mScreen);
	mScreen = SDL_SetVideoMode(SAL_SCREEN_WIDTH, SAL_SCREEN_HEIGHT, mBpp, SDL_HWSURFACE |
#ifdef SDL_TRIPLEBUF
		SDL_TRIPLEBUF
#else
		SDL_DOUBLEBUF
#endif
		);
	if (SDL_MUSTLOCK(mScreen)) SDL_LockSurface(mScreen);
#endif
}

void sal_VideoBitmapDim(u16* img, u32 pixelCount)
{
	u32 i;
	for (i = 0; i < pixelCount; i += 2)
		*(u32 *) &img[i] = (*(u32 *) &img[i] & 0xF7DEF7DE) >> 1;
	if (pixelCount & 1)
		img[i - 1] = (img[i - 1] & 0xF7DE) >> 1;
}

void sal_VideoFlip(s32 vsync)
{
	if (SDL_MUSTLOCK(mScreen)) SDL_UnlockSurface(mScreen);
	SDL_Flip(mScreen);
	if (SDL_MUSTLOCK(mScreen)) SDL_LockSurface(mScreen);
}

void *sal_VideoGetBuffer()
{
	return (void*)mScreen->pixels;
}

void sal_VideoPaletteSync() 
{ 	
	
} 

void sal_VideoPaletteSet(u32 index, u32 color)
{
	*mPaletteCurr++=index;
	*mPaletteCurr++=color;
	if(mPaletteCurr>mPaletteEnd) mPaletteCurr=&mPaletteBuffer[0];
}

void sal_Reset(void)
{

	sal_AudioClose();
	SDL_Quit();
}

int mainEntry(int argc, char *argv[]);

// Prove entry point wrapper
int main(int argc, char *argv[])
{	
	return mainEntry(argc,argv);
//	return mainEntry(argc-1,&argv[1]);
}
