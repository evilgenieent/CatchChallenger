#include "BotTargetList.h"
#include "ui_BotTargetList.h"
#include "../../client/base/interface/DatapackClientLoader.h"

BotTargetList::BotTargetList(QHash<CatchChallenger::Api_client_real *,MultipleBotConnection::CatchChallengerClient *> apiToCatchChallengerClient,
                             QHash<CatchChallenger::ConnectedSocket *,MultipleBotConnection::CatchChallengerClient *> connectedSocketToCatchChallengerClient,
                             QHash<QSslSocket *,MultipleBotConnection::CatchChallengerClient *> sslSocketToCatchChallengerClient,
                             ActionsAction *actionsAction) :
    ui(new Ui::BotTargetList),
    apiToCatchChallengerClient(apiToCatchChallengerClient),
    connectedSocketToCatchChallengerClient(connectedSocketToCatchChallengerClient),
    sslSocketToCatchChallengerClient(sslSocketToCatchChallengerClient),
    actionsAction(actionsAction),
    botsInformationLoaded(false)
{
    ui->setupUi(this);
    mapId=0;

    QHash<CatchChallenger::Api_client_real *,MultipleBotConnection::CatchChallengerClient *>::const_iterator i = apiToCatchChallengerClient.constBegin();
    while (i != apiToCatchChallengerClient.constEnd()) {
        MultipleBotConnection::CatchChallengerClient * const client=i.value();
        if(client->have_informations)
        {
            const CatchChallenger::Player_private_and_public_informations &player_private_and_public_informations=client->api->get_player_informations();
            const QString &qtpseudo=QString::fromStdString(player_private_and_public_informations.public_informations.pseudo);
            ui->bots->addItem(qtpseudo);
            pseudoToBot[qtpseudo]=client;
        }
        ++i;
    }
}

BotTargetList::~BotTargetList()
{
    delete ui;
}

void BotTargetList::loadAllBotsInformation()
{
    if(botsInformationLoaded)
        return;
    botsInformationLoaded=true;
    if(!actionsAction->preload_the_map())
        return;
    if(!actionsAction->preload_the_map_step1())
        return;
}

void BotTargetList::on_bots_itemSelectionChanged()
{
    const QList<QListWidgetItem*> &selectedItems=ui->bots->selectedItems();
    if(selectedItems.size()!=1)
        return;
    const QString &pseudo=selectedItems.at(0)->text();
    if(!pseudoToBot.contains(pseudo))
        return;
    MultipleBotConnection::CatchChallengerClient * client=pseudoToBot.value(pseudo);
    if(!actionsAction->clientList.contains(client->api))
        return;

    ui->localTargets->clear();
    ui->label_local_target->setEnabled(true);
    ui->comboBoxStep->setEnabled(true);
    ui->globalTargets->setEnabled(true);

    const ActionsBotInterface::Player &player=actionsAction->clientList.value(client->api);
    mapId=player.mapId;

    updateMapInformation();
}

