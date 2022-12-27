#include "pch.h"
#include "AStar.h"
#include "Functions.h"
#include "GameState.h"
#include "struct.h"

extern bool bOffgrid;
extern GameState* gs;

cNode::cNode(int _x, int _y, bool _bPassable, int _wx, int _wy, unsigned char _pfdata) {
	x = _x;
	y = _y;
	parent = nullptr;
	bPassable = _bPassable;
	WorldX = _wx;
	WorldY = _wy;
	cPFData = _pfdata;
}
AStar::AStar(int _xsize, int _ysize)
	: iGridSizeX(_xsize), iGridSizeY(_ysize)
{
	if (_ysize % 2 == 0)
		iGridSizeY += 1;
	if (_xsize % 2 == 0)
		iGridSizeX += 1;
}

AStar::~AStar() {
	if (!grid.empty()) {
		//std::vector<cNode*>::iterator it = grid.begin();
		//while (it != grid.end()) {
		//	delete* it;
		//	it = grid.erase(it);
		//}
		grid.clear();
	}
}

void AStar::BuildPacketPath() {
	// add player pos to start of path for packet send
	//path.insert(path.begin(), Vec2Int{ GetPosX(), GetPosY() });
	if (path.empty())
		return;


	std::vector<Vec2Int> pathvec(path.begin(), path.end());
	int n = 11;

	Vec2Int direction = { 0 };

	if (pathvec.size() <= n + 1) {

		std::shared_ptr<PacketPath> pp = std::make_shared<PacketPath>();
		pp->startX = GetPosXInt();
		pp->startY = GetPosYInt();
		pp->destX = pathvec.back().x;
		pp->destY = pathvec.back().y;
		pathvec.insert(pathvec.begin(), getXYVecInt());

		for (int i = 1, p = 0; i < pathvec.size(); i++, p++) {
			direction = { (pathvec[i].x - pathvec[i - 1].x), (pathvec[i].y - pathvec[i - 1].y) };
			pp->pathbytes[p] = GetPktByte(direction);
		}
		pp->MoveSpeed = (local->MoveSpd & 0xF);
		packetPath.push_back(std::move(pp));
		return;
	}
	

	// determine the total number of sub-vectors of size `n`
	int size = (pathvec.size() - 1) / n + 1;

	// create a vector of vectors to store the sub-vectors
	std::vector<std::vector<Vec2Int>> vec(size);
	std::vector<Vec2Int>::const_iterator end_itr;

	// each iteration of this loop process the next set of `n` elements
	// and store it in a vector at k'th index in `vec`
	for (int k = 0; k < size; ++k)
	{
		// get range for the next set of `n` elements
		auto start_itr = std::next(pathvec.cbegin(), k * n);

		// code to handle the last sub-vector as it might
		// contain fewer elements
		if (k * n + n > pathvec.size())
		{
			end_itr = pathvec.cend();
			vec[k].resize(pathvec.size() - k * n);
		}
		else 
		{

			end_itr = std::next(pathvec.cbegin(), k * n + n);
			// allocate memory for the sub-vector
			vec[k].resize(n);
		}
		

		

		// copy elements from the input range to the sub-vector
		std::copy(start_itr, end_itr, vec[k].begin());
	}

	for (int pv = 0; pv < vec.size(); pv++) {
		if (pv > 0)
			(vec[pv]).insert((vec[pv]).begin(), (vec[pv - 1]).back());
		//if (pv == 0)
		//	(vec[pv]).insert((vec[pv]).begin(), Vec2Int{ GetPosX(), GetPosY() });

		std::shared_ptr<PacketPath> pp = std::make_shared<PacketPath>();
		if (pv == 0) {
			(vec[pv]).insert((vec[pv]).begin(), getXYVecInt());
			pp->startX = GetPosXInt();
			pp->startY = GetPosYInt();
		}
		else
		{
			pp->startX = (vec[pv]).front().x;
			pp->startY = (vec[pv]).front().y;
		}


		pp->destX = (vec[pv]).back().x;
		pp->destY = (vec[pv]).back().y;
		for (int i = 1, p = 0; i < (vec[pv]).size(); i++, p++) {
			if (i == 0) {
				if (pv == 0) {
					direction = { (vec[pv][i].x - GetPosX()), (vec[pv][i].y - GetPosY()) };
					pp->pathbytes[p] = GetPktByte(direction);
					continue;
				}
			}

		/*	if (i + 1 == (vec[pv]).size()) {
				direction = { ((vec[pv]).back().x - vec[pv][i].x), ((vec[pv]).back().y - vec[pv][i].y) };
				pp->pathbytes[i] = GetPktByte(direction);
				continue;
			}*/

			direction = { (vec[pv][i].x - vec[pv][i - 1].x), (vec[pv][i].y - vec[pv][i - 1].y) };
			pp->pathbytes[p] = GetPktByte(direction);
		}
		/*for (int i = 0; i < (vec[pv]).size(); i++) {
			if (i == 0) {
				if (pv == 0) {
					direction = { (vec[pv][i].x - GetPosX()), (vec[pv][i].y - GetPosY()) };
					pp->pathbytes[i] = GetPktByte(direction);
					continue;
				}
			}

			if (i + 1 == (vec[pv]).size()) {
				direction = { ((vec[pv]).back().x - vec[pv][i].x), ((vec[pv]).back().y - vec[pv][i].y)};
				pp->pathbytes[i] = GetPktByte(direction);
				continue;
			}

			direction = { (vec[pv][i + 1].x - vec[pv][i].x), (vec[pv][i + 1].y - vec[pv][i].y) };
			pp->pathbytes[i] = GetPktByte(direction);
		}*/

		/*for (int i = 1; i < n; i++) {
			direction = { (pv[i - 1].x - pv[i].x), (pv[i - 1].y - pv[i].y) };
			pp->pathbytes[i - 1] = GetPktByte(direction);
		}*/
		pp->MoveSpeed = (local->MoveSpd & 0xF);
		//packetPath.insert(packetPath.begin(),std::move(pp));
		packetPath.push_back(std::move(pp));
	}
}

