// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "GameState.h"
#include "Functions.h"
#include "struct.h"
#include "UI/ConsoleMenu/Menu.h"
#include "Bot.h"


extern DWORD ccmode_addr;
HMODULE myhModule;
bool bPathingTest = false, bLastEntIdx, bMoveToTargets = false, bLanBotEnabled = false, bQuestBotEnabled = false, bFarmBotEnabled = false;
GameState* gs;
pfd* pd;
DWORD moduleBase;

//LAN bot var
Vec2Int tgtxy = { 0 };
Rec rmrTargetRegion = rmrLanN;


DWORD WINAPI MainThread() {
    //setup
    pd = new pfd;
    SetUp();
    PrintHotkeyInfo();
    Hooks();
    gs = new GameState();

    while (1)
    {
        start:
        // loop
        if (GetAsyncKeyState(VK_END) && GetAsyncKeyState(VK_CONTROL) && 0x8000) {
            break;
        }
        //Hotkey logic
        HotkeyLoop();

        //wait for login
        if (!gs->IsLoggedIn()) {
            Sleep(100);
            continue;
        }

        //update gamestate if logged in
        if (gs->IsLoggedIn()) {
            //load the treenode pointer to the last element of the entity list
            if ((Ent*)FindDMAAddy(moduleBase + playerinfo_offset, { 0x5C, 0x0 }) != nullptr) {
                if (!bLastEntIdx) {
                    lastEntIdx = FindDMAAddy(moduleBase + playerinfo_offset, { 0x5C, 0x4, 0x10 });
                    bLastEntIdx = true;
                }
            }
            //update gamestate
            gs->Update();
        }


        //gamestate init success
        if (gs->bInitialized) {

            //Logged in
            if (gs->bIsloggedIn) {


                //Respawn if dead
                if (local->isDead) {
                    gs->bPendingMove = false;
                    gs->v2GoalPos = { 0 };
                    gs->bGoalIsMob = false;
                    Sleep(1000);
                    gs->gsActions.push_front(std::make_shared<Action>(RESPAWN));
                    continue;
                }

                if (gs->bRegionChanged)
                    continue;

                //Execute move if pending
                if (gs->bPendingMove) {
                    MoveToXY(gs->v2GoalPos.x, gs->v2GoalPos.y);
                    //bPendingMove will be cleared if location ends up being the correct XY
                    continue;
                }

                //Manage and Execute actions in list
                if (!gs->gsActions.empty()) {
                    //Have actions
                    if (gs->gsActions.front()->bCompleted) {
                        //if the action is complete, remove it from the front of the action list
                        gs->gsActions.pop_front();
                        continue;
                    }
                    //if the action isn't completed, try to execute
                    gs->gsActions.front()->Execute();
                    continue;
                }

                //Lan Bot loop logic
                if (bLanBotEnabled) {

                    if (*(unsigned char*)ccmode_addr == (unsigned char)0x0)
                        gs->ChangeCCMode(3);

                    //In Target Region
                    if (gs->rmrRegion == rmrTargetRegion) {
                        
                        if (gs->TargetList.empty()) {
                            //No targets in range, wait for respawn
                            Sleep(5);
                            continue;
                        }

                        //Target is within 2 tiles, this should be set to something like attack range probably
                        if (findDistanceInt(getXYVecInt(), Vec2Int{ gs->TargetList.front().get()->posInt.x, gs->TargetList.front().get()->posInt.y }) <= 2) {
                            if (*(unsigned char*)ccmode_addr == (unsigned char)0x3 || *(unsigned char*)ccmode_addr == (unsigned char)0x0)
                                gs->gsActions.push_front(std::make_shared<Action>(MELEE, FREEMOVE));
                            continue;
                        }

                        //move needed
                        gs->gsActions.push_front(std::make_shared<Action>(gs->TargetList.front().get()->posInt, true));
                        gs->gsActions.push_front(std::make_shared<Action>(POTION, NONE));
                        continue;

                    }

                    //In Armia Region
                    if (gs->rmrRegion == rmrArmia) {

                        //Check inventory and sell/store items if needed

                        //Look for LAN NPC
                        for (auto& e : gs->EntityList) {
                            if (e.get()->name == "LAN N") {
                                if (e.get()->fDistance < 8) {
                                    //Use NPC
                                    pktUseNPC(e.get()->sEntId, false);
                                    tgtxy = { 0 };
                                    continue;
                                }
                                tgtxy = { e.get()->posInt.x,e.get()->posInt.y };
                            }
                        }
                        if (tgtxy.x == 2138 && tgtxy.y >= 2085) {
                            tgtxy.y += 5;
                            if (tgtxy.y >= 2125)
                                tgtxy = { 0 };
                        }
                        //NPC not in range
                        if (tgtxy.x == 0) {
                            tgtxy = { 2138,2085 };
                        }

                        AStar* asArmia = new AStar(256, 256); // consolidate to global object?
                        walkPathPkt(*asArmia, tgtxy);
                        //Search for NPC
                        delete asArmia;
                    }
                }

                //Quest Bot loop logic
                if (bQuestBotEnabled) {

                    if (gs->currentQuest == nullptr) {
                        gs->currentQuest = gs->GetProperQuest();
                        gs->gsActions.clear();
                        gs->targets.clear();
                        gs->targets.insert(gs->targets.begin(), gs->currentQuest->vQuestMobNames.begin(), gs->currentQuest->vQuestMobNames.end());
                    }

                    if (*(unsigned char*)ccmode_addr == (unsigned char)0x0)
                        gs->ChangeCCMode(3);

                    //Use Quest Items
                    bool hadItems = false;
                    for (int i = 0; i < carrySize; i++) {
                        for (auto& qi : vQuestItemIds) {
                            if (qi == gs->Inv->inv->items[i].Index) {
                                //use item
                                hadItems = true;
                                gs->Inv->UseItem(i);
                                gs->gsActions.push_front(std::make_shared<Action>(i, true));
                            }
                        }
                    }
                    if (hadItems)
                        continue;

                    //Determine quest by level
                    if ((local->Level + 1)< gs->currentQuest->iReqLevel || (local->Level + 1) > gs->currentQuest->iMaxLevel) {
                        gs->currentQuest = gs->GetProperQuest();
                        gs->targets.clear();
                        gs->targets.insert(gs->targets.begin(), gs->currentQuest->vQuestMobNames.begin(), gs->currentQuest->vQuestMobNames.end());
                    }

                    //In Target Region
                    if (gs->rmrRegion == gs->currentQuest->rect_QuestRegion) {
                        //std::cout << dye::yellow("Inside Target Region") << "\n";
                        //inside quest zone
                        Vec2Int pos = getXYVecInt();
                        if (pos.x == 0 || pos.y == 0)
                            continue;
                        if (FindPoint(gs->currentQuest->rect_InsideQuest, pos)) {
                            //std::cout << dye::yellow("Inside Quest") << "\n";
                            if (gs->TargetList.empty()) {
                                //No targets in range, wait for respawn
                                Sleep(5);
                                continue;
                            }
                            
                            if(gs->iTempTargetHP == 0)
                                gs->iTempTargetHP = gs->TargetList.front()->iCurrentHp;

                            if (gs->iTargetHP == gs->iTempTargetHP) {
                                gs->iMobHpCounter++;
                                Sleep(100);
                                if (gs->iMobHpCounter > 10) {
                                    gs->gsActions.push_front(std::make_shared<Action>(randomXY(gs->TargetList.front().get()->posInt, 2), true));
                                    gs->iMobHpCounter = 0;
                                    continue;
                                }
                            }
                            gs->iTempTargetHP = gs->TargetList.front()->iCurrentHp;
                            //Target is within 2 tiles, this should be set to something like attack range probably
                            if (findDistanceInt(getXYVecInt(), Vec2Int{ gs->TargetList.front().get()->posInt.x, gs->TargetList.front().get()->posInt.y }) <= 2 && (getXYVecInt() != gs->TargetList.front()->posInt)) {
                                if(gs->bPendingMove)
                                    gs->gsActions.front()->bCompleted = true;
                                if (*(unsigned char*)ccmode_addr == (unsigned char)0x3 || *(unsigned char*)ccmode_addr == (unsigned char)0x0)
                                    gs->gsActions.push_front(std::make_shared<Action>(MELEE, FREEMOVE));
                                continue;
                            }

                            //move needed
                            gs->gsActions.push_front(std::make_shared<Action>(randomXY(gs->TargetList.front().get()->posInt, 1), true));
                            gs->gsActions.push_front(std::make_shared<Action>(POTION, NONE));
                            continue;
                        }
                        //outside quest zone, find and use NPC
                        //std::cout << dye::yellow("Outside Quest") << getXYVecInt().x << " " << getXYVecInt().y  << "\n";
                        

                        //find the npc
                        if (findDistanceInt(getXYVecInt(), gs->currentQuest->v2QuestWarpLocation) < 15) {
                            std::cout << dye::yellow("NPC nearby, trying path to NPC") << "\n";
                            for (auto& e : gs->EntityList) {
                                if (e->name == gs->currentQuest->sQuestNpcName) {
                                    if (e->fDistance < 8) {
                                        //Use NPC
                                        gs->gsActions.push_front(std::make_shared<Action>(e->name, USE_NPC));
                                        goto start;
                                    }
                                    //Move to NPC
                                    gs->gsActions.push_front(std::make_shared<Action>(e->posInt, true));
                                    gs->gsActions.push_front(std::make_shared<Action>(POTION, NONE));
                                    goto start;
                                }
                            }
                        }
                        std::cout << dye::yellow("NPC not found, pathing to location") << "\n";
                        gs->gsActions.push_front(std::make_shared<Action>(gs->currentQuest->v2QuestWarpLocation, true));
                        gs->gsActions.push_front(std::make_shared<Action>(POTION, NONE));
                        continue;
                    }



                    //In Other Region
                    if (gs->rmrRegion != gs->currentQuest->rect_QuestRegion) {
                        std::cout << dye::yellow("Other Region") << "\n";
                        if (GetPosXInt() == 0 || GetPosYInt() == 0)
                            continue;
                        std::list<std::shared_ptr<Action>> a = gs->currentQuest->GetRegionAction(gs->RegionsCurrent);
                        gs->gsActions.splice(gs->gsActions.begin(), a);
                    }
                    continue;
                }
                
                //Farm Bot loop logic
                if (bFarmBotEnabled) {

                }



            }
        }



        EveryLoop();
        Sleep(1);
    }


    CleanUp();
    return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        myhModule = hModule;
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MainThread, NULL, 0, NULL);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

