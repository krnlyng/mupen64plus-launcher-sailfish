#include <fstream>

#include "gamelauncher.h"

#include "debug.h"
#include "core_interface.h"
#include "osal_dynamiclib.h"
#include "emulator_thread.h"

using namespace std;

GameLauncher::GameLauncher()
{

}

qint32 GameLauncher::launchGame(QUrl fileurl)
{
    the_emulator_thread.set_game(fileurl.path().toStdString());
    the_emulator_thread.start();
}

qint32 GameLauncher::checkActive()
{
    return the_emulator_thread.isRunning();
}