void BotTargetList::updateMapInformation()
{
    const QList<QListWidgetItem*> &selectedItems=ui->bots->selectedItems();
    if(selectedItems.size()!=1)
        return;
    const QString &pseudo=selectedItems.at(0)->text();
    if(!pseudoToBot.contains(pseudo))
        return;
    MultipleBotConnection::CatchChallengerClient * client=pseudoToBot.value(pseudo);
    if(!actionsAction->clientList.contains(client->api))
        return;

    QList<QColor> brushList;
    brushList << QColor(200, 70, 70, 255);
    brushList << QColor(255, 255, 0, 255);
    brushList << QColor(70, 187, 70, 255);
    brushList << QColor(100, 100, 200, 255);
    brushList << QColor(255, 128, 128, 255);
    brushList << QColor(180, 70, 180, 255);

    const ActionsBotInterface::Player &player=actionsAction->clientList.value(client->api);

    if(actionsAction->id_map_to_map.find(mapId)!=actionsAction->id_map_to_map.cend())
    {
        const std::string &mapStdString=actionsAction->id_map_to_map.at(mapId);
        CatchChallenger::CommonMap *map=actionsAction->map_list.at(mapStdString);
        MapServerMini *mapServer=static_cast<MapServerMini *>(map);

        if(actionsAction->id_map_to_map.find(player.mapId)!=actionsAction->id_map_to_map.cend())
        {
            const std::string &playerMapStdString=actionsAction->id_map_to_map.at(mapId);
            CatchChallenger::CommonMap *playerMap=actionsAction->map_list.at(playerMapStdString);
            QString mapString=QString::fromStdString(playerMap->map_file)+QString(" (%1,%2)").arg(player.x).arg(player.y);
            ui->label_local_target->setTitle("Target on the map: "+mapString+", displayed map: "+QString::fromStdString(mapStdString));
        }
        else
            ui->label_local_target->setTitle("Unknown player map ("+QString::number(player.mapId)+")");

        ui->mapPreview->setColumnCount(0);
        ui->mapPreview->setRowCount(0);
        ui->mapPreview->setColumnCount(mapServer->width);
        ui->mapPreview->setRowCount(mapServer->height);

        {
            int y=0;
            while(y<mapServer->height)
            {
                int x=0;
                while(x<mapServer->width)
                {
                    //color
                    int codeZone=mapServer->step1.map[x+y*mapServer->width];
                    if(codeZone>0)
                    {
                        QBrush brush1(brushList[codeZone%brushList.size()]);
                        brush1.setStyle(Qt::SolidPattern);
                        QTableWidgetItem *tablewidgetitem = ui->mapPreview->itemAt(y,x);
                        if(tablewidgetitem==NULL)
                            tablewidgetitem = new QTableWidgetItem();
                        tablewidgetitem->setBackground(brush1);
                        tablewidgetitem->setText(QString::number(codeZone));
                        ui->mapPreview->setItem(y,x,tablewidgetitem);
                    }
                    //icon
                    if(x==player.x && y==player.y && player.mapId==mapId)
                    {
                        QIcon icon;
                        icon.addFile(QStringLiteral(":/playerloc.png"), QSize(), QIcon::Normal, QIcon::Off);
                        QTableWidgetItem *tablewidgetitem = ui->mapPreview->itemAt(y,x);
                        if(tablewidgetitem==NULL)
                            tablewidgetitem = new QTableWidgetItem();
                        tablewidgetitem->setText("");
                        tablewidgetitem->setIcon(icon);
                        ui->mapPreview->setItem(y,x,tablewidgetitem);
                    }

                    x++;
                }
                y++;
            }
        }
        {
            ui->comboBox_Layer->clear();
            unsigned int index=0;
            while(index<mapServer->step1.layers.size())
            {
                const MapServerMini::MapParsedForBot::Layer &layer=mapServer->step1.layers.at(index);
                ui->comboBox_Layer->addItem(QString::fromStdString(layer.name),index);
                index++;
            }
        }
    }
    else
        ui->label_local_target->setTitle("Unknown map ("+QString::number(mapId)+")");
    updateLayerElements();
}

void BotTargetList::updateLayerElements()
{
    ui->localTargets->clear();
    const QList<QListWidgetItem*> &selectedItems=ui->bots->selectedItems();
    if(selectedItems.size()!=1)
        return;
    QListWidgetItem* selectedItem=selectedItems.at(0);
    const QString &pseudo=selectedItem->text();
    if(!pseudoToBot.contains(pseudo))
        return;
    MultipleBotConnection::CatchChallengerClient * client=pseudoToBot.value(pseudo);
    if(!actionsAction->clientList.contains(client->api))
        return;
    if(ui->comboBox_Layer->count()==0)
        return;

    const ActionsBotInterface::Player &player=actionsAction->clientList.value(client->api);
    QString mapString="Unknown map ("+QString::number(mapId)+")";
    if(actionsAction->id_map_to_map.find(mapId)==actionsAction->id_map_to_map.cend())
        return;
    const std::string &mapStdString=actionsAction->id_map_to_map.at(mapId);
    mapString=QString::fromStdString(mapStdString)+QString(" (%1,%2)").arg(player.x).arg(player.y);
    CatchChallenger::CommonMap *map=actionsAction->map_list.at(mapStdString);
    MapServerMini *mapServer=static_cast<MapServerMini *>(map);
    int index=0;
    while(index<mapServer->teleporter_list_size)
    {
        const CatchChallenger::CommonMap::Teleporter &teleporter=mapServer->teleporter[index];
        const uint8_t &codeZone=mapServer->step1.map[teleporter.source_x+teleporter.source_y*mapServer->width];

        if(codeZone>0 && (codeZone-1)==ui->comboBox_Layer->currentIndex())
        {
            QListWidgetItem *item=new QListWidgetItem();
            item->setText(QString("From (%1,%2) to %3 (%4,%5)")
                          .arg(teleporter.source_x)
                          .arg(teleporter.source_y)
                          .arg(QString::fromStdString(teleporter.map->map_file))
                          .arg(teleporter.destination_x)
                          .arg(teleporter.destination_y)
                          );
            item->setIcon(QIcon(":/7.png"));
            ui->localTargets->addItem(item);
        }
        index++;
    }
    if(ui->comboBox_Layer->currentIndex()==0)
    {
        if(mapServer->border.top.map!=NULL)
        {
            QListWidgetItem *item=new QListWidgetItem();
            item->setText(QString("Top border %1 (offset: %2)")
                          .arg(QString::fromStdString(mapServer->border.top.map->map_file))
                          .arg(mapServer->border.top.x_offset)
                          );
            item->setIcon(QIcon(":/7.png"));
            ui->localTargets->addItem(item);
        }
        if(mapServer->border.right.map!=NULL)
        {
            QListWidgetItem *item=new QListWidgetItem();
            item->setText(QString("Right border %1 (offset: %2)")
                          .arg(QString::fromStdString(mapServer->border.right.map->map_file))
                          .arg(mapServer->border.right.y_offset)
                          );
            item->setIcon(QIcon(":/7.png"));
            ui->localTargets->addItem(item);
        }
        if(mapServer->border.bottom.map!=NULL)
        {
            QListWidgetItem *item=new QListWidgetItem();
            item->setText(QString("Botton border %1 (offset: %2)")
                          .arg(QString::fromStdString(mapServer->border.bottom.map->map_file))
                          .arg(mapServer->border.bottom.x_offset)
                          );
            item->setIcon(QIcon(":/7.png"));
            ui->localTargets->addItem(item);
        }
        if(mapServer->border.left.map!=NULL)
        {
            QListWidgetItem *item=new QListWidgetItem();
            item->setText(QString("Left border %1 (offset: %2)")
                          .arg(QString::fromStdString(mapServer->border.left.map->map_file))
                          .arg(mapServer->border.left.y_offset)
                          );
            item->setIcon(QIcon(":/7.png"));
            ui->localTargets->addItem(item);
        }
    }
    const MapServerMini::MapParsedForBot::Layer &layer=mapServer->step1.layers.at(ui->comboBox_Layer->currentIndex());
    ui->label_zone->setText(QString::fromStdString(layer.text));
}

