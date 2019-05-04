#pragma once

#include "..\PEFunctions.h"

using namespace PEFunctions;

void InitiateHooks();
__declspec(naked) void InitCodeCave(void);
void ExecCodeCave(void);
