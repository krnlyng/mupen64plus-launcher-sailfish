#ifndef RECENTS_HANDLER_H
#define RECENTS_HANDLER_H

#include <QObject>
#include <QVector>
#include <QUrl>
#include <QVariantList>

#include <string>
#include <vector>

extern std::vector<std::string> recent_games;

class RecentsHandler : public QObject
{
    Q_OBJECT
public:
    RecentsHandler();
public slots:
    qint32 addRecentGame(QUrl game);

    QVariantList getRecentGames();
    QUrl getMostRecent();

    qint32 removeRecentGame(QUrl game);
};

#endif // RECENTS_HANDLER_H
