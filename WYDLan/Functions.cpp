#include "pch.h"
#include "Functions.h"
#include "GameState.h"
#include "UI/ConsoleMenu/Menu.h"
#include "AStar.h"
#include "Bot.h"
#include "src/Wyd_Rupture/wyd_Rupture.h"
#include <winuser.h>
#include "UI/GUI/EasyWindow.h"
#include <stdio.h>
#include "UI/GUI/ImWindow/ImWindow/ImWindowDX11/ImwWindowManagerDX11.h"
#include "UI/GUI/ImWindow/ImWindow/sample.h"

extern DWORD moduleBase;
FILE* fp;
FILE* fi;
FILE* fe;
Ent* local;
DWORD addr_SceneState, lastEntIdx, base_addr1, base_addr2, pfXInit_addr, pfYInit_addr, pfAddr, moveXaddr, moveYaddr, sendpacket_addr, movechar_addr, ccmode_addr, ccmove_addr, zoomlimit_addr, zoom_addr, sourceX_addr, sourceY_addr, destX_addr, destY_addr;
extern bool bPathingTest, bLastEntIdx, bMoveToTargets, bLanBotEnabled, bQuestBotEnabled, bFarmBotEnabled;
bool logPackets = false;
extern GameState* gs;
extern pfd* pd;
bool bOffgrid = false;
SendPacketT pSendPacket = nullptr;
 
/* Other "fixes"      -possible crash cause
bool bZoomLimit = true;
float fZoomLimit = 32.0f;
bool bZoomLock = false;
float fZoom = 32.0f;
*/



//Pattern Scanner
char exename[] = "WYD.exe";
char sendPacketSig[] = "\x55\x8b\xec\x56\x8b\x75\x00\x85\xf6\x0f\x84\x00\x00\x00\x00\xa1";
char sendPacketMask[] = "xxxxxx?xxxx????x";

char buildRegionSig[] = "\x55\x8b\xec\x51\x8b\x45\x0c\x8b\x0d\xa4\x78\x58\x00\x53\x56";
char buildRegionMask[] = "xxxxxx?xx????xx";

char receivePacketsSig[] = "\x56\x57\x8b\xf1\x8b\x46\x10\x8b\x4e\x08\x8b\x16\x6a\x00\xbf\x00\x00\x02\x00\x2b\xf8\x57";
char receivePacketsMask[] = "xxxxxxxxxxxxxxxxxxxxxx";

char moveCharAddr[] = "\x55\x8b\xec\x57\x8b\xf9\x83\xbf\xc8\x01\x00\x00\x00\x74\x07";
char moveCharAddrMask[] = "xxxxxxxxxxxxxxx";



void Patch(BYTE* dst, BYTE* src, unsigned int size)
{
	DWORD oldprotect;
	VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldprotect);

	memcpy(dst, src, size);
	VirtualProtect(dst, size, oldprotect, &oldprotect);
}

Vec2Int randomXY(const Vec2Int& _start, const int& variance) {
	Vec2Int High = { _start.x + variance, _start.y + variance };
	Vec2Int Low = { _start.x - variance, _start.y - variance };

	int x = rand() % (High.x - Low.x) + Low.x;
	int y = rand() % (High.y - Low.y) + Low.y;

	return Vec2Int{ x,y };

}

short GetLimitSlot() { // Returns the limit of possible slots for the player (according to the wanderer's bag)
	short limitSlot = 30;
	if (GetInvSlot(60) == 3467) limitSlot += 15;
	if (GetInvSlot(61) == 3467) limitSlot += 15;
	return limitSlot;
}

bool FindPoint(Rec rect, Vec2Int pos) {
	if (pos.x < rect.h.x and pos.x > rect.l.x and pos.y < rect.h.y and pos.y > rect.l.y)
		return true;
	return false;
}

float sQ(float val) {
	return val * val;
}

uintptr_t FindDMAAddy(uintptr_t ptr, std::vector<unsigned int> offsets)
{
	uintptr_t addr = ptr;
	for (unsigned int i = 0; i < offsets.size(); ++i)
	{
		addr = *(uintptr_t*)addr;
		addr += offsets[i];
	}
	return addr;
}

