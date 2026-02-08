#pragma once
#include <iostream>
#include "discord_rpc.h"

class Discord {
public:
    static void Initialize();
    static void Update(bool isEnabled);
};
