#include "Logger.h"
#include "Application.h"
#include "utils/Config.h"
#include "utils/Fonts.h"
#include "widgets/MainMenu.h"
#include <iostream>
#include <stdexcept>


Logger logger;

static Logging::LogLevelEnum_t logLevel = Logging::LOG_LEVEL_DEBUG;
static bool isReadyToGoDownToFlavortown = false;
static int hitCount = 0;
static double timeElapsed = 0;

static void RenderColoredText(const std::string& msg);

Logger::Logger(void)
{
    m_AutoScroll = true;
    m_ScrollToBottom = false;
    try
    {
        logLevel = Config::GetField < Logging::LogLevelEnum_t >("LogLevel");
    }
    catch ( std::invalid_argument )
    {
        // Field didn't exist in the config file, use default level.
        logLevel = Logging::DEFAULT_LOG_LEVEL;
        Config::SetField("LogLevel", logLevel);
    }
    Clear();
}


Logger::~Logger(void)
{
}

void Logger::Clear(void)
{
    m_Buf.clear();
    m_LineOffsets.clear();
    m_LineOffsets.push_back(0);
}

void Logger::AddLog(const char* fmt)
{
    m_Buf.push_back(fmt);
    if ( m_AutoScroll == true )
    {
        m_ScrollToBottom = true;
    }
}

void Logger::Draw(const char* title)
{

    if ( m_Open == false )
    {
        return;
    }
    if ( !ImGui::Begin(title, &m_Open) )
    {
        ImGui::End();
        return;
    }

    // Main Window.

    if ( ImGui::Checkbox("Auto-scroll", &m_AutoScroll) )
    {
        if ( m_AutoScroll == true )
        {
            m_ScrollToBottom = true;
        }
    }
    ImGui::SameLine();
    bool clear = ImGui::Button("Clear");
    ImGui::SameLine();

    bool copy = false;
    if ( hitCount == 6 && Fonts::Push("Blazed") == Fonts::FONT_OK )
    {
        copy = ImGui::Button("Let's go!");
        Fonts::Pop();
    }
    else
    {
        copy = ImGui::Button("Copy");
    }

    ImGui::SameLine();
    m_Filter.Draw("Filter", -100.f);

    ImGui::Separator();
    ImGui::BeginChild("scrolling", ImVec2(0, 0), false,
                      ImGuiWindowFlags_HorizontalScrollbar);

    if ( clear == true )
    {
        Clear();
    }
    if ( copy == true )
    {
        ImGui::LogToClipboard();
    }

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

    if ( m_Filter.IsActive() == true )
    {
        for ( const std::string& line : m_Buf )
        {
            if ( m_Filter.PassFilter(line.c_str()) )
            {
                RenderColoredText(line);
            }
        }
    }
    else
    {
        for ( const std::string& line : m_Buf )
        {
            RenderColoredText(line);
        }
    }
    ImGui::PopStyleVar();

    if ( m_ScrollToBottom == true )
    {
        ImGui::SetScrollHereY(1.0f);
    }
    m_ScrollToBottom = false;

    ImGui::EndChild();
    ImGui::End();
}

void Logging::Clear(void)
{
    static int frameCount = 0;
    static double deltaTime = ImGui::GetIO().DeltaTime;

    if ( frameCount != ImGui::GetFrameCount() )
    {
        frameCount = ImGui::GetFrameCount();
        timeElapsed += deltaTime;
        if ( timeElapsed >= 2.0f )
        {
            timeElapsed = 0;
            if ( hitCount < 6 )
            {
                hitCount = 0;
            }
        }
    }
    logger.Clear();
}

void Logging::Draw(void)
{
    logger.Draw("Logger");
}

void Logging::OpenConsole(void)
{
    logger.Open();
}

void Logging::SetLogLevel(LogLevelEnum_t level)
{
    logLevel = level;
}

namespace Logging
{
    LogSource System("[SYSTEM     ]");
    LogSource TestBench("[TEST_BENCH ]");
    LogSource Interpreter("[INTERPRETER]");

    void Logging::Debug(const std::string& fmt)
    {
        if ( logLevel > LOG_LEVEL_DEBUG )
        {
            return;
        }

        logger.AddLog(fmt.c_str());
    }

    void Logging::Info(const std::string& fmt)
    {
        if ( logLevel > LOG_LEVEL_INFO )
        {
            return;
        }

        logger.AddLog(fmt.c_str());
    }

    void Logging::Warning(const std::string& fmt)
    {
        if ( logLevel > LOG_LEVEL_WARNING )
        {
            return;
        }

        logger.AddLog(fmt.c_str());
    }

    void Logging::Error(const std::string& fmt)
    {
        if ( logLevel > LOG_LEVEL_ERROR )
        {
            return;
        }

        logger.AddLog(fmt.c_str());
    }

    void Logging::Critical(const std::string& fmt)
    {
        if ( logLevel > LOG_LEVEL_CRITICAL )
        {
            return;
        }

        logger.AddLog(fmt.c_str());
    }

}

void RenderColoredText(const std::string& msg)
{
    std::string line(msg);
    ImVec4 color(0, 0, 0, 0);

    if ( line.find("[DEBUG   ]") != std::string::npos )
    {
        // 0x037BFC - Light Blue.
        color = ImVec4(0.01171875f, 0.48046875f, 0.984375f, 1.0f);
    }
    else if ( line.find("[INFO    ]") != std::string::npos )
    {
        // 0x03FCE8 - Cyan.
        color = ImVec4(0.01171875f, 0.984375f, 0.90625f, 1.0f);
    }
    else if ( line.find("[WARNING ]") != std::string::npos )
    {

        // 0xFCDF03 - Yellow.
        color = ImVec4(0.984375, 0.87109375f, 0.01171875f, 1.0f);
    }
    else if ( line.find("[ERROR   ]") != std::string::npos )
    {
        // 0xFC6F03 - Orange.
        color = ImVec4(0.984375f, 0.43359375f, 0.01171875f, 1.0f);
    }
    else if ( line.find("[CRITICAL]") != std::string::npos )
    {
        // 0xFC0303 - Red.
        color = ImVec4(0.984375f, 0.01171875f, 0.01171875f, 1.0f);
    }
    else
    {
        // Default style color.
        color = ImGui::GetStyleColorVec4(ImGuiCol_Text);
    }

    // Set the text color.
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::TextUnformatted(line.c_str());
    ImGui::PopStyleColor();
}
