#include "CommonDatapack.h"
#include "GeneralVariable.h"
#include "FacilityLib.h"
#include "../fight/FightLoader.h"
#include "DatapackGeneralLoader.h"

#include <QDebug>
#include <QFile>
#include <QDomElement>
#include <QDomDocument>
#include <QByteArray>
#include <QDir>
#include <QFileInfoList>
#include <QMutexLocker>

using namespace CatchChallenger;

CommonDatapack CommonDatapack::commonDatapack;

CommonDatapack::CommonDatapack()
{
    isParsed=false;
}

void CommonDatapack::parseDatapack(const QString &datapackPath)
{
    QMutexLocker mutexLocker(&inProgress);
    if(isParsed)
        return;
    this->datapackPath=datapackPath;
    parseItems();
    parsePlants();
    parseCraftingRecipes();
    parseBuff();
    parseSkills();
    parseMonsters();
    parseReputation();
    parseQuests();
    parseBotFights();
    isParsed=true;
}

void CommonDatapack::parseItems()
{
    items=DatapackGeneralLoader::loadItems(datapackPath+DATAPACK_BASE_PATH_ITEM);
    qDebug() << QString("%1 items(s) loaded").arg(items.size());
}

void CommonDatapack::parseCraftingRecipes()
{
    QPair<QHash<quint32,CrafingRecipe>,QHash<quint32,quint32> > multipleVariables=DatapackGeneralLoader::loadCraftingRecipes(datapackPath+DATAPACK_BASE_PATH_CRAFTING+"recipes.xml",items);
    crafingRecipes=multipleVariables.first;
    itemToCrafingRecipes=multipleVariables.second;
    qDebug() << QString("%1 crafting recipe(s) loaded").arg(crafingRecipes.size());
}

void CommonDatapack::parsePlants()
{
    plants=DatapackGeneralLoader::loadPlants(datapackPath+DATAPACK_BASE_PATH_PLANTS+"plants.xml");
    qDebug() << QString("%1 plant(s) loaded").arg(plants.size());
}

void CommonDatapack::parseQuests()
{
    quests=DatapackGeneralLoader::loadQuests(datapackPath+DATAPACK_BASE_PATH_QUESTS);
    qDebug() << QString("%1 quest(s) loaded").arg(quests.size());
}

void CommonDatapack::parseReputation()
{
    reputation=DatapackGeneralLoader::loadReputation(datapackPath+DATAPACK_BASE_PATH_PLAYER+"reputation.xml");
    qDebug() << QString("%1 reputation(s) loaded").arg(reputation.size());
}

void CommonDatapack::parseBuff()
{
    monsterBuffs=FightLoader::loadMonsterBuff(datapackPath+DATAPACK_BASE_PATH_MONSTERS+"buff.xml");
    qDebug() << QString("%1 monster buff(s) loaded").arg(monsterBuffs.size());
}

void CommonDatapack::parseSkills()
{
    monsterSkills=FightLoader::loadMonsterSkill(datapackPath+DATAPACK_BASE_PATH_MONSTERS+"skill.xml",monsterBuffs);
    qDebug() << QString("%1 monster skill(s) loaded").arg(monsterSkills.size());
}

void CommonDatapack::parseMonsters()
{
    monsters=FightLoader::loadMonster(datapackPath+DATAPACK_BASE_PATH_MONSTERS+"monster.xml",monsterSkills);
    qDebug() << QString("%1 monster(s) loaded").arg(monsters.size());
}

void CommonDatapack::parseBotFights()
{
    botFights=FightLoader::loadFight(datapackPath+DATAPACK_BASE_PATH_FIGHT, monsters, monsterSkills);
    qDebug() << QString("%1 bot fight(s) loaded").arg(botFights.size());
}

void CommonDatapack::unload()
{
    QMutexLocker mutexLocker(&inProgress);
    if(!isParsed)
        return;
    botFights.clear();
    plants.clear();
    crafingRecipes.clear();
    reputation.clear();
    quests.clear();
    monsters.clear();
    monsterSkills.clear();
    monsterBuffs.clear();
    itemToCrafingRecipes.clear();
    items.clear();
    isParsed=false;
}
