#pragma once
#include "pch.h"
#include "struct.h"
#include <unordered_set>
#include <chrono>

	extern HMODULE myhModule;
	//Offsets
#define SceneOffset 0x59cc94 // 0x14
#define playerinfo_offset 0x65C900
#define pObj_offset 0x22C4758
#define itemlistoffset 0x9EF680
#define sendpacket_offset 0x1901A0
#define buildregion_offset 0x17e170
#define moveX_offset 0x0277FA
#define moveY_offset 0x0277FE
#define pfAll_offset 0xb2c438
#define pfXInit_offset 0x05878A0
#define pfYInit_offset 0x05878A4
#define size_pGridX 15
#define size_pGridY 15
#define ccMode_offset 0x5A61F0
#define ccMove_offset 0x27BF7



#define SetByte(val,offset) *(char*)&buffer[offset]=(char)(val)
#define SetShort(val,offset) *(short*)&buffer[offset]=(short)(val)
#define SET_INT(val,offset) *(int*)&buffer[offset]=(int)(val)
#define SetStr(val,offset) strcpy(&buffer[offset], val)

typedef void(__cdecl* SendPacketT)(char* buffer, int len);


void __cdecl hSendPacket(char* buffer, int len);

//Vars
	extern DWORD addr_SceneState, lastEntIdx, base_addr1, base_addr2, pfXInit_addr, pfYInit_addr;


#pragma region Inline Functions

	__forceinline int GetClientID()
	{
		__asm {
			MOV EAX, DWORD PTR DS : [base_addr2]
			MOV ECX, [EAX]
			ADD ECX, 0xFE0
			XOR EAX, EAX
			MOV AX, WORD PTR DS : [ECX]
		}
	}

	__forceinline int GetTMBase()
	{
		__asm {
			MOV EAX, DWORD PTR DS : [base_addr1]
			MOV ECX, [EAX]
			ADD ECX, 0x5C
			//MOV EAX, [ECX]
			//MOV ECX, EAX
			XOR EAX, EAX
			MOV EAX, DWORD PTR DS : [ECX]
		}
	}

	__forceinline int GetTMClass()
	{
		__asm {
			MOV EAX, DWORD PTR DS : [base_addr1]
			MOV ECX, [EAX]
			ADD ECX, 0x5C
			MOV EAX, [ECX]
			MOV ECX, EAX
			XOR EAX, EAX
			MOV EAX, DWORD PTR DS : [ECX]
		}
	}

	__forceinline short GetInvSlot(int slot)
	{
		int pointer = (0x8A8 + (slot * 8));
		__asm
		{
			MOV EAX, DWORD PTR DS : [base_addr2]
			MOV ECX, [EAX]
			ADD ECX, pointer
			XOR EAX, EAX
			MOV EAX, DWORD PTR DS : [ECX]
		}
	}
	__forceinline short GetPosX()
	{
		return (short)(*(float*)((*(int*)((char*)(*(int**)base_addr1) + 92)) + 40));
	}

	__forceinline short GetPosY()
	{
		return (short)(*(float*)((*(int*)((char*)(*(int**)base_addr1) + 92)) + 44));
	}


	__forceinline int GetPosXInt()
	{
		return floor(GetPosX());
		//return (*(int*)((*(int*)((char*)(*(int**)base_addr1) + 0x5C)) + 0x664));
	}

	__forceinline int GetPosYInt()
	{
		return floor(GetPosY());
		//return (*(int*)((*(int*)((char*)(*(int**)base_addr1) + 0x5C)) + 0x668));
	}


#pragma endregion


	class Timer {
	private:
		std::chrono::steady_clock::time_point pr_StartTime;
		std::chrono::steady_clock::time_point pr_EndTime;

	public:
		Timer()
		{
			Start();
		}

		~Timer()
		{
			Finish();
		}

		void Start()
		{
			pr_StartTime = std::chrono::steady_clock::now();
		}

		void Finish()
		{
			using namespace std::chrono;
			pr_EndTime = steady_clock::now();
			auto Duration = duration_cast<milliseconds>(pr_EndTime - pr_StartTime);
			std::cout << "Duration " << Duration.count() << " milliseconds." << std::endl;
		}
	};
	

#pragma region DLLFunctions

	void EveryLoop();
	void SetUp();
	void Hooks();
	void CleanUp();
	void PrintHotkeyInfo();
	void HotkeyLoop();


#pragma endregion


#pragma region Memory
	void Patch(BYTE* dst, BYTE* src, unsigned int size);
#pragma endregion

	Vec2Int randomXY(const Vec2Int& _start, const int& variance);
	short GetLimitSlot();
	bool FindPoint(Rec rect, Vec2Int pos);
	Vec2Int FindRandomPtInRec(Rec rect, int range = -1);
	Vec2Int getXYVecInt();
	int findDistanceInt(Vec2Int pt1, Vec2Int pt2);
	uintptr_t FindDMAAddy(uintptr_t ptr, std::vector<unsigned int> offsets);
	float sQ(float val);
	float findDistance(Vec2 pt1, Vec2 pt2);
	Vec2 getXYVec();
	class pNode
	{
	public:
		int bPassable = true;
		pNode(int worldX, int worldY, int _x, int _y);
		int WorldX = 0;
		int WorldY = 0;
		int x = 0;
		int y = 0;
		int NodeValue = 0;
		bool pfData();
	};

	void LoadPFData();
	unsigned char* GetPFdata(int x, int y);


	class WaveProp
	{
	public:
		bool bOffgrid = false;
		Vec2Int startpos = { 0 };
		Vec2Int tgtpos = { 0 };
		int GridSizeX = 0;
		int GridSizeY = 0;
		bool PathFound = false;
		std::shared_ptr<pNode> targetNode;
		std::vector<std::shared_ptr<pNode>> Grid;
		std::vector<Vec2Int> path;
		std::unordered_set<pNode*> processing;
		std::unordered_set<pNode*> discovered;


		WaveProp(int _gridSizeX, int _gridSizeY, Vec2Int _start, Vec2Int _tgt);
		void InitGrid();
		//void ResizeGrid(int _gridSizeX, int _gridSizeY);
		bool IsOffgrid();
		int iHomeX();
		int iHomeY();
		int iHomeIdx();
		int XYtoIdx(int _x, int _y);
		std::shared_ptr<pNode> GetTargetNode();
		void ProcessNodes();
		void BuildPath();
		void ParsePath();
	};

