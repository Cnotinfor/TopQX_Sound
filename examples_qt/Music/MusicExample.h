#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
// SoundManager
#include <../../include/CnotiAudio.h>

namespace Ui {
	class MusicExample;
}

namespace CnotiAudio {
	class SoundManager;
	class Music;
}

class MusicExample : public QWidget
{
	Q_OBJECT

public:
	explicit MusicExample(QWidget *parent = 0);
	~MusicExample();

	void selectInstrument(CnotiAudio::EnumInstrument inst);
	void setVolume(float volume);

private:
	Ui::MusicExample *ui;
	CnotiAudio::SoundManager *_soundMgr;
	CnotiAudio::Music *_music;
	//Instrument
	CnotiAudio::EnumInstrument _instrument;
	CnotiAudio::TempoType _tempo;

	// Functions
	void changeRhythm(CnotiAudio::EnumInstrument instrument, int variation);
	void resetGui();

private slots:
	void on_pushButtonLoadMusic_clicked();
	void on_pushButtonSaveMusic_clicked();
	void on_pushButtonSaveMusicWav_clicked();
	void on_pushButtonSaveMusicMp3_clicked();
	void on_commandLinkButton_stop_clicked();
	void on_commandLinkButton_play_clicked();
	void on_comboBox_instrument_currentIndexChanged(int index);
	void on_verticalSlider_volume_valueChanged(int value);
	void selectNote(int pos);
	void unselectNote(int pos);
	void on_comboBox_drum_currentIndexChanged(int index);
	void on_comboBox_beatBox_currentIndexChanged(int index);
	void on_comboBox_chineseBox_currentIndexChanged(int index);
	void on_comboBox_congas_currentIndexChanged(int index);
	void on_comboBox_Tamborine_currentIndexChanged(int index);
	void on_comboBox_triangle_currentIndexChanged(int index);
};

#endif // WIDGET_H
