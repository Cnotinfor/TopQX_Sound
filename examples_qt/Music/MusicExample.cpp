#include "MusicExample.h"
#include "ui_MusicExample.h"
// Qt
#include <QFileDialog>
#include <QDebug>
// SoundManager
#include <../../include/SoundManager>
#include <../../include/Music>
#include <../../include/Note>

MusicExample::MusicExample(QWidget *parent) :
    QWidget(parent),
	ui(new Ui::MusicExample),
	_instrument(CnotiAudio::FLUTE),
	_tempo(CnotiAudio::TEMPO_160)
{
    ui->setupUi(this);
	ui->commandLinkButton_stop->hide();

	_soundMgr = CnotiAudio::SoundManager::instance();
	_soundMgr->init("Music");
	_soundMgr->initOpenAl("../resources/samples/");

	_music = _soundMgr->createMusic("MyMusic", _tempo, _instrument);
	connect(_music, SIGNAL(soundStopped(QString)), this, SLOT(on_commandLinkButton_stop_clicked()));
	connect(_music, SIGNAL(notePlaying(int)), this, SLOT(selectNote(int)));
	connect(_music, SIGNAL(noteStopped(int)), this, SLOT(unselectNote(int)));

	setVolume(1.0f);
}

MusicExample::~MusicExample()
{
    delete ui;
}


void MusicExample::selectInstrument(CnotiAudio::EnumInstrument inst)
{
	_music->setInstrument(inst);
	ui->comboBox_instrument->blockSignals(true);
	ui->comboBox_instrument->setCurrentIndex(inst - 1);
	ui->comboBox_instrument->blockSignals(false);
}

void MusicExample::setVolume(float volume)
{
	_music->setIntensity(_instrument, volume);
	QListIterator<CnotiAudio::EnumInstrument> it(_music->rhythmList());
	while(it.hasNext())
	{
		CnotiAudio::EnumInstrument inst = it.next();
		_music->setIntensity(inst, volume);
	}
//	ui->verticalSlider_volume->blockSignals(true);
//	ui->verticalSlider_volume->setValue(volume * 10);
//	ui->verticalSlider_volume->blockSignals(false);
}


void MusicExample::changeRhythm(CnotiAudio::EnumInstrument instrument, int variation)
{
	CnotiAudio::EnumRhythmVariation rVariation = CnotiAudio::SoundManager::convertIntToRhythmVariation(variation - 1);
//	if(rVariation != CnotiAudio::RHYTHM_UNKNOWN)
	{
		_music->changeRhythmVariation(instrument, rVariation);
	}
}

void MusicExample::resetGui()
{
	ui->listWidget->clear();

	ui->comboBox_instrument->blockSignals(true);
	ui->comboBox_instrument->setCurrentIndex(0);
	ui->comboBox_instrument->blockSignals(false);

	ui->comboBox_drum->blockSignals(true);
	ui->comboBox_drum->setCurrentIndex(0);
	ui->comboBox_drum->blockSignals(false);

	ui->comboBox_beatBox->blockSignals(true);
	ui->comboBox_beatBox->setCurrentIndex(0);
	ui->comboBox_beatBox->blockSignals(false);

	ui->comboBox_chineseBox->blockSignals(true);
	ui->comboBox_chineseBox->setCurrentIndex(0);
	ui->comboBox_chineseBox->blockSignals(false);

	ui->comboBox_congas->blockSignals(true);
	ui->comboBox_congas->setCurrentIndex(0);
	ui->comboBox_congas->blockSignals(false);

	ui->comboBox_Tamborine->blockSignals(true);
	ui->comboBox_Tamborine->setCurrentIndex(0);
	ui->comboBox_Tamborine->blockSignals(false);

	ui->comboBox_triangle->blockSignals(true);
	ui->comboBox_triangle->setCurrentIndex(0);
	ui->comboBox_triangle->blockSignals(false);
}

