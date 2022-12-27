#include "pch.h"
#include "Bot.h"
#include "AStar.h"
#include "Functions.h"
#include "GameState.h"
#include "struct.h"
#include "logging.h"

extern GameState* gs;
extern bool bOffgrid;
extern DWORD moveXaddr, moveYaddr, movechar_addr, ccmode_addr, ccmove_addr, sourceX_addr, sourceY_addr, destX_addr, destY_addr;

//probably not needed anymore
Vec2Int v2LanNpcWp1 = { 2138,2085 };
Vec2Int v2LanNpcWp2 = { 2138,2100 };
Vec2Int v2LanNpcWp3 = { 2138,2118 };

//// just in case
//DWORD __stdcall BotThread(LPVOID lpParameter) {
//	Sleep(100);
//	BotLoop();
//	return 0;
//}
//
//void BotLoop() {
//	//setup
//
//	//loop
//	while (1) {
//
//	}
//
//
//
//}

void UseTelePad() {
	p290 sendData;
	ZeroMemory(&sendData, sizeof(sendData));
	sendData.Header.packetSize = sizeof(p290);
	sendData.Header.packetID = 0x290;
	sendData.Header.referID = GetClientID();
	sendData.unk = 0x0;
	hSendPacket((char*)&sendData, sizeof(p290));
	Sleep(100);
}


void MoveCharacter(int OPCode, char* buffer) { // Makes .exe read the motion packet buffer, changing clientside position

	static int movecharacter_addr = movechar_addr;
	__asm {
		MOV EAX, base_addr1
		MOV EDX, DWORD PTR DS : [EAX]
		ADD EDX, 0x5C
		MOV ECX, DWORD PTR DS : [EDX]
		PUSH buffer
		PUSH OPCode
		CALL movecharacter_addr
	}
}

void PacketMove(short dstx, short dsty) {
	char buffer[0x34];
	ZeroMemory(buffer, 0x34);
	SetShort(0x36C, 4);
	SetShort(GetClientID(), 6);
	SetShort(GetPosX(), 12);
	SetShort(GetPosY(), 14);
	SET_INT(0, 16);
	SET_INT(6, 20);
	// 3 int size space for path direction array
	SetShort(dstx, 48);
	SetShort(dsty, 50);
	//hSendPacket(buffer, 0x34);

	//MoveCharacter(0x36C, buffer);
}


int PacketMove(PacketPath& pp) {
	//gs->bPktBufferSet = false;
	char buffer[0x34];
	int steps = 0;
	ZeroMemory(buffer, 0x34);
	SetShort(0x36C, 4);
	SetShort(GetClientID(), 6);
	SetShort(pp.startX, 12);
	SetShort(pp.startY, 14);
	//SetInt(0, 16);
	SET_INT(pp.MoveSpeed, 20);
	// 3 int size space for path direction array
	for (int i = 0; i < 12; i++) {
		if ((unsigned char)pp.pathbytes[i] > (unsigned char)0x0) {
			SetByte((unsigned char)pp.pathbytes[i], (24 + i));
			steps++;
		}
	}
	SetShort(pp.destX, 48);
	SetShort(pp.destY, 50);
	////std::cout << std::hex << hue::aqua;
	////for (int i = 0; i < 0x34; i++) {
	////	char n[10];
	////	sprintf_s(n, "%02x", (unsigned char)buffer[i]);


	////	std::cout << n << " ";
	////}
	////std::cout << std::dec << hue::reset << std::endl;


	hSendPacket(buffer, 0x34);
	*(short*)sourceX_addr = (short)((p36C*)buffer)->StartX;
	*(short*)sourceY_addr = (short)((p36C*)buffer)->StartY;
	*(short*)destX_addr = (short)((p36C*)buffer)->DestX;
	*(short*)destY_addr = (short)((p36C*)buffer)->DestY;
	MoveCharacter(0x36C, buffer);
	//
	//Sleep(2000);
	//gs->pktBuffer.clear();
	//gs->pktBuffer.resize(0x34);	
	//for (int i = 0; i < gs->pktBuffer.size(); i++) {
	//	gs->pktBuffer[i] = buffer[i];		
	//}
	//gs->bPktBufferSet = true;
	std::cout << hue::green<<"XY " << pp.destX << " " << pp.destY << hue::reset <<"\n";
	//I think this edit was actually causing more rubber banding 
	//*(short*)moveXaddr = pp.destX;
	//*(short*)moveYaddr = pp.destY;
	
	Sleep(220 * steps); // figure out a better solution to this delay
	
	
	//std::cout << "Start:\n" << *(short*)sourceX_addr << " " << *(short*)sourceY_addr << "\nDest:\n" << *(short*)destX_addr << " " << *(short*)destY_addr << "\n";
	return 1;


	////Timer* t = new Timer();
	//while (1) {
	//	if (GetAsyncKeyState(VK_HOME) & 0x8000) {
	//		std::cout << dye::red("BAILED") << std::endl;
	//		return -1;
	//		break;
	//	}
	//	if (GetPosX() == pp.destX && GetPosY() == pp.destY) {
	//		//t->Finish();
	//		//std::cout << "Destination Reached" << std::endl;
	//		break;
	//	}
	//	Sleep(50);
	//	
	//}
	//return 1;
}



