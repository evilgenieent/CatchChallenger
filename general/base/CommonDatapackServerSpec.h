#ifndef COMMONDATAPACK_ServerSpec_H
#define COMMONDATAPACK_ServerSpec_H

#include <QObject>
#include <QMutex>
#include <QHash>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QMultiHash>

#include "GeneralStructures.h"

namespace CatchChallenger {
class CommonDatapackServerSpec
        #ifndef EPOLLCATCHCHALLENGERSERVER
        : public QObject
        #endif
{
#ifndef EPOLLCATCHCHALLENGERSERVER
Q_OBJECT
#endif
public:
    explicit CommonDatapackServerSpec();
public:
    void unload();
    void parseDatapack(const QString &datapackPath);
public:
    QHash<quint16,BotFight> botFights;
    QHash<quint16,Quest> quests;
    QList<MonstersCollision> monstersCollision;
    QHash<quint32,Shop> shops;
    QList<ServerProfile> serverProfileList;
    static CommonDatapackServerSpec commonDatapackServerSpec;
private:
    QMutex inProgressSpec;
    bool isParsedSpec;
    QString datapackPath;
private:
    void parseQuests();
    void parseBotFights();
    void parseMonstersCollision();
    void parseShop();
    void parseServerProfileList();
};
}


#endif // COMMONDATAPACK_ServerSpec_H