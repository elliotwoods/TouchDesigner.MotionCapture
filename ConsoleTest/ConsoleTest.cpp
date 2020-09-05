#include "pch_ConsoleTest.h"
// ConsoleTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "../CHOP_ThreadTo/CHOP_ThreadTo.h"

extern "C"
{
    CHOP_CPlusPlusBase*
        CreateCHOPInstance(const OP_NodeInfo* info);
    void
        DestroyCHOPInstance(CHOP_CPlusPlusBase* instance);
}

int main()
{
    std::cout << "Hello World!\n";
    auto plugin = CreateCHOPInstance(nullptr);
    DestroyCHOPInstance(plugin);
}
