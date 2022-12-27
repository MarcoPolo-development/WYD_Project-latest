#pragma once
#include "pch.h"
#include "struct.h"

extern Ent* local;

struct PacketPath
{
	short startX;
	short startY;
	int unk = 0;
	int MoveSpeed;
	char pathbytes[12];
	char padding[12] = { 0 };
	short destX;
	short destY;
};

class cNode : public std::enable_shared_from_this<cNode>
{
public:

	bool bPassable = true;
	int gCost = 0;
	int hCost = 0;
	int movePenalty = 0;
	int x = 0;
	int y = 0;
	int z = 0;
	int WorldX = 0;
	int WorldY = 0;
	cNode(int _x, int _y, bool _bPassable, int _wx, int _wy, unsigned char _pfdata);
	std::shared_ptr<cNode> parent = nullptr;
	int fCost();
	/*bool Passable();*/
	bool compareTo(cNode& CompareNode);
	int heapIndex = 0;
	unsigned char cPFData = 0x0;
};

class AStar
{
	
public:
	std::list<Vec2Int> path;
	AStar(int _xsize, int _ysize);
	~AStar();

	std::vector<std::shared_ptr<PacketPath>> packetPath;
	int iGridSizeX = 0;
	int iGridSizeY = 0;
	int iMobOffset = 0;
	bool bIsMobDst = false;
	Vec2Int startpos = { 0 };
	Vec2Int tgtpos = { 0 };
	void populateAStarNodes();

	std::list<Vec2Int> ParsePath(std::vector<std::shared_ptr<cNode>> cPath, bool trim = false);
	std::vector<std::shared_ptr<cNode>> FindPath(Vec2Int src, Vec2Int dst);
	std::vector<std::shared_ptr<cNode>> grid;
	std::list<std::shared_ptr<cNode>> GetNeighbors(cNode& node);
	std::vector<std::shared_ptr<cNode>> TracePath(std::shared_ptr<cNode> start, std::shared_ptr<cNode> end);
	std::shared_ptr<cNode> GetNodeFromWorld(const Vec2Int& worldps);
	int GetNodeDistance(cNode& nodeA, cNode& nodeB);


	void BuildPacketPath();
	char GetPktByte(Vec2Int direction);
	Vec2Int GetHomeXY();
	
};

class minHeap {
private:

	// parent index
	int parent(int i) { return (i - 1) / 2; }
	// left child
	int left(int i) { return 2 * i + 1; }
	// right child
	int right(int i) { return 2 * i + 2; }

public:
	//size of heap
	int size;
	// max size
	int capacity;
	//store elements
	std::vector<std::shared_ptr<cNode>> heap;
	//Construct
	minHeap(int capacity);
	// insert key
	void Add(std::shared_ptr<cNode>& k);
	// extract min
	std::shared_ptr<cNode> extractMin();
	// recursive heapify
	void heapify(int i);
	// print heap
	void printHeap();
};

