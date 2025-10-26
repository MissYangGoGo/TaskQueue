#include "IThreadPool.h"
#include "SerialThreadPool.h"
#include "ConcurrencyThreadPool.h"
#include <memory>
namespace task
{
const std::shared_ptr<IThreadPool>& IThreadPool::serialThreadPool()
{
    static std::shared_ptr<IThreadPool> sSerialThreadPool =
        std::static_pointer_cast<IThreadPool>(std::make_shared<SerialThreadPool>());
    return sSerialThreadPool;
}

const std::shared_ptr<IThreadPool>& IThreadPool::parallelThreadPool()
{
    static std::shared_ptr<IThreadPool> sGlobalThreadPool =
        std::static_pointer_cast<IThreadPool>(std::make_shared<ConcurrencyThreadPool>());
    return sGlobalThreadPool;
}

}  // namespace task