#pragma once
#include "pch.h"
#include "AStar.h"
#include "struct.h"
#include "Functions.h"



//DWORD __stdcall BotThread(LPVOID lpParameter);

//void BotLoop();
void MoveCharacter(int OPCode, char* buffer);
void walkPath(AStar* astar, Vec2Int dst);
void PacketMove(short dstx, short dsty);
int PacketMove(PacketPath& pp);
void walkPathPkt(AStar& astar, Vec2Int dst, bool isMob = false, bool loopOffgrid = false);
void MoveToTargets(int count = -1);
void MoveToXY(int x, int y, int maxdistance = 300, int bufferoffset = 30);
void pktUseNPC(int npcid, bool test);
void SetMoveToTarget();
void UseTelePad();
void pktOpenNPC(int npcid, int uitype = 0x45);
//void pktSplitItem(int slot, int itemid, int qty);
//void pktDeleteItem(int slot);
//void pktMoveItem(int srcType, int dstType, int srcSlot, int dstSlot);


enum UITYPE {
	SHOP = 0x0,
	SHOP_NAOTUN = 0x4484,
};

enum CCMODE {
	OFF,
	MELEE,
	MAGIC,
	POTION,
};

enum CCMOVE {
	FREEMOVE,
	ORBIT,
	NONE,
};

enum InventoryType {
	EQUIPMENT,
	INVENTORY,
	CARGO
};

enum ActionType {
	IDLE,
	IDLE_UNTIL_LEVEL,
	DIST_STATS,
	DIST_SM,
	PATHING,
	USE_TP,
	USE_ITEM,
	EQ_ITEM,
	BUY_ITEM,
	SELL_ITEM,
	SPLIT_ITEM,
	OPEN_SHOP,
	USE_NPC,
	DELETE_ITEM,
	MOVE_ITEM,
	SET_CCMODE,
	RESPAWN,
};

enum Regions {
	ARMIA,
	AZRAN,
	ERION,
	SNOWTOWN,
	NAOTUN,
	OUT_OF_TOWN,
};


class Action {
public:
	bool bCompleted = false;
	Vec2Int v2Goal = {0};
	Rec rect_Goal = { {0}, {0} };
	int iGoalInt = 0;
	int iIncrement = 0;
	bool bGoalBool = false;
	bool bGoalBoolB = false;
	std::string sGoal = "";
	std::vector<int> vGoalIntVec = {};
	ActionType Type = IDLE;
	Action(); //Idle
	Action(const Action&) = default;
	Action(ActionType _type);
	Action(ActionType _type, int _level); // idle until level
	Action(Vec2Int _goal); // pathing
	Action(Vec2Int _goal, bool _isMob); // pathing
	Action(Rec _recGoal); // use tp
	Action(int _itemid); // use item
	Action(int _slotid, bool _slots); // use item by slot
	Action(int _slot, ActionType _type); // Delete item
	Action(int _itemid, ActionType _type, bool _deleteitems); // Delete all items with id
	Action(int _start, int _end, ActionType _type); //Delete range of items
	Action(std::string _npcName, int _itemid, bool sellall); //sell all items with id
	Action(std::string _npcName, int _itemid, ActionType _type);// buy/sell item from npc
	Action(std::string _npcName, ActionType _type); // Open shop & Use Npc
	Action(int _slot, int _itemid, int _qty); //split item
	Action(InventoryType _srctype, InventoryType _dsttype, int _srcslot, int _dstslot); //move item
	Action(CCMODE _mode, CCMOVE _move);
	~Action();

	virtual void Execute();
private:
	void IdleUntilLevel();
	void SetPath();
	void UseTP();
	void UseItem();
	void BuyItem();
	void SellItem();
	void OpenShop();
	void UseNpc();
	void SplitItem();
	void StackItem();
	void DeleteItem();
	void MoveItem();
	void SetCCMode();
	void Respawn();
	void Dist_Stats();
	void Dist_SM();
};

//Not sure if this is how I want to implement this yet...
enum TaskType {
	OTHER,
	EQUAL,
	GREATER,
	GEQUAL,
	LEQUAL,
	LESS,
	NOT_EQUAL,
	PATHLIST
};

enum TaskDataType {
	DEFAULT,
	PLAYER_LEVEL,
	PLAYER_X,
	PLAYER_Y,
	INV,
	GOLD,
	PLAYER_POS,
};

