//
//  HETimerHelper.hpp
//  videosystem
//
//  Created by SDK Team on 2022/2/14.
//

#pragma once

#include <stdio.h>
#include <chrono>

namespace task
{

class HETimerHelper
{
public:
    static uint64_t millisecondTimestamp64();

    static uint64_t currentTimeMillis();
};

}
