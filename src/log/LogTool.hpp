#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <memory>
#include <mutex>

constexpr auto logFilePath = "./1.txt";

class LogTool
{
public:
    enum LogLevel
    {
        INFO,
        DEBUG,
        WARN,
        ERROR,
        MSG
    };

    // 获取单例实例
    static LogTool &getInstance()
    {
        return *instance;
    }

    // 打印日志到控制台
    template <typename... Args>
    void printLog(LogLevel level, const Args &...args)
    {
        std::cout << getTime() << " [" << levelToString(level) << "] ";
        ((std::cout << args), ...);
        std::cout << std::endl;
    }

    // 追加日志到文件
    template <typename... Args>
    void appendLog(LogLevel level, const Args &...args)
    {
        std::ofstream file(logFilePath, std::ios::app);
        if (file.is_open())
        {
            file << getTime() << " [" << levelToString(level) << "] ";
            ((file << args), ...);
            file << std::endl;
        }
        else
        {
            std::cerr << "Unable to open file for writing" << std::endl;
        }
    }

    // 清除日志文件
    void clearLog()
    {
        if (std::remove(logFilePath) != 0)
            std::cerr << "Failed to delete log file." << std::endl;
        else
            std::cout << "Log file deleted successfully." << std::endl;
    }

private:
    LogTool()
    {
        std::ofstream file(logFilePath, std::ios::app);
        if (file.is_open())
        {
            file << "-- Start Log: " << getTime() << " --" << std::endl;
        }
        else
        {
            std::cerr << "Unable to open file" << std::endl;
        }
    }

    // 时间格式化
    std::string getTime() const
    {
        time_t ts = time(nullptr);
        std::tm *timeinfo = std::localtime(&ts);
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
        return buffer;
    }

    // 日志级别转换
    const char *levelToString(LogLevel level) const
    {
        switch (level)
        {
        case INFO:
            return "INFO";
        case DEBUG:
            return "DEBUG";
        case WARN:
            return "WARN";
        case ERROR:
            return "ERROR";
        case MSG:
            return "MSG";
        default:
            return "UNKNOWN";
        }
    }

    static std::unique_ptr<LogTool> instance;
    static std::once_flag initInstanceFlag;
};

// 单例实例初始化
std::unique_ptr<LogTool> LogTool::instance;
std::once_flag LogTool::initInstanceFlag;

// 定义全局日志函数
#define LOG(...) LogTool::getInstance().printLog(LogTool::MSG, __VA_ARGS__)
#define LOG_INFO(...) LogTool::getInstance().appendLog(LogTool::INFO, __VA_ARGS__)
#define LOG_DEBUG(...) LogTool::getInstance().appendLog(LogTool::DEBUG, __VA_ARGS__)
#define LOG_WARN(...) LogTool::getInstance().appendLog(LogTool::WARN, __VA_ARGS__)
#define LOG_ERROR(...) LogTool::getInstance().appendLog(LogTool::ERROR, __VA_ARGS__)