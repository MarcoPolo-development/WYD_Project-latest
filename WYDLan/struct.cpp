#include "pch.h"
#include "struct.h"
#include "Functions.h"

extern Ent* local;

float Ent::distanceToPlayer() {
	return sqrt(sQ(ABS(pos.x - ::local->pos.x)) + sQ(ABS(pos.y - ::local->pos.y)));

}