bool WalkToPtAndVerify(Vec2Int pt) {
	auto tEnt = gs->localPlayer;
	int InverseMoveSpeed = 7 - (tEnt->MoveSpd & 0xF);
	int x = pt.x;
	int y = pt.y;
	Vec2 fpt = { pt.x,pt.y };
	float dis = findDistance(getXYVec(), fpt);
	Patch((BYTE*)moveXaddr, (BYTE*)&x, sizeof(int));
	Patch((BYTE*)moveYaddr, (BYTE*)&y, sizeof(int));
	int breakout = 0;
	while (true) {
		if (breakout > 10) {
			std::cout << "Walking to " << x << " " << y << " failed." << std::endl;
			return false;
			break;
		}
		Sleep(((InverseMoveSpeed * 20)) * dis);
		if (floor(tEnt->vecOriginX) == x && floor(tEnt->vecOriginZ) == y)
			break;
		breakout++;
	}
	if (breakout <= 10) {
		std::cout << "Walking to " << x << " " << y << " success." << std::endl;
		return true;
	}
	return false;
}


void SetMoveToTarget() {
	std::list<std::shared_ptr<SlimEnt>> tTgt;
	if (gs->TargetList.empty())
		return;

	Sleep(100);
	tTgt = gs->TargetList;

	int fTX = tTgt.front().get()->posInt.x;
	int fTY = tTgt.front().get()->posInt.y;
	int pTx = getXYVecInt().x;
	int pTy = getXYVecInt().y;

	//No move needed
	if (findDistanceInt(getXYVecInt(), Vec2Int{ tTgt.front().get()->posInt.x, tTgt.front().get()->posInt.y }) <= 2) {
		if (*(unsigned char*)ccmode_addr == (unsigned char)0x3 || *(unsigned char*)ccmode_addr == (unsigned char)0x0)
			gs->ChangeCCMode(1, 0);
		return;
	}

	//move needed
	gs->ChangeCCMode(3);
	//walkPathPkt(*ast, Vec2Int{ fTX,fTY }, true);
	gs->bPendingMove = true;
	gs->v2GoalPos = { fTX,fTY };
	gs->bGoalIsMob = true;
}

void MoveToTargets(int count) {
	std::list<std::shared_ptr<SlimEnt>> tTgt;
	int i = 0;
	if (gs->TargetList.empty())
		return;
	AStar* ast = new AStar(256, 256);
	//while (true && count != i) {
		Sleep(100);
		tTgt = gs->TargetList;
		//if (GetAsyncKeyState(VK_END) && GetAsyncKeyState(VK_SHIFT) && 0x8000)
		//	break;
		//if (tTgt.empty())
		//	continue;
		int fTX = tTgt.front().get()->posInt.x;
		int fTY = tTgt.front().get()->posInt.y;
		int pTx = getXYVecInt().x;
		int pTy = getXYVecInt().y;

		//if (fTX < pTx)
		//	fTX++;
		//if (fTX > pTx)
		//	fTX--;
		//if (fTY < pTy)
		//	fTY++;
		//if (fTY > pTy)
		//	fTY--;

		//std::cout << findDistance(getXYVec(), Vec2{ fTX, fTY }) << std::endl;
		if (findDistanceInt(getXYVecInt(), Vec2Int{ tTgt.front().get()->posInt.x, tTgt.front().get()->posInt.y }) <= 2) {
			if (*(unsigned char*)ccmode_addr == (unsigned char)0x3 || *(unsigned char*)ccmode_addr == (unsigned char)0x0)
				gs->ChangeCCMode(1, 0);
			return;
		}
		gs->ChangeCCMode(3);
		walkPathPkt(*ast, Vec2Int{ fTX,fTY }, true);
		
		if (count > 0)
			count--;
	//}
	delete ast;
}

