/*!
 \class CnotiAudio::XmlSoundHandler
 \brief Simple SAX handler to read ours xml sound files.
 
 No error handling is done.

 \version 2.1
 \date 10-11-2008
 \file Melody.h
*/

#ifndef CNOTI_MIDI_HANDLER_H
#define CNOTI_MIDI_HANDLER_H
//
// Qt
//
#include <QtXml/QXmlDefaultHandler>

#include "CnotiAudio.h"

/**
 * 
 */
namespace CnotiAudio {

	class XmlSoundHandler : public QXmlDefaultHandler {
	public:
		struct Music {
			QString tempo;
			QString name;
			QString duration;
		};

		struct Melody {
			QString instrument;
			QString compass;
		};		
		
		struct Note {
			QString duration;
			QString intensity;
			QString height;
			QString position;
		};

		typedef QList<Note> NoteList;
		
		XmlSoundHandler();

		//
		// Redefinitions
		//
		bool startElement( const QString&, const QString&, const QString& tag, const QXmlAttributes& atts);
		bool endElement( const QString&, const QString&, const QString& tag);
		bool characters( const QString& ch );
		//
		//Getters
		//
		const int       getMusicDuration();
		const TempoType getMusicTempo();		

		const int             getMelodySize();
		const CompassType     getMelodyCompass( const int i );
		const EnumInstrument  getMelodyInstrument( const int i );

		const int           getNoteListSize( const int i );
		const int           getNoteIntensity( const int i, const int j );
		const int           getNoteHeight( const int i, const int j );
		const int           getNotePosition( const int i, const int j );
		const DurationType  getNoteDuration( const int i, const int j );
		
		const QList<int> getBreakLineList();
		
	private:
		QString _buf;
		
		Music _music;
		QList<Melody>   _melodyList;
		QList<NoteList> _noteList;
		QList<int>      _breakLineList;

		// Dont offer copy c'tor and assignment operator:
		XmlSoundHandler( const XmlSoundHandler & );
		XmlSoundHandler &operator=( const XmlSoundHandler& );
	};
}


#endif // CNOTI_MIDI_HANDLER_H