class Task : public Action
{
public:
	int lVal;
	int rVal;
	int iIncrement = 0;
	TaskType Type = OTHER;
	TaskDataType DataType = DEFAULT;
	bool bRepeatable = false;
	Task(){};
	Task(const Task&) = default;
	Task(TaskType _type, int _lVal, int _rVal, bool _repeatable, const std::list<std::shared_ptr<Action>> &_actions) {
		lActionList = _actions;
		lVal = _lVal;
		rVal = _rVal;
		Type = _type;
		bRepeatable = _repeatable;
	}
	Task(TaskType _type, TaskDataType _datatype, int _rVal, bool _repeatable, const std::list<std::shared_ptr<Action>>& _actions) {
		lActionList = _actions;
		lVal = -1;
		rVal = _rVal;
		Type = _type;
		DataType = _datatype;
		bRepeatable = _repeatable;
	}
	Task(TaskDataType _datatype, int _lVal, int _rVal, bool _repeatable, const std::list<std::shared_ptr<Action>>& _actions) {
		lActionList = _actions;
		lVal = lVal;
		rVal = _rVal;
		DataType = _datatype;
		bRepeatable = _repeatable;
	}
	Task(TaskType _type, std::list<Vec2Int> _pathlist);
	virtual void Execute() override;
private:
	std::list<std::shared_ptr<Action>> lActionList;
	bool Evaluate();
};




//define rmRegions
//2230,1716
//if (pos.x < rect.h.x and pos.x > rect.l.x and pos.y < rect.h.y and pos.y > rect.l.y)
	//return true;
//return false;


static Rec rmrTrainingField		= { {2163,2047} ,{2062,1934} };
static Rec rmrArmia				= { {2175,2162} ,{2062,2048} };
static Rec rmrArmiaField		= { {2446,2162} ,{2176,2062} };
static Rec rmrArmiaFieldNorth	= { {2418,2290} ,{2319,2163} };
static Rec rmrQuest40			= { {2426,2132} ,{2379,2077} }; //Quest {2426,2132},{2379,2077}, {2425,2131},{2380,2078},
static Rec rmrErionFieldNorth	= { {2674,2161} ,{2447,2048} };
static Rec rmrErionField		= { {2674,2047} ,{2546,1934} };
static Rec rmrErion				= { {2545,2047} ,{2447,1918} };
static Rec rmrTower				= { {2546,1917} ,{2447,1778} };
static Rec rmrAzran				= { {2674,1777} ,{2432,1608} };
static Rec rmrQuest116			= { {2257,1728} ,{2228,1700} };//{ {2256,1727} ,{2229,1701} }; //Quest
static Rec rmrAzranField		= { {2431,1779} ,{2189,1550} };
static Rec rmrAzranFieldSouth	= { {2302,1549} ,{2175,1166} };
static Rec rmrWaters			= { {2188,1778} ,{1799,1550} };
static Rec rmrKingdoms			= { {1798,1748} ,{1677,1704} };
static Rec rmrKingdomSouth		= { {1798,1703} ,{1677,1550} };
static Rec rmrKingdomNorth		= { {1798,1906} ,{1677,1749} };
static Rec rmrKingdomDesert		= { {1676,1778} ,{1392,1679} };
static Rec rmrNaotunDesert		= { {1391,1778} ,{1152,1679} };
static Rec rmrKefraDesert		= { {1391,1904} ,{1284,1779} };
static Rec rmrNaotun			= { {1151,1778} ,{1038,1679} };
static Rec rmrKefraDungeon		= { {2545,4082} ,{2193,3856} };
static Rec rmrHellgate			= { {1907,3699} ,{1678,3598} };
static Rec rmrUnderworld		= { {1523,4082} ,{1166,3982} };
static Rec rmrUnderworld2		= { {1519,3826} ,{1296,3728} };
static Rec rmrQuest321			= { {1347,4054} ,{1312,4028} }; //Quest
static Rec rmrDungeon1a			= { { 755,3827} ,{ 141,3725} };
static Rec rmrDungeon1b			= { { 498,4083} ,{ 399,3828} };
static Rec rmrQuest191			= { { 496,3915} ,{ 460,3888} }; //Quest
static Rec rmrQuest266			= { { 703,3762} ,{ 659,3729} }; //Quest
static Rec rmrDungeon2			= { {1011,4082} ,{ 655,3854} };
static Rec rmrDungeon3			= { {1139,3826} ,{ 910,3726} };
static Rec rmrSnowTown			= { {3700,3188} ,{3598,3086} };
static Rec rmrSnowField			= { {3954,3085} ,{3448,2702} };
static Rec rmrLanA				= { {3826,3569} ,{3727,3471} };
static Rec rmrLanM				= { {3954,3697} ,{3855,3599} };
static Rec rmrLanN				= { {3698,3698} ,{3598,3599} };
static Rec rmrDungeonEnt1		= { {2668,2157} ,{2671,2156} };

