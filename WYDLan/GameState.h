#pragma once
#include "pch.h"
#include "struct.h"
#include "Inventory.h"
#include "Bot.h"


//Functions
bool IsLoggedIn();
int SceneState();

typedef struct {
	unsigned char data[0x1000000];
} pfd;


//class PathingData
//{
//public:
//	pfd grid;
//	PathingData();
//	char* GetGridData(const int& x, const int& y);
//private:
//	void Init();
//};

class SlimEnt
{
public:
	int iCurrentHp = 0;
	Vec2 pos = { 0 };
	Vec2Int posInt = { 0 };
	int movespeed = 0;
	std::string name;
	bool bIsVisible = false;
	float fDistance = 0.0f;
	unsigned short sEntId = 0;
	SlimEnt(int _hp, Vec2 _pos, Vec2Int _posInt, std::string _name, bool _bIsVis, int _movespd, float dis, unsigned short _entID);
};

class GameState
{
public:
	bool bInitialized = false;
	bool bIsloggedIn = false;
	bool bIsDead = false;
	bool target_isAlive = false;
	bool bRegionChanged = false;
	bool bTargetChanged = false;
	bool bNoTarget = false;
	bool bTargetInRange = false;
	bool bPendingMove = false;
	bool bGoalIsMob = false;

	int iTargetHP = 0;
	int iTempTargetHP = 0;
	int iMobHpCounter = 0;

	Inventory* Inv = nullptr;
	std::shared_ptr<Quest> currentQuest = nullptr;

	std::list<std::shared_ptr<Action>> gsActions;

	std::vector<char> pktBuffer;
	bool bPktBufferSet = false;

	Vec2Int v2GoalPos = { 0 };
	Rec rmrRegion = { {0},{0} };
	Rec rmrTemp = { {0},{0} };
	Regions RegionsTemp = OUT_OF_TOWN;
	Regions RegionsCurrent = OUT_OF_TOWN;
	Vec2Int v2postion = { 0 };
	Vec2Int v2tgtpos = { 0 };
	Vec2Int v2Region = { 0 };
	Vec2Int tempRegion = { 0 };
	Ent* localPlayer = nullptr;
	std::shared_ptr<SlimEnt> target_Ent;
	std::shared_ptr<SlimEnt> last_target;
	//PathingData* pd;
	std::list<std::shared_ptr<SlimEnt>> EntityList;
	std::list<std::shared_ptr<SlimEnt>> TargetList;
	std::vector<std::string> targets;
	std::list<Ent*> GetEntList();
	GameState();
	bool IsLoggedIn();
	//~GameState();
	void ChangeCCMode(int mode, int move = (unsigned char)0x2);
	void Update();
	void PrintTgtList();
	void PrintEntList();
	bool TileIsPathable(Vec2Int _pos);
	bool TileIsPathable(int x, int y);
	bool TileHasEnt(Vec2Int _pos);
	bool TileHasEnt(int x, int y);
	std::shared_ptr<Quest> GetProperQuest();
private:
	bool Init();
	void CopyEntList();
	Rec GetCurrentRegion();
	void GetTgtList();
	std::vector<Ent> EntityListCopy;
	
	int SceneState();
};

//class ItemSlot
//{
//public:
//};
//
//class Inventory
//{
//public:
//	ItemSlot items[60];
//};




//
//class Task
//{
//public:
//	bool bIsActive = false;
//	bool bExitWork = false;
//	int iTaskStatus = 0;
//	Task();
//	virtual int Work();
//	virtual void StartTask();
//	virtual void StopTask();
//};


class WayPoint
{
public:
	std::string name = "";
	int x = 0;
	int y = 0;
	bool bIsGoal = false;
};

class Path
{
public:
	std::vector<WayPoint*> waypoints;
};


class SavedPaths
{
	std::vector<WayPoint*> waypoints;
};