Vec2Int getXYVecInt() {
	return Vec2Int{ GetPosXInt(), GetPosYInt() };
}
Vec2 getXYVec() {
	return Vec2{ gs->localPlayer->vecOriginX, gs->localPlayer->vecOriginZ };
}
int findDistanceInt(Vec2Int pt1, Vec2Int pt2) {
	return sqrt(sQ(ABS(pt1.x - pt2.x)) + sQ(ABS(pt1.y - pt2.y)));
}
float findDistance(Vec2 pt1, Vec2 pt2) {
	return sqrt(sQ(ABS(pt1.x - pt2.x)) + sQ(ABS(pt1.y - pt2.y)));
}

//========================= EasyWindow ==================================
void OnSize(const EasyWindow* /*pWindow*/, int iW, int iH)
{
	printf("OnSize %d %d\n", iW, iH);
}

bool OnClose(const EasyWindow* /*pWindow*/)
{
	printf("OnClose\n");
	static int s_iCloseCount = 0;
	return s_iCloseCount++ < 3;
}

void OnChar(const EasyWindow* /*pWindow*/, int iChar)
{
	printf("OnChar %c\n", iChar);
}

void OnKey(const EasyWindow* /*pWindow*/, EasyWindow::EKey eKey, bool bDown)
{
	printf("OnKey %s %d %s\n", bDown ? "down" : "up", eKey, EasyWindow::KeyToString(eKey));
}
//========================EasyWindowEND===================================


void SetUp() {
	AllocConsole();
	freopen_s(&fp, "CONOUT$", "w", stdout);
	freopen_s(&fi, "CONIN$", "r", stdin);
	freopen_s(&fe, "CONOUT$", "w", stderr);
	SigScan ss;
	moduleBase = (uintptr_t)GetModuleHandle("WYD.exe");
	addr_SceneState = FindDMAAddy((moduleBase + SceneOffset), { 0x14 });
	base_addr1 = (DWORD)(moduleBase + playerinfo_offset);
	base_addr2 = (DWORD)(moduleBase + pObj_offset);
	pfAddr = (uintptr_t)(*(uintptr_t**)base_addr1) + 0xD4;
	ccmode_addr = (DWORD)(moduleBase + ccMode_offset);
	pfXInit_addr = (DWORD)(moduleBase + pfXInit_offset);
	pfYInit_addr = (DWORD)(moduleBase + pfYInit_offset);
	local = (Ent*)FindDMAAddy(moduleBase + playerinfo_offset, { 0x5C, 0x0 });
	std::cout << hue::aqua << "Local Player: 0x" << local << hue::reset << std::endl;
	std::cout << hue::aqua << "Local Player: 0x" << local << hue::reset << std::endl;
	std::cout << hue::aqua << "Local Player: 0x" << local << hue::reset << std::endl;
	std::cout << hue::aqua << "Local Player: 0x" << local << hue::reset << std::endl;
	moveXaddr = FindDMAAddy(moduleBase + playerinfo_offset, { moveX_offset });
	moveYaddr = FindDMAAddy(moduleBase + playerinfo_offset, { moveY_offset });
	zoomlimit_addr = FindDMAAddy(moduleBase + playerinfo_offset, { 0x4,0x10,0xC,0xBC });
	zoom_addr = FindDMAAddy(moduleBase + playerinfo_offset, { 0x3C,0x4,0xC,0x34});
	ccmove_addr = FindDMAAddy(moduleBase + playerinfo_offset, { ccMove_offset });
	sendpacket_addr = ss.FindPattern(exename, sendPacketSig, sendPacketMask);
	movechar_addr = ss.FindPattern(exename, moveCharAddr, moveCharAddrMask);
	sourceX_addr = FindDMAAddy(base_addr1, { 0x285d5 });
	sourceY_addr = FindDMAAddy(base_addr1, { 0x285d9 });
	destX_addr	 = FindDMAAddy(base_addr1, { 0x285c5 });
	destY_addr	 = FindDMAAddy(base_addr1, { 0x285c9 });
	if (sendpacket_addr == NULL)
		std::cout << dye::red("Sendpacket Address not found.") << std::endl;


	LoadPFData();
	std::cout << hue::yellow << std::hex << "sendpacket_addr 0x" << sendpacket_addr << hue::reset << std::dec << std::endl;
	if(*(int*)addr_SceneState > 3){
		lastEntIdx = FindDMAAddy(moduleBase + playerinfo_offset, { 0x5C, 0x4, 0x10 });
		bLastEntIdx = true;
	}

}