void BotTargetList::on_comboBox_Layer_activated(int index)
{
    (void)index;
    updateLayerElements();
}

void BotTargetList::on_localTargets_itemActivated(QListWidgetItem *item)
{
    (void)item;

    const QList<QListWidgetItem*> &selectedItems=ui->bots->selectedItems();
    if(selectedItems.size()!=1)
        return;
    QListWidgetItem* selectedItem=selectedItems.at(0);
    const QString &pseudo=selectedItem->text();
    if(!pseudoToBot.contains(pseudo))
        return;
    MultipleBotConnection::CatchChallengerClient * client=pseudoToBot.value(pseudo);
    if(!actionsAction->clientList.contains(client->api))
        return;
    if(ui->comboBox_Layer->count()==0)
        return;

    const ActionsBotInterface::Player &player=actionsAction->clientList.value(client->api);
    QString mapString="Unknown map ("+QString::number(mapId)+")";
    if(actionsAction->id_map_to_map.find(mapId)==actionsAction->id_map_to_map.cend())
        return;
    const std::string &mapStdString=actionsAction->id_map_to_map.at(mapId);
    mapString=QString::fromStdString(mapStdString)+QString(" (%1,%2)").arg(player.x).arg(player.y);
    CatchChallenger::CommonMap *map=actionsAction->map_list.at(mapStdString);
    MapServerMini *mapServer=static_cast<MapServerMini *>(map);
    int elementCount=0;
    int index=0;
    while(index<mapServer->teleporter_list_size)
    {
        const CatchChallenger::CommonMap::Teleporter &teleporter=mapServer->teleporter[index];
        const uint8_t &codeZone=mapServer->step1.map[teleporter.source_x+teleporter.source_y*mapServer->width];

        if(codeZone>0 && (codeZone-1)==ui->comboBox_Layer->currentIndex())
        {
            if(ui->localTargets->currentRow()==elementCount)
            {
                mapId=teleporter.map->id;
                updateMapInformation();
                return;
            }
            elementCount++;
        }
        index++;
    }
    if(ui->comboBox_Layer->currentIndex()==0)
    {
        if(mapServer->border.top.map!=NULL)
        {
            if(ui->localTargets->currentRow()==elementCount)
            {
                mapId=mapServer->border.top.map->id;
                updateMapInformation();
                return;
            }
            elementCount++;
        }
        if(mapServer->border.right.map!=NULL)
        {
            if(ui->localTargets->currentRow()==elementCount)
            {
                mapId=mapServer->border.right.map->id;
                updateMapInformation();
                return;
            }
            elementCount++;
        }
        if(mapServer->border.bottom.map!=NULL)
        {
            if(ui->localTargets->currentRow()==elementCount)
            {
                mapId=mapServer->border.bottom.map->id;
                updateMapInformation();
                return;
            }
            elementCount++;
        }
        if(mapServer->border.left.map!=NULL)
        {
            if(ui->localTargets->currentRow()==elementCount)
            {
                mapId=mapServer->border.left.map->id;
                updateMapInformation();
                return;
            }
            elementCount++;
        }
    }
    abort();
}