#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <tcl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

extern DLLEXPORT int Tclmixer_Init(Tcl_Interp * interp);

/*
 * end block for C++
 */

#ifdef __cplusplus
}
#endif

#define TCLMIXER_VERSION "2.0.1"

#ifdef WINDOWS
#define BUFFER 4096
#else
#define BUFFER 1024
#endif

Tcl_HashTable sounds;
Tcl_HashTable musics;
char* soundFinishHandle = NULL;
char* musicFinishHandle = NULL;
Tcl_Interp* interpreter;

int TclMixer_Sound(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj * const objv[])
{
	if (objc != 2)
	{
		Tcl_WrongNumArgs(interp, 1, objv, "fileName");
		return TCL_ERROR;
	}

	Mix_Chunk* snd = Mix_LoadWAV(Tcl_GetString(objv[1]));

	if (!snd)
	{
		Tcl_SetResult(interp, Mix_GetError(), TCL_STATIC);
		return TCL_ERROR;
	}

	int i, sw = 0;
	char key[18];

	Tcl_HashEntry* entry;
	for ( i= 0; !sw; i++)
	{
		sprintf(key, "sound#%d", i);
		entry = Tcl_FindHashEntry(&sounds, key);
		if (!entry)
			break;
	}
	entry = Tcl_CreateHashEntry(&sounds, key, &sw);
	if (!sw)
	{
		Tcl_SetResult(interp, "An error hass occured while creating hash entry.", TCL_STATIC);
		return TCL_ERROR;
	}

	Tcl_SetHashValue(entry, snd);
	Tcl_SetResult(interp, key, TCL_VOLATILE);

	return TCL_OK;
}

int TclMixer_Music(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj * const objv[])
{
	if (objc != 2)
	{
		Tcl_WrongNumArgs(interp, 1, objv, "fileName");
		return TCL_ERROR;
	}

	Mix_Music* mus = Mix_LoadMUS(Tcl_GetString(objv[1]));

	if (!mus)
	{
		Tcl_SetResult(interp, Mix_GetError(), TCL_STATIC);
		return TCL_ERROR;
	}

	int i, sw = 0;
	char key[18];

	Tcl_HashEntry* entry;
	for ( i= 0; !sw; i++)
	{
		sprintf(key, "music#%d", i);
		entry = Tcl_FindHashEntry(&musics, key);
		if (!entry)
			break;
	}
	entry = Tcl_CreateHashEntry(&musics, key, &sw);
	if (!sw)
	{
		Tcl_SetResult(interp, "An error hass occured while creating hash entry.", TCL_STATIC);
		return TCL_ERROR;
	}

	Tcl_SetHashValue(entry, mus);
	Tcl_SetResult(interp, key, TCL_VOLATILE);

	return TCL_OK;
}

int TclMixer_Free(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj * const objv[])
{
	if (objc != 2)
	{
		Tcl_WrongNumArgs(interp, 1, objv, "soundOrMusicId");
		return TCL_ERROR;
	}

	Tcl_HashEntry* entry = Tcl_FindHashEntry(&sounds, Tcl_GetString(objv[1]));
	Tcl_HashEntry* musicEntry = Tcl_FindHashEntry(&musics, Tcl_GetString(objv[1]));
	if (entry)
	{
		Mix_Chunk* snd = (Mix_Chunk*)Tcl_GetHashValue(entry);
		Tcl_DeleteHashEntry(entry);
		Mix_FreeChunk(snd);
	}
	else if (musicEntry)
	{
		Mix_Music* mus = (Mix_Music*)Tcl_GetHashValue(musicEntry);
		Tcl_DeleteHashEntry(musicEntry);
		Mix_FreeMusic(mus);
	}
	else
	{
		Tcl_SetResult(interp, "This sound or music doesn't exists.", TCL_STATIC);
		return TCL_ERROR;
	}
	return TCL_OK;
}

