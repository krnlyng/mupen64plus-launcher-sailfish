#ifndef EMULATOR_THREAD_H
#define EMULATOR_THREAD_H

#include <QObject>
#include <QThread>
#include <AudioResourceQt>

#include <string>
#include <vector>

class emulator_thread : public QThread
{
    Q_OBJECT
public:
    emulator_thread();
    void stop();
    void set_game(std::string game);
private:
    std::string my_game;
    bool stopped;
    bool audio_resource_acquired;
    AudioResourceQt::AudioResource m_audio_resource;
protected:
    void run();
public slots:
    void onAcquiredChanged(void);
};

extern emulator_thread the_emulator_thread;

#endif // EMULATOR_THREAD_H