void Hooks() {
	pSendPacket = (SendPacketT)DetourFunction((PBYTE)sendpacket_addr, (PBYTE)hSendPacket);
}

void PrintHotkeyInfo() {


	Menu menu = Menu("Hotkey Information", "Press [Ctrl][End] to Exit");
	std::cout << dye::yellow("Press [Ctrl][End] to Exit") << std::endl;
	std::cout << dye::yellow("Press [INSERT] to toggle GUI_0.0.1") << std::endl;
	std::cout << dye::yellow("Press [NUMPAD0] to") << dye::light_aqua(" Clear Console and Reset to Default") << std::endl;
	std::cout << dye::yellow("Press [NUMPAD1] to") << dye::light_aqua(" Toggled Mortal Quest On") << std::endl;
	std::cout << dye::yellow("Press [NUMPAD2] to") << dye::light_aqua(" Toggle Lan Bot On") << std::endl;
	std::cout << dye::yellow("Press [NUMPAD3] to") << dye::light_aqua(" Toggle Farming Bot ON") << std::endl;
	std::cout << dye::yellow("Press [NUMPAD4] to") << dye::light_aqua("") << std::endl;
	std::cout << dye::yellow("Press [NUMPAD5] to") << dye::light_aqua(" Sending Packet Test") << std::endl;
	std::cout << dye::yellow("Press [NUMPAD6] to") << dye::light_aqua("") << std::endl;
	std::cout << dye::yellow("Press [NUMPAD7] to") << dye::light_aqua(" Print Entity List") << std::endl;
	std::cout << dye::yellow("Press [NUMPAD8] to") << dye::light_aqua(" Load new TK Char") << std::endl;
	std::cout << dye::yellow("Press [NUMPAD9] to") << dye::light_aqua("") << std::endl;
	std::cout << dye::yellow("Press [NUMPAD*] to") << dye::light_aqua(" Toggle print path info On") << std::endl;
	std::cout << dye::yellow("Press [NUMPAD/] to") << dye::light_aqua(" Toggle printing packets On") << std::endl;
	std::cout << dye::yellow("Press [NUMPAD-] to") << dye::light_aqua(" Toggle Move to targets On") << std::endl;
}

