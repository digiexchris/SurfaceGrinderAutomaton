#include "Planner.hpp"

/**
@brief It will travel from one end stop at a set speed to the other end stop and reverse.
 */
class Reciprocate : public Planner
{
public:
	Reciprocate(Axis anAxis);
	void Notify(PlannerEvent anEvent);
};