//void pktBuyItem(int npcid, int slotid) { 
//	p379 sendData;
//	ZeroMemory(&sendData, sizeof(sendData));
//	sendData.Header.packetSize = sizeof(p379);
//	sendData.Header.packetID = 0x379;
//	sendData.Header.referID = GetClientID();
//	for (int i = 0; i < GetLimitSlot(); i++) {
//		if (GetInvSlot(i) == 0) {
//			sendData.invSlot = i;
//			break;
//		}
//	}
//	sendData.mobID = npcid;
//	sendData.sellSlot = slotid;
//	hSendPacket((char*)&sendData, sizeof(p379));
//	std::cout << "Buy Item Sent" << std::endl;
//	return;
//}
//
//void pktSellItem(int npcid, int slotid) {
//	p37A sendData;
//	ZeroMemory(&sendData, sizeof(sendData));
//	sendData.Header.packetSize = sizeof(p37A);
//	sendData.Header.packetID = 0x37A;
//	sendData.Header.referID = GetClientID();
//	sendData.npcId = npcid;
//	sendData.type = 1;
//	sendData.sellSlot = slotid;
//	//if (slotid < 0) {
//	//	for (int i = 0; i < GetLimitSlot(); i++) {
//	//		if (GetInvSlot(i) == item) {
//	//			sendData.sellSlot = i;
//	//			break;
//	//		}
//	//	}
//	//}
//	hSendPacket((char*)&sendData, sizeof(p37A));
//	std::cout << "Sell Item Sent" << std::endl;
//	return;
//}
//
//void pktMoveItem(int srcType, int dstType, int srcSlot, int dstSlot) {
//	p376 sendData;
//	ZeroMemory(&sendData, sizeof(sendData));
//	sendData.Header.packetSize = sizeof(p376);
//	sendData.Header.packetID = 0x376;
//	sendData.Header.referID = GetClientID();
//	sendData.DstType = dstType;
//	sendData.SrcType = srcType;
//	sendData.SrcSlot = srcSlot;
//	sendData.DstSlot = dstSlot;
//	hSendPacket((char*)&sendData, sizeof(p376));
//	std::cout << "Move Item Sent" << std::endl;
//	return;
//
//}

void pktOpenNPC(int npcid, int uitype) {
	p27B sendData;
	ZeroMemory(&sendData, sizeof(sendData));
	sendData.Header.packetSize = sizeof(p27B);
	sendData.Header.packetID = 0x28B;
	sendData.Header.referID = GetClientID();
	sendData.npcid = npcid;
	sendData.uitype = uitype;
	hSendPacket((char*)&sendData, sizeof(p28B));
	std::cout << "Open NPC Sent" << std::endl;
	return;
}

void pktUseNPC(int npcid, bool test) {
	p28B sendData;
	ZeroMemory(&sendData, sizeof(sendData));
	sendData.Header.packetSize = sizeof(p28B);
	sendData.Header.packetID = 0x28B;
	sendData.Header.referID = GetClientID();
	sendData.npcid = npcid;
	if (!test) {
		hSendPacket((char*)&sendData, sizeof(p28B));
		std::cout << "Use NPC Sent" << std::endl;
		return;
	}

	BYTE b[sizeof(sendData)];
	memcpy(&b, &sendData, sizeof(sendData));
	std::cout << std::hex << "Size: " << sizeof(sendData) << std::dec << std::endl;

	for (int i = 0; i < sizeof(b) / sizeof(b[0]); i++) {
		std::cout << std::hex << "0x" << int(b[i]) << ' ';
	}
	std::cout << std::dec << std::endl;
}

//void pktSplitItem(int slot, int itemid, int qty) {
//	p2E5 sendData;
//	ZeroMemory(&sendData, sizeof(sendData));
//	sendData.Header.packetSize = sizeof(p2E5);
//	sendData.Header.packetID = 0x2E5;
//	sendData.Header.referID = GetClientID();
//	sendData.slot = slot;
//	sendData.itemid = itemid;
//	sendData.splitqt = qty;
//	hSendPacket((char*)&sendData, sizeof(p2E5));
//	std::cout << "Split Item Sent" << std::endl;
//	return;
//}
//
//void pktDeleteItem(int slot) {
//	p2E4 sendData;
//	ZeroMemory(&sendData, sizeof(sendData));
//	sendData.Header.packetSize = sizeof(p2E4);
//	sendData.Header.packetID = 0x2E4;
//	sendData.Header.referID = GetClientID();
//	sendData.item = GetInvSlot(slot);
//	sendData.slot = slot;
//	hSendPacket((char*)&sendData, sizeof(p2E4));
//}