char AStar::GetPktByte(Vec2Int dir) {
	if (dir.x == 0) {
		switch (dir.y) {
		case -1:
			return (char)0x32;
			break;
		case 1:
			return (char)0x38;
			break;
		}
	}
	if (dir.x == -1) {
		switch (dir.y) {
		case -1:
			return (char)0x31;
			break;
		case 1:
			return (char)0x37;
			break;
		case 0:
			return (char)0x34;
			break;
		}
		
	}
	if (dir.x == 1) {
		switch (dir.y) {
		case -1:
			return (char)0x33;
			break;
		case 1:
			return (char)0x39;
			break;
		case 0:
			return (char)0x36;
			break;
		}
	}
	return (char)0x0;

}

std::list<Vec2Int> AStar::ParsePath(std::vector<std::shared_ptr<cNode>> cPath, bool trim) {
	path.clear();
	std::vector<Vec2Int> vpath;
	for (auto& n : cPath) {
		path.push_back(Vec2Int{ n->WorldX,n->WorldY });
		if (trim)
			vpath.push_back(Vec2Int{ n->WorldX,n->WorldY });
	}
	if (bIsMobDst && path.size() > (iMobOffset + 1)) {
		for (int i = 0; i < iMobOffset; i++) {
			if (path.empty())
				break;
			path.pop_back();
		}
	}
	if(!trim)
		return path;

	if (vpath.empty())
		return path;
	
	std::list<Vec2Int> parsed;
	Vec2Int lastAdded = vpath[0];
	Vec2Int directionOld = { 0 };


	for (int i = 1; i < vpath.size(); i++) {
		Vec2Int directionNew = Vec2Int{ (vpath[i - 1].x - vpath[i].x), (vpath[i - 1].y - vpath[i].y) };
		if (directionOld.x != directionNew.x && directionOld.y != directionOld.y) {
			parsed.push_back(Vec2Int{ vpath[i].x, vpath[i].y });
			lastAdded = vpath[i];
		}
		directionOld = directionNew;
		if (i == vpath.size() - 1) {
			parsed.push_back(Vec2Int{ vpath[i].x, vpath[i].y });
			break;
		}
		if (findDistanceInt(Vec2Int{ lastAdded.x,lastAdded.y }, Vec2Int{ vpath[i].x ,vpath[i].y }) <= 2) {
			continue;
		}
		parsed.push_back(Vec2Int{ vpath[i].x, vpath[i].y });
		lastAdded = vpath[i];
	}


	return parsed;
	
}

