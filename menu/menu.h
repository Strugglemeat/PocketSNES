#ifndef _MENU_H_
#define _MENU_H_

#define SYSTEM_DIR		"pocketsnes"

#define ROM_LIST_FILENAME			"romlist.bin"
#define SRAM_FILE_EXT				"srm"
#define SAVESTATE_EXT				"sv"
#define MENU_OPTIONS_FILENAME		"pocketsnes_options"
#define MENU_OPTIONS_EXT			"opt"
#define DEFAULT_ROM_DIR_FILENAME	"romdir"
#define DEFAULT_ROM_DIR_EXT			"opt"

#define SAVESTATE_MODE_SAVE			0
#define SAVESTATE_MODE_LOAD			1
#define SAVESTATE_MODE_DELETE		2

#define POCKETSNES_VERSION			"version 2"

#define MENU_NORMAL_CPU_SPEED 		336
#define MENU_FAST_CPU_SPEED			336

enum  MENU_ENUM
{
	SAVESTATE_MENU_LOAD = 0,
	SAVESTATE_MENU_SAVE,
	// SAVESTATE_MENU_DELETE,
	// MENU_STATE,
	MENU_SETTINGS,
	MENU_RESET_GAME,
#ifndef NO_ROM_BROWSER
	MENU_ROM_SELECT,	
#endif
	// MENU_CREDITS,
	MENU_EXIT_APP,
	MENU_COUNT
};

// enum SAVESTATE_MENU_ENUM
// {
// 	// SAVESTATE_MENU_LOAD = 0,
// 	// SAVESTATE_MENU_SAVE,
// 	// SAVESTATE_MENU_DELETE,
// 	SAVESTATE_MENU_SAVE_SRAM,
// 	SAVESTATE_MENU_COUNT
// };

enum SETTINGS_MENU_ENUM
{
	SETTINGS_MENU_FULLSCREEN = 0, //scaling
	SETTINGS_MENU_FRAMESKIP,//1
	SETTINGS_MENU_FPS,//2
	SETTINGS_MENU_SOUND_ON,//3
//	SETTINGS_MENU_SOUND_VOL,//
	SETTINGS_MENU_SOUND_RATE,//4
	//SETTINGS_MENU_SOUND_STEREO,//5
	SETTINGS_MENU_SOUND_SYNC,//6
//	MENU_KEYSTROKE,
	SETTINGS_MENU_AUTO_SAVE_SRAM,//7
	SAVESTATE_MENU_SAVE_SRAM,//8
	//SETTINGS_MENU_CPU_SPEED,//9
	SETTINGS_MENU_LOAD_GLOBAL_SETTINGS,//10
	SETTINGS_MENU_SAVE_GLOBAL_SETTINGS,//11
	SETTINGS_MENU_LOAD_CURRENT_SETTINGS,//12
	SETTINGS_MENU_SAVE_CURRENT_SETTINGS,//13
	SETTINGS_MENU_DELETE_CURRENT_SETTINGS,//14
//	MENU_CREDITS,//15
	SETTINGS_MENU_COUNT//16
};


enum SRAM_MENU_ENUM
{
	SRAM_MENU_LOAD = 0,
	SRAM_MENU_SAVE,
	SRAM_MENU_DELETE,
	SRAM_MENU_RETURN,
	SRAM_MENU_COUNT,
};

enum EVENT_TYPES
{
	EVENT_NONE = 0,
	EVENT_EXIT_APP,
	EVENT_LOAD_ROM,
	EVENT_RUN_ROM,
	EVENT_RESET_ROM,
};

enum MENU_MESSAGE_BOX_MODE
{
	MENU_MESSAGE_BOX_MODE_MSG = 0,
	MENU_MESSAGE_BOX_MODE_PAUSE,
	MENU_MESSAGE_BOX_MODE_YESNO,
};

//Graphic size definitions
#define MENU_TILE_WIDTH      64
#define MENU_TILE_HEIGHT     64
#define MENU_HEADER_WIDTH    320
#define MENU_HEADER_HEIGHT   48
#define HIGHLIGHT_BAR_WIDTH  320
#define HIGHLIGHT_BAR_HEIGHT 16

#define INP_BUTTON_MENU_SELECT			SAL_INPUT_A
#define INP_BUTTON_MENU_CANCEL			SAL_INPUT_B
#define INP_BUTTON_MENU_ENTER			SAL_INPUT_SELECT
#define INP_BUTTON_MENU_DELETE			SAL_INPUT_SELECT
#define INP_BUTTON_MENU_PREVIEW_SAVESTATE	SAL_INPUT_Y
#define INP_BUTTON_MENU_QUICKSAVE1		SAL_INPUT_R
#define INP_BUTTON_MENU_QUICKSAVE2		SAL_INPUT_SELECT
#define INP_BUTTON_MENU_QUICKLOAD1		SAL_INPUT_L
#define INP_BUTTON_MENU_QUICKLOAD2		SAL_INPUT_SELECT
#define MENU_TEXT_LOAD_SAVESTATE 		"Press A to load"
#define MENU_TEXT_OVERWRITE_SAVESTATE	"A to overwrite | X to delete"
#define MENU_TEXT_DELETE_SAVESTATE 		"Press X to delete"
#define MENU_TEXT_PREVIEW_SAVESTATE 	"Press Y to preview"


struct MENU_OPTIONS
{
  unsigned int optionsVersion;
  unsigned int frameSkip;
  unsigned int soundEnabled;
  /* The following setting was 'transparency', which is now always enabled.
   * This setting word cannot be reused for any other purpose. It is not
   * guaranteed to be initialised to 0 in most installs of PocketSNES. */
  unsigned int Unused_20140603_1;
  unsigned int volume;
  unsigned int pad_config[32];
  unsigned int country;
  unsigned int showFps;
  unsigned int stereo;
  unsigned int fullScreen;
  unsigned int autoSaveSram;
  unsigned int cpuSpeed;
  unsigned int soundRate;
  unsigned int soundSync;
};

struct SAVE_STATE
{
  s8 filename[SAL_MAX_PATH];
  s8 fullFilename[SAL_MAX_PATH];
  u32 inUse;
};

//####################################
//# Functions
//####################################
void MenuInit(const char *systemDir, struct MENU_OPTIONS *menuOptions);
s32 MenuRun(s8 *romName);
s32 SaveMenuOptions(const char *path, const char *filename, const char *ext,
			const char *optionsmem, s32 maxSize, s32 showMessage);
s32 LoadMenuOptions(const char *path, const char *filename, const char *ext,
			const char *optionsmem, s32 maxSize, s32 showMessage);
s32 DeleteMenuOptions(const char *path, const char *filename,
			const char *ext, s32 showMessage);
void LoadCurrentOptions(void);
void DefaultMenuOptions(void);
void MenuPause(void);
void PrintTitle(const char *title);
void PrintTile();
void PrintBar(u32 givenY);
s32 MenuMessageBox(const char *message1, const char *message2,
			const char *message3, enum MENU_MESSAGE_BOX_MODE mode);
u32 IsPreviewingState();


#endif /* _MENU_H_ */
