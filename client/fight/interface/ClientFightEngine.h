#ifndef FightEngine_H
#define FightEngine_H

#include <QObject>
#include <QByteArray>
#include <QHash>
#include <QString>
#include <QPixmap>

#include "../base/GeneralStructures.h"
#include "../../general/base/Map.h"
#include "../../general/fight/CommonFightEngine.h"
#include "../base/Api_protocol.h"

namespace CatchChallenger {
//only the logique here, store nothing
class ClientFightEngine : public CommonFightEngine
{
    Q_OBJECT
public:
    struct MonsterSkillEffect
    {
        quint32 skill;
    };
    static ClientFightEngine fightEngine;
    virtual void resetAll();
    bool isInFight() const;
    //current fight
    QList<PublicPlayerMonster> battleCurrentMonster;
    QList<quint8> battleStat,botMonstersStat;
    QList<quint8> battleMonsterPlace;
    QList<quint32> otherMonsterAttack;
    bool dropKOOtherMonster();
    void fightFinished();
    void setBattleMonster(const QList<quint8> &stat,const quint8 &monsterPlace,const PublicPlayerMonster &publicPlayerMonster);
    void setBotMonster(const QList<PlayerMonster> &publicPlayerMonster);
    void addBattleMonster(const quint8 &monsterPlace,const PublicPlayerMonster &publicPlayerMonster);
    bool haveWin();
    void addAndApplyAttackReturnList(const QList<Skill::AttackReturn> &attackReturnList);
    QList<Skill::AttackReturn> getAttackReturnList() const;
    void removeTheFirstLifeEffectAttackReturn();
    bool firstLifeEffectQuantityChange(qint32 quantity);
    virtual PublicPlayerMonster *getOtherMonster() const;
    quint8 getOtherSelectedMonsterNumber() const;
    void setVariable(Player_private_and_public_informations player_informations_local);
    Skill::AttackReturn generateOtherAttack();
    bool useBattleSkill(const quint32 &skill,const quint8 &skillLevel);
    bool isInBattle() const;
private:
    QList<Skill::AttackReturn> attackReturnList;
    Player_private_and_public_informations player_informations_local;
    Skill::AttackReturn doTheCurrentMonsterAttack(const quint32 &skill, const quint8 &skillLevel);
    bool applyCurrentLifeEffectReturn(const Skill::LifeEffectReturn &effectReturn);
    bool internalTryEscape();
    void addXPSP();
private:
    explicit ClientFightEngine();
    ~ClientFightEngine();
signals:
    void newError(QString error,QString detailedError) const;
};
}

#endif // FightEngine_H