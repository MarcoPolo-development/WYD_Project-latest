#include "pch.h"
#include "GameState.h"
#include "Functions.h"
#include "struct.h"
#include "Bot.h"


extern Ent* local;

extern DWORD ccmode_addr, ccmove_addr, moduleBase;

extern bool bQuestBotEnabled, bLanBotEnabled, bFarmBotEnabled, bLastEntIdx;


GameState::GameState() {
	if (!Init())
		std::cout << dye::red("Failed to init due to login.") << std::endl;
	else
		bInitialized = true;
}

//GameState::~GameState() {
//	delete[] pd;
//}

std::shared_ptr<Quest> GameState::GetProperQuest() {
	int pLevel = local->Level + 1;
	if (pLevel < 40) {
		return std::make_unique<Quest>();
	}
	if (pLevel >= 40 && pLevel <= 115) {
		return std::make_unique<Quest40>();
	}
	if (pLevel >= 116 && pLevel <= 190) {
		return  std::make_unique<Quest116>();
	}
	if (pLevel >= 191 && pLevel <= 265) {
		return  std::make_unique<Quest191>();
	}
	if (pLevel >= 266 && pLevel <= 320) {
		return  std::make_unique<Quest266>();
	}
	if (pLevel >= 321 && pLevel <= 355) {
		return  std::make_unique<Quest321>();
	}
	return nullptr;
}

bool GameState::TileHasEnt(Vec2Int _pos)
{
	for (auto& ent : EntityList) {
		if (ent->posInt == _pos)
			return true;
	}
	return false;
}
bool GameState::TileHasEnt(int x, int y)
{
	for (auto& ent : EntityList) {
		if (ent->posInt.x == x && ent->posInt.y == y)
			return true;
	}
	return false;
}

bool GameState::TileIsPathable(Vec2Int _pos)
{
	if ((*GetPFdata(_pos.x, _pos.y) & 0xF0) > 0) {
		return false;
	}
	return true;
}

bool GameState::TileIsPathable(int x, int y)
{
	if ((*GetPFdata(x, y) & 0xF0) > 0) {
		return false;
	}
	return true;
}

bool GameState::Init() {
	if (!IsLoggedIn()) {
		//return for now, add login function later
		return false;
	}
	Inv = new Inventory();
	//pd = new PathingData();
	bIsloggedIn = true;
	v2Region = { *(int*)pfXInit_addr,*(int*)pfYInit_addr };
	Sleep(1000);
	if (local == nullptr) {
		local = (Ent*)FindDMAAddy(moduleBase + playerinfo_offset, { 0x5C, 0x0 });
		std::cout << hue::aqua << "Local Player Updated: 0x" << local << hue::reset << std::endl;
		lastEntIdx = FindDMAAddy(moduleBase + playerinfo_offset, { 0x5C, 0x4, 0x10 });
	}
	CopyEntList();
	GetTgtList();
	localPlayer = local;
	targets.clear();
	if(bLanBotEnabled)
		targets.insert(targets.end(), { "PerGorgon","PerGagoil","PerKnight","PerSpider" });
	if (bQuestBotEnabled) {
		currentQuest = GetProperQuest();
		targets.clear();
		targets.insert(targets.begin(), currentQuest->vQuestMobNames.begin(), currentQuest->vQuestMobNames.end());
	}


	//targets.insert(targets.end(), { "Gremlin"});
	return true;
}



void GameState::Update() {
	if (!IsLoggedIn()) {
		bIsloggedIn = false;
		bInitialized = false;
		return;
	}
	if (!bInitialized) {
		if (!Init()) {
			return;
		}
		bInitialized = true;
	}
	Ent* templocal = (Ent*)FindDMAAddy(moduleBase + playerinfo_offset, { 0x5C, 0x0 });
	if (templocal == nullptr) {
		bIsloggedIn = false;
		bLastEntIdx = false;
		return;
	}

	if (local != templocal) {
		local = templocal;
		std::cout << hue::aqua << "Local Player Updated: 0x" << local << hue::reset << std::endl;
		lastEntIdx = FindDMAAddy(moduleBase + playerinfo_offset, { 0x5C, 0x4, 0x10 });
	}
	//delete templocal;
	if(!bIsloggedIn)
		bIsloggedIn = true;
	if (bRegionChanged)
		bRegionChanged = false;

	rmrTemp = GetCurrentRegion();
	if (rmrTemp != rmrRegion) {
		bRegionChanged = true;
		bPendingMove = false;
		bGoalIsMob = false;
		v2GoalPos = {0};
		rmrRegion = rmrTemp;
	}
	RegionsTemp = OUT_OF_TOWN;
	if (rmrRegion == rmrArmia)
		RegionsTemp = ARMIA;
	if (rmrRegion == rmrAzran)
		RegionsTemp = AZRAN;
	if (rmrRegion == rmrErion)
		RegionsTemp = ERION;
	if (rmrRegion == rmrSnowTown)
		RegionsTemp = SNOWTOWN;
	if (rmrRegion == rmrNaotun)
		RegionsTemp = NAOTUN;

	if (RegionsCurrent != RegionsTemp)
		RegionsCurrent = RegionsTemp;

	


	//bIsDead = localPlayer->isDead;
	
	if (!bPendingMove && bGoalIsMob)
		bGoalIsMob = false;

	if (v2GoalPos.x == 0 && v2GoalPos.y == 0 && bPendingMove)
		bPendingMove = false;

	CopyEntList();
    GetTgtList();
	if (!TargetList.empty()) {
		target_Ent = TargetList.front();
		iTargetHP = TargetList.front()->iCurrentHp;
		if (bNoTarget)
			bNoTarget = false;
	}
	else
		bNoTarget = true;
	
	
}

