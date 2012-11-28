#include "notemisc.h"

#include "CnotiAudio.h"

#include <QFile>
#include <QString>
#include <QTextStream>
#include <QStringList>

namespace CnotiAudio
{
	NoteMisc::NoteMisc()
	{
		_notesNameList[0] = "Desconhecida";
		_notesNameList[1] = "Quartifusa";
		_notesNameList[2] = "Semifusa";
		_notesNameList[4] = "Fuse";
		_notesNameList[8] = "Semicolcheia";
		_notesNameList[16] = "Colcheia";
		_notesNameList[24] = "Colcheia e meia";
		_notesNameList[32] = QString::fromUtf8( "Semínima" );
		_notesNameList[48] = QString::fromUtf8( "Semínima e meia" );
		_notesNameList[64] = QString::fromUtf8( "Mínima" );
		_notesNameList[96] = QString::fromUtf8( "Míma e meia" );
		_notesNameList[128] = "Semibreve";
		_notesNameList[192] = "Semibreve e meia";
		_notesNameList[256] = "Breve";
		_notesNameList[512] = "Longa";

		_durationDeviation = 100;
	}
	
	NoteMisc::~NoteMisc()
	{

	}

	void NoteMisc::loadNoteInfo( QString filePath )
	{
		QFile file( filePath );
		if( !file.open(QIODevice::ReadOnly | QIODevice::Text) )
		{
			return;
		}

		QTextStream in(&file);
		in.setCodec("UTF-8"); 

		while (!in.atEnd()) {
			QString line = in.readLine();
			// Process line
			if( line.trimmed() != "" && line.indexOf( "=" ) != -1 )
			{
				QStringList strList = line.split("=");
				int key = strList[0].toInt();
				QString value = strList[1];
				if( _notesNameList.value( key ) != "" )
				{
					_notesNameList[key] = value;
				} 
				else
				{
					_notesNameList.insert( key , value );
				}
				
			} 
		}    

		file.close();
	}


	/*******************
	*  NOTE DURATION  *
	*******************/
	QString NoteMisc::getDurationTypeText( float duration, int tempo )
	{
		int noteDuration = this->getDurationType( duration, tempo );
		
		return _notesNameList.value( noteDuration, _notesNameList.value( 0 ) );
	}

	int NoteMisc::getDurationType( float duration, int tempo )
	{
		QList<int> noteList;
		//noteList  << Longa << Breve << Semibreve << Minima << Seminima 
		//		  << Colcheia << Semicolcheia;// << Fuse << Semifusa << Quartifusa;
		noteList << SEMIQUAVER << QUAVER << CROTCHET_DOTTED << MINIM << SEMIBREVE << BREVE << LONGA;

		int auxDuration = duration * 1000;

		//
		//	Get the duration of a note in the tempo 
		//
		//
			//// Calculate values in seconds
			//
			// half | semibreve     = 120 / $bpm;
			// quarter | minima     = 60 / $bpm;
			// eighth | seminima    = 30 / $bpm;
			// sixteenth | colcheia = 15 / $bpm;
			// dotted_quarter       = 90 / $bpm;
			// dotted_eighth        = 45 / $bpm;
			// dotted_sixteenth     = 22.5 / $bpm;
			// triplet_quarter      = 40 / $bpm;
			// triplet_eighth       = 20 / $bpm;
			// triplet_sixteenth    = 10 / $bpm;

		float checkTime = QUAVER * 120 / SEMIBREVE;
		checkTime /= tempo;
		checkTime *= 1000;

		//
		// Starts checking from the smaller note (in this case quaver) to see where the
		// note "fits"
		//
		for( int i = 0; i < noteList.size(); i++ )
		{
			if( auxDuration + _durationDeviation > checkTime && auxDuration - _durationDeviation < checkTime )
			{
				return noteList[i];
			}
			if( auxDuration < checkTime )
			{
				if( i == 0 )
				{
					return noteList[0];
				}
				return noteList[i-1] + (noteList[i-1] / 2);
			}
			//
			// Advances to the next duration type time
			//
			checkTime *= 2;
		}

		return 0;
	}

	/*******************
	*      MIDI       *
	*******************/

	QString NoteMisc::midiNoteToName( int midiValue )
	{
		if( midiValue == 0 )
		{
			return "Pausa";
		}

		switch( midiValue % 12 )
		{
		case 0:
			return "do";
			break;
		case 1:
			return "do#";
			break;
		case 2:
			return "re";
			break;
		case 3:
			return "re#";
			break;
		case 4:
			return "mi";
			break;
		case 5:
			return "fa";
			break;
		case 6:
			return "fa#";
			break;
		case 7:
			return "sol";
			break;
		case 8:
			return "sol#";
			break;
		case 9:
			return "la";
			break;
		case 10:
			return "la#";
			break;
		case 11:
			return "si";
			break;
		}

		return "?";

	}

	int NoteMisc::midiToNote( int midiValue )
	{
		if( midiValue == 0 )
		{
			return CnotiAudio::PAUSE ;
		}

		int noteValue = midiValue % 12;
		if( noteValue >= 0 && noteValue <= 11 )
		{
			return noteValue;	// 0 = CnotiAudio::DO [...] 11 = CnotiAudio::SI
		}

		return -999;

	}
/*
+----------------------------------------------+
|                   GETTERS                    |
+----------------------------------------------+
*/
	float NoteMisc::getDurationDeviation()
	{
		return _durationDeviation;
	}
/*
+----------------------------------------------+
|                   SETTERS                    |
+----------------------------------------------+
*/
	void NoteMisc::setDurationDeviation( float newDeviation )
	{
		_durationDeviation = newDeviation;
	}
}