void MoveToXY(int x, int y, int maxdistance, int bufferoffset) {

	Vec2Int src = getXYVecInt();
	Vec2Int dst = { x,y };

	int gridX = ABS(src.x - x) * 2;
	int gridY = ABS(src.y - y) * 2;

	if (gridX < bufferoffset)
		gridX = bufferoffset;
	if (gridY < bufferoffset)
		gridY = bufferoffset;
	if (gridX > maxdistance)
		gridX = maxdistance;
	if (gridY > maxdistance)
		gridY = maxdistance;



	gridX += bufferoffset;
	gridY += bufferoffset;

	

	//int distance = findDistanceInt(src, dst);
	//if (distance > maxdistance) {
	//	distance = maxdistance;
	//}
	gs->bPendingMove = true;
	AStar* as = new AStar(gridX, gridY);
	if (gs->bGoalIsMob) {
		as->bIsMobDst = true;
		as->iMobOffset = 2;
	}
	as->ParsePath(as->FindPath(src, dst), false);
	as->BuildPacketPath();
	gs->Update();
	if (!gs->bPendingMove) {
		delete as;
		return;
	}
	for (auto& pkt : as->packetPath) {
		if (gs->TileHasEnt(pkt->destX, pkt->destY)) {
			//if the destination tile has an entity, try a neighboring tile
			int pX = pkt->destX;
			int pY = pkt->destY;

			//this nested loop is for checking each neighbor tile, if this works its possible to add additional bias for a certain direction
			for (int x = -1; x <= 1; x++) { 
				for (int y = -1; y <= 1; y++) {
					if (x == 0 && y == 0)
						continue;
					int nX = pX + x;
					int nY = pY + y;
					if (gs->TileHasEnt(nX, nY) && gs->TileIsPathable(nX, nY)) {
						continue;
					}
					pX = nX;
					pY = nY;
					goto exitLoops; // break out of nested loop
				}
			}
			exitLoops:
			//either it found a better tile and it sets that tile as the destination, or the destination will be unchanged.
			pkt->destX = pX;
			pkt->destY = pY;

		}

		if (PacketMove(*pkt) < 1) {
			bOffgrid = false;
			break;
		}
	}
	int minX = gs->v2GoalPos.x - 1;
	int maxX = gs->v2GoalPos.x + 1;
	int minY = gs->v2GoalPos.y - 1;
	int maxY = gs->v2GoalPos.y + 1;
	int posX = GetPosXInt();
	int posY = GetPosYInt();

	gs->v2GoalPos = { 0 };
	gs->bPendingMove = false;
	gs->bGoalIsMob = false;

	//redundant
	/*if ( (posX <= maxX && !(posX < minX)) && (posY <= maxY && !(posY < minY)) ) {
		gs->v2GoalPos = { 0 };
		gs->bPendingMove = false;
		gs->bGoalIsMob = false;
	}*/
	delete as;
}



void walkPathPkt(AStar& astar, Vec2Int dst, bool isMob, bool loopOffgrid) {
start:
	Vec2Int src = getXYVecInt();
	//std::list<Vec2Int> path;
	if (isMob) {
		astar.bIsMobDst = true;
		astar.iMobOffset = 1;
	}
	
	astar.ParsePath(astar.FindPath(src, dst), false);	
	astar.BuildPacketPath();
	for (auto& pkt : astar.packetPath) {
		if (PacketMove(*pkt) < 1) {
			bOffgrid = false;
			break;
		}
	}
	if(isMob)
		gs->ChangeCCMode(1, 0);
	astar.packetPath.clear();

	if (bOffgrid) {
		bOffgrid = false;
		std::cout << dye::aqua("Offgrid detected.") << std::endl;
		if (loopOffgrid) {
			astar.path.clear();
			goto start;
		}
	}

}


