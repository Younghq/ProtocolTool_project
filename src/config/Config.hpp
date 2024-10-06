#ifndef _Config_hpp_
#define _Config_hpp_
#include "ConfigLoader.hpp"
#include <vector>

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
#endif