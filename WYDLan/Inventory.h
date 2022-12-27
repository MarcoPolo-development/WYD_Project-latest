#pragma once
#include "pch.h"
#include "struct.h"


static const int equipSize = 16;
static const int carrySize = 60;
static const int cargoSize = 120;

static const int iWarpScroll10	= 776;
static const int iLodestarGem	= 700;
static const int iRecallScroll10 = 411;


struct Equip
{
	ITEM_ST items[equipSize];
};
struct Carry
{
	ITEM_ST items[carrySize];
};
struct Cargo
{
	ITEM_ST items[cargoSize];
};


class Inventory
{
public:
	Equip* eq;
	Carry* inv;
	Cargo* cargo;
	
	int iCharGold = 0;
	int iCargoGold = 0;

	Inventory();
	bool Contains(int itemid);
	int  FindFirst(int itemid);
	void DeleteItem(int slot);
	void MoveItem(int srcSlot, int dstSlot, int srcType = 1, int dstType = 1 );
	void StackItem(int from, int to);
	void SplitItem(int slot, int itemid, int qty);
	void SellItem(int slot, int NPCid, int item = 0);
	void BuyItem(int npcid, int slotid);
	void UseItem(int slot);
	void EquipItem(int from, int to);
	void DepositGold(int amount);
	void WithdrawGold(int amount);
};