void HotkeyLoop() {

	if (!GetAsyncKeyState(VK_CONTROL) && GetAsyncKeyState(VK_INSERT) && 0x8000) {
		std::cout << "Starting WYDLan GUI_v0.0.1" << std::endl;
		std::cout.flush();
		for (;;)
		{
			for (int j = 0; j < 3; j++)
			{
				std::cout << "\rLoading   \rLoading";
				for (int i = 0; i < 3; i++)
				{
					std::cout << ".";
					Sleep(1000);
				}
				std::cout << "GUI Started" << std::endl;

			}
			PreInitSample();
			ImwWindowManagerDX11 oMgr(true);
			oMgr.Init();
			InitSample();
			while (oMgr.Run(false) && oMgr.Run(true)) Sleep(16);
			{
				{
					
				}
			}
			Sleep(5000);
		}
		
	}

	// Clear Console // Action List
	if (!GetAsyncKeyState(VK_CONTROL) && GetAsyncKeyState(VK_NUMPAD0) && 0x8000) {
		std::cout << "Clear Console and Reset to Default" << std::endl;
		gs->gsActions.clear();
		gs->bPendingMove = false;
		CREATE_NEW_CONSOLE;
		PrintHotkeyInfo();
		Sleep(5000);
	}
	// Print Path ON/OFF
	if (GetAsyncKeyState(VK_CONTROL) && GetAsyncKeyState(VK_MULTIPLY) && 0x8000) {
		std::cout << "Toggled print path On" << std::endl;
		bPathingTest = true;
		Sleep(1000);
	}
	if (GetAsyncKeyState(VK_CONTROL) && GetAsyncKeyState(VK_MULTIPLY) && 0x8000) {
		std::cout << "Toggled print path OFF" << std::endl;
		bPathingTest = false;
		Sleep(1000);
	} 
	// Toggle Print Packets ON/OFF
	if (!GetAsyncKeyState(VK_CONTROL) && GetAsyncKeyState(VK_DIVIDE) && 0x8000) {
		std::cout << "Toggle print packets On" << std::endl;
		bool wasLogging = logPackets;
		logPackets = true;
		Sleep(1000);
	}
	if (GetAsyncKeyState(VK_CONTROL) && GetAsyncKeyState(VK_DIVIDE) && 0x8000) {
		std::cout << "Toggle print packets OFF" << std::endl;
		bool wasLogging = logPackets;
		logPackets = false;
		Sleep(1000);
	}
	// Toggle Move to Target ON/OFF
	if (!GetAsyncKeyState(VK_CONTROL) && GetAsyncKeyState(VK_SUBTRACT) && 0x8000) {
		std::cout << "Toggled Move to Target On" << std::endl;
		bMoveToTargets = true;
		Sleep(1000);
	}
	if (GetAsyncKeyState(VK_CONTROL) && GetAsyncKeyState(VK_SUBTRACT) && 0x8000) {
		std::cout << "Toggled Move to Target Off" << std::endl;
		bMoveToTargets = false;
		Sleep(1000);
	}
	//Toggle Mortal Bot -> 1
	if (!GetAsyncKeyState(VK_CONTROL) && GetAsyncKeyState(VK_NUMPAD1) && 0x8000) {
		bQuestBotEnabled = true;
		std::cout << "Toggled Mortal Quest Bot On" << std::endl;
		Sleep(1000);
	}
	if (GetAsyncKeyState(VK_CONTROL) && GetAsyncKeyState(VK_NUMPAD1) && 0x8000) {
		bQuestBotEnabled = false;
		gs->gsActions.clear();
		gs->bPendingMove = false;
		std::cout << "Toggled Mortal Quest Bot Off" << std::endl;
		Sleep(1000);
	}
	// LAN BOT ON/OFF -> 2
	if (!GetAsyncKeyState(VK_CONTROL) && GetAsyncKeyState(VK_NUMPAD2) && 0x8000) {
		bLanBotEnabled = true;
		gs->targets.clear();
		gs->targets.insert(gs->targets.end(), { "PerGorgon","PerGagoil","PerKnight","PerSpider" });
		std::cout << "Toggled Lan Bot On" << std::endl;
		Sleep(1000);
	}
	if (GetAsyncKeyState(VK_CONTROL) && GetAsyncKeyState(VK_NUMPAD2) && 0x8000) {
		bLanBotEnabled = false;
		gs->targets.clear();
		std::cout << "Toggled Lan Bot Off" << std::endl;
		Sleep(1000);
	}
	//Toggle Farm Bot -> 3
	if (!GetAsyncKeyState(VK_CONTROL) && GetAsyncKeyState(VK_NUMPAD3) && 0x8000) {
		bFarmBotEnabled = true;
		std::cout << "Toggled Farm Bot On" << std::endl;
		Sleep(1000);
	}
	if (GetAsyncKeyState(VK_CONTROL) && GetAsyncKeyState(VK_NUMPAD3) && 0x8000) {
		bFarmBotEnabled = false;
		gs->gsActions.clear();
		gs->bPendingMove = false;
		std::cout << "Toggled Farm Bot Off" << std::endl;
		Sleep(1000);
	}
	/*/Load New TK Char
	if (!GetAsyncKeyState(VK_CONTROL) && GetAsyncKeyState(VK_NUMPAD7) && 0x8000) {
		gs->gsActions = actionNewCharTK;
		std::cout << "Loaded New Char TK" << std::endl;
		Sleep(1000);
	}
	//Clear Action List
	if (GetAsyncKeyState(VK_CONTROL) && GetAsyncKeyState(VK_NUMPAD8) && 0x8000) {
		gs->gsActions.clear();
		gs->bPendingMove = false;
		std::cout << "Cleared Action List" << std::endl;
		Sleep(100);
	}
	*/
	if (GetAsyncKeyState(VK_NUMPAD4) && 0x8000) {
		p290 sendData;
		ZeroMemory(&sendData, sizeof(sendData));
		sendData.Header.packetSize = sizeof(p290);
		sendData.Header.packetID = 0x290;
		sendData.Header.referID = GetClientID();
		sendData.unk = 0x0;
		hSendPacket((char*)&sendData, sizeof(p290));
		Sleep(100);
	}

	if (GetAsyncKeyState(VK_NUMPAD6) && 0x8000) {
		if (!local->isDead)
			return;
		p289 sendData;
		ZeroMemory(&sendData, sizeof(sendData));
		sendData.Header.packetSize = sizeof(p289);
		sendData.Header.packetID = 0x289;
		sendData.Header.referID = GetClientID();
		hSendPacket((char*)&sendData, sizeof(p289));
		Sleep(100);
	}

	if (!GetAsyncKeyState(VK_CONTROL) && GetAsyncKeyState(VK_NUMPAD7) && 0x8000) {
		gs->PrintEntList();
		Sleep(100);
	}

	if (GetAsyncKeyState(VK_NUMPAD9) && 0x8000) {
		bool wasLogging = logPackets;
		logPackets = false;
		int x = 0;
		int y = 0;
		std::cout
			<< "[Prompt]Enter Target Coords(XXXX) :" << std::endl;
		std::string xs;
		std::getline(std::cin, xs);
		if (!xs.empty()) {
			x = stoi(xs);
			std::cout
				<< "[Prompt]Enter Target Coords(YYYY) :" << std::endl;
			std::string s;
			std::getline(std::cin, s);
			if (!s.empty()) {
				if (wasLogging)
					logPackets = true;
				y = stoi(s);
				gs->gsActions.push_front(std::make_unique<Action>(Vec2Int{ x,y }));
			}
		}
		else
		{
			std::cout << "Target coords unchanged" << std::endl;
		}
		
		Sleep(1000);
	}


}

