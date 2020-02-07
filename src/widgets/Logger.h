#pragma once

#include "imgui/imgui.h"
#include "utils/StringUtils.h"
#include <iostream>
#include <sstream>
#include <vector>


class Logger
{
public:
    Logger(void);
    ~Logger(void);

    void Clear(void);
    void AddLog(const char* fmt);
    void Draw(const char* title);
    inline void Open(void)
    {
        m_Open = true;
    }
    inline void Close()
    {
        m_Open = false;
    }

private:
    std::vector<std::string> m_Buf;
    ImGuiTextFilter m_Filter;
    ImVector<int>   m_LineOffsets;  // Index to lines offset.
    bool            m_AutoScroll;
    bool            m_ScrollToBottom;
    bool            m_Open = false;
};

namespace Logging
{
#define DEFAULT_LOG_LEVEL LogLevelEnum_t::LOG_LEVEL_DEBUG
enum class LogLevelEnum_t
{
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_CRITICAL,
    LOG_LEVEL_NONE,
};

void Init();
void Clear();
void Draw();
void OpenConsole();
void SetLogLevel(LogLevelEnum_t level);

void Debug(const std::string& fmt, bool save = false);
void Info(const std::string& fmt, bool save = false);
void Warning(const std::string& fmt, bool save = false);
void Error(const std::string& fmt, bool save = false);
void Critical(const std::string& fmt, bool save = false);

class LogSource
{
public:
    LogSource(const std::string& sourceName) :m_Source(sourceName)
    {
    }

    template<typename T = std::string>
    void Debug(const std::string& str, T val = "", bool save = false)
    {
        std::ostringstream msg;

        msg << StringUtils::GetCurrentTimeFormated() << m_Source
            << "[DEBUG   ] "
            << str << val << "\n\r";

        Logging::Debug(msg.str(), save);
    }

    template<typename T = std::string>
    void Info(const std::string& str, T val = "", bool save = false)
    {
        std::ostringstream msg;

        msg << StringUtils::GetCurrentTimeFormated() << m_Source
            << "[INFO    ] "
            << str << val << "\n\r";

        Logging::Info(msg.str(), save);
    }

    template<typename T = std::string>
    void Warning(const std::string& str, T val = "", bool save = false)
    {
        std::ostringstream msg;

        msg << StringUtils::GetCurrentTimeFormated() << m_Source
            << "[WARNING ] "
            << str << val << "\n\r";

        Logging::Warning(msg.str(), save);
    }

    template<typename T = std::string>
    void Error(const std::string& str, T val = "", bool save = false)
    {
        std::ostringstream msg;

        msg << StringUtils::GetCurrentTimeFormated() << m_Source
            << "[ERROR   ] "
            << str << val << "\n\r";

        Logging::Error(msg.str(), save);
    }

    template<typename T = std::string>
    void Critical(const std::string& str, T val = "", bool save = false)
    {
        std::ostringstream msg;

        msg << StringUtils::GetCurrentTimeFormated() << m_Source
            << "[CRITICAL] "
            << str << val << "\n\r";

        Logging::Critical(msg.str(), save);
    }

private:
    std::string m_Source = "";
};

extern LogSource System;
extern LogSource Audit;

}