void walkPath(AStar* astar, Vec2Int dst) {
	start:
	Vec2Int src = getXYVecInt();
	std::list<Vec2Int> path;
	path = astar->ParsePath(astar->FindPath(src, dst), false);
	int gu = 0;
	if (path.size() > 0)
	{
		//return; // - Debug
		//int buf = 0;
		//auto cache = path;
		//vPathBuffer = cache;
		for (auto& p : path)
		{
			if (gu > 4) {
				bOffgrid = false;
				//vPathBuffer.clear();
				std::cout << dye::red("Walking Path Failed") << std::endl;
				break;
			}

			int fX = p.x;
			int fY = p.y;
			std::cout << dye::blue("Walking to ") << dye::blue(fX) << " " << dye::blue(fY) << std::endl;
			bool trywalk = WalkToPtAndVerify(Vec2Int{ fX, fY });
			if (!trywalk)
				gu++;
			else if (gu > 0)
				gu--;
			//if (!vPathBuffer.empty() && i > 0)
				//vPathBuffer.erase(vPathBuffer.begin());
		}
	}
	if (bOffgrid) {
		bOffgrid = false;
		std::cout << dye::aqua("Offgrid detected.") << std::endl;
		path.clear();
		goto start;
	}

}


Action::Action()
{
}



Action::Action(ActionType _type)
{
	Type = _type;
}

Action::Action(ActionType _type, int _level)
{
	Type = _type;
	iGoalInt = _level;
}

Action::Action(Vec2Int _goal)
{
	Type = PATHING;
	v2Goal = _goal;
}

Action::Action(Vec2Int _goal, bool _isMob)
{
	Type = PATHING;
	v2Goal = _goal;
	bGoalBool = true;
}

Action::Action(Rec _recGoal)
{
	Type = USE_TP;
	rect_Goal = _recGoal;
}

Action::Action(int _itemid)
{
	Type = USE_ITEM;
	iGoalInt = _itemid;
}

Action::Action(int _slotid, bool _slots)
{
	Type = USE_ITEM;
	bGoalBool = _slots;
	iGoalInt = _slotid;
}

Action::Action(int _slot, ActionType _type)
{
	if (_type != DELETE_ITEM)
		return;
	Type = _type;
	iGoalInt = _slot;
}

Action::Action(int _itemid, ActionType _type, bool _deleteitems)
{
	if (!_deleteitems)
		return;
	if (_type != DELETE_ITEM)
		return;
	Type = _type;
	bGoalBoolB = _deleteitems;
	bGoalBool = true;
	iGoalInt = _itemid;

}

Action::Action(int _start, int _end, ActionType _type)
{
	if (_type != DELETE_ITEM)
		return;
	Type = _type;
	vGoalIntVec.resize(2);
	bGoalBool = true;
	vGoalIntVec[0] = _start;
	vGoalIntVec[1] = _end;

}

Action::Action(std::string _npcName, int _itemid, bool sellall)
{
	//Sell all items of type
	Type = SELL_ITEM;
	bGoalBool = true;
	sGoal = _npcName;
	iGoalInt = _itemid;
}

Action::Action(std::string _npcName, int _itemslot, ActionType _type) {
	//buy/sell single item
	Type = _type;
	sGoal = _npcName;
	iGoalInt = _itemslot;
}

Action::Action(std::string _npcName, ActionType _type)
{
	Type = _type;
	sGoal = _npcName;
}

Action::Action(int _slot, int _itemid, int _qty)
{
}

//Move item
Action::Action(InventoryType _srctype, InventoryType _dsttype, int _srcslot, int _dstslot)
{
	Type = MOVE_ITEM;
	vGoalIntVec.resize(4 * sizeof(int));
	vGoalIntVec[0] = (int)_srctype;
	vGoalIntVec[1] = (int)_dsttype;
	vGoalIntVec[2] = _srcslot;
	vGoalIntVec[3] = _dstslot;
}

Action::Action(CCMODE _mode, CCMOVE _move)
{
	Type = SET_CCMODE;
	vGoalIntVec.resize(2 * sizeof(int));
	vGoalIntVec[0] = _mode;
	vGoalIntVec[1] = _move;
}

Action::~Action()
{
}

void Action::Execute()
{
	bCompleted = false;
	switch (Type) {
	default:
	case IDLE:
		Sleep(100);
		bCompleted = true;
		break;
	case PATHING:
		SetPath();
		break;
	case USE_TP:
		UseTP();
		break;
	case USE_ITEM:
		UseItem();
		break;
	case BUY_ITEM:
		BuyItem();
		break;
	case SELL_ITEM:
		SellItem();
		break;
	case SPLIT_ITEM:
		SplitItem();
		break;
	case OPEN_SHOP:
		OpenShop();
		break;
	case USE_NPC:
		UseNpc();
		break;
	case DELETE_ITEM:
		DeleteItem();
		break;
	case MOVE_ITEM:
		MoveItem();
		break;
	case SET_CCMODE:
		SetCCMode();
		break;
	case IDLE_UNTIL_LEVEL:
		IdleUntilLevel();
		break;
	case RESPAWN:
		Respawn();
		break;
	}
	
}