void EveryLoop()
{
	//if (bZoomLimit)
		//*(float*)zoomlimit_addr = fZoomLimit;
	//if (bZoomLock)
		//*(float*)zoom_addr = fZoom;
}

Vec2Int FindRandomPtInRec(Rec rect, int range) {
	int difx = rect.h.x - rect.l.x;
	int dify = rect.h.y - rect.l.y;
	int randX = rand() % difx + rect.l.x;
	int randY = rand() % dify + rect.l.y;

	while (*GetPFdata(randX, randY) == (unsigned char)0x7F) {
		randX = rand() % difx + rect.l.x;
		randY = rand() % dify + rect.l.y;
	}

	return Vec2Int{ randX, randY };

}

DWORD __stdcall EjectThread(LPVOID lpParameter) {
	Sleep(100);
	FreeLibraryAndExitThread(myhModule, 0);
}

void CleanUp() {
	DetourRemove((PBYTE)pSendPacket, (PBYTE)hSendPacket);
	delete gs;
	delete pd;
	fclose(fe);
	fclose(fi);
	fclose(fp);
	FreeConsole();
	CreateThread(0, 0, EjectThread, 0, 0, 0);
}

void LoadPFData() {
	std::ifstream infile("testHM.dat", std::ios_base::binary);
	char* buf = new char[0x1000000];
	size_t chars_read;
	//read file
	if (!(infile.read(buf, 0x1000000))) // read up to the size of the buffer
	{
		if (!infile.eof()) // end of file is an expected condition here and not worth 
						   // clearing. What else are you going to read?
		{
			// something went wrong while reading. Find out what and handle.
			std::cout << dye::red("Unable to read PF data file") << std::endl;
		}
	}
	chars_read = infile.gcount();
	pd = (pfd*)buf;
}

