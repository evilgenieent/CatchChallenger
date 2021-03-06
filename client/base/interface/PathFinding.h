#ifndef PATHFINDING_H
#define PATHFINDING_H

#include <QObject>
#include <QMutex>
#include "../../../general/base/GeneralStructures.h"
#include "../render/MapVisualiserThread.h"

class PathFinding : public QThread
{
    Q_OBJECT
public:
    explicit PathFinding();
    virtual ~PathFinding();
signals:
    void result(const std::string &current_map,const uint8_t &x,const uint8_t &y,std::vector<std::pair<CatchChallenger::Orientation,uint8_t> > path);
    void internalCancel();
    void emitSearchPath(const std::string &destination_map,const uint8_t &destination_x,const uint8_t &destination_y,const std::string &current_map,const uint8_t &x,const uint8_t &y,const std::unordered_map<uint16_t,uint32_t> &items);
public slots:
    void searchPath(const std::unordered_map<std::string, MapVisualiserThread::Map_full *> &all_map,const std::string &destination_map,
                    const uint8_t &destination_x,const uint8_t &destination_y,const std::string &current_map,const uint8_t &x,const uint8_t &y,
                    const std::unordered_map<uint16_t,uint32_t> &items);
    void internalSearchPath(const std::string &destination_map,const uint8_t &destination_x,const uint8_t &destination_y,const std::string &current_map,const uint8_t &x,const uint8_t &y,const std::unordered_map<uint16_t,uint32_t> &items);
    void cancel();
    #ifdef CATCHCHALLENGER_EXTRA_CHECK
    static void extraControlOnData(const std::vector<std::pair<CatchChallenger::Orientation,uint8_t/*step number*/> > &controlVar,const CatchChallenger::Orientation &orientation);
    #endif
private:
    struct SimplifiedMapForPathFinding
    {
        bool *walkable;
        uint8_t *monstersCollisionMap;//to know if have the item
        bool *dirt;
        uint8_t *ledges;

        uint8_t width,height;

        struct Map_Border
        {
            struct Map_BorderContent_TopBottom
            {
                SimplifiedMapForPathFinding *map;
                int32_t x_offset;
            };
            struct Map_BorderContent_LeftRight
            {
                SimplifiedMapForPathFinding *map;
                int32_t y_offset;
            };
            Map_BorderContent_TopBottom top;
            Map_BorderContent_TopBottom bottom;
            Map_BorderContent_LeftRight left;
            Map_BorderContent_LeftRight right;
        };
        Map_Border border;
        struct PathToGo
        {
            std::vector<std::pair<CatchChallenger::Orientation,uint8_t/*step number*/> > left;
            std::vector<std::pair<CatchChallenger::Orientation,uint8_t/*step number*/> > right;
            std::vector<std::pair<CatchChallenger::Orientation,uint8_t/*step number*/> > top;
            std::vector<std::pair<CatchChallenger::Orientation,uint8_t/*step number*/> > bottom;
        };
        std::unordered_map<std::pair<uint8_t,uint8_t>,PathToGo,pairhash> pathToGo;
        std::unordered_set<std::pair<uint8_t,uint8_t>,pairhash> pointQueued;
    };

    struct MapPointToParse
    {
        std::string map;
        uint8_t x,y;
    };

    QMutex mutex;
    std::unordered_map<std::string,SimplifiedMapForPathFinding> simplifiedMapList;
    bool tryCancel;
    std::vector<MapVisualiserThread::Map_full> mapList;
public:
    static bool canGoOn(const SimplifiedMapForPathFinding &simplifiedMapForPathFinding,const uint8_t &x, const uint8_t &y);
};

#endif // PATHFINDING_H
