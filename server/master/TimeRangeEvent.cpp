#include "TimeRangeEvent.h"
#include "EpollClientLoginMaster.h"

using namespace CatchChallenger;

TimeRangeEvent::TimeRangeEvent()
{
    setInterval(1000*60*60*24);
}

void TimeRangeEvent::exec()
{
    EpollClientLoginMaster::sendTimeRangeEvent();
}
