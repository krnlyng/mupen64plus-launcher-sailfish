#ifndef EMULATOR_THREAD_H
#define EMULATOR_THREAD_H

#include <QObject>
#include <QThread>
#include <AudioResourceQt>

#include <QWindow>
#include <QOpenGLContext>
#include <QProcess>

#include <string>
#include <vector>

#include "core_interface.h"

#define ATTACH_TO_CORE 0

class emulator_thread : public QThread
{
    Q_OBJECT
public:
    emulator_thread();
    void stop();
    void set_game(std::string game);
private:
    std::string my_game;
    QProcess *game;
    bool stopped;
#if ATTACH_TO_CORE
    bool audio_resource_acquired;
    AudioResourceQt::AudioResource m_audio_resource;
#endif
protected:
    void run();
#if ATTACH_TO_CORE
public slots:
    void onAcquiredChanged(void);
#endif
};

extern emulator_thread the_emulator_thread;

#endif // EMULATOR_THREAD_H