// Determine whether or not the client is logged into a server
bool GameState::IsLoggedIn() {
	if (SceneState() > 3)
		return true;
	return false;
}


//4 - in game, 3 - char select, 2 - server select
int GameState::SceneState() {
	int state = *(int*)addr_SceneState;
	return state;
}

void GameState::CopyEntList() {
	EntityList.clear();
	Ent* cur = *(Ent**)lastEntIdx;

	while (cur != 0x0)
	{
		EntityList.push_front(std::move(std::make_shared<SlimEnt>(cur->entHpCur,
			cur->pos, Vec2Int{cur->entIntX1,cur->entIntY1},
			cur->entName.name, cur->isVisible, cur->MoveSpd, cur->distanceToPlayer(), cur->entId)));
		cur = (Ent*)cur->pParent;
	}


}

//
//PathingData::PathingData() {
//	Init();
//}
//
//void PathingData::Init() {
//	std::ifstream in("HeightMap.dat");
//	in.read((char*)&grid, sizeof(pfd));
//}
//
//char* PathingData::GetGridData(const int& x, const int& y) {
//	int idx = y * 4096 + x;
//	return &grid.data[idx];
//}



std::list<Ent*> GameState::GetEntList() {
	std::list<Ent*> ents;
	Ent* cur = *(Ent**)lastEntIdx;
	while (cur != 0x0)
	{
		ents.push_back(cur);
		cur = (Ent*)cur->pParent;
	}
	//reverse so that player is first in list
	//ents.reverse();

	return ents;
}

void GameState::GetTgtList() {
	auto tList = EntityList;
	std::list<std::shared_ptr<SlimEnt>> rList;
	for (int t = 0; t < targets.size(); t++) {
		for (auto& en : tList) {

			if (targets[t] == en->name && en->iCurrentHp > 0) {
				float dis = findDistance( local->pos, en->pos);
				if (dis < 25)
					rList.push_back(en);
			}
		}
	}
	rList.sort([](std::shared_ptr<SlimEnt> a, std::shared_ptr<SlimEnt> b) {return (findDistance(local->pos, (*a).pos)) < (findDistance(local->pos, (*b).pos)); });


	TargetList.clear();
	TargetList = rList;
}

//void GameState::GetTgtList() {
//	auto tList = GetEntList();
//	std::list<Ent*> rList;
//	for (int t = 0; t < targets.size(); t++) {
//		for (auto en : tList) {
//			
//			if (targets[t] == en->entName.name && en->isDead == false) {
//				int dis = en->distanceToPlayer();
//				if (dis < 25)
//					rList.push_back(en);
//			}
//		}
//	}
//	rList.sort([](Ent* a, Ent* b) {return (*a).distanceToPlayer() < (*b).distanceToPlayer(); });
//
//
//	TargetList.clear();
//	for (auto t : rList) {
//		TargetList.push_back(std::move(std::make_shared<SlimEnt>(t->entHpCur, Vec2 { t->vecBase.x, t->vecBase.y }, Vec2Int{ t->entIntX1,t->entIntY1 },
//			t->entName.name, t->isVisible, t->MoveSpd, t->distanceToPlayer(), t->entId)));
//	}
//}

void GameState::PrintTgtList() {
	using namespace std;
	cout << hue::yellow;
	if (TargetList.empty())
		cout << "No Targets\n";
	for (auto& t : TargetList) {
		cout
			<< "Target Name: " << t.get()->name
			<< "\nTarget Distance: " << t.get()->fDistance << "\n";

	}
	cout << hue::reset << endl;
}

void GameState::PrintEntList() {
	using namespace std;
	cout << hue::yellow;
	if (EntityList.empty())
		cout << "No Entitys\n";
	for (auto& t : EntityList) {
		cout
			<< "Entity Name: ["  << t.get()->name << "]"
			<< "\nEntity Distance: " << t.get()->fDistance << "\n";

	}
	cout << hue::reset << endl;
}

Rec GameState::GetCurrentRegion() {
	for (auto& r : lRegionList) {
		if (FindPoint(r, getXYVecInt()))
			return r;
	}
	return Rec{ {0},{0} };
}

void GameState::ChangeCCMode(int mode, int move) {
	*(unsigned char*)ccmode_addr = (unsigned char)mode;
	*(unsigned char*)ccmove_addr = (unsigned char)move;
}
//
//Task::Task() {
//
//}
//
////Override these functions for other tasks
//int Task::Work() {
//	while (!bExitWork) {
//		//Do something
//
//	}
//	bIsActive = false;
//	return 1;
//}
//
//void Task::StartTask() {
//	bIsActive = true;
//	bExitWork = false;
//	iTaskStatus = Work();
//}
//
//void Task::StopTask() {
//	bExitWork = true;
//}

SlimEnt::SlimEnt(int _hp, Vec2 _pos, Vec2Int _posInt, std::string _name, bool _bIsVis, int _movespd, float dis, unsigned short _entID) {
	iCurrentHp = _hp;
	pos = _pos;
	posInt = _posInt;
	name = _name;
	bIsVisible = _bIsVis;
	movespeed = _movespd;
	fDistance = dis;
	sEntId = _entID;
}