Vec2Int ConvertSubGrid(int grid, int desgrid, Vec2Int coords) {
	if (grid == desgrid)
		return coords;
	Vec2Int newgrid = { 0 };

	if (grid == 0) {
		switch (desgrid) {
		case 1:
			return Vec2Int{ coords.x + 128, coords.y };
			break;
		case 2:
			return Vec2Int{ coords.x, coords.y + 128 };
			break;
		case 3:
			return Vec2Int{ coords.x + 128, coords.y + 128 };
			break;
		}
	}

	if (grid == 1) {
		switch (desgrid) {
		case 0:
			return Vec2Int{ coords.x - 128, coords.y };
			break;
		case 2:
			return Vec2Int{ coords.x - 128, coords.y + 128 };
			break;
		case 3:
			return Vec2Int{ coords.x, coords.y + 128 };
			break;
		}
	}

	if (grid == 2) {
		switch (desgrid) {
		case 0:
			return Vec2Int{ coords.x , coords.y - 128 };
			break;
		case 1:
			return Vec2Int{ coords.x + 128, coords.y - 128 };
			break;
		case 3:
			return Vec2Int{ coords.x + 128, coords.y };
			break;
		}
	}

	if (grid == 3) {
		switch (desgrid) {
		case 0:
			return Vec2Int{ coords.x - 128, coords.y - 128 };
			break;
		case 1:
			return Vec2Int{ coords.x , coords.y - 128 };
			break;
		case 2:
			return Vec2Int{ coords.x - 128, coords.y };
			break;
		}
	}

	return newgrid;
}

int GetCurrentSubGrid() {
	int xOrg = *(int*)pfXInit_addr;
	int yOrg = *(int*)pfYInit_addr;
	int x = GetPosXInt() - xOrg;
	int y = GetPosYInt() - yOrg;
	int subGridIdx = -1;
	if (x <= 127 && y <= 127)
		subGridIdx = 0;
	if (x >= 128 && x <= 127)
		subGridIdx = 1;
	if (x <= 127 && y >= 128)
		subGridIdx = 2;
	if (x >= 128 && y >= 128)
		subGridIdx = 3;
	return subGridIdx;
}

unsigned char* GetPFdata(int x, int y) {
	int idx = y * 4096 + x;
	return &pd->data[idx];
}

std::vector<Vec2Int> FindWavePropagatedPath(Vec2Int tgtpos, int gridx, int gridy) {
	std::vector<Vec2Int> path;
	Vec2Int startpos = getXYVecInt();
	int iGridSzX = size_pGridX + gridx;
	int iGridSzY = size_pGridY + gridy;

	std::shared_ptr<WaveProp> wp = std::make_shared<WaveProp>(iGridSzX, iGridSzY, startpos, tgtpos);

	if (wp->IsOffgrid()) {
		bOffgrid = true;
		return path;
	}

	wp->ProcessNodes();
	wp->BuildPath();
	//wp->ParsePath(); // optional
	path = wp->path;
	return path;
}

pNode::pNode(int worldX, int worldY, int _x, int _y) {
	x = _x;
	y = _y;
	WorldX = worldX;
	WorldY = worldY;
	bPassable = pfData();
}

bool pNode::pfData() {

	if (*GetPFdata(WorldX, WorldY) == (char)0x7F)
		return false;

		return true;
}

void WaveProp::ParsePath() {
	std::vector<Vec2Int> parsed;
	Vec2Int lastAdded = path[0];
	Vec2Int directionOld = { 0 };


	for (int i = 1; i < path.size(); i++) {
		Vec2Int directionNew = Vec2Int{ (path[i - 1].x - path[i].x), (path[i - 1].y - path[i].y) };
		if (directionOld.x != directionNew.x && directionOld.y != directionOld.y) {
			parsed.push_back(path[i]);
			lastAdded = path[i];
		}
		directionOld = directionNew;
		if (i == path.size() - 1) {
			parsed.push_back(path[i]);
			break;
		}
		if (findDistanceInt(Vec2Int{ lastAdded.x,lastAdded.y }, Vec2Int{ path[i].x ,path[i].y }) <= 3) {
			continue;
		}
		parsed.push_back(path[i]);
		lastAdded = path[i];
	}

	path = parsed;
}

WaveProp::WaveProp(int _gridSizeX, int _gridSizeY, Vec2Int _start, Vec2Int _tgt) {
	GridSizeX = _gridSizeX;
	GridSizeY = _gridSizeY;
	startpos = _start;
	tgtpos = _tgt;
	Grid.resize(GridSizeX * GridSizeY);
	InitGrid();
}


void WaveProp::InitGrid() {
	int xFit = startpos.x - iHomeX();
	int yFit = startpos.y - iHomeY();
	for (int x = 0; x < GridSizeX; x++) {
		for (int y = 0; y < GridSizeY; y++) {

			Grid[y * GridSizeY + x] = std::make_shared<pNode>(x + xFit, y + yFit, x, y);
		}
	}
}