void AStar::populateAStarNodes() {
	if (!grid.empty()) {
		//std::vector<cNode*>::iterator it = grid.begin();
		//while (it != grid.end()) {
		//	delete* it;
		//	it = grid.erase(it);
		//}
		grid.clear();
	}

	grid.resize(iGridSizeX * iGridSizeY);

	bool walkable = true;
	char t = (unsigned char)0x7F;
	for (int x = 0; x < iGridSizeX; x++) {
		for (int y = 0; y < iGridSizeY; y++) {
			walkable = true;
			//Vec2Int xy = { x + offsetXY.x , y + offsetXY.y };
			int hx = GetHomeXY().x;
			int hy = GetHomeXY().y;
			int nx = (startpos.x - hx) + x;
			int ny = (startpos.y - hy) + y;
			if (nx < 4096 && ny < 4096 && nx > 0 && ny > 0)
				t = *GetPFdata(nx, ny);
			if ((t & (char)0xF0) > 0x0) {
				//std::cout << dye::red("Unwalkable:") << " " << x + offsetXY.x << " " << y + offsetXY.y << std::endl;
				walkable = false;
			}
			else {
				//std::cout << dye::blue("Walkable:") << " " << x + offsetXY.x << " " << y + offsetXY.y << std::endl;
			}
			//if ((nx == 2111 && ny == 2049) || nx == 2112 && ny == 2049 || nx == 2113 && ny == 2049)
			//	std::cout << x << " " << y << " " << walkable << " " << nx << " " << ny << " " << t << "\n";
			grid[y * iGridSizeX + x] = std::move(std::make_shared<cNode>(x, y, walkable, nx, ny, t));
		}
	}

}

//void AStar::populateAStarNodes() {
//	if (!grid.empty()) {
//		std::vector<cNode*>::iterator it = grid.begin();
//		while (it != grid.end()) {
//			delete *it;
//			it = grid.erase(it);
//		}
//		grid.clear();
//	}
//	
//	grid.resize(iGridSizeX * iGridSizeY);
//
//	bool walkable = true;
//	char t = (unsigned char)0x7F;
//	for (int x = 0; x < iGridSizeX; x++) {
//		for (int y = 0; y < iGridSizeY; y++) {
//			walkable = true;
//			//Vec2Int xy = { x + offsetXY.x , y + offsetXY.y };
//			int hx = GetHomeXY().x;
//			int hy = GetHomeXY().y;
//			int nx = (startpos.x - hx) + x;
//			int ny = (startpos.y - hy) + y;
//			if (nx < 4096 || ny < 4096 || nx > 0 || ny > 0)
//				t = *GetPFdata(nx, ny);
//			if (t == (char)0x7f) {
//				//std::cout << dye::red("Unwalkable:") << " " << x + offsetXY.x << " " << y + offsetXY.y << std::endl;
//				walkable = false;
//			}
//			else {
//				//std::cout << dye::blue("Walkable:") << " " << x + offsetXY.x << " " << y + offsetXY.y << std::endl;
//			}
//			grid[y * iGridSizeY + x] = new cNode(x, y, walkable, nx , ny, t);
//		}
//	}
//
//}

std::list<std::shared_ptr<cNode>> AStar::GetNeighbors(cNode& node) {
	std::list<std::shared_ptr<cNode>> neighbors;
	for (int x = -1; x <= 1; x++) {
		for (int y = -1; y <= 1; y++) {
			if (x == 0 && y == 0)
				continue;

			int checkX = node.x + x;
			int checkY = node.y + y;
			if (checkX >= 0 && checkX < iGridSizeX && checkY >= 0 && checkY < iGridSizeY) {

				neighbors.push_back(grid[checkY * iGridSizeX + checkX]);
			}
		}
	}

	for (auto& n : neighbors) {
		if (n->cPFData == (char)0x7F)
			node.movePenalty += 1;
	}

	return neighbors;
}


std::vector<std::shared_ptr<cNode>> AStar::TracePath(std::shared_ptr<cNode> start, std::shared_ptr<cNode> end) {
	std::list<std::shared_ptr<cNode>> path;
	std::shared_ptr<cNode> currentNode = end;

	while (currentNode.get() != start.get()) {
		path.push_front(currentNode);
		currentNode = currentNode->parent;
	}
	//path.reverse();

	std::vector<std::shared_ptr<cNode>> vPath;
	vPath.reserve(path.size());
	std::copy(std::begin(path), std::end(path), std::back_inserter(vPath));
	return vPath;
}

int AStar::GetNodeDistance(cNode& nodeA, cNode& nodeB) {
	int dstX = ABS(nodeA.x - nodeB.x);
	int dstY = ABS(nodeA.y - nodeB.y);

	if (dstX > dstY)
		return 14 * dstY + 10 * (dstX - dstY);
	return 14 * dstX + 10 * (dstY - dstX);
}