int TclMixer_Play(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj * const objv[])
{
	// Syntax
	if (objc < 2 || objc > 2 && (objc%2))
	{
		Tcl_WrongNumArgs(interp, 1, objv, "?-loops times? ?-channel number? ?-fadein time? soundOrMusicId");
		return TCL_ERROR;
	}

	// Default settings
	int fadeIn = 0,
		loops = 0,
		chan = -1;

	// Getting options
	int arg = 1;
	if (objc > 2)
	{
		char* opts[] = {
			"-loops",
			"-channel",
			"-fadein"
		};
		int i, match;
		for ( ; arg < objc-1; arg++ )
		{
			match = 0;
			for ( i = 0; i < 3; i++ )
			{
				if ( !strcmp(opts[i], Tcl_GetString(objv[arg])) )
				{
					match = 1;
					arg++;
					switch (i)
					{
						case 0:
						{
							if (Tcl_GetIntFromObj(interp, objv[arg], &loops) == TCL_ERROR)
								return TCL_ERROR;

							break;
						}
						case 1:
						{
							if (Tcl_GetIntFromObj(interp, objv[arg], &chan) == TCL_ERROR)
								return TCL_ERROR;

							break;
						}
						case 2:
						{
							if (Tcl_GetIntFromObj(interp, objv[arg], &fadeIn) == TCL_ERROR)
								return TCL_ERROR;

							break;
						}
					}
				}
			}
			if (!match)
			{
				Tcl_WrongNumArgs(interp, 0, NULL, "play ?-loops times? ?-channel number? ?-fadein time? soundOrMusicId");
				return TCL_ERROR;
			}
		}
	}

	// Creating Mix_Chunk and adding it to hash table
	Tcl_HashEntry* entry = Tcl_FindHashEntry(&sounds, Tcl_GetString(objv[arg]));
	Tcl_HashEntry* musicEntry = Tcl_FindHashEntry(&musics, Tcl_GetString(objv[arg]));
	if (entry)
	{
		Mix_Chunk* snd = (Mix_Chunk*)Tcl_GetHashValue(entry);
		int chanRet;
		if (fadeIn > 0)
		{
			chanRet = Mix_FadeInChannel(chan, snd, loops, fadeIn);
		}
		else
		{
			chanRet = Mix_PlayChannel(chan, snd, loops);
		}
		if (chanRet == -1)
		{
			Tcl_SetResult(interp, Mix_GetError(), TCL_VOLATILE);
			return TCL_ERROR;
		}
		char buff[10];
		sprintf(buff, "%d", chanRet);
		Tcl_SetResult(interp, buff, TCL_VOLATILE);
	}
	else if (musicEntry)
	{
		Mix_Music* mus = (Mix_Music*)Tcl_GetHashValue(musicEntry);
		int ret;
		if (fadeIn > 0)
		{
			ret = Mix_FadeInMusic(mus, loops, fadeIn);
		}
		else
		{
			ret = Mix_PlayMusic(mus, loops);
		}
		if (ret == -1)
		{
			Tcl_SetResult(interp, Mix_GetError(), TCL_VOLATILE);
			return TCL_ERROR;
		}
	}
	else
	{
		Tcl_SetResult(interp, "No such sound or music object.", TCL_STATIC);
		return TCL_ERROR;
	}

	return TCL_OK;
}

int TclMixer_Stop(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj * const objv[])
{
	if (objc < 2 || objc > 3)
	{
		Tcl_WrongNumArgs(interp, 1, objv, "channelNumOrMUSIC ?delay?");
		return TCL_ERROR;
	}

	if ( !strcasecmp("music", Tcl_GetString(objv[1])) )
	{
		Mix_HaltMusic();
	}
	else
	{
		int chan, delay = 0;
		if (Tcl_GetIntFromObj(interp, objv[1], &chan) == TCL_ERROR)
			return TCL_ERROR;

		if (objc == 3)
		{
			if (Tcl_GetIntFromObj(interp, objv[2], &delay) == TCL_ERROR)
				return TCL_ERROR;
		}

		if (delay > 0)
		{
			Mix_ExpireChannel(chan, delay);
		}
		else
		{
			Mix_HaltChannel(chan);
		}
	}

	return TCL_OK;
}

