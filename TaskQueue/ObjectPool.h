// 对象池化管理系统
#ifndef __OBJECT_POOL_H__
#define __OBJECT_POOL_H__

#include <memory>
#include <vector>
#include <stack>
#include <mutex>
#include <atomic>
#include <functional>
#include <thread>
#include <chrono>

namespace task
{

// 对象池配置
struct ObjectPoolConfig {
    size_t initialSize = 10;      // 初始池大小
    size_t maxSize = 100;         // 最大池大小
    size_t minSize = 5;           // 最小池大小
    std::chrono::seconds maxIdleTime = std::chrono::seconds(300); // 对象最大空闲时间
    bool enableAutoShrink = true;  // 启用自动收缩
    std::chrono::seconds shrinkInterval = std::chrono::seconds(60); // 收缩检查间隔
};

// 池化对象的包装器
template<typename T>
struct PooledObject {
    std::shared_ptr<T> object;
    std::chrono::steady_clock::time_point lastUsedTime;
    bool inUse = false;
    
    PooledObject(std::shared_ptr<T> obj) 
        : object(std::move(obj))
        , lastUsedTime(std::chrono::steady_clock::now()) {}
};

// 通用对象池模板类
template<typename T>
class ObjectPool {
public:
    using ObjectFactory = std::function<std::shared_ptr<T>()>;
    using ObjectResetter = std::function<void(std::shared_ptr<T>&)>;
    
    explicit ObjectPool(ObjectFactory factory, 
                       ObjectResetter resetter = nullptr,
                       const ObjectPoolConfig& config = ObjectPoolConfig())
        : mFactory(factory)
        , mResetter(resetter)
        , mConfig(config)
        , mTotalCreated(0)
        , mTotalAcquired(0)
        , mTotalReleased(0)
        , mIsShutdown(false)
    {
        initialize();
        startMaintenanceThread();
    }
    
    ~ObjectPool() {
        shutdown();
    }
    
    // 获取对象
    std::shared_ptr<T> acquire() {
        if (mIsShutdown.load()) {
            return nullptr;
        }
        
        std::lock_guard<std::mutex> lock(mMutex);
        
        // 尝试从池中获取可用对象
        for (auto& pooledObj : mObjects) {
            if (!pooledObj.inUse) {
                pooledObj.inUse = true;
                pooledObj.lastUsedTime = std::chrono::steady_clock::now();
                mTotalAcquired++;
                
                // 重置对象状态
                if (mResetter) {
                    mResetter(pooledObj.object);
                }
                
                return createManagedObject(pooledObj.object);
            }
        }
        
        // 池中没有可用对象，创建新对象
        if (mObjects.size() < mConfig.maxSize) {
            auto newObject = mFactory();
            if (newObject) {
                mObjects.emplace_back(newObject);
                auto& pooledObj = mObjects.back();
                pooledObj.inUse = true;
                pooledObj.lastUsedTime = std::chrono::steady_clock::now();
                mTotalCreated++;
                mTotalAcquired++;
                
                return createManagedObject(pooledObj.object);
            }
        }
        
        // 池已满，直接创建临时对象
        mTotalCreated++;
        mTotalAcquired++;
        return mFactory();
    }
    
    // 获取池统计信息
    struct PoolStats {
        size_t poolSize;
        size_t availableObjects;
        size_t inUseObjects;
        size_t totalCreated;
        size_t totalAcquired;
        size_t totalReleased;
        double hitRate; // 命中率
    };
    
    PoolStats getStats() const {
        std::lock_guard<std::mutex> lock(mMutex);
        
        PoolStats stats;
        stats.poolSize = mObjects.size();
        stats.inUseObjects = 0;
        
        for (const auto& pooledObj : mObjects) {
            if (pooledObj.inUse) {
                stats.inUseObjects++;
            }
        }
        
        stats.availableObjects = stats.poolSize - stats.inUseObjects;
        stats.totalCreated = mTotalCreated;
        stats.totalAcquired = mTotalAcquired;
        stats.totalReleased = mTotalReleased;
        stats.hitRate = (mTotalAcquired > 0) ? 
            (static_cast<double>(mTotalReleased) / mTotalAcquired) : 0.0;
        
        return stats;
    }
    