std::shared_ptr<cNode> AStar::GetNodeFromWorld(const Vec2Int& worldps) {
	bOffgrid = false;
	bool xHi = false, xLo = false, yHi = false, yLo = false;
	int x = (worldps.x - startpos.x) + GetHomeXY().x;
	int y = (worldps.y - startpos.y) + GetHomeXY().y;
	if (x < 0) {
		x = 0;
		bOffgrid = true;
		xLo = true;
	}
	if (x >= iGridSizeX) {
		x = iGridSizeX - 1;
		bOffgrid = true;
		xHi = true;
	}
	if (y < 0) {
		y = 0;
		bOffgrid = true;
		yLo = true;
	}
	if (y >= iGridSizeY) {
		y = iGridSizeY - 1;
		bOffgrid = true;
		yHi = true;
	}


	if (xHi or xLo or yHi or yLo) {
		std::cout << dye::red("Offgrid") << "\n";
		int n = 0;
		while ((*GetPFdata(grid[y * iGridSizeX + x]->WorldX, grid[y * iGridSizeX + x]->WorldY) & (unsigned char)0xF0) > 0 && (FindPoint(gs->currentQuest->rect_InsideQuest, Vec2Int{ grid[y * iGridSizeX + x]->WorldX,grid[y * iGridSizeX + x]->WorldY }))) {
			if (xHi) {
				if (x > 0) {
					x--;
					continue;
				}
				x = iGridSizeX;
			}
			if (xLo) {
				if (x < iGridSizeX) {
					x++;
					continue;
				}
			}
			if (yHi) {
				if (y > 0) {
					y--;
					continue;
				}
			}
			if (xLo) {
				if (y < iGridSizeY) {
					y++;
					continue;
				}
			}

		}

		std::cout << hue::red << "Selected: " << x << " " << y << " InRect: " << (FindPoint(gs->currentQuest->rect_InsideQuest, Vec2Int{ grid[y * iGridSizeX + x]->WorldX,grid[y * iGridSizeX + x]->WorldY })) 
			<< " PF: " << (*GetPFdata(grid[y * iGridSizeX + x]->WorldX, grid[y * iGridSizeX + x]->WorldY) & 0xF0) << "\n";
	}
	return grid[y * iGridSizeX + x];
}

Vec2Int AStar::GetHomeXY() {
	int x = ((iGridSizeX - 1) / 2);
	int y = ((iGridSizeY - 1) / 2);
	return Vec2Int{ x,y };
}