void MusicExample::on_pushButtonLoadMusic_clicked()
{
	resetGui();
	QString filename = QFileDialog::getOpenFileName(this, "load music", "../resources/music");
	if(_music->load(filename))
	{
		// Add notes to list
		ui->listWidget->clear();
		CnotiAudio::Note* note;
		QListIterator<CnotiAudio::Note *> it(_music->noteList());
		while(it.hasNext())
		{
			note = it.next();
			QString itemTxt = QString::number(note->getHeight());
			itemTxt.append(" [" + QString::number(note->getDuration()/32)  + "]");
			new QListWidgetItem(itemTxt, ui->listWidget);
		}
		// Update instrument
		_instrument = _music->instrument();
		// Update rhythms
		CnotiAudio::EnumInstrument inst;
		QListIterator<CnotiAudio::EnumInstrument> itR(_music->rhythmList());
		while(itR.hasNext())
		{
			inst = itR.next();
			int rVariation = _music->rhythmVariation(inst) + 1;
			if(rVariation == CnotiAudio::RHYTHM_UNKNOWN)
			{
				rVariation = 0;
			}
			switch(inst)
			{
			case CnotiAudio::BASS_DRUM:
				ui->comboBox_drum->setCurrentIndex(rVariation);
				break;
			case CnotiAudio::BEAT_BOX:
				ui->comboBox_beatBox->setCurrentIndex(rVariation);
				break;
			case CnotiAudio::CHINESE_BOX:
				ui->comboBox_chineseBox->setCurrentIndex(rVariation);
				break;
			case CnotiAudio::CONGAS:
				ui->comboBox_congas->setCurrentIndex(rVariation);
				break;
			case CnotiAudio::TAMBOURINE:
				ui->comboBox_Tamborine->setCurrentIndex(rVariation);
				break;
			case CnotiAudio::TRIANGLE:
				ui->comboBox_triangle->setCurrentIndex(rVariation);
				break;
			}
		}
	}
}

void MusicExample::on_pushButtonSaveMusic_clicked()
{
	if(!_music->isEmpty())
	{
		QString filename = QFileDialog::getSaveFileName(this, "load music", "../resources/music");
		_music->save(filename);
	}
}

void MusicExample::on_pushButtonSaveMusicWav_clicked()
{
	if(!_music->isEmpty())
	{
		QString filename = QFileDialog::getSaveFileName(this, "load music", "../resources/music");
		_music->saveWav(filename);
	}
}

void MusicExample::on_pushButtonSaveMusicMp3_clicked()
{
	if(!_music->isEmpty())
	{
		QString filename = QFileDialog::getSaveFileName(this, "load music", "../resources/music");
		_music->saveMp3(filename, 128);
	}
}


void MusicExample::on_commandLinkButton_stop_clicked()
{
	_music->stopSound();
	ui->commandLinkButton_stop->hide();
	ui->commandLinkButton_play->show();
}

void MusicExample::on_commandLinkButton_play_clicked()
{
	if(_music->playSound())
	{
		ui->commandLinkButton_stop->show();
		ui->commandLinkButton_play->hide();
	}
}

void MusicExample::selectNote(int pos)
{
	ui->listWidget->item(pos)->setBackgroundColor(QColor(255,0,0));
}

void MusicExample::unselectNote(int pos)
{
	ui->listWidget->item(pos)->setBackgroundColor(QColor(255,255,255));
}

void MusicExample::on_comboBox_instrument_currentIndexChanged(int index)
{
	CnotiAudio::EnumInstrument inst = CnotiAudio::SoundManager::convertIntToInstrument(index + 1);
	if(inst != CnotiAudio::INSTRUMENT_UNKNOWN)
	{
		selectInstrument(inst);
	}
}

void MusicExample::on_verticalSlider_volume_valueChanged(int value)
{
	setVolume(value / 10.0f);
}

void MusicExample::on_comboBox_drum_currentIndexChanged(int index)
{
	changeRhythm(CnotiAudio::BASS_DRUM, index);
}

void MusicExample::on_comboBox_beatBox_currentIndexChanged(int index)
{
	changeRhythm(CnotiAudio::BEAT_BOX, index);
}

void MusicExample::on_comboBox_chineseBox_currentIndexChanged(int index)
{
	changeRhythm(CnotiAudio::CHINESE_BOX, index);
}

void MusicExample::on_comboBox_congas_currentIndexChanged(int index)
{
	changeRhythm(CnotiAudio::CONGAS, index);
}

void MusicExample::on_comboBox_Tamborine_currentIndexChanged(int index)
{
	changeRhythm(CnotiAudio::TAMBOURINE, index);
}

void MusicExample::on_comboBox_triangle_currentIndexChanged(int index)
{
	changeRhythm(CnotiAudio::TRIANGLE, index);
}