void Action::IdleUntilLevel()
{
	if (local->Level + 1 < iGoalInt) {
		Sleep(100);
		return;
	}
	std::cout << hue::yellow << "Idling Action completed, level is now " << iGoalInt << hue::reset << "\n";
	bCompleted = true;
}

void Action::SetPath()
{
	//bail 
	if (iIncrement > 25) {
		bCompleted = true;
		return;
	}

	
	int minX = v2Goal.x - 1;
	int maxX = v2Goal.x + 1;
	int minY = v2Goal.y - 1;
	int maxY = v2Goal.y + 1;
	int posX = GetPosXInt();
	int posY = GetPosYInt();

	//	if (GetPosXInt() != v2Goal.x || GetPosYInt() != v2Goal.y) {
	if (!((posX <= maxX && !(posX < minX)) && (posY <= maxY && !(posY < minY)))){
		gs->bGoalIsMob = bGoalBool;
		gs->bPendingMove = true;
		gs->v2GoalPos = v2Goal;
		Sleep(10);
		iIncrement++;
		return;
	}
	bCompleted = true;
	Sleep(10);
}

void Action::UseTP()
{

	//bail 
	if (iIncrement > 25) {
		bCompleted = true;
		return;
	}

	if (FindPoint(rect_Goal, getXYVecInt())) {
		UseTelePad();
		Sleep(100);
		bCompleted = true;
		return;
	}
	iIncrement++;
}

void Action::UseItem()
{
	//if this is true, iGoalInt is slot number not item id	
	if (bGoalBool) {
		gs->Inv->UseItem(iGoalInt);
		bCompleted = true;
		Sleep(25);
	}


	if (int slot = gs->Inv->FindFirst(iGoalInt) >= 0) {
		gs->Inv->UseItem(slot);
		bCompleted = true;
		Sleep(25);
	}
}

void Action::BuyItem()
{
	Sleep(100);
	for (auto& e : gs->EntityList) {
		if (e->name == sGoal) {
			if (e->fDistance < 13) {
				//Buy Item
				Sleep(100);
				gs->Inv->BuyItem(e->sEntId, iGoalInt);
				bCompleted = true;
				break;
			}
		}
	}
	
}

void Action::SellItem()
{
	Sleep(100);
	for (auto& e : gs->EntityList) {
		if (e->name == sGoal) {
			if (e->fDistance < 13) {
				//Sell Item
				Sleep(100);
				//if this is true then loop sell all items with id, otherwise sell one
				//if true iGoalInt will be itemid
				if (bGoalBool) {
					int slot = gs->Inv->FindFirst(iGoalInt);
					while(slot = gs->Inv->FindFirst(iGoalInt) >= 0)
						gs->Inv->SellItem( slot, e->sEntId);
					bCompleted = true;
					return;
				}
				//if not sell all, iGoalInt will be slot number
				gs->Inv->SellItem(iGoalInt, e->sEntId);
				bCompleted = true;
				return;
			}
		}
	}
}

void Action::OpenShop()
{

}


void Action::UseNpc()
{
	Sleep(100);
	for (auto& e : gs->EntityList) {
		if (e->name == sGoal) {
			if (e->fDistance < 8) {
				//Use NPC
				Sleep(100);
				pktUseNPC(e->sEntId, false);
				bCompleted = true;
				break;
			}			
		}
	}
}

void Action::SplitItem()
{
}

void Action::StackItem()
{
}

void Action::DeleteItem()
{
	//if this is true then its a delete loop instead of single item
	if (bGoalBool && !bGoalBoolB) {
		for (int i = 0; i < (vGoalIntVec[1] - vGoalIntVec[0]); i++) {
			if (gs->Inv->inv->items[i].Index == 0) {
				continue;
			}
			gs->Inv->DeleteItem(i);
			Sleep(100);
		}
		bCompleted = true;
		return;
	}
	//delete all items with this id
	if (bGoalBoolB) {
		while (gs->Inv->FindFirst(iGoalInt) >= 0) {
			int inv = gs->Inv->FindFirst(iGoalInt);
			if (inv < 0)
				break;
			gs->Inv->DeleteItem(inv);
			Sleep(100);

		}
		bCompleted = true;
		return;
	}

	//if the item slot is already empty, skip
	if (gs->Inv->inv->items[iGoalInt].Index == 0) {
		bCompleted = true;
		return;
	}
	gs->Inv->DeleteItem(iGoalInt);
	Sleep(100);
	bCompleted = true;
}

