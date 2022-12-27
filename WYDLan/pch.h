#pragma once
#define WIN32_LEAN_AND_MEAN 

#include <windows.h>
#include <iostream>
#include <vector>
#include <fstream>



#pragma comment(lib, "detours.lib")
#include "detours.h"
#include "color.hpp"
#include "signal.hpp"
#include "sigscan.h"

#include "UI/GUI/EasyWindow.h"