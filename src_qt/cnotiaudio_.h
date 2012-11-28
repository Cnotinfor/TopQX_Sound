#if !defined(_CNOTIAUDI_H)
#define _CNOTIAUDI_H

//#ifndef _WIN32
//#include "MyOpenALSupport.h"
//#endif

//#include <string>
#include <QString>

namespace CnotiAudio{

	const int CS_NUMBERNOTE = 12;
	const int CS_MAXOCTAVE = 4;
	const int CS_MINOCTAVE = 3;
	const int CS_NUMBERRYTHM = 5;
	const int CS_NUMBERRYTHM_BEATBOX = 10;
	const int CS_NUMBERCHANNEL = 1;
	const int CS_UNITTIMEDEFAULT = 128;
	const QString pauseName = "pause_";
//	const std::string soundLog = "Pequeno Mozart - Sound.log";
	
	enum TypeSound{
		CS_SAMPLE = 0,
		CS_XML,
		CS_STREAM
	};
	enum EnumInstrument{
		Unknow=-1,
		Piano=1,
		Xylophone=4,
		Organ=33,
		Violin=3,
		Trombone=5,
		Flute=2,
		Bombo=257,
		Caixa_Chinesa=258,
		Congas=259,
		Pandeireta=260,
		Triangulo=261,
		BeatBox=262
	};
	enum CnotiErrorSound{
		CS_NO_ERROR,
		CS_NOT_IMPLEMENT,
		CS_ID_MELODY_UNKNOW,
		CS_ID_MELODY_NULL,
		CS_FILE_ERROR,
		CS_VALUE_OCTAVE_ERROR,
		CS_VALUE_POSITION_ERROR,
		CS_MELODY_EMPTY,
		CS_SOUND_EMPTY,
		CS_NOTE_NOT_EXISTE,
		CS_FILE_NOT_EXISTE,
		CS_PARSER_ERROR,
		CS_EXTENTION_ERROR,
		CS_SOUND_UNKNOW,
		CS_NAME_ALREADY_USED,
		CS_INIT_OPENAL,
		CS_IS_ALREADY_PLAYING,
		CS_IS_ALREADY_STOPPED,
		CS_AL_ERROR,
		CS_ERROR_FILE_OGG,
		CS_IS_NOT_XMLSOUND,
		CS_IS_XMLSOUND,
		CS_IS_NOT_SAMPLE,
		CS_IS_SAMPLE,
		CS_EXTENSION_UNKNOW,
		CS_MISSING_VORBISDLL,
		CS_OPENAL_NOT_INIT,
		CS_OGG_NOT_INIT,
		CS_FILE_EXISTS,
		CS_FILE_NOT_FOUND
	};
	enum CompassType{
		binario_simples=24,
		ternario_simple=34,
		quaternario_simple=44
	};
	enum TempoType{
		Tempo_0     = 0,
		Tempo_60    = 60,
		Tempo_120   = 120,
		Tempo_160   = 160,
		Tempo_200   = 200
	};
	enum DurationType{
		Longa=512,
		Breve=256,
		SemibreveMeia=192,
		Semibreve=128,
		MinimaMeia=96,
		Minima=64,
		SeminimaMeia=48,
		Seminima=32,
		ColcheiaMeia=24,
		Colcheia=16,
		Semicolcheia=8,
		Fuse=4,
		Semifusa=2,
		Quartifusa=1
	};
	enum NoteType{
		PAUSE = -1,
		DO = 0,					// Midi = 60
		REb,		// = Do#	// Midi = 61
		RE,						// Midi = 62
		MIb,		// = Re#	// Midi = 63
		MI,						// Midi = 64
		FA,						// Midi = 65
		SOLb,		// = Fa#	// Midi = 66
		SOL,					// Midi = 67
		LAb,		// = Sol#	// Midi = 68
		LA,						// Midi = 69
		SIb,		// = La#	// Midi = 70
		SI						// Midi = 71
	};
}
#endif  //_CNOTIAUDI_H
