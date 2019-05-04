#pragma once

#include "..\PEFunctions.h"

void InitiateHooks();
__declspec(naked) void InitCodeCave(void);
void ExecCodeCave(void);