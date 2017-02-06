#ifndef GAMELAUNCHER_H
#define GAMELAUNCHER_H

#include <QProcess>
#include <QObject>
#include <QUrl>

#include "core_interface.h"

extern m64p_dynlib_handle video_plugin;
extern m64p_dynlib_handle audio_plugin;
extern m64p_dynlib_handle rsp_plugin;
extern m64p_dynlib_handle input_plugin;

class GameLauncher : public QObject
{
    Q_OBJECT
public:
    GameLauncher();
public slots:
    qint32 launchGame(QUrl str);
    qint32 checkActive();
    qint32 stopGame();
};

#endif // GAMELAUNCHER_H