void Action::MoveItem()
{
	gs->Inv->MoveItem(vGoalIntVec[2], vGoalIntVec[3], vGoalIntVec[0], vGoalIntVec[1]);
	Sleep(100);
	bCompleted = true;
}

void Action::SetCCMode()
{
	gs->ChangeCCMode(vGoalIntVec[0], vGoalIntVec[1]);
	//Sleep(1);
	bCompleted = true;
}

void Action::Respawn()
{
	Sleep(2000); // just to not have it be instant
	p289 sendData;
	ZeroMemory(&sendData, sizeof(sendData));
	sendData.Header.packetSize = sizeof(p289);
	sendData.Header.packetID = 0x289;
	sendData.Header.referID = GetClientID();
	hSendPacket((char*)&sendData, sizeof(p289));
	Sleep(100);
}


Task::Task(TaskType _type, std::list<Vec2Int> _pathlist)
{
	Type = _type;
	for (auto& p : _pathlist) {
		lActionList.push_back(std::make_shared<Action>(randomXY(p, 1)));
	}

}

void Task::Execute()
{
	//if expression is false, mark completed and return
	if (!Evaluate()) {
		bCompleted = true;
		return;
	}

	//if expression is true, replace this task with its action list

	//remove this task from the gamestate action list so it can be replaced with its own action list
	//if the task is repeatable, leave the task in the list and insert actions in front
	if(!bRepeatable)
		gs->gsActions.pop_front();
	gs->gsActions.splice(gs->gsActions.begin(), lActionList);

	if (bRepeatable) {
		iIncrement++;
		return;
	}

	//this shouldn't matter since the task removes itself from the list
	bCompleted = true;
}

bool Task::Evaluate()
{
	if (lVal == -1) {
		switch (DataType) {
		case PLAYER_LEVEL:
			lVal = local->Level + 1;
			break;
		case PLAYER_X:
			lVal = local->pos.x;
			break;
		case PLAYER_Y:
			lVal = local->pos.y;
			break;
		case INV:
			return gs->Inv->FindFirst(rVal) > 0;
		}
	}

	if (Type == OTHER) {
		if (DataType == INV)
		{

		}
	}



	switch (Type) {
	case EQUAL:
		return lVal == rVal;
		break;
	case GREATER:
		return lVal > rVal;
		break;
	case GEQUAL:
		return lVal >= rVal;
		break;
	case LEQUAL:
		return lVal <= rVal;
		break;
	case LESS:
		return lVal < rVal;
		break;
	case NOT_EQUAL:
		return lVal != rVal;
		break;
	}
	return false;
}

Quest::Quest() {

}

std::list<std::shared_ptr<Action>> Quest::GetRegionAction(Regions _region)
{


	switch (_region) {
	case ARMIA:
		return std::move(ArmiaActionList);
		break;
	case AZRAN:
		return std::move(AzranActionList);
		break;
	case ERION:
		return std::move(ErionActionList);
		break;
	case SNOWTOWN:
		return std::move(SnowtownActionList);
		break;
	case NAOTUN:
		return std::move(NaotunActionList);
		break;
	default:
	case OUT_OF_TOWN:
		return std::move(ActionList);
		break;
	}
}


Quest40::Quest40() {
	bWarpSet = false;
	iReqLevel = 40;
	iMaxLevel = 115;
	vNoWarpPath = {};
	rect_InsideQuest = rmrQuest40;
	rect_QuestRegion = rmrArmiaField;
	v2QuestWarpLocation = { 2373, 2098 };
	sQuestNpcName = "Gravekeeper";
	vQuestMobNames = { "Skelton ", "SoulSkelton" };
	QuestActions = { };
	ActionList = {};
	ArmiaActionList = {};
	AzranActionList = {};
	ErionActionList = {};
	SnowtownActionList = {};
	NaotunActionList = {};
}