std::vector<std::shared_ptr<cNode>> AStar::FindPath(Vec2Int src, Vec2Int dst) {
	startpos = src;
	tgtpos = dst;
	populateAStarNodes();
	std::shared_ptr<cNode> startNode = GetNodeFromWorld(src); // starting node
	std::shared_ptr<cNode> targetNode = GetNodeFromWorld(dst); // end node
	std::cout << dye::aqua("Target Node: ") << targetNode->WorldX << " " << targetNode->WorldY << " " << std::hex << int(targetNode->cPFData) << std::dec << "\n";
	std::vector<std::shared_ptr<cNode>> c;
	
	if (startNode == nullptr || targetNode == nullptr) {
		std::cout << "AStar failed to find the target nodes!" << std::endl;
		return c;
	}

	//std::vector<cNode*> openSet; // set of nodes to be evaluated
	minHeap openSet(iGridSizeX * iGridSizeY);
	std::vector<std::shared_ptr<cNode>> closedSet; // set of nodes already evaluated
	openSet.Add(startNode); // add the start node to the open set


	//loop
	while (!openSet.heap.empty()) {
		std::shared_ptr<cNode> node = openSet.extractMin();
		if (node == nullptr)
			break;
		closedSet.push_back(node);

		//path found YAY
		//if ((node->WorldX == 2111 && node->WorldY == 2049) || node->WorldX == 2112 && node->WorldY == 2049 || node->WorldX == 2113 && node->WorldY == 2049)
		//	std::cout << "Node: " << node->WorldX << " " << node->WorldY << " Passable: " << node->bPassable << "\n";
		if(node->WorldX == targetNode->WorldX && node->WorldY == targetNode->WorldY)
			std::cout << "Target Node Found: " << node->WorldX << " " << node->WorldY << " Passable: " << node->bPassable << "\n";
		if (node.get() == targetNode.get()) {
			//std::cout << "AStar found a path." << std::endl;
			return TracePath(startNode, targetNode);
		}

		//Neighbor Nodes

		// for every neighbor of the current node
		auto lNeighbors = GetNeighbors(*node);
		for (auto& n : lNeighbors) {
			//if (n == NULL)
				//continue;

			std::vector<std::shared_ptr<cNode>>::iterator it = std::find_if(closedSet.begin(), closedSet.end(), [n](const std::shared_ptr<cNode> o) -> bool {return o.get() == n.get(); });
			if (closedSet.size() < 2)
				if (closedSet.front().get() == n.get())
					continue;

			// if unpassable terrain or already in closed set, goto next neighbor
			if (!n->bPassable && n != targetNode) {

				continue;
			}
			if (it != closedSet.end()) {
				/*std::cout << "[Debug] Node: " << n->x + xOrg << " " << n->y + yOrg << " - In Closed Set C:"
					<< (it != closedSet.end()) << std::endl;*/
				continue;
			}
			//if the new path to this neighbor is shorter OR neighbor is NOT in the open set, update values and add to openset 


			//
			int newCostToNeighbor = node->gCost + GetNodeDistance(*node, *n) + n->movePenalty;

			if (newCostToNeighbor < n->gCost || std::find(openSet.heap.begin(), openSet.heap.end(), n) == openSet.heap.end()) {
				n->gCost = newCostToNeighbor;
				n->hCost = GetNodeDistance(*n, *targetNode);
				n->parent = node;

				if (std::find(openSet.heap.begin(), openSet.heap.end(), n) == openSet.heap.end()) {
					//std::cout << "[Debug] Node Added to Open: " << n->x + xOrg  << " " << n->y + yOrg << " " << n->gCost << " " << n->fCost()
					//	<< " Parent: " << n->parent->x + xOrg << " " << n->parent->y + yOrg << std::endl;
					openSet.Add(n);
				}
				else
					openSet.heapify(n->heapIndex);
				//std::cout << "[Debug] Node Already in Open: " << n->x + xOrg << " " << n->y + yOrg << " " << n->gCost << " " << n->fCost()
				//	<< " Parent: " << n->parent->x + xOrg << " " << n->parent->y + yOrg << std::endl;
			}
		}
	}
	
	std::cout << "AStar failed to find a path!\nStart:\n" << startNode->WorldX << " " << startNode->WorldY << std::endl;
	return c;


}

bool cNode::compareTo(cNode& CompareNode) {
	int compare = fCost() - CompareNode.fCost();
	if (compare == 0) {
		compare = hCost - CompareNode.hCost;
	}
	if (compare > 0)
		return true;
	return false;
}

int cNode::fCost() {
	return gCost + hCost;
}

minHeap::minHeap(int capacity) {
	size = 0;
	this->capacity = capacity;
	heap.resize(capacity);
}

void minHeap::Add(std::shared_ptr<cNode>& k) {
	// check for space
	if (size == capacity) {
		std::cout << "[Error]Minheap is full, unable to insert: " << &k << std::endl;
		return;
	}
	//heap[parent(i)] > heap[i]

	size++;

	int i = size - 1;
	k->heapIndex = i;
	heap[i] = k;

	while (i != 0 && heap[parent(i)]->compareTo(*heap[i])) {
		std::swap(heap[i], heap[parent(i)]);
		i = parent(i);

	}

}

void minHeap::heapify(int i) {
	int l = left(i);
	int r = right(i);
	int smallest = i;

	if ((l < size) && (heap[smallest]->compareTo(*heap[l]))) {
		smallest = l;
	}if ((r < size) && (heap[smallest]->compareTo(*heap[r]))) {
		smallest = r;
	}

	if (smallest != i) {
		heap[smallest]->heapIndex = i;
		heap[i]->heapIndex = smallest;
		std::swap(heap[i], heap[smallest]);
		heapify(smallest);
	}
}

std::shared_ptr<cNode> minHeap::extractMin() {
	if (size == 0) {
		std::cout << "[Error]Minheap empty" << std::endl;
		return nullptr;
	}
	else if (size == 1) {
		size--;
		return std::move(heap[0]);


	}
	else {
		std::shared_ptr<cNode> root = heap[0];
		heap[size - 1]->heapIndex = 0;
		heap[0] = heap[size - 1];
		size--;
		heapify(0);

		return root;
	}
}