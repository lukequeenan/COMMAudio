#ifndef AUDIOCOMPONENT_H
#define AUDIOCOMPONENT_H

#include <QObject>
#include <Phonon>
#include <QDir>
#include <QAudioInput>
#include <QAudioOutput>
#include <QBuffer>

class AudioComponent : public QObject
{
    Q_OBJECT
public:
    explicit AudioComponent(QObject *parent = 0);

    Phonon::MediaObject* getPlaylist(){
        return playlist_;
    }

    void addSong(QString filename);
    void setSourceFolder();
    QDir getSourceFolder(){
        return sourceFolder_;
    }
    QStringList getFileList();
    void startMic();
    QList<Phonon::MediaSource> getQueue();
    void addSongToBegining(QString filename);
    void setCurrentSong(QString fileName);
    Phonon::State getState();

private:
    Phonon::MediaObject* playlist_;
    Phonon::AudioOutput* output_;
    QDir sourceFolder_;


    QAudioInput* input_;
    QFile outputFile;
    QAudioFormat format;
    QBuffer* buffer_;
    QBuffer* inputBuffer_;
    QList<QBuffer*> allBuffers_;
signals:

public slots:
void play();
void pause();
void stop();
void stopMic();

};

#endif // AUDIOCOMPONENT_H
