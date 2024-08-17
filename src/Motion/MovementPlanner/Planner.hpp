#include "Motion/Axis.hpp"
#include <sys/_stdint.h>
#include <vector>

/**
The idea behind Planner is that the class wrapping this can instantiate one of these movement modes
for the primary grinding axis it will control. It can then call update() and this class will take care of
executing moves according to the logic in the child class. Update() will return true if a new target position was
requested or false if it did nothing.

The primary grinding axis is the axis that will do the passes that make cuts. For example. in a surface grinder, the X axis most commonly traverses from one end of it's travel to the other in order to take a cut. After a cut, the Z axis is advanced by a small amount, and the X axis is run across the part to the other end of it's travel at the cutting speed.

For the purposes of this class in this example, the X axis is the primary axis. It should be possible to have multiple secondary axis. Like, if a taper was being cut, the Z and Y axis could be both moved at the end of each X pass, by both subscribing to the end stop events and moving an appropriate amount. eg. 1mm each, in order to cut a 45 degree.

The base Planner has events that will be fired, and it will wait for all subscribers to return from
processing the event before continuing. This allows synchronization between multiple axis with different
movement logic.

eg. X axis can Reciprocate, but it should emit an "at min stop" event. The Z axis can subscribe to that event, and
when it's recieved, if it's in the AdvanceIncrementRepeated planner type, it will execute the next move (like,
advance by 100mm and stop), and notify the emitting Planner (the X axis Reciprocate planner) so that it may take
another pass to the opposite stop, where it will fire another event and repeat.

There should only be one active primary axis at one time.

Notify on Secondary should block so that Primary is forced to process them in order. We might want to revisit this later if coordinated synchronized movement is needed from secondaries. A possible use case for synchronized movement is dressing an angle on the grinding wheel, but we can make canned cycles to do that instead.
*/

// this might need to be more complex than an enum
enum class PlannerEvent
{
	AT_MIN_STOP,
	AT_MAX_STOP
};

class Planner
{
public:
	Planner(Axis anAxis);
};

class Secondary : public Planner
{
public:
	Secondary(Axis anAxis);
	virtual void Notify(PlannerEvent anEvent);
};

class Primary : public Planner
{
public:
	Primary(Axis anAxis);
	void Update();
	void AddSubscriber(Secondary *aPlanner);
	void RemoveSubscriber(Secondary *aPlanner);

protected:
	void PrivNotifySubscribers();
	std::vector<Secondary *> mySubscribers;
};

/**
@brief No moves will happen during an update. */
class Stopped : public Secondary
{
public:
	Stopped(Axis anAxis);
	void Notify(PlannerEvent anEvent)
	{
		return;
	}
};