//Region master list, probably there is a better way to do this
static std::list<Rec> lRegionList = {
	rmrLanA,rmrLanM,rmrLanN,rmrTrainingField,rmrArmia,rmrArmiaField,rmrArmiaFieldNorth,
	rmrErionFieldNorth,rmrErionField,rmrErion,rmrTower,rmrAzran,rmrAzranField,rmrAzranFieldSouth,
	rmrWaters,rmrKingdoms,rmrKingdomSouth,rmrKingdomNorth,rmrKingdomDesert,rmrNaotunDesert,
	rmrKefraDesert,rmrNaotun,rmrKefraDungeon,rmrHellgate,rmrUnderworld,rmrUnderworld2,rmrDungeon1a,
	rmrDungeon1b,rmrDungeon2,rmrDungeon3,rmrSnowTown,rmrSnowField };




//Define Quest levels, locations, paths, etc
static std::vector<int> vQuestItemIds = {4117,4118,4119,4120,4121};

//Naotun 
static Rec rect_TpPadNaotunToArmia	= { {1047,1727}, {1044,1724} };
static Rec rect_TpPadNaotunToAzran	= { {1047,1719}, {1044,1716} };
static Rec rect_TpPadNaotunToErion	= { {1047,1711}, {1044,1708} };
static Rec rect_TpPadNaotunToSnow	= { {1055,1711}, {1052,1708} };


//Armia
static Rec rect_TpPadArmiaToNaotun	= { {2119,2103}, {2116,2100} };
static Rec rect_TpPadArmiaToField	= { {2143,2071}, {2140,2068} };

static std::list<std::shared_ptr<Action>> actionArmiaWarpScroll{ 
	std::make_shared<Action>(Vec2Int{ 2118,2102 }),
	std::make_shared<Action>(rect_TpPadArmiaToNaotun),
	std::make_shared<Action>(Vec2Int{1057,1730}),
	std::make_shared<Action>("Hestia", 0x1D, BUY_ITEM),
	std::make_shared<Action>(Vec2Int{1045,1725}),
	std::make_shared<Action>(rect_TpPadNaotunToArmia)
};


//Azran
static Rec rect_TpPadAzranToNaotun	= { {2483,1719}, {2480,1716} };
static Rec rect_TpPadAzranToField1	= { {2471,1719}, {2468,1716} };
static Rec rect_TpPadAzranToField2	= { {2455,1719}, {2452,1716} };

//Erion
static Rec rect_TpPadErionToNaotun	= { {2459,2019}, {2456,2016} };
static Rec rect_TpPadErionToField	= { {2455,1991}, {2452,1988} };


//Snowfield
static Rec rect_TpPadSnowToNaotun	= { {3651,3111}, {3648,3108} };


//Other Actions


//New character TK
static std::list<std::shared_ptr<Action>> actionNewCharTK{
	
	std::make_shared<Task>(LESS,PLAYER_LEVEL,2,false, // player is level 1
	(std::list<std::shared_ptr<Action>>{
	std::make_shared<Action>(400, DELETE_ITEM, true), // Delete all hp pots
	std::make_shared<Action>(405, DELETE_ITEM, true), // Delete all mp pots
	std::make_shared<Action>(861, DELETE_ITEM, true), // Delete dagger
	std::make_shared<Action>(917, DELETE_ITEM, true), // Delete "Cuttluss"
	std::make_shared<Action>(410, DELETE_ITEM, true), // Delete recall scrolls
})),
	std::make_shared<Task>(LESS,PLAYER_LEVEL,2,false, //player is level 1
	(std::list<std::shared_ptr<Action>>{
	std::make_shared<Action>(randomXY(Vec2Int{ 2128,2036 }, 1)), //path towards Unicorn
	std::make_shared<Action>("Unicorn  ", 0x0, BUY_ITEM), //Buy 2x weapon
	std::make_shared<Action>("Unicorn  ", 0x0, BUY_ITEM),
	std::make_shared<Action>(INVENTORY, EQUIPMENT, 0, 6), // slot 0 inventory to slot 6(left weapon) equipment //Equip 2x weapon
	std::make_shared<Action>(INVENTORY, EQUIPMENT, 1, 7), // slot 1 inventory to slot 7(right weapon) equipment
	std::make_shared<Action>("CabuncleWind", USE_NPC), //Use CabuncleWind	
	std::make_shared<Action>(randomXY(Vec2Int{ 2128,2028 }, 1)),//path to mobs		
	std::make_shared<Action>(MELEE, ORBIT), //Enable CC Mode
	std::make_shared<Action>(IDLE_UNTIL_LEVEL, 25) //Idle until level 25
})),
	
};