Quest116::Quest116()
{
	iReqLevel = 116;
	iMaxLevel = 190;
	
	rect_QuestRegion = rmrAzranField;
	rect_InsideQuest = rmrQuest116;	
	v2QuestWarpLocation = { 2221,1715 };
	sQuestNpcName = "Gardener";
	vQuestMobNames = { "Cabuncle High "};
	
	ArmiaActionList = {
		std::make_shared<Action>(POTION, NONE),
		std::make_shared<Action>(randomXY(Vec2Int{ 2118,2102 }, 1)), // path to naotun
		std::make_shared<Action>(rect_TpPadArmiaToNaotun),
	};
	AzranActionList = {
		std::make_shared<Action>(POTION, NONE),
		std::make_shared<Action>(randomXY(v2QuestWarpLocation, 1)), //path to quest
		std::make_shared<Action>(sQuestNpcName, USE_NPC),
	};
	ErionActionList = {
		std::make_shared<Action>(POTION, NONE),
		std::make_shared<Action>(randomXY(Vec2Int{ 2458,2018 }, 1)), // path to naotun
		std::make_shared<Action>(rect_TpPadErionToNaotun),
	};
	SnowtownActionList = {
		std::make_shared<Action>(POTION, NONE),
		std::make_shared<Action>(randomXY(Vec2Int{ 3649,3109 }, 1)), // path to naotun
		std::make_shared<Action>(rect_TpPadSnowToNaotun),
	};
	NaotunActionList = {
		std::make_shared<Action>(POTION, NONE),
		std::make_shared<Action>(randomXY(Vec2Int{ 1046,1718 }, 1)), // path to azran
		std::make_shared<Action>(rect_TpPadNaotunToAzran)
	};
	ActionList = AzranActionList;
}

Quest191::Quest191()
{
	iReqLevel = 191;
	iMaxLevel = 265;
	
	rect_QuestRegion = rmrDungeon1b;
	rect_InsideQuest = rmrQuest191;
	v2QuestWarpLocation = { 451,3912 };
	sQuestNpcName = "";
	vQuestMobNames = { "", "" };
	ArmiaActionList = {
		std::make_shared<Action>(POTION, NONE),
		std::make_shared<Action>(randomXY(Vec2Int{ 2118,2102 }, 1)), // path to naotun
		std::make_shared<Action>(rect_TpPadArmiaToNaotun),
	};
	AzranActionList = {
		std::make_shared<Action>(POTION, NONE),
		std::make_shared<Action>(randomXY(Vec2Int{2481,1718}, 1)), //path to naotun
		
	};
	ErionActionList = {
		std::make_shared<Action>(POTION, NONE),
		std::make_shared<Task>(PATHLIST, pathErionToDungeon1aEntrance), // path to quest
		std::make_shared<Action>(rmrDungeonEnt1), //use tp
		std::make_shared<Task>(PATHLIST, pathDungeonEntranceToQuest191),
		std::make_shared<Action>(sQuestNpcName, USE_NPC),
	};
	SnowtownActionList = {
		std::make_shared<Action>(POTION, NONE),
		std::make_shared<Action>(randomXY(Vec2Int{ 3649,3109 }, 1)), // path to naotun
		std::make_shared<Action>(rect_TpPadSnowToNaotun),
	};
	NaotunActionList = {
		std::make_shared<Action>(POTION, NONE),
		std::make_shared<Action>(randomXY(Vec2Int{ 1045,1709 }, 1)), // path to erion
		std::make_shared<Action>(rect_TpPadNaotunToAzran)
	};
	ActionList = AzranActionList;
}

Quest266::Quest266()
{
	iReqLevel = 266;
	iMaxLevel = 320;
	
	rect_QuestRegion = rmrDungeon1a;
	rect_InsideQuest = rmrQuest266;
	v2QuestWarpLocation =  {668,3771 } ;
	sQuestNpcName = "";
	vQuestMobNames = { "", "" };
	ActionList = {};
	ArmiaActionList = {};
	AzranActionList = {};
	ErionActionList = {};
	SnowtownActionList = {};
	NaotunActionList = {};
}

Quest321::Quest321()
{
	iReqLevel = 321;
	iMaxLevel = 355;
	
	rect_QuestRegion = rmrUnderworld;
	rect_InsideQuest = rmrQuest321;
	v2QuestWarpLocation = { 1300,4040 };
	sQuestNpcName = "";
	vQuestMobNames = { "", "" };
	ActionList = {};
	ArmiaActionList = {};
	AzranActionList = {};
	ErionActionList = {};
	SnowtownActionList = {};
	NaotunActionList = {};
}