int TclMixer_Volume(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj * const objv[])
{
	if (objc < 2 || objc > 3)
	{
		Tcl_WrongNumArgs(interp, 1, objv, "soundIdOrMUSIC ?volume?");
		return TCL_ERROR;
	}

	int newVol = -1, chan = -2;
	Tcl_GetIntFromObj(interp, objv[1], &chan);
	if (objc == 3)
	{
		if (Tcl_GetIntFromObj(interp, objv[2], &newVol) == TCL_ERROR)
			return TCL_ERROR;
	}

	Tcl_HashEntry* entry = Tcl_FindHashEntry(&sounds, Tcl_GetString(objv[1]));
	if (entry)
	{
		Mix_Chunk* snd = (Mix_Chunk*)Tcl_GetHashValue(entry);
		int oldVol = Mix_VolumeChunk(snd, newVol);

		char buff[4];
		sprintf(buff, "%d", oldVol);
		Tcl_SetResult(interp, buff, TCL_VOLATILE);
	}
	else if (chan > -2)
	{
		int oldVol = Mix_Volume(chan, newVol);

		char buff[4];
		sprintf(buff, "%d", oldVol);
		Tcl_SetResult(interp, buff, TCL_VOLATILE);
	}
	else if ( !strcasecmp("music", Tcl_GetString(objv[1])) )
	{
		int oldVol = Mix_VolumeMusic(newVol);

		char buff[4];
		sprintf(buff, "%d", oldVol);
		Tcl_SetResult(interp, buff, TCL_VOLATILE);
	}
	else
	{
		Tcl_SetResult(interp, "This sound doesn't exists.", TCL_STATIC);
		return TCL_ERROR;
	}
	return TCL_OK;
}

int TclMixer_FadeOut(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj * const objv[])
{
	if (objc < 2 || objc > 3)
	{
		Tcl_WrongNumArgs(interp, 1, objv, "channelOrMUSIC time");
		return TCL_ERROR;
	}

	int chan, time;

	if (Tcl_GetIntFromObj(interp, objv[2], &time) == TCL_ERROR)
		return TCL_ERROR;

	if ( !strcasecmp("music", Tcl_GetString(objv[1])) )
	{
		int ret = Mix_FadeOutMusic(time);
		if (!ret)
		{
			Tcl_SetResult(interp, Mix_GetError(), TCL_VOLATILE);
			return TCL_ERROR;
		}
	}
	else
	{
		if (Tcl_GetIntFromObj(interp, objv[1], &chan) == TCL_ERROR)
			return TCL_ERROR;

		int chans = Mix_FadeOutChannel(chan, time);

		char buff[10];
		sprintf(buff, "%d", chans);
		Tcl_SetResult(interp, buff, TCL_VOLATILE);
	}

	return TCL_OK;
}

int TclMixer_IsFading(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj * const objv[])
{
	if (objc != 2)
	{
		Tcl_WrongNumArgs(interp, 1, objv, "channelOrMUSIC");
		return TCL_ERROR;
	}

	Mix_Fading fading;

	if ( !strcasecmp("music", Tcl_GetString(objv[1])) )
	{
		fading = Mix_FadingMusic();
	}
	else
	{
		int chan;
		if (Tcl_GetIntFromObj(interp, objv[1], &chan) == TCL_ERROR)
			return TCL_ERROR;

		fading = Mix_FadingChannel(chan);
	}

	switch (fading)
	{
		case MIX_NO_FADING:
		{
			Tcl_SetResult(interp, "no", TCL_STATIC);
			break;
		}
		case MIX_FADING_OUT:
		{
			Tcl_SetResult(interp, "out", TCL_STATIC);
			break;
		}
		case MIX_FADING_IN:
		{
			Tcl_SetResult(interp, "in", TCL_STATIC);
			break;
		}
	}

	return TCL_OK;
}

int TclMixer_IsPlaying(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj * const objv[])
{
	if (objc != 2)
	{
		Tcl_WrongNumArgs(interp, 1, objv, "channelOrMUSIC");
		return TCL_ERROR;
	}

	int result;

	if ( !strcasecmp("music", Tcl_GetString(objv[1])) )
	{
		result = Mix_PlayingMusic();
	}
	else
	{
		int chan;
		if (Tcl_GetIntFromObj(interp, objv[1], &chan) == TCL_ERROR)
			return TCL_ERROR;

		result = Mix_Playing(chan);
	}

	char buff[10];
	sprintf(buff, "%d", result);
	Tcl_SetResult(interp, buff, TCL_VOLATILE);
	return TCL_OK;
}

