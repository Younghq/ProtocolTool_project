#include "Config.hpp"

void ConfigSubject::addObserver(ObserverBase *observer)
{
    this->mObserversContainer.push_back(observer);
}

void ConfigSubject::removeObserver(ObserverBase *observer)
{
    auto it = std::remove(this->mObserversContainer.begin(), this->mObserversContainer.end(), observer);
    this->mObserversContainer.erase(it, this->mObserversContainer.end());
}

void ConfigSubject::notifyObservers(ObserverBase *observer, const StateChangeEvent &event)
{
    // 查询观察者是否存在
    if (std::find(mObserversContainer.begin(), mObserversContainer.end(), observer) != mObserversContainer.end())
    {
        observer->stateChanged(event); // 通知单个观察者
    }
}

void ConfigSubject::notifyAllObservers(const StateChangeEvent &event)
{
    for (auto observer : mObserversContainer)
    {
        if (observer)
        {
            observer->stateChanged(event); // 通知每个观察者
        }
    }
}