//void WaveProp::ResizeGrid(int _gridSizeX, int _gridSizeY) {
//	GridSizeX = _gridSizeX;
//	GridSizeY = _gridSizeY;
//}

bool WaveProp::IsOffgrid() {
	if (std::abs(startpos.x - tgtpos.x) > ((GridSizeX - 1) / 2) || std::abs(startpos.y - tgtpos.y) > ((GridSizeY - 1) / 2))
		return true;
	return false;
}

int WaveProp::iHomeX() {
	return ((GridSizeX - 1) / 2);
}
int WaveProp::iHomeY() {
	return ((GridSizeY - 1) / 2);
}

int WaveProp::iHomeIdx() {
	return XYtoIdx(iHomeX(), iHomeY());
}

int WaveProp::XYtoIdx(int _x, int _y) {
	return (_y * GridSizeY + _x);
}

std::shared_ptr<pNode> WaveProp::GetTargetNode() {
	int x = (tgtpos.x - startpos.x) + iHomeX();
	int y = (tgtpos.y - startpos.y) + iHomeY();
	return Grid[y * GridSizeY + x];
}

void WaveProp::ProcessNodes() {
	targetNode = GetTargetNode();
	/*std::cout << hue::aqua << "Target Node: " << hue::blue << "Grid XY: "
		<< targetNode->x << " " << targetNode->y << hue::purple
		<< " World XY: " << targetNode->WorldX << " " << targetNode->WorldY
		<< hue::reset << std::endl;*/

	targetNode->NodeValue = 1;
	processing.insert(targetNode.get());
	while (!processing.empty()) {

		for (auto it = processing.begin(); it != processing.end();) {
			auto n = *it;
			for (int x = -1; x <= 1; x++) {
				for (int y = -1; y <= 1; y++) {
					if (x == 0 && y == 0)
						continue;
					int checkX = n->x + x;
					int checkY = n->y + y;
					if (checkX >= 0 && checkX < GridSizeX && checkY >= 0 && checkY < GridSizeY) {
						int checkIdx = XYtoIdx(checkX, checkY);
						if (Grid[checkIdx]->NodeValue > 0 || !Grid[checkIdx]->bPassable) {
							if (Grid[checkIdx]->NodeValue == 0)
								Grid[checkIdx]->NodeValue = -1;
							/*std::cout << dye::purple("Node obstructed bool: ") << targetNode->bPassable << " "
								<< std::hex << int(unsigned char(*GetPFdata(Grid[checkIdx]->WorldX, Grid[checkIdx]->WorldY))) << std::dec << hue::yellow << " XY: " << Grid[checkIdx]->WorldX
								<< " " << Grid[checkIdx]->WorldY << hue::reset << std::endl;*/
							continue;
						}
						Grid[checkIdx]->NodeValue = n->NodeValue + 1;

						/*std::cout << dye::green("Node: ") << checkIdx << " Grid XY:"
							<< Grid[checkIdx]->x << " " << Grid[checkIdx]->y << " "
							<< hue::yellow << " World XY: " << Grid[checkIdx]->WorldX
							<< " " << Grid[checkIdx]->WorldY <<  hue::light_green
							<< " Value:" << Grid[checkIdx]->NodeValue << hue::reset << std::endl;*/

						discovered.insert(Grid[checkIdx].get());
					}

				}
			}
			it = processing.erase(it);
		}


		for (auto d : discovered) {
			processing.insert(d);
		}
		discovered.clear();

	}
}

