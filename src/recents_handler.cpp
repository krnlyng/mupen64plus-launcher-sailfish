#include <string>
#include <vector>
#include <algorithm>

#include "recents_handler.h"

using namespace std;

vector<string> recent_games;

RecentsHandler::RecentsHandler()
{
}

QVariantList RecentsHandler::getRecentGames()
{
    unsigned int i = 0;
    QVariantList games;

    for(vector<string>::iterator it=recent_games.begin();it!=recent_games.end();it++)
    {
        games.append(QUrl::fromUserInput(QString::fromStdString(*it)));
        i++;
    }

    return games;
}

qint32 RecentsHandler::addRecentGame(QUrl game)
{
    QByteArray arr = game.toString().toLatin1();
    string tmp(arr.constData(), arr.length());

    vector<string>::iterator it = find(recent_games.begin(), recent_games.end(), tmp);

    if(it != recent_games.end())
    {
        recent_games.erase(it);
    }

    recent_games.insert(recent_games.begin(), tmp);

    return 0;
}

qint32 RecentsHandler::removeRecentGame(QUrl game)
{
    QByteArray arr = game.toString().toLatin1();
    string tmp(arr.constData(), arr.length());

    vector<string>::iterator it = find(recent_games.begin(), recent_games.end(), tmp);

    if(it != recent_games.end())
    {
        qDebug("Removed recent game");
        recent_games.erase(it);
        return 0;
    }

    qDebug("Didn't remove recent game");
    return -1;
}
