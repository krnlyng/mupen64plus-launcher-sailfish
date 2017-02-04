/*
  Copyright (C) 2013 Jolla Ltd.
  Contact: Thomas Perl <thomas.perl@jollamobile.com>
  All rights reserved.

  You may use this file under the terms of BSD license as follows:

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Jolla Ltd nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR
  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif

#include <QObject>
#include <QUrl>
#include <QVariantList>
#include <QScreen>

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

#include <sailfishapp.h>

#include "gamelauncher.h"
#include "recents_handler.h"
#include "settingsmanager.h"

#include "core_interface.h"
#include "osal_dynamiclib.h"
#include "m64p_types.h"
#include "debug.h"
#include "osal_preproc.h"
#include "version.h"

#include "emulator_thread.h"

#define MAX_RECENTS 25

using namespace std;

#ifdef __cplusplus
extern "C"
{
#endif
int g_Verbose = 0;
#ifdef __cplusplus
}
#endif
void DebugMessage(int level, const char *message, ...)
{
  char msgbuf[1024];
  va_list args;

  va_start(args, message);
  vsnprintf(msgbuf, 1024, message, args);

  DebugCallback((void*)"Mupen64plus Launcher", level, msgbuf);

  va_end(args);
}

void DebugCallback(void *Context, int level, const char *message)
{
    char output[4096];

    if (level == M64MSG_ERROR) {
        sprintf(output, "%s Error: %s\n", (const char *) Context, message);

        qDebug(output);
    }
    else if (level == M64MSG_WARNING) {
        sprintf(output, "%s Warning: %s\n", (const char *) Context, message);

        qDebug(output);
    }
    else if (level == M64MSG_INFO) {
        sprintf(output, "%s: %s\n", (const char *) Context, message);

        qDebug(output);
    }
    else if (level == M64MSG_STATUS) {
        sprintf(output, "%s Status: %s\n", (const char *) Context, message);

        qDebug(output);
    }
    else if (level == M64MSG_VERBOSE) {
        if (g_Verbose) {
            sprintf(output, "%s: %s\n", (const char *) Context, message);

            qDebug(output);
        }
    }
    else {
        sprintf(output, "%s Unknown: %s\n", (const char *) Context, message);

        qDebug(output);
    }
}

int main(int argc, char *argv[])
{
    // SailfishApp::main() will display "qml/template.qml", if you need more
    // control over initialization, you can use:
    //
    //   - SailfishApp::application(int, char *[]) to get the QGuiApplication *
    //   - SailfishApp::createView() to get a new QQuickView * instance
    //   - SailfishApp::pathTo(QString) to get a QUrl to a resource file
    //
    // To display the view, call "show()" (will show fullscreen on device).

    qmlRegisterType<GameLauncher>("GameLauncher", 1, 0, "GameLauncher");
    qmlRegisterType<RecentsHandler>("RecentsHandler", 1, 0, "RecentsHandler");
    qmlRegisterType<SettingsManager>("SettingsManager", 1, 0, "SettingsManager");

    m64p_handle config_section;
    m64p_handle video_general_section;
    m64p_error err;

    err = AttachCoreLib(NULL);

    qDebug(("attached core lib, return code: " + to_string(err)).c_str());

    err = CoreStartup(CORE_API_VERSION, NULL, NULL, (void*)"Core", DebugCallback, NULL, NULL);

    qDebug(("started core lib, return code: " + to_string(err)).c_str());

    err = ConfigOpenSection("mupen64plus-launcher-sailfish", &config_section);

    qDebug(("config section opened: " + to_string(err)).c_str());

    err = ConfigOpenSection("Video-General", &video_general_section);

    qDebug(("config section opened: " + to_string(err)).c_str());

    QGuiApplication *app = SailfishApp::application(argc, argv);
    QQuickView *view = SailfishApp::createView();

    QScreen *screen = QGuiApplication::screens()[0];
    int height = screen->geometry().height();
    int width = screen->geometry().width();

    ConfigSetDefaultBool(video_general_section, "Fullscreen", true, "enable fullscreen");
    ConfigSetDefaultBool(video_general_section, "VerticalSync", true, "enable vertical sync");
    ConfigSetDefaultInt(video_general_section, "ScreenHeight", height, "Screen height");
    ConfigSetDefaultInt(video_general_section, "ScreenWidth", width, "Screen width");
    ConfigSetDefaultInt(video_general_section, "Rotate", 3, "Rotate (3 is landscape, 0 is portrait)");

    ConfigSetDefaultString(config_section, "VideoPlugin", "mupen64plus-video-n64" OSAL_DLL_EXTENSION, "The video plugin");
    ConfigSetDefaultString(config_section, "AudioPlugin", "mupen64plus-audio-sdl" OSAL_DLL_EXTENSION, "The audio plugin");
    ConfigSetDefaultString(config_section, "InputPlugin", "mupen64plus-input-sdltouch" OSAL_DLL_EXTENSION, "The input plugin");
    ConfigSetDefaultString(config_section, "RspPlugin", "mupen64plus-rsp-hle" OSAL_DLL_EXTENSION, "The rsp plugin");
    ConfigSetDefaultString(config_section, "PluginPath", "/usr/lib/mupen64plus", "The plugin path");

    for(unsigned int i=0;i<MAX_RECENTS;i++)
    {
        string game = ConfigGetParamString(config_section, (const char*)((string)"Recent" + to_string(i)).c_str());
        if(game != "")
        {
            qDebug(game.c_str());
            recent_games.push_back(game.c_str());
        }
    }

    qDebug() << "saving and detaching core";
    ConfigSaveSection("mupen64plus-launcher-sailfish");
    ConfigSaveSection("Video-General");
    ConfigSaveFile();
    CoreShutdown();
    DetachCoreLib();

    view->setSource(SailfishApp::pathTo("qml/harbour-mupen64plus-gui-sailfishos.qml"));
    view->showFullScreen();
    int ret = app->exec();

    // stop the thread
    the_emulator_thread.stop();

    the_emulator_thread.wait();

    if(ConfigSaveSection == NULL)
    {
        err = AttachCoreLib(NULL);

        qDebug(("attached core lib, return code: " + to_string(err)).c_str());

        err = CoreStartup(CORE_API_VERSION, NULL, NULL, (void*)"Core", DebugCallback, NULL, NULL);

        qDebug(("started core lib, return code: " + to_string(err)).c_str());
    }
    else
    {
        qDebug("core was not previously detached. might happen because the app was closed within a sub page");
    }

    err = ConfigOpenSection("mupen64plus-launcher-sailfish", &config_section);

    qDebug(("config section opened: " + to_string(err)).c_str());

    for(unsigned int i=0;i<recent_games.size();i++)
    {
        ConfigSetParameter(config_section, (const char*)((string)"Recent" + to_string(i)).c_str(), M64TYPE_STRING, (const void*)(recent_games[i].c_str()));
    }

    for(unsigned int i=recent_games.size();i<MAX_RECENTS;i++)
    {
        ConfigSetParameter(config_section, (const char*)((string)"Recent" + to_string(i)).c_str(), M64TYPE_STRING, "");
    }

    qDebug() << "saving and detaching core";
    ConfigSaveSection("mupen64plus-launcher-sailfish");
    ConfigSaveFile();
    CoreShutdown();
    DetachCoreLib();
    return ret;
}