void WaveProp::BuildPath() {
	std::shared_ptr<pNode> curNode = Grid[iHomeIdx()];
	//for every node starting with the player
	std::shared_ptr<pNode> best = curNode;
	for (int p = 0; p < sizeof(Grid); p++) {
		path.push_back(Vec2Int{ curNode->WorldX, curNode->WorldY });
		if (curNode == targetNode || curNode->NodeValue == 1) {

		}
		//check each neighbor

		for (int x = -1; x <= 1; x++) {
			for (int y = -1; y <= 1; y++) {
				if (x == 0 && y == 0)
					continue;

				int checkX = curNode->x + x;
				int checkY = curNode->y + y;
				if (checkX >= 0 && checkX < GridSizeX && checkY >= 0 && checkY < GridSizeY) {
					int checkIdx = XYtoIdx(checkX, checkY);
					//std::cout << hue::purple << "Node Checked for Path: NodeValue: " << Grid[checkIdx]->NodeValue << " XY: "
					//	<< Grid[checkIdx]->x << " " << Grid[checkIdx]->y << " World XY: " << Grid[checkIdx]->WorldX << " " << Grid[checkIdx]->WorldY << " Passable: " << Grid[checkIdx]->bPassable << hue::reset << std::endl;
					if (Grid[checkIdx]->NodeValue < best->NodeValue && Grid[checkIdx]->bPassable == true && Grid[checkIdx]->NodeValue > 0)
						best = Grid[checkIdx];
				}
			}
		}
		//std::cout << hue::yellow << "Node Selected for Path: NodeValue: " << best->NodeValue << " XY: " 
		//	<< best->x << " " << best->y << " World XY: " << best->WorldX << " " << best->WorldY  << " Passable: " << best->bPassable << hue::reset << std::endl;
		curNode = best;
	}

	//for (auto p : path) {
	//	std::cout << hue::light_blue << "Path XY " << p.x << " " << p.y << hue::reset << std::endl;
	//}

}



void __cdecl hSendPacket(char* buffer, int len) {
	if (!logPackets) {
		pSendPacket(buffer, len);
		return;
	}

	//do whatever with packets
	std::vector<char> vecData(len);
	memcpy(vecData.data(), buffer, len);

	packetHeader* header = (packetHeader*)vecData.data();

	/*	if (!hSendPackCommands(header, vecData))
			return;*/

			//chat packet header


	std::cout << dye::black_on_white("Size:") << " " << std::hex << len << " " << dye::black_on_white("OP code:") << "0x" << header->packetID << std::dec << std::endl;

	std::cout << dye::yellow("Client: \n") << dye::black_on_white("Header:") << " " << std::hex;

	int c = 0;
	for (char i : vecData) {
		if (c == sizeof(packetHeader))
			std::cout << std::endl << "Struct: ";
		char n[10];
		sprintf_s(n, "%02x", (unsigned char)i);
		if ((unsigned char)i > (unsigned char)0x0) {
			std::cout << dye::on_white(n) << ' ';
			c++;
			continue;
		}
		std::cout << n << ' ';
		c++;
	}
	std::cout << "\n";

	/*for (int i = 0; i < len; i++) {
		if (i == sizeof(packetHeader) + 1)
			std::cout << std::endl << "Struct: ";
		std::cout << std::hex  << int(buffer[i]) << ' ';

	}*/
	
	
	//movement packet
	//if (header->packetID == 0x36C && gs->bPktBufferSet) {
	//	std::cout << dye::yellow("AStar: \n") << dye::black_on_white("Header:") << " " << std::hex;
	//	int c = 0;
	//	for (auto i : gs->pktBuffer) {
	//		if (c == sizeof(packetHeader))
	//			std::cout << std::endl << "Struct: ";
	//		char n[10];
	//		sprintf_s(n, "%02x", (unsigned char)i);
	//		if ((unsigned char)i > (unsigned char)0x0) {
	//			std::cout << dye::on_white(n) << ' ';
	//			c++;
	//			continue;
	//		}
	//		std::cout << n << ' ';
	//		c++;
	//	}
	//	pSendPacket(gs->pktBuffer.data(), len);
	//	/**(short*)sourceX_addr = (short)((p36C*)buffer)->StartX;
	//	*(short*)sourceY_addr = (short)((p36C*)buffer)->StartY;
	//	*(short*)destX_addr = (short)((p36C*)buffer)->DestX;
	//	*(short*)destY_addr = (short)((p36C*)buffer)->DestY;*/
	//	//MoveCharacter(0x36C, gs->pktBuffer.data());

	//	gs->bPktBufferSet = false;
	//	std::cout << std::dec << std::endl;
	//	return;
	//}

	std::cout << std::dec << std::endl;


	//call original function to send packet
	pSendPacket(buffer, len);
}

