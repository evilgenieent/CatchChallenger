#include "MapVisualiser.h"

#include <QCoreApplication>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QStyleOptionGraphicsItem>
#include <QTime>
#include <QDebug>
#include <QFileInfo>
#include <QPointer>

#include "../../general/base/MoveOnTheMap.h"

MapVisualiser::MapVisualiser(QWidget *parent) :
    QGraphicsView(parent),
    mScene(new QGraphicsScene(this)),
    inMove(false)
{
    xPerso=0;
    yPerso=0;

    lookToMove.setInterval(200);
    lookToMove.setSingleShot(true);
    connect(&lookToMove,SIGNAL(timeout()),this,SLOT(transformLookToMove()));

    moveTimer.setInterval(66);
    moveTimer.setSingleShot(true);
    connect(&moveTimer,SIGNAL(timeout()),this,SLOT(moveStepSlot()));
    setWindowTitle(tr("map-visualiser-qt"));
    //scale(2,2);

    setScene(mScene);
/*    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setDragMode(QGraphicsView::ScrollHandDrag);*/
    setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing
                         | QGraphicsView::DontSavePainterState);
    setBackgroundBrush(Qt::black);
    setFrameStyle(QFrame::NoFrame);

    //viewport()->setAttribute(Qt::WA_StaticContents);
    //setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
}

MapVisualiser::~MapVisualiser()
{
    //remove the not used map
    QHash<QString,Map_full *>::const_iterator i = other_map.constBegin();
    while (i != other_map.constEnd()) {
        delete (*i)->logicalMap.parsed_layer.walkable;
        delete (*i)->logicalMap.parsed_layer.water;
        qDeleteAll((*i)->tiledMap->tilesets());
        delete (*i)->tiledMap;
        delete (*i)->tiledRender;
        delete (*i);
        other_map.remove((*i)->logicalMap.map_file);
        i = other_map.constBegin();//needed
    }

    //delete mapItem;
    delete playerTileset;
    //delete playerMapObject;
    delete tagTileset;
}

void MapVisualiser::viewMap(const QString &fileName)
{
    current_map=NULL;

    QTime startTime;
    startTime.restart();

    mapItem=new MapItem();

    playerTileset = new Tiled::Tileset("player",16,24);
    playerTileset->loadFromImage(QImage(":/player_skin.png"),":/player_skin.png");
    playerMapObject = new Tiled::MapObject();

    tagTilesetIndex=0;
    tagTileset = new Tiled::Tileset("tags",16,16);
    tagTileset->loadFromImage(QImage(":/tags.png"),":/tags.png");

    //commented to not blink
    //blink_dyna_layer.start(200);
    connect(&blink_dyna_layer,SIGNAL(timeout()),this,SLOT(blinkDynaLayer()));

    mScene->clear();
    //centerOn(0, 0);

    QString current_map_fileName=loadOtherMap(fileName);
    if(current_map_fileName.isEmpty())
        return;
    current_map=other_map[current_map_fileName];
    other_map.remove(current_map_fileName);

    //the direction
    direction=Pokecraft::Direction_look_at_bottom;
    playerMapObject->setTile(playerTileset->tileAt(7));

    //position
    if(!current_map->logicalMap.rescue_points.empty())
    {
        xPerso=current_map->logicalMap.rescue_points.first().x;
        yPerso=current_map->logicalMap.rescue_points.first().y;
    }
    else if(!current_map->logicalMap.bot_spawn_points.empty())
    {
        xPerso=current_map->logicalMap.bot_spawn_points.first().x;
        yPerso=current_map->logicalMap.bot_spawn_points.first().y;
    }
    else
    {
        xPerso=current_map->logicalMap.width/2;
        yPerso=current_map->logicalMap.height/2;
    }

    loadCurrentMap();
    playerMapObject->setPosition(QPoint(xPerso,yPerso+1));

    qDebug() << startTime.elapsed();
    mScene->addItem(mapItem);
    //mScene->setSceneRect(QRectF(xPerso*TILE_SIZE,yPerso*TILE_SIZE,64,32));

    viewport()->update();
}

bool MapVisualiser::RectTouch(QRect r1,QRect r2)
{
    if (r1.isNull() || r2.isNull())
        return false;

    if((r1.x()+r1.width())<r2.x())
        return false;
    if((r2.x()+r2.width())<r1.x())
        return false;

    if((r1.y()+r1.height())<r2.y())
        return false;
    if((r2.y()+r2.height())<r1.y())
        return false;

    return true;
}

void MapVisualiser::displayTheDebugMap()
{
    /*qDebug() << QString("xPerso: %1, yPerso: %2, map: %3").arg(xPerso).arg(yPerso).arg(current_map->logicalMap.map_file);
    int y=0;
    while(y<current_map->logicalMap.height)
    {
        QString line;
        int x=0;
        while(x<current_map->logicalMap.width)
        {
            if(x==xPerso && y==yPerso)
                line+="P";
            else if(current_map->logicalMap.parsed_layer.walkable[x+y*current_map->logicalMap.width])
                line+="_";
            else
                line+="X";
            x++;
        }
        qDebug() << line;
        y++;
    }*/
}