int TclMixer_IsPaused(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj * const objv[])
{
	if (objc != 2)
	{
		Tcl_WrongNumArgs(interp, 1, objv, "channelOrMusic");
		return TCL_ERROR;
	}

	int result;

	if ( !strcasecmp("music", Tcl_GetString(objv[1])) )
	{
		result = Mix_PausedMusic();
	}
	else
	{
		int chan;
		if (Tcl_GetIntFromObj(interp, objv[1], &chan) == TCL_ERROR)
			return TCL_ERROR;

		result = Mix_Paused(chan);
	}

	char buff[10];
	sprintf(buff, "%d", result);
	Tcl_SetResult(interp, buff, TCL_VOLATILE);
	return TCL_OK;
}

int TclMixer_MixConfig(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj * const objv[])
{
	if (objc < 3 || (objc % 2) != 1)
	{
		Tcl_WrongNumArgs(interp, 1, objv, "option value ?option value ...?");
		return TCL_ERROR;
	}

	char* opts[] = {
		"-channels",
		"-sound",
		"-music",
		"-reserve"
	};
	int argc;
	for ( argc = 1; argc < objc; argc++ )
	{
		char* opt = Tcl_GetString(objv[argc]);
		argc++;
		Tcl_Obj* val = objv[argc];

		int i, res = -1;
		for ( i = 0; i < 4; i++ )
		{
			if ( !strcmp(opts[i], opt) )
			{
				res = i;
				break;
			}
		}

		switch (res)
		{
			case 0:
			{
				int chans;
				if (Tcl_GetIntFromObj(interp, val, &chans) == TCL_ERROR)
					return TCL_ERROR;
	
				Mix_AllocateChannels(chans);
				break;
			}
			case 1:
			{
				char *argv2 = Tcl_GetString(val);
				int lgt = strlen(argv2);
			if (	soundFinishHandle)
					free(soundFinishHandle);

				soundFinishHandle = (char*)malloc(lgt+1);
				soundFinishHandle = strcpy(soundFinishHandle, argv2);
				break;
			}
			case 2:
			{
				char *argv2 = Tcl_GetString(val);
				int lgt = strlen(argv2);
				if (musicFinishHandle)
				free(musicFinishHandle);

				musicFinishHandle = (char*)malloc(lgt+1);
				musicFinishHandle = strcpy(musicFinishHandle, argv2);
				break;
			}
			case 3:
			{
				int chans, res;
				if (Tcl_GetIntFromObj(interp, val, &chans) == TCL_ERROR)
					return TCL_ERROR;

				res = Mix_ReserveChannels(chans);
				if (res != chans)
				{
					Mix_ReserveChannels(0);
					Tcl_SetResult(interp, "You ware trying to reserve more channels than allocated was.", TCL_STATIC);
					return TCL_ERROR;
				}
				break;
			}
			default:
			{
				Tcl_SetResult(interp, "Wrong mixer configuration option.", TCL_STATIC);
				return TCL_ERROR;
			}
		}
	}
	return TCL_OK;
}

int TclMixer_Balance(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj * const objv[])
{
	if (objc != 4)
	{
		Tcl_WrongNumArgs(interp, 1, objv, "channel leftVolume rightVolume");
		return TCL_ERROR;
	}

	int chan, left, right;
	if (Tcl_GetIntFromObj(interp, objv[1], &chan) == TCL_ERROR)
		return TCL_ERROR;

	if (Tcl_GetIntFromObj(interp, objv[2], &left) == TCL_ERROR)
		return TCL_ERROR;

	if (Tcl_GetIntFromObj(interp, objv[3], &right) == TCL_ERROR)
		return TCL_ERROR;

	int ret = Mix_SetPanning(chan, left, right);

	if (!ret)
	{
		Tcl_SetResult(interp, Mix_GetError(), TCL_VOLATILE);
		return TCL_ERROR;
	}
	return TCL_OK;
}