//Paths

static std::list<Vec2Int> pathArmiaToQuest40Npc = { {2146,2102},{2170,2102},{2187,2106},{2215,2105},{2241,2096},{2262,2108},{2292,2111},{2344,2110},{2369,2107},{2371,2098} };
static std::list<Vec2Int> pathTrainingFieldToArmia = { {2112,2042},{2112,2054},{2112,2072},{2112,2081},{2113,2094},{2120,2095} };
static std::list<Vec2Int> pathAzranToQuest116Npc = { {2473,1724},{2458,1724},{2444,1724},{2436,1718},{2421,1719},{2388,1712},{2365,1714},{2335,1705},{2316,1702},{2292,1705},{2269,1714} };
static std::list<Vec2Int> pathErionToDungeon1aEntrance = { {2470,2010},{2482,2011},{2493,2018},{2515,2018},{2528,2015},{2542,2016},{2542,2034},{2542,2055},{2543,2078},{2562,2088},{2597,2082},{2623,2093},{2641,2098},{2641,2110},{2646,2121},{2645,2134},{2664,2152} };
static std::list<Vec2Int> pathDungeonEntranceToQuest191 = { {152,3783},{160,3776},{192,3776},{216,3767},{244,3771},{268,3777},{279,3785},{340,3799},{427,3794},{437,3833},{448,3871},{445,3894},{453,3901} };
static std::list<Vec2Int> pathQuest191ToQuest266 = { {452,3895},{445,3874},{446,3842},{465,3817},{504,3809},{538,3802},{555,3769},{600,3786},{633,3777},{667,3778} };
static std::list<Vec2Int> pathDungeonEntranceToQuest266 = { {156,3776},{191,3776},{243,3772},{281,3785},{351,3783},{393,3767},{436,3797},{500,3786},{547,3777},{594,3785},{634,3776},{647,3765},{669,3775} };
static std::list<Vec2Int> pathAzranTp2ToUnderworldEntrance = { {1953,1722},{1933,1724},{1913,1725},{1893,1720},{1876,1730},{1849,1749},{1826,1771} };
static std::list<Vec2Int> pathUnderworldEntranceToQuest321 = { {1177,4068},{1217,4066},{1236,4053},{1274,4053},{1282,4036},{1299,4034} };
static std::list<Vec2Int> pathAzranTp2ToWaterEntrance = { {1965,1714},{1964,1729},{1964,1756},{1973,1767} };


class Quest 
{
public:
	bool bWarpSet = false;
	int iReqLevel = 25;
	int iMaxLevel = 39;
	std::vector<Action*> QuestActions;
	std::vector<Vec2Int> vNoWarpPath;
	std::list<std::shared_ptr<Action>> ActionList;
	std::list<std::shared_ptr<Action>> ArmiaActionList;
	std::list<std::shared_ptr<Action>> AzranActionList;
	std::list<std::shared_ptr<Action>> ErionActionList;
	std::list<std::shared_ptr<Action>> SnowtownActionList;
	std::list<std::shared_ptr<Action>> NaotunActionList;
	Rec rect_InsideQuest = { {2378,2134},{2358,2075} };
	Rec rect_QuestRegion = rmrArmiaField;
	Vec2Int v2QuestRegion = { 2176,2048 };
	Vec2Int v2QuestRegion2 = { 0 }; // in case the quest zone is on the boundary of regions
	Vec2Int v2QuestWarpLocation = { 2373, 2098 };
	std::string sQuestNpcName = "NONE";
	std::vector<std::string> vQuestMobNames = { "Gremlin", "Boar", "GremlinKing", "ChiefCrill"};
	Quest();
	std::list<std::shared_ptr<Action>> GetRegionAction(Regions _region);
};


class Quest40 : public Quest
{
public:
	Quest40();
};



class Quest116 : public Quest
{
public:
	Quest116();
};

class Quest191 : public Quest
{
public:
	Quest191();
};

class Quest266 : public Quest
{
public:
	Quest266();
};

class Quest321 : public Quest
{
public:
	Quest321();
};

