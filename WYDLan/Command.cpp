#include "pch.h"
/* ##########################Had to be adjusted for our Bot to work. For later use probably####################################
#include "Functions.h"
#include "Defines.h"
#include "Config.h"
#include <windows.h>
#include <stdio.h>

int HKD_SendChat(char *command, short clientid) {
	if(*command != '#') // Se o comando do chat não começar com '#', retorna falso e o chat é enviado (0x333, 0x334)
        return false;

	if(strnicmp(command, "#id", 3) == 0) { // Mostra o Id do player selecionado
		char msg[20];
		int addr = GetPlayerID();
		sprintf(msg, "PlayerID: %d", GetPlayerID());
		EXE_ShowMessage(0xFFFFCCFF, msg);
		return true;
	}

	if(stricmp(command, "#myid") == 0) { // Mostra o ClientId local
		char msg[20];
		int addr = GetClientID();
		sprintf(msg, "ClientID: %d", addr);
		EXE_ShowMessage(0xFFFFCCFF, msg);
		return true;
	}

	if(stricmp(command, "#speedatkon") == 0) { // Ativa o Speed Atk
		EXE_AttackDelay(0);
		char msg[30]; sprintf(msg, "Speed Atk ON.", atkDelay);
		EXE_ShowMessage(0xFF99FF22, msg);
		return true;
	}

	else if(stricmp(command, "#speedatkoff") == 0) { // Desativa o Speed Atk
		EXE_AttackDelay(1000);
		char msg[30]; sprintf(msg, "Speed Atk OFF.", atkDelay);
		EXE_ShowMessage(0xFFFFAAAA, msg);
		return true;
	}

#ifdef VIP_MEMBER
	if(strnicmp(command, "#smo",3) == 0) { // by raphaelxd
		int op, exp = 0;
		sscanf(command,"#smo %d %d", &op, &exp);

		BYTE explosao[] = {0x66 ,0x81 ,0xF9 ,0xE8 ,0x03 ,0x73 ,0x30 ,0x66 ,0x89 ,0x4C ,0x95 ,0x9A ,0x66 ,0xC7 ,0x44 ,0x95 ,0x9C ,0xFF ,0xFF ,0x90
							,0x90 ,0x90 ,0x90 ,0x90 ,0x83 ,0xC2 ,0x01 ,0x89 ,0x95 ,0x60 ,0xFF ,0xFF ,0xFF};
		BYTE bkp_exp[] = {0x66 ,0x89 ,0x4C ,0x95 ,0x9A ,0x8B ,0x95 ,0x60 ,0xFF ,0xFF ,0xFF ,0x66 ,0xC7 ,0x44 ,0x95 ,0x9C ,0xFF ,0xFF ,0x8B ,0x85
							,0x60 ,0xFF ,0xFF ,0xFF ,0x83 ,0xC0 ,0x01 ,0x89 ,0x85 ,0x60 ,0xFF ,0xFF ,0xFF};;
		
		DWORD OldProtect; VirtualProtect((int*)0x0401000,0x5F0FFF - 0x0401000,0x40,&OldProtect);
		if(op != 1000) {
			if(exp)
				memcpy((void*)smo_addr1, explosao, sizeof explosao);
		}
		else memcpy((void*)smo_addr1, bkp_exp, sizeof bkp_exp);
		*(BYTE*)smo_addr2 = 0xEB;
		*(int*)smo_addr3 = op;
		VirtualProtect((int*)0x0401000,0x5F0FFF - 0x0401000,OldProtect,&OldProtect);
        EXE_ShowMessage(0xFF99FF22, "New delay macro.");
		return TRUE;
    }
#endif

#ifdef VIP_MEMBER
	if(strnicmp(command, "#skdelayon", 10) == 0) { // Altera o delay das skills
		EXE_SkillDelay(0);
		EXE_ShowMessage(0xFF99FF22, "Sk Delay ON");
		return true;
	}

	else if(strnicmp(command, "#skdelayoff", 11) == 0) {
		EXE_ShowMessage(0xFFFFAAAA, "Relogue para desativar!");
		return true;
	}
#endif

#ifdef VIP_MEMBER
	if(strnicmp(command, "#macroaguaon", 12) == 0) { // Ativa o macro água
		_MACROAGUA = true;
		firstRoom = true;
		EXE_ShowMessage(0xFF99FF22, "Macro Agua ON.");
		return true;
	}

	else if(strnicmp(command, "#macroaguaoff", 13) == 0) {
		_MACROAGUA = false;
		EXE_ShowMessage(0xFFFFAAAA, "Macro Agua OFF.");
		return true;
	}
#endif

#ifdef VIP_MEMBER
	if(strnicmp(command, "#mkillon", 8) == 0) { // Ativa o multikill
		_MACROKILL = true;
		memset((void*)mkill_addr1, 0x90, 7);
		*(char*)mkill_addr2 = 0xEB;
		EXE_ShowMessage(0xFF99FF22, "Macro Kill ON.");
		return true;
	}

	else if(strnicmp(command, "#mkilloff", 9) == 0) {
		_MACROKILL = false;
		char byteList[] = {0x83,0xBA,0x18,0x02,0x00,0x00,0x42};
		memcpy((void*)mkill_addr1, byteList, 7);
		*(char*)mkill_addr2 = 0x75;
		EXE_ShowMessage(0xFFFFAAAA, "Macro Kill OFF.");
		return true;
	}
#endif

#ifdef VIP_MEMBER
	if(strnicmp(command, "#tkskon", 7) == 0) { // Ativa o puxar na cidade
		*(int*)tksk_addr1 = 2;
		*(int*)tksk_addr2 = 8;
		*(int*)tksk_addr3 = 0;
		EXE_ShowMessage(0xFF99FF22, "TK Skills ON.");
		return true;
	}

	else if(strnicmp(command, "#tkskoff", 8) == 0) {
		*(int*)tksk_addr1 = 1;
		*(int*)tksk_addr2 = 0;
		*(int*)tksk_addr3 = 1;
		EXE_ShowMessage(0xFFFFAAAA, "TK Skills OFF.");
		return true;
	}
#endif

#ifdef VIP_MEMBER
	if(stricmp(command, "#soul") == 0) {
		skillid = 102;
		*(int*)(sklist_addr + (102 * 96) + 12) = 0;
		EXE_ShowMessage(0xFF99FF22, "Soul ON.");
		return true;
	}
#endif

#ifdef VIP_MEMBER
	if(strnicmp(command, "#putitem", 8) == 0) {
		short itemID = 0;
		BYTE ef1,efv1,ef2,efv2,ef3,efv3;
		int ret = sscanf(command, "#putitem %d %d %d %d %d %d %d", &itemID, &ef1, &efv1, &ef2, &efv2, &ef3, &efv3);
		if(ret <= 0 || ret > 7) {
			EXE_ShowMessage(0xFFFFAAAA, "Item inválido.");
			return true;
		}
		for(int i = 0; i < DLL_GetLimitSlot(); i++)
		if(GetInvSlot(i) == 0) {
			*(DWORD*)(*(DWORD*)base_addr1 +  (0x85C + (i * 8))) = itemID;
			*(DWORD*)(*(DWORD*)base_addr1 +  (0x85C + (i * 8) + 2)) = ef1;
			*(DWORD*)(*(DWORD*)base_addr1 +  (0x85C + (i * 8) + 3)) = efv1;
			*(DWORD*)(*(DWORD*)base_addr1 +  (0x85C + (i * 8) + 4)) = ef2;
			*(DWORD*)(*(DWORD*)base_addr1 +  (0x85C + (i * 8) + 5)) = efv2;
			*(DWORD*)(*(DWORD*)base_addr1 +  (0x85C + (i * 8) + 6)) = ef3;
			*(DWORD*)(*(DWORD*)base_addr1 +  (0x85C + (i * 8) + 7)) = efv3;
			char buffer[24];
			SetShort(24, 0);
			SetShort(0, 2);
			SetShort(0x182, 4);
			SetShort(GetClientID(), 6);
			SetInt(0, 8);
			SetShort(1, 12);
			SetShort(i, 14);
			SetShort(itemID, 16);
			SetByte(ef1, 18);
			SetByte(efv1, 18);
			SetByte(ef2, 18);
			SetByte(efv2, 18);
			SetByte(ef3, 18);
			SetByte(efv3, 18);

			__asm {
				MOV EAX, base_addr1
				MOV EDX, DWORD PTR DS:[EAX]
				ADD EDX, 0x4C
				MOV ECX, DWORD PTR DS:[EDX]
			}

			EXE_AddItem(buffer);
			char msg[20]; sprintf(msg, "#putitem - [%s]", DLL_GetItemName(itemID));
			EXE_ClientMessage(3000, msg);
			sprintf(msg, "Item %d adicionado.", itemID);
			EXE_ShowMessage(0xFF99FF22, msg);
			return true;
		}
		return true;
	}
#endif

#ifdef VIP_MEMBER
	if(strnicmp(command, "#skill", 6) == 0) {
		sscanf(command, "#skill %d", &skillid);
		if(skillid < 0 || skillid > 110) {
			EXE_ShowMessage(0xFFFFAAAA, "Skill inválida.");
			return true;
		}

		*(int*)(sklist_addr + (skillid * 96) + 4) = 2; // TargetType
		*(int*)(sklist_addr + (skillid * 96) + 76) = 0; // Agressive

		char msg[30]; sprintf(msg, "Skill ID %d definido", skillid);
		EXE_ShowMessage(0xFF99FF22, msg);
		return true;
	}
#endif

#ifdef VIP_MEMBER
	if(strnicmp(command, "#buysk", 6) == 0) {
		sscanf(command, "#buysk %d", &buyskid);
		if(buyskid < 0 || buyskid > 110) {
			buyskid = 0;
			EXE_ShowMessage(0xFFFFAAAA, "Skill inválida.");
			return true;
		}
		char msg[30]; sprintf(msg, "BuySkill ID %d.", buyskid);
		EXE_ShowMessage(0xFF99FF22, msg);
		return true;
	}
#endif

#ifdef VIP_MEMBER
	if(strnicmp(command, "#shtdwn", 7) == 0) {
		int value;
		sscanf(command, "#shtdwn %d", &value);
		char buffer[20];
		SetShort(0, 0);
		SetShort(0, 2);
		SetShort(0x374, 4);
		SetShort(GetClientID(), 6);
		SetInt(0, 8);
		SetInt(value, 12);
		SetInt(0, 16);
		SendPacket(buffer, 20);

		EXE_ClientMessage(5000, "Tentativa de derrubar servidor enviada.");
		EXE_ShowMessage(0xFF99FF22, "SHTDWN");
		return TRUE;
	}
#endif

#ifdef VIP_MEMBER
	if(strnicmp(command, "#dc", 3) == 0) {
		int index;
		sscanf(command, "#dc %d", &index);
		char msg[20]; sprintf(msg, "%d desconectado.", index);
		EXE_ShowMessage(0xFF99FF22, msg);
		return TRUE;
	}
#endif

	if(strnicmp(command, "#effect", 7) == 0) {
		short effectid, effectid2;
		sscanf(command, "#effect %d %d", &effectid, &effectid2);

		char buffer[0x34];
		SetShort(0x34, 0);
		SetShort(0x3C9, 4);
		SetShort(GetClientID(), 6);
		SetShort(1, 12);
		SetShort(GetPosX() , 28);
		SetShort(GetPosY() , 30);
		SetShort(effectid, 32);
		SetShort(effectid2, 44);
		SendPacket(buffer, 0x34);

		char msg[30]; sprintf(msg, "Effect: %d - %d", effectid, effectid2);
		EXE_ShowMessage(0xFF99FF22, msg);
		return true;
	}

	if(strnicmp(command, "#goto", 5) == 0) {
		short index;
		char name[16];
		int ret = sscanf(command, "#goto %d %s", &index, &name[0]);
		if(ret != 2 || index < 0 || index > 1000) {
			EXE_ShowMessage(0xFFFFAAAA, "Indices inválidos.");
			return true;
		}
		EXE_FollowPlayer(index, name);
		char msg[30]; sprintf(msg, "Moved to %s (%d,%d).", name, GetPosX(), GetPosY());
		EXE_ShowMessage(0xFF99FF22, msg);
		return true;
	}

	if(strnicmp(command, "#chat", 5) == 0) {
		sscanf(command, "#chat %d", &chatid);
		if(chatid < 0 || chatid > 1000) {
			chatid = 0;
			EXE_ShowMessage(0xFFFFAAAA, "Somente possível em players.");
			return true;
		}
		
		char msg[30]; sprintf(msg, "Chat: %d", chatid);
		EXE_ClientMessage(5000, "Chat Hack - Fake ID");
		EXE_ShowMessage(0xFF99FF22, msg);
		return true;
	}

	if(strnicmp(command, "#tp", 3) == 0) {
		short tpIndex = 0;
		int ret = sscanf(command, "#tp %d", &tpIndex);
		if(ret == 0) {
			EXE_ShowMessage(0xFFFFAAAA, "Índice inválido.");
	        return true;
		}
		FILE *fp;
		char buf[4096];
		fopen_s(&fp, "wHook/Teleport.txt", "rt");
		if(fp == NULL)
    	{
    	    EXE_ShowMessage(0xFFFFAAAA, "Falha ao ler o arquivo Teleport.txt");
	        return true;
	    }
	
	    while(fgets(buf, sizeof(buf), fp))
	    {
			short index, posx, posy;
	        if(*buf == '\n' || *buf == '#') continue;
	        int ret = sscanf(buf, "%d-%d,%d", &index, &posx, &posy);
			if(ret != 4) continue;
			if(tpIndex == index) {
				if(posx < 0 || posy < 0 || posx > 4096 || posy > 4096) {
					EXE_ShowMessage(0xFFFFAAAA, "Coordenada INVALIDA.");
					fclose(fp);
					return true;
				}
				else {
					EXE_SendTeleport(posx, posy);
					char msg[30]; sprintf(msg, "Moved to (%d,%d).", posx, posy);
					EXE_ShowMessage(0xFF99FF22, msg);
					fclose(fp);
					return true;
				}
			}
	    }
		EXE_ShowMessage(0xFFFFAAAA, "Teleporte não configurado.");
		return true;
	}

	if(strnicmp(command, "#party", 6) == 0) {
        int index; char name[12];
        int ret = sscanf(command, "#party %d %s", &index, &name[0]);
		if(index < 0 || index > 1000) {
			EXE_ShowMessage(0xFFFFAAAA, "Index inválido.");
			return TRUE;
		}
		if(ret == 2) {
			char buffer[32];
			ZeroMemory(buffer, 32);
			SetShort(0, 0);
			SetShort(0, 2);
			SetShort(0x3AB, 4);
			SetShort(GetClientID(), 6);
			SetInt(0, 8);
			SetShort(index, 12);
			strcpy(&buffer[14], &name[0]);
			SendPacket(buffer, 32);

			EXE_ShowMessage(0xFF99FF22, "Party ON.");
			return TRUE;
		}

		EXE_ShowMessage(0xFFFFAAAA, "Valores Inválidos.");
		return TRUE;
    }

	if(stricmp(command, "#learnall") == 0) {
        GetChar()->Learn = 0xFFFFFFFF;
		EXE_ShowMessage(0xFFFFAAAA, "Skills Learned.");
		return TRUE;
    }

	if(stricmp(command, "#speedmoveon") == 0) {
        EXE_SpeedMove(true);
        EXE_ShowMessage(0xFF99FF22, "Speed Move Hack ON.");
		return TRUE;
    }
    else if(stricmp(command, "#speedmoveoff") == 0) {
        EXE_SpeedMove(false);
        EXE_ShowMessage(0xFFFFAAAA, "Speed Move Hack OFF.");
		return TRUE;
    }

	if(stricmp(command, "#indexon") == 0) {
        _INDEX = true;
        EXE_ShowMessage(0xFF99FF22, "Index ON.");
		return TRUE;
    }
    else if(stricmp(command, "#indexoff") == 0) {
        _INDEX = false;
        EXE_ShowMessage(0xFFFFAAAA, "Index OFF.");
		return TRUE;
    }

	if(strnicmp(command, "#atkinvon", 9) == 0) {
		_ATKINV = true;
		EXE_ShowMessage(0xFF99FF22, "Atk Inv ON.");
		return true;
	}

	else if(strnicmp(command, "#atkinvoff", 10) == 0) {
		_ATKINV = false;
		EXE_ShowMessage(0xFFFFAAAA, "Atk Inv OFF.");
		return true;
	}

	if(strnicmp(command, "#onehiton", 9) == 0) {
		DWORD OldProtect; VirtualProtect((int*)0x0401000,0x5F0FFF - 0x0401000,0x40,&OldProtect);
		*(char*)onehit_addr = 0x5E;
		VirtualProtect((int*)0x0401000,0x5F0FFF - 0x0401000,OldProtect,&OldProtect);
		_ONEHIT = true;
		EXE_ShowMessage(0xFF99FF22, "One Hit ON.");
		return true;
	}

	else if(strnicmp(command, "#onehitoff", 10) == 0) {
		DWORD OldProtect; VirtualProtect((int*)0x0401000,0x5F0FFF - 0x0401000,0x40,&OldProtect);
		*(char*)onehit_addr = 0x48;
		VirtualProtect((int*)0x0401000,0x5F0FFF - 0x0401000,OldProtect,&OldProtect);
		_ONEHIT = false;
		EXE_ShowMessage(0xFFFFAAAA, "One Hit OFF.");
		return true;
	}

	if(strnicmp(command, "#rangeon", 8) == 0) {
		EXE_SkillRange(true);
		_RANGE = true;
		EXE_ShowMessage(0xFF99FF22, "Range Hack ON.");
		return true;
	}

	else if(strnicmp(command, "#rangeoff", 9) == 0) {
		EXE_SkillRange(false);
		_RANGE = false;
		EXE_ShowMessage(0xFFFFAAAA, "Range Hack OFF.");
		return true;
	}

	if(strnicmp(command, "#wallon", 7) == 0) {
		DWORD OldProtect;
		VirtualProtect((int*)0x0401000,0x5F0FFF - 0x0401000,0x40,&OldProtect);
		memset((void*)wall_addr1, 0x90, 6);
		memset((void*)wall_addr2, 0x90, 6);
		memset((void*)wall_addr3, 0x90, 6);
		memset((void*)wall_addr4, 0x90, 6);
		VirtualProtect((int*)0x0401000,0x5F0FFF - 0x0401000,OldProtect,&OldProtect);
		_WALL = true;
		EXE_ShowMessage(0xFF99FF22, "Wall Hack ON.");
		return true;
	}

	else if(strnicmp(command, "#walloff", 8) == 0) {
		DWORD OldProtect;
		VirtualProtect((int*)0x0401000,0x5F0FFF - 0x0401000,0x40,&OldProtect);
		static char wallhack_jnz[4][6] = {{0x0F,0x85,0xD7,0x0A,0x00,0x00},{0x0F,0x85,0xC3,0x0A,0x00,0x00},{0x0F,0x85,0x68,0x09,0x00,0x00},{0x0F,0x85,0x54,0x09,0x00,0x00}};
		memset((void*)wall_addr1, *(char*)wallhack_jnz[0], 6);
		memset((void*)wall_addr2, *(char*)wallhack_jnz[0], 6);
		memset((void*)wall_addr3, *(char*)wallhack_jnz[0], 6);
		memset((void*)wall_addr4, *(char*)wallhack_jnz[0], 6);
		VirtualProtect((int*)0x0401000,0x5F0FFF - 0x0401000,OldProtect,&OldProtect);
		_WALL = false;
		EXE_ShowMessage(0xFFFFAAAA, "Wall Hack OFF.");
		return true;
	}

	if(strnicmp(command, "#reviveon", 9) == 0) {
		_REV = true;
		EXE_ShowMessage(0xFF99FF22, "Revive ON.");
		return true;
	}

	else if(strnicmp(command, "#reviveoff", 10) == 0) {
		_REV = false;
		EXE_ShowMessage(0xFFFFAAAA, "Revive OFF.");
		return true;
	}

	if(strnicmp(command, "#invon", 6) == 0) {
		_INV = true;
		EXE_Invisibility(1);
		EXE_ShowMessage(0xFF99FF22, "Inv ON.");
		return true;
	}

	else if(strnicmp(command, "#invoff", 7) == 0) {
		_INV = false;
		EXE_Invisibility(2);
		EXE_ShowMessage(0xFFFFAAAA, "Inv OFF.");
		return true;
	}

	if(strnicmp(command, "#cskills", 8) == 0) {
		char skList[] = {1,2,6,47,49,51};
		for(int i = 0; i < 6; i++) {
			*(int*)(sklist_addr + (skList[i] * 96) + 4) = 2; // TargetType
			*(int*)(sklist_addr + (skList[i] * 96) + 20) = 8; // InstanceType
			*(int*)(sklist_addr + (skList[i] * 96) + 76) = 0; // Agressive
		}
		EXE_ShowMessage(0xFF99FF22, "Skills Changed.");
		return true;
	}

	if(strnicmp(command, "#buffhackon", 11) == 0) {
		for(int i = 0; i < 110; i++) 
			*(int*)(sklist_addr + (i * 96) + 4) = 2;
		EXE_ShowMessage(0xFF99FF22, "Buff Hack ON.");
		return true;
	}

	else if(strnicmp(command, "#buffhackoff", 12) == 0) {
		for(int i = 0; i < 110; i++) 
			*(int*)(sklist_addr + (i * 96) + 4) = 0;
		EXE_ShowMessage(0xFFFFAAAA, "Buff Hack OFF.");
		return true;
	}

	if(strnicmp(command, "#moved", 6) == 0) {
        int x = 0, y = 0, retn = sscanf(command, "#moved %d %d", &x, &y);
        if(retn != 2 || x <= 0 || x >= 4096 || y <= 0 || y >= 4096) {
			EXE_ShowMessage(0xFFFFAAAA, "Coordenada INVALIDA.");
            return true;
        }

		EXE_SendTeleport(x, y);
		char msg[30]; sprintf(msg, "Moved to X: %d Y: %d.", x, y);
		EXE_ShowMessage(0xFF99FF22, msg);
		return true;

	}

	if(stricmp(command, "#secon") == 0) {
        sec_x = GetPosX();
		sec_y = GetPosY();
		char msg[30]; sprintf(msg, "Sec ON. (%d,%d)", sec_x, sec_y);
		EXE_ShowMessage(0xFF99FF22, msg);
		return true;
	}

	else if(stricmp(command, "#secoff") == 0) {
        sec_x = 0;
		sec_y = 0;
		char msg[30]; sprintf(msg, "Sec OFF.");
		EXE_ShowMessage(0xFF99FF22, msg);
		return true;
	}

	if(strnicmp(command, "#move", 5) == 0) {
        int x = 0, y = 0, retn = sscanf(command, "#move %d %d", &x, &y);
        if(retn != 2 || x <= 0 || x >= 4096 || y <= 0 || y >= 4096) {
			EXE_ShowMessage(0xFFFFAAAA, "Coordenada INVALIDA.");
            return true;
        }

		EXE_ClientTeleport(x, y);
		char msg[30]; sprintf(msg, "Move to X: %d Y: %d.", x, y);
		EXE_ShowMessage(0xFF99FF22, msg);
		return true;

	}

#ifdef VIP_MEMBER
	if(strnicmp(command, "#teleport", 9) == 0) {
        int x = 0, y = 0, index = 0;
		int retn = sscanf(command, "#teleport %d %d %d", &index, &x, &y);
        if(retn != 3 || x <= 0 || x >= 4096 || y <= 0 || y >= 4096 || index > 1000 || index < 0) {
			EXE_ShowMessage(0xFFFFAAAA, "Coordenada INVALIDA.");
            return true;
        }
		DWORD OldProtect; VirtualProtect((int*)0x0401000,0x5F0FFF - 0x0401000,0x40,&OldProtect);
		*(char*)moveid_addr = 0xEB;
		EXE_SimulateTeleport(index, x, y);
		*(char*)moveid_addr = 0x76;
		VirtualProtect((int*)0x0401000,0x5F0FFF - 0x0401000,OldProtect,&OldProtect);
		char msg[30]; sprintf(msg, "Moved %d to X: %d Y: %d.", index, x, y);
		EXE_ShowMessage(0xFF99FF22, msg);
		return true;

	}
#endif

	if(strnicmp(command, "#opencargo", 10) == 0) {
		__asm {
				MOV EAX,base_addr1
				MOV EDX,DWORD PTR DS:[EAX]
					
				PUSH 1
				MOV ECX,EDX
				MOV EAX, cargo_addr
				CALL EAX
		}
		EXE_ShowMessage(0xFF99FF22, "Cargo Opened");
		return true;
	}

	if(strnicmp(command, "#dropon", 7) == 0) {
		_DROP = true;
		EXE_ShowMessage(0xFF99FF22, "Auto Drop ON.");
		return true;
	}

	else if(strnicmp(command, "#dropoff", 8) == 0) {
		_DROP = true;
		EXE_ShowMessage(0xFFFFAAAA, "Auto Drop OFF.");
		return true;
	}

	if(strnicmp(command, "#list", 5) == 0) {
		short add_it;
		if(sscanf(command, "#list add %d", &add_it)) {
			for(int i = 0; i < 20; i++) {
				if(DropList[i] == add_it) {
					char msg[30]; sprintf(msg, "%d já está na lista.", add_it);
					EXE_ShowMessage(0xFFFFFF00, msg);
					sprintf(msg, "[%s] já está na lista.", DLL_GetItemName(add_it));
					EXE_ClientMessage(5000, msg);
					return true;
				}

				else if(DropList[i] == 0) {
					DropList[i] = add_it;
					char msg[30]; sprintf(msg, "%d adicionado à lista.", add_it);
					EXE_ShowMessage(0xFFFFFF00, msg);
					sprintf(msg, "[%s] adicionado à lista.", DLL_GetItemName(add_it));
					EXE_ClientMessage(5000, msg);
					return true;
				}

				else if(i == 20) {
					EXE_ShowMessage(0xFFFFFF00, "Lista cheia.");
					return true;
				}
			}
		}

		else if(sscanf(command, "#list remove %d", &add_it)) {
			for(int i = 0; i < 20; i++) {
				if(DropList[i] == add_it) {
					DropList[i] = 0;
					char msg[30]; sprintf(msg, "%d removido da lista.", add_it);
					EXE_ShowMessage(0xFFFFFF00, msg);
					sprintf(msg, "[%s] removido da lista.", DLL_GetItemName(add_it));
					EXE_ClientMessage(5000, msg);
					return true;
				}

				if(i == 20) {
					char msg[30]; sprintf(msg, "%d não está na lista.", add_it);
					EXE_ShowMessage(0xFFFFFF00, msg);
					sprintf(msg, "[%s] não está na lista.", DLL_GetItemName(add_it));
					EXE_ClientMessage(5000, msg);
					return true;
				}
			}			
		}

		else if(sscanf(command, "#list clear %d", &add_it)) {
			ZeroMemory(DropList, 20);
			EXE_ShowMessage(0xFFFFAAAA, "#drop - Lista apagada.");
			return true;
		}
		
		return true;
	}

	EXE_ShowMessage(0xFFFFAAAA, "Comando inválido");
    return true;
}

__declspec(naked) void NKD_SendChat() {
	static int hook_cmd_jmp1 = execcmd_jmp1;
	static int hook_cmd_jmp2 = execcmd_jmp2;
	__asm {
		MOV EAX,DWORD PTR SS:[EBP-0x98]
		MOV EDX,DWORD PTR DS:[EAX]
		MOV ECX,DWORD PTR SS:[EBP-0x98]
		CALL DWORD PTR DS:[EDX+0x88]

		PUSH EAX
		CALL HKD_SendChat
		ADD ESP, 0x08

		TEST EAX,EAX
		JE chk_other_cmd

		MOV EAX, 0x01
		JMP hook_cmd_jmp1

		chk_other_cmd:
		JMP hook_cmd_jmp2
	}
}


*/