int TclMixer_Distance(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj * const objv[])
{
	if (objc != 3)
	{
		Tcl_WrongNumArgs(interp, 1, objv, "channel distance");
		return TCL_ERROR;
	}

	int chan, dist;
	if (Tcl_GetIntFromObj(interp, objv[1], &chan) == TCL_ERROR)
		return TCL_ERROR;

	if (Tcl_GetIntFromObj(interp, objv[2], &dist) == TCL_ERROR)
		return TCL_ERROR;

	int ret = Mix_SetDistance(chan, dist);

	if (!ret)
	{
		Tcl_SetResult(interp, Mix_GetError(), TCL_VOLATILE);
		return TCL_ERROR;
	}
	return TCL_OK;
}

int TclMixer_Position(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj * const objv[])
{
	if (objc != 4)
	{
		Tcl_WrongNumArgs(interp, 1, objv, "channel angle distance");
		return TCL_ERROR;
	}

	int chan, angle, dist;
	if (Tcl_GetIntFromObj(interp, objv[1], &chan) == TCL_ERROR)
		return TCL_ERROR;

	if (Tcl_GetIntFromObj(interp, objv[2], &angle) == TCL_ERROR)
		return TCL_ERROR;

	if (Tcl_GetIntFromObj(interp, objv[3], &dist) == TCL_ERROR)
		return TCL_ERROR;

	int ret = Mix_SetPosition(chan, angle, dist);

	if (!ret)
	{
		Tcl_SetResult(interp, Mix_GetError(), TCL_VOLATILE);
		return TCL_ERROR;
	}
	return TCL_OK;
}

int TclMixer_PlayPosition(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj * const objv[])
{
	if (objc != 2)
	{
		Tcl_WrongNumArgs(interp, 1, objv, "position");
		return TCL_ERROR;
	}

	double position;
	if (Tcl_GetDoubleFromObj(interp, objv[1], &position) == TCL_ERROR)
		return TCL_ERROR;

	int ret = Mix_SetMusicPosition(position);

	if (ret == -1)
	{
		Tcl_SetResult(interp, Mix_GetError(), TCL_VOLATILE);
		return TCL_ERROR;
	}
	return TCL_OK;
}

int TclMixer_Rewind(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj * const objv[])
{
	if (objc != 1)
	{
		Tcl_WrongNumArgs(interp, 1, objv, "");
		return TCL_ERROR;
	}

	Mix_RewindMusic();
	return TCL_OK;
}

int TclMixer_MusicType(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj * const objv[])
{
	if (objc > 2)
	{
		Tcl_WrongNumArgs(interp, 1, objv, "?musicId?");
		return TCL_ERROR;
	}

	Mix_MusicType type;

	if (objc == 2)
	{
		Tcl_HashEntry* musicEntry = Tcl_FindHashEntry(&musics, Tcl_GetString(objv[1]));
		if (musicEntry)
		{
			Mix_Music* mus = (Mix_Music*)Tcl_GetHashValue(musicEntry);
			type = Mix_GetMusicType(mus);
		}
		else
		{
			Tcl_SetResult(interp, "This music doesn't exists.", TCL_STATIC);
			return TCL_ERROR;
		}
	}
	else
	{
		type = Mix_GetMusicType(NULL);
	}

	switch (type)
	{
		case MUS_CMD:
		{
			Tcl_SetResult(interp, "CMD", TCL_STATIC);
			break;
		}
		case MUS_WAV:
		{
			Tcl_SetResult(interp, "WAV", TCL_STATIC);
			break;
		}
		case MUS_MOD:
		{
			Tcl_SetResult(interp, "MOD", TCL_STATIC);
			break;
		}
		case MUS_MID:
		{
			Tcl_SetResult(interp, "MID", TCL_STATIC);
			break;
		}
		case MUS_OGG:
		{
			Tcl_SetResult(interp, "OGG", TCL_STATIC);
			break;
		}
		case MUS_MP3:
		{
			Tcl_SetResult(interp, "MP3", TCL_STATIC);
			break;
		}
		case MUS_NONE:
		{
			Tcl_SetResult(interp, "NONE", TCL_STATIC);
			break;
		}
		default:
		{
			Tcl_SetResult(interp, "UNKNOWN", TCL_STATIC);
		}
	}
	return TCL_OK;
}