    // 配置管理
    void setConfig(const ObjectPoolConfig& config) {
        std::lock_guard<std::mutex> lock(mMutex);
        mConfig = config;
    }
    
    ObjectPoolConfig getConfig() const {
        std::lock_guard<std::mutex> lock(mMutex);
        return mConfig;
    }
    
    // 手动触发池收缩
    void shrink() {
        std::lock_guard<std::mutex> lock(mMutex);
        shrinkPool();
    }
    
    // 清空池
    void clear() {
        std::lock_guard<std::mutex> lock(mMutex);
        mObjects.clear();
        initialize();
    }

private:
    void initialize() {
        // 创建初始对象
        for (size_t i = 0; i < mConfig.initialSize; ++i) {
            auto obj = mFactory();
            if (obj) {
                mObjects.emplace_back(obj);
                mTotalCreated++;
            }
        }
    }
    
    void startMaintenanceThread() {
        if (!mConfig.enableAutoShrink) {
            return;
        }
        
        mMaintenanceThread = std::thread([this]() {
            while (!mIsShutdown.load()) {
                std::this_thread::sleep_for(mConfig.shrinkInterval);
                
                if (!mIsShutdown.load()) {
                    std::lock_guard<std::mutex> lock(mMutex);
                    shrinkPool();
                }
            }
        });
    }
    
    void shrinkPool() {
        if (mObjects.size() <= mConfig.minSize) {
            return;
        }
        
        auto now = std::chrono::steady_clock::now();
        
        // 移除超时的未使用对象
        mObjects.erase(
            std::remove_if(mObjects.begin(), mObjects.end(),
                [this, now](const PooledObject<T>& pooledObj) {
                    if (pooledObj.inUse) {
                        return false; // 正在使用的对象不能移除
                    }
                    
                    auto idleTime = now - pooledObj.lastUsedTime;
                    return idleTime > mConfig.maxIdleTime && mObjects.size() > mConfig.minSize;
                }),
            mObjects.end());
    }
    
    std::shared_ptr<T> createManagedObject(std::shared_ptr<T> pooledObject) {
        // 创建一个自定义删除器，当对象被释放时将其归还到池中
        return std::shared_ptr<T>(pooledObject.get(), [this, pooledObject](T*) {
            this->release(pooledObject);
        });
    }
    
    void release(std::shared_ptr<T> pooledObject) {
        if (mIsShutdown.load()) {
            return;
        }
        
        std::lock_guard<std::mutex> lock(mMutex);
        
        // 找到对应的池化对象并标记为可用
        for (auto& pooledObj : mObjects) {
            if (pooledObj.object == pooledObject) {
                pooledObj.inUse = false;
                pooledObj.lastUsedTime = std::chrono::steady_clock::now();
                mTotalReleased++;
                break;
            }
        }
    }
    
    void shutdown() {
        mIsShutdown.store(true);
        
        if (mMaintenanceThread.joinable()) {
            mMaintenanceThread.join();
        }
        
        std::lock_guard<std::mutex> lock(mMutex);
        mObjects.clear();
    }

private:
    ObjectFactory mFactory;
    ObjectResetter mResetter;
    ObjectPoolConfig mConfig;
    
    mutable std::mutex mMutex;
    std::vector<PooledObject<T>> mObjects;
    
    // 统计信息
    std::atomic<size_t> mTotalCreated;
    std::atomic<size_t> mTotalAcquired;
    std::atomic<size_t> mTotalReleased;
    
    // 维护线程
    std::thread mMaintenanceThread;
    std::atomic<bool> mIsShutdown;
};

} // namespace task

#endif // __OBJECT_POOL_H__