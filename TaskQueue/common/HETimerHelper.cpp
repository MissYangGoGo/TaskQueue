//
//  HETimerHelper.cpp
//  videosystem
//
//  Created by SDK Team on 2022/2/14.
//

#include "HETimerHelper.h"

namespace task
{

uint64_t HETimerHelper::millisecondTimestamp64()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now().time_since_epoch())
        .count();
}

uint64_t HETimerHelper::currentTimeMillis()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count();
}

}  // namespace comm
