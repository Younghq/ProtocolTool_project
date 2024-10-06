#ifndef _Config_hpp_
#define _Config_hpp_
#include <vector>
#include <string>
#include <algorithm>
#include <thread>
#include <atomic>
#include <iostream>
#include <map>
#include <functional>
///////////////////////////////基类与各种枚举////////////////////////////////

/// @brief 主题容器基类
class SubjectBase
{
public:
    virtual void addObserver(ObserverBase *observer) = 0;                                    ///< 添加观察者
    virtual void removeObserver(ObserverBase *observer) = 0;                                 ///< 删除观察者
    virtual void notifyObservers(ObserverBase *observer, const StateChangeEvent &event) = 0; ///< 通知单个观察者
    virtual void notifyAllObservers(const StateChangeEvent &event) = 0;                      ///< 通知所有观察者
    virtual ~SubjectBase() = default;

protected:
    std::vector<ObserverBase *> mObserversContainer; ///< 观察者容器池
};

/// @brief 观察者状态枚举
enum class StateType
{
    ST_Update,       ///< 更新状态
    ST_Error,        ///< 错误状态
    ST_Connected,    ///< 连接成功
    ST_Disconnected, ///< 断开连接
    ST_Initializing, ///< 初始化状态
    ST_Shutdown,     ///< 关闭状态
    ST_DataReceived, ///< 数据接收状态
    ST_DataSent,     ///< 数据发送状态
    ST_Ready,        ///< 准备就绪状态
    ST_Processing,   ///< 处理中的状态
    ST_Completed,    ///< 完成状态
};

/// @brief 观察者状态事件对象，包含状态类型、状态码和附加信息
class StateChangeEvent
{
public:
    /// @brief 构造函数
    /// @param type 状态类型
    /// @param stateCode 状态码，默认值为 0
    /// @param message 附加的消息，默认值为空字符串
    StateChangeEvent(StateType type, int stateCode = 0, const std::string &message = "")
        : mType(type), mStateCode(stateCode), mMessage(message) {}

    /// @brief 获取状态类型
    /// @return 返回状态类型
    StateType getType() const { return mType; }

    /// @brief 获取状态码
    /// @return 返回状态码
    int getCode() const { return mStateCode; }

    /// @brief 获取消息
    /// @return 返回消息
    const std::string &getMessage() const { return mMessage; }

private:
    StateType mType;      ///< 状态类型
    int mStateCode;       ///< 状态码（可选）
    std::string mMessage; ///< 附带的消息（可选）
};

/// @brief 观察者基类，定义状态改变的处理接口
class ObserverBase
{
public:
    /// @brief 状态改变的处理逻辑
    /// @param event 包含状态改变的相关信息
    virtual void stateChanged(const StateChangeEvent &event) = 0;

    /// @brief 析构函数
    virtual ~ObserverBase() = default;
};

///////////////////////////////config相关实现类/////////////////////////////////
/// @brief 配置检查器主题类
class ConfigSubject : public SubjectBase
{
public:
    void addObserver(ObserverBase *observer) override;                                    ///< 添加观察者
    void removeObserver(ObserverBase *observer) override;                                 ///< 删除观察者
    void notifyAllObservers(const StateChangeEvent &event) override;                      ///< 通知所有观察者
    void notifyObservers(ObserverBase *observer, const StateChangeEvent &event) override; ///< 通知单个观察者
};
/// @brief 配置观察者类
class configObserver : public ObserverBase
{
public:
    void stateChanged(const StateChangeEvent &event) override;
    // void stateChanged(const std::function<void()>& func) override; 传入函数对象或lambda 也可以用模板实现，后续考虑升级
};

/// @brief 静态全局配置文件管理器，后续考虑单例模式
class configManager
{
public:
    void addConfigFile(const std::string &configPath, configObserver *observer); ///< 加载配置文件,并添加观察者
    void watchConfig(const std::string &configPath);                             ///< 配置文件修改更新的监听器，修改时候通知所有观察者
    void removeConfig(const std::string &configPath);                            ///< 删除配置文件，同时删除对应的主题，清空所有观察者
    void getConfig(const std::string &configPath);                               ///< 得到对应的配置文件内容
    // 设计思路：
    // 1. addConfigFile 加载配置文件就向检查mConfigSubjectMap是否存在<文件路径，配置主题容器>，存在就将观察者加入配置主题容器，不存在就构造一个，并加入
    // 2. watchConfig 监听配置文件就向mConfigSubjectMap查询是否存在这个路径以及配置主题容器，如果有就以这个路径的文件，启动监听线程监听修改或删除，修改就通知所有观察者
    // 3. removeConfig 就从mConfigSubjectMap中删除配置文件，同时删除对应的主题，清空所有观察者
    // 4. getConfig 读取配置文件，直接从mConfigSubjectMap中读取，如果存在就返回，不存在就返回空
    static configManager &instance();

private:
    configManager() = default;                                ///< 禁止直接构造
    configManager(const configManager &) = delete;            ///< 禁止拷贝构造
    configManager &operator=(const configManager &) = delete; ///< 禁止拷贝赋值
    
    std::thread *mWatchThread{nullptr};                       ///< 配置文件状态监听器线程对象
    std::atomic<bool> mStopThread{false};                     ///< 原子线程停止标志
    std::map<std::string, ConfigSubject> mConfigSubjectMap;   ///< 管理多个文件的配置主题
};
#endif