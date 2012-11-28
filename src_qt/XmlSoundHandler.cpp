/**
	\file XmlSoundHandler.cpp
*/
#include "XmlSoundHandler.h"

namespace CnotiAudio {

/*!
	Constructs a Xml sound handler
*/
	XmlSoundHandler::XmlSoundHandler( )
	{

	}

/*!
	Function redefinition.

	The reader calls this function when it has parsed a start element tag.
*/
	bool XmlSoundHandler::startElement( const QString&, const QString&, const QString& tag, const QXmlAttributes& atts )
	{
		QString atr, atr2, atr3, atr4;
		_buf = "";
		if( tag == "music" ) {	
			atr = atts.value( "name" );
			if( atr=="" ){
				return false;
			}
			atr2 = atts.value( "tempo" );
			if( atr2=="" ){
				return false;
			}
			atr3 = atts.value( "duration" );
			if( atr3=="" ){
				atr3 = "0";
			}
			_music.name = atr;
			_music.tempo = atr2;
			_music.duration = atr3;
		}
		if( tag == "melody" ) {	
			atr = atts.value( "instrument" );
			atr2 = atts.value( "compass" );
			if( atr=="" ){
				return false;
			}
			if( atr2=="" ){
				return false;
			}
			Melody mel;
			mel.instrument = atr;
			mel.compass = atr2;
			_melodyList.push_back( mel );
			NoteList l;
			_noteList.push_back(l);
		}
		if( tag == "note" ) {
			atr = atts.value( "height" );
			if( atr=="" ){
				return false;
			}
			atr2 = atts.value( "duration" );
			if( atr2=="" ){
				return false;
			}
			atr3 = atts.value( "intensity" );
			if( atr3=="" ){
				return false;
			}
			atr4 = atts.value( "position" );
			if( atr4=="" ){
				return false;
			}
			Note note;
			note.height = atr;
			note.duration = atr2;
			note.intensity = atr3;
			note.position = atr4;
			_noteList[_noteList.size()-1].push_back( note );
		}
//		if( tag == "breakline" ) {
//			_breakLineList.push_back( _noteList[_noteList.size()-1].size() );
//		}
		return true;
	}

/*!
	Function redefinition.

	The reader calls this function when it has parsed an end element tag with the \a tag.
*/
	bool XmlSoundHandler::endElement( const QString&, const QString&, const QString& tag )
	{
		if( tag == "music" )
		{
		
		}
		else if( tag == "melody" )
		{
				
		}
		else if( tag == "note" )
		{
		
		}
//		else if( tag == "breakline" )
//		{

//		}
		else
		{
			return false;
		}
		return true;
	}
/*!
	Function redefinition.

	The reader calls this function when it has parsed a chunk of character data.
*/
	bool XmlSoundHandler::characters( const QString& ch )
	{
		_buf += ch;
		return true;
	}

//
//Getters
//

/*!
	Returns the music duration.
*/
	const int XmlSoundHandler::getMusicDuration()
	{
		return _music.duration.toInt();
	}

/*!
	Returns the music tempo.
*/
	const TempoType XmlSoundHandler::getMusicTempo()
	{
		return (TempoType)_music.tempo.toInt();
	}

/*!
	Returns the melody size.
*/
	const int XmlSoundHandler::getMelodySize()
	{
		return _melodyList.size();
	}

/*!
	Returns the melody compass type indexed by \a i.
*/
	const CompassType XmlSoundHandler::getMelodyCompass( const int i )
	{
		return (CompassType)_melodyList[i].compass.toInt();
	}

/*!
	Returns the melody instrument indexed by \a i.
*/
	const EnumInstrument XmlSoundHandler::getMelodyInstrument( const int i )
	{
		return (EnumInstrument)_melodyList[i].instrument.toInt();
	}

/*!
	Returns the note list size indexed by \a i.
*/
	const int XmlSoundHandler::getNoteListSize( const int i )
	{
		return _noteList[i].size();
	}

/*!
	Returns the melody intensity indexed by \a i and \a j.
*/
	const int XmlSoundHandler::getNoteIntensity( const int i, const int j )
	{
		return _noteList[i][j].intensity.toInt();
	}

/*!
	Returns the note height indexed by \a i and \a j.
*/
	const int XmlSoundHandler::getNoteHeight( const int i, const int j )
	{
		return _noteList[i][j].height.toInt();
	}

/*!
	Returns the note position indexed by \a i and \a j.
*/
	const int XmlSoundHandler::getNotePosition( const int i, const int j )
	{
		return _noteList[i][j].position.toInt();
	}

/*!
	Returns the note duration indexed by \a i and \a j.
*/
	const DurationType XmlSoundHandler::getNoteDuration( const int i, const int j )
	{
		return (DurationType)_noteList[i][j].duration.toInt();
	}

/*!
	Returns the break line points.
*/
	const QList<int> XmlSoundHandler::getBreakLineList()
	{
		return _breakLineList;
	}

}