void soundFinishCallback(int channel)
{
	if (soundFinishHandle)
	{
		char buff[10];
		sprintf(buff, "%d", channel);
		int ret = Tcl_VarEval(interpreter, soundFinishHandle, " ", buff, NULL);
		if (ret == TCL_ERROR)
			Tcl_BackgroundError(interpreter);
	}
}

void musicFinishCallback(void)
{
	if (musicFinishHandle)
	{
		int ret = Tcl_Eval(interpreter, musicFinishHandle);
		if (ret == TCL_ERROR)
			Tcl_BackgroundError(interpreter);
	}
}

/*
	Init
*/

int Tclmixer_Init(Tcl_Interp* interp)
{
#ifdef USE_TCL_STUBS
	// Initialising stubs engine
	if (Tcl_InitStubs(interp, TCL_VERSION, 0) == NULL)
		return TCL_ERROR;
#endif

	interpreter = interp;

	// Initialize SDL audio system
	if (SDL_Init(SDL_INIT_AUDIO) == -1)
	{
		Tcl_SetResult(interp, "Could not initialize SDL audio system.", TCL_STATIC);
		return TCL_ERROR;
	}

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, BUFFER) == -1)
	{
		Tcl_SetResult(interp, "Could not initialize SDL_mixer audio system.", TCL_STATIC);
		return TCL_ERROR;
	}

	Mix_AllocateChannels(16);
	Mix_ChannelFinished(soundFinishCallback);
	Mix_HookMusicFinished(musicFinishCallback);

	// Initialize Tcl Hash tables
	Tcl_InitHashTable(&sounds, TCL_STRING_KEYS);
	Tcl_InitHashTable(&musics, TCL_STRING_KEYS);

	// Register Tcl commands
	Tcl_CreateObjCommand(interp, "::tclmixer::sound", TclMixer_Sound, NULL, NULL);
	Tcl_CreateObjCommand(interp, "::tclmixer::music", TclMixer_Music, NULL, NULL);
	Tcl_CreateObjCommand(interp, "::tclmixer::free", TclMixer_Free, NULL, NULL);
	Tcl_CreateObjCommand(interp, "::tclmixer::play", TclMixer_Play, NULL, NULL);
	Tcl_CreateObjCommand(interp, "::tclmixer::stop", TclMixer_Stop, NULL, NULL);
	Tcl_CreateObjCommand(interp, "::tclmixer::volume", TclMixer_Volume, NULL, NULL);
	Tcl_CreateObjCommand(interp, "::tclmixer::fadeOut", TclMixer_FadeOut, NULL, NULL);
	Tcl_CreateObjCommand(interp, "::tclmixer::fading", TclMixer_IsFading, NULL, NULL);
	Tcl_CreateObjCommand(interp, "::tclmixer::playing", TclMixer_IsPlaying, NULL, NULL);
	Tcl_CreateObjCommand(interp, "::tclmixer::paused", TclMixer_IsPaused, NULL, NULL);
	Tcl_CreateObjCommand(interp, "::tclmixer::balance", TclMixer_Balance, NULL, NULL);
	Tcl_CreateObjCommand(interp, "::tclmixer::distance", TclMixer_Distance, NULL, NULL);
	Tcl_CreateObjCommand(interp, "::tclmixer::position", TclMixer_Position, NULL, NULL);
	Tcl_CreateObjCommand(interp, "::tclmixer::playPosition", TclMixer_PlayPosition, NULL, NULL);
	Tcl_CreateObjCommand(interp, "::tclmixer::rewind", TclMixer_Rewind, NULL, NULL);
	Tcl_CreateObjCommand(interp, "::tclmixer::musicType", TclMixer_MusicType, NULL, NULL);
	Tcl_CreateObjCommand(interp, "::tclmixer::mixConfig", TclMixer_MixConfig, NULL, NULL);

	Tcl_Eval(interp, "namespace eval tclmixer {namespace export *}");

	Tcl_PkgProvide(interp, "tclmixer", TCLMIXER_VERSION);

	return TCL_OK;
}
