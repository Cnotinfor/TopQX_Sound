#if !defined(_CNOTIAUDI_H)
#define _CNOTIAUDI_H

#ifndef _WIN32
#include "openal/MacOSX/MyOpenALSupport.h"
#endif

#include <QString>

namespace CnotiAudio{

	const int CS_NUMBERNOTE = 12;
	const int CS_MAXOCTAVE = 4;
	const int CS_MINOCTAVE = 3;
	const int CS_NUMBERCHANNEL = 1;
	const int CS_UNITTIMEDEFAULT = 128;
	const QString pauseName = "pause_";
	
	const int CS_NUMBERRYTHM = 5;
	const int CS_NUMBERRYTHM_BEATBOX = 10;
//	const std::string soundLog = "Pequeno Mozart - Sound.log";
	
	enum TypeSound{
		CS_SAMPLE = 0,
		CS_XML,
		CS_STREAM
	};

	enum EnumInstrument{
		INSTRUMENT_UNKNOWN=0,
		PIANO=1,
		FLUTE=2,
		VIOLIN=3,
		XYLOPHONE=4,
		TRUMPET=5
//		ORGAN=33,
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
		quaternario_simples=44
	};
	enum TempoType{
		TEMPO_UNKNOWN = 0,
		TEMPO_60      = 60,
		TEMPO_120     = 120,
		TEMPO_160     = 160,
		TEMPO_200     = 200
	};
	enum DurationType{
		LONGA=512,
		BREVE=256,
		SEMIBREVE_DOTTED=192,
		SEMIBREVE=128,
		MINIM_DOTTED=96,
		MINIM=64,
		CROTCHET_DOTTED=48,
		CROTCHET=32,				// Semiminima
		QUAVER_HALF=24,
		QUAVER=16,					// Colcheia
		SEMIQUAVER=8,
		DEMISEMIQUAVER=4,			// Fuse
		HEMIDEMISEMIQUAVER=2,		// Semifusa
		SEMIHEMIDEMISEMIQUAVER=1,	// Quartifusa
		UNKNOWN_DURATION
	};
	enum NoteType{
		UNKNOWN_NOTE = -2,
		PAUSE = -1,
		DO = 0,
		REb,
		RE,
		MIb,
		MI,
		FA,		// 5
		SOLb,
		SOL,
		LAb,
		LA,
		SIb,	// 10
		SI
	};
	
	enum EnumOctave{
		OCTAVE_UNKNOWN,
		OCTAVE_C3 = 3,
		OCTAVE_C4
	};

	enum EnumRhythmInstrument{
		RHYTHM_INST_CHINESE_BOX=0,	// Caixa Chinesa
		RHYTHM_INST_BASS_DRUM,		// Bombo
		RHYTHM_INST_CONGAS,
		RHYTHM_INST_TAMBOURINE,		// Pandeireta
		RHYTHM_INST_TRIANGLE,
		RHYTHM_INST_BEAT_BOX,
		RHYTHM_INST_UNKNOWN
	};
	
	enum EnumRhythmVariation{
		RHYTHM_01 = 0,
		RHYTHM_02,
		RHYTHM_03,
		RHYTHM_04,
		RHYTHM_05,
		RHYTHM_06,
		RHYTHM_07,
		RHYTHM_08,
		RHYTHM_09,
		RHYTHM_10,
		RHYTHM_UNKNOWN
	};
}
#endif  //_CNOTIAUDI_H
