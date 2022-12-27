#include "pch.h"
#include "Inventory.h"
#include "Functions.h"
#include "GameState.h"
#include "struct.h"


Inventory::Inventory() {
	//Init Inventory
	eq = (Equip*)FindDMAAddy(base_addr2, { 0x820 });
	inv = (Carry*)FindDMAAddy(base_addr2, { 0x8A8 });
	cargo = (Cargo*)FindDMAAddy(base_addr2, { 0x38C });

}

bool Inventory::Contains(int itemid) {
	for (auto& i : inv->items) {
		if (i.Index == itemid)
			return true;
	}
	return false;
}

int Inventory::FindFirst(int itemid) {
	for (int i = 0; i < carrySize; i++)
		if (inv->items[i].Index == itemid)
			return i;
	return -1;
}


void Inventory::DeleteItem(int slot) {
	int temp = inv->items[slot].Index;
	//Delete Item
	p2E4 sendData;
	ZeroMemory(&sendData, sizeof(sendData));
	sendData.Header.packetSize = sizeof(p2E4);
	sendData.Header.packetID = 0x2E4;
	sendData.Header.referID = GetClientID();
	sendData.item = GetInvSlot(slot);
	sendData.slot = slot;
	hSendPacket((char*)&sendData, sizeof(p2E4));
	if (inv->items[slot].Index == temp)
		inv->items[slot].Index = 0;
}


//Move/stack Item same thing
void Inventory::MoveItem(int srcSlot, int dstSlot, int srcType, int dstType ) {
	p376 sendData;
	ZeroMemory(&sendData, sizeof(sendData));
	sendData.Header.packetSize = sizeof(p376);
	sendData.Header.packetID = 0x376;
	sendData.Header.referID = GetClientID();
	sendData.DstType = dstType;
	sendData.SrcType = srcType;
	sendData.SrcSlot = srcSlot;
	sendData.DstSlot = dstSlot;
	hSendPacket((char*)&sendData, sizeof(p376));
	std::cout << "Move Item Sent" << std::endl;
}
void Inventory::StackItem(int from, int to) {
	MoveItem(from, to);
}

void Inventory::SplitItem(int slot, int itemid, int qty)
{
	p2E5 sendData;
	ZeroMemory(&sendData, sizeof(sendData));
	sendData.Header.packetSize = sizeof(p2E5);
	sendData.Header.packetID = 0x2E5;
	sendData.Header.referID = GetClientID();
	sendData.slot = slot;
	sendData.itemid = itemid;
	sendData.splitqt = qty;
	hSendPacket((char*)&sendData, sizeof(p2E5));
	std::cout << "Split Item Sent" << std::endl;
}

//Sell to NPC
void Inventory::SellItem(int slot, int NPCid, int item) {
	p37A sendData;
	ZeroMemory(&sendData, sizeof(sendData));
	sendData.Header.packetSize = sizeof(p37A);
	sendData.Header.packetID = 0x37A;
	sendData.Header.referID = GetClientID();
	sendData.npcId = NPCid;
	sendData.type = 1;
	sendData.sellSlot = slot;
	if (slot < 0) {
		for (int i = 0; i < GetLimitSlot(); i++) {
			if (GetInvSlot(i) == item) {
				sendData.sellSlot = i;
				break;
			}
		}
	}

	hSendPacket((char*)&sendData, sizeof(p37A));
	std::cout << "Sell Item Sent" << std::endl;

}
void Inventory::BuyItem(int npcid, int slotid)
{
	p379 sendData;
	ZeroMemory(&sendData, sizeof(sendData));
	sendData.Header.packetSize = sizeof(p379);
	sendData.Header.packetID = 0x379;
	sendData.Header.referID = GetClientID();
	for (int i = 0; i < GetLimitSlot(); i++) {
		if (GetInvSlot(i) == 0) {
			sendData.invSlot = i;
			break;
		}
	}
	sendData.mobID = npcid;
	sendData.sellSlot = slotid;
	hSendPacket((char*)&sendData, sizeof(p379));
	std::cout << "Buy Item Sent" << std::endl;
}
//Use Item in inventory
void Inventory::UseItem(int slot) {
	int temp = inv->items[slot].Index;
	p373 sendData;
	ZeroMemory(&sendData, sizeof(sendData));
	sendData.Header.packetSize = sizeof(p373);
	sendData.Header.packetID = 0x373;
	sendData.Header.referID = GetClientID();
	sendData.SrcType = sendData.DstType = 1;
	sendData.DstSlot = sendData.SrcSlot = slot;
	sendData.PosX = GetPosX();
	sendData.PosY = GetPosY();
	hSendPacket((char*)&sendData, sizeof(p373));
	//decrement quantity if stacked item
	if (inv->items[slot].EF1 == 61) {
		inv->items[slot].EFV1--;
		if (inv->items[slot].EFV1 == 0) {
			inv->items[slot].Index = 0;
			return;
		}
	}
	if (inv->items[slot].EF2 == 61) {
		inv->items[slot].EFV2--;
		if (inv->items[slot].EFV2 == 0) {
			inv->items[slot].Index = 0;
			return;
		}
	}
	if (inv->items[slot].EF2 == 61) {
		inv->items[slot].EFV2--;
		if (inv->items[slot].EFV2 == 0) {
			inv->items[slot].Index = 0;
			return;
		}
	}
	//clear item if not stacked
	if(inv->items[slot].Index == temp)
		inv->items[slot].Index = 0;
	
}
//Equip a piece of gear
void Inventory::EquipItem(int from, int to) {
	MoveItem(from, to);
}



//put gold in cargo
void Inventory::DepositGold(int amount) {

}

//take gold out of cargo
void Inventory::WithdrawGold(int amount) {

}