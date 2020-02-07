#include "Viewer.h"
#include "version.h"
#include "utils/Config.h"
#include "utils/db/MongoCore.h"
#include "utils/db/Category.h"
#include "utils/db/Item.h"
#include "utils/db/Bom.h"
#include "vendor/imgui/imgui.h"
#include "widgets/BomViewer.h"
#include "widgets/ItemViewer.h"
#include "widgets/Logger.h"
#include "widgets/Popup.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <string.h>

struct Version
{
    std::vector<std::string> changes;
    std::string version;
};

static bool VerifySoftwareVersion(bool showChangeLog = false);
static void HandleChangeLogPopup();
static void HandleNewVersionInputPopup();
static void HandleFeedbackPopup();
static void SaveChanges();
static void SendFeedback();
static void Refresh();

static std::vector<std::string> changeLog;
static std::vector<Version> changeHistory;
static char changes[1000] = { 0 };
static char feedback[1000] = { 0 };

void Viewer::Init()
{
    // Look for URI in config file. 
    // If it is found, use it to connect to the database.
    // Otherwise, use the default parameter of `DB::Init`.
    auto uri = Config::GetField<std::string>("uri");
    if (uri.empty())
    {
        Logging::System.Error("No URI found in config file, using localhost database.");
        DB::Init();
    }
    else
    {
        DB::Init(uri);
    }
    DB::Category::Init();
    DB::Item::Init();
    DB::BOM::Init();

}

void Viewer::Render()
{
    static bool hasVersionBeenChecked = false;
#ifdef USE_DEBUG_DB
    ImGui::Text("USING DEBUG DATABASE!");
#endif
    if (hasVersionBeenChecked == false)
    {
        hasVersionBeenChecked = true;
        VerifySoftwareVersion();
    }
    Refresh();

    ImGui::BeginTabBar("##TabBar");

    if (ImGui::BeginTabItem("Inventory"))
    {
        ItemViewer::Render();
        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("BOM"))
    {
        BomViewer::Render();
        ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
}

void Viewer::ShowChangeLog()
{
    VerifySoftwareVersion(true);

}

void Viewer::ShowSuggestionBox()
{
    memset(feedback, 0, sizeof(feedback));
    Popup::Init("Feedback/Suggestion", false);
    Popup::AddCall(HandleFeedbackPopup);
    Popup::AddCall(Popup::Button, "Send", std::function<void()>(SendFeedback), true);
    Popup::AddCall(Popup::SameLine);
    Popup::AddCall(std::function<bool(std::string&)>(Popup::Button), "Cancel");
}

bool VerifySoftwareVersion(bool showChangeLog)
{
    bool isUpToDate = true;
    bool isNewerVersion = false;
    bsoncxx::stdx::optional<mongocxx::cursor> docs = DB::GetAllDocuments("CEP", "Version");
    bsoncxx::document::view doc;

    // Get the last document from the collection. That will be the latest version of the software.
    // It's fucking hacky, but it works.
    if (docs)
    {
        changeHistory.clear();
        for (auto d : docs.value())
        {
            Version version;
            doc = d;
            // Fill-in change history.
            bsoncxx::document::element el = doc["ChangeLog"];
            if (el.raw() != nullptr)
            {
                if (el.type() == bsoncxx::type::k_array)
                {
                    for (auto& i : el.get_array().value)
                    {
                        version.changes.emplace_back(i.get_utf8().value.data());
                    }
                }
            }

            el = doc["Version"];
            if (el.raw() != nullptr)
            {
                std::stringstream ss;
                ss << "Version " << el["Major"].get_int32().value
                    << "." << el["Minor"].get_int32().value
                    << "." << el["Revision"].get_int32().value
                    << " (Build " << el["Build"].get_int32().value << ")" << std::endl;
                version.version = ss.str();
            }
            changeHistory.emplace_back(version);
        }
    }


    bsoncxx::document::element el = doc["Version"];
    if (el.raw() != nullptr)
    {
        if (el.type() == bsoncxx::type::k_document)
        {
            bsoncxx::document::element subEl = el["Build"];
            if (subEl.raw() != nullptr)
            {
                if (subEl.type() == bsoncxx::type::k_int32)
                {
                    if (VERSION_BUILD < subEl.get_int32().value)
                    {
                        isUpToDate = false;
                    }
                    else if (VERSION_BUILD > subEl.get_int32().value)
                    {
                        isNewerVersion = true;
                    }
                }
            }
            else
            {
                isNewerVersion = true;
            }
        }
    }
    else
    {
        isNewerVersion = true;
    }

    if (isUpToDate == false || showChangeLog == true)
    {
        el = doc["ChangeLog"];
        if (el.raw() != nullptr)
        {
            if (el.type() == bsoncxx::type::k_array)
            {
                for (auto& i : el.get_array().value)
                {
                    changeLog.emplace_back(i.get_utf8().value.data());
                }
            }
        }
        std::string title = showChangeLog == true ? "Change Log" : "A new version is available!";
        Popup::Init(title.c_str(), true, ImVec2(800, 600));
        Popup::AddCall(HandleChangeLogPopup);
    }
    else if (isNewerVersion == true)
    {
        Popup::Init("Enter New Version's Description", std::function<void()>(SaveChanges));
        Popup::AddCall(HandleNewVersionInputPopup);
    }
    return true;
}

void HandleChangeLogPopup()
{
    std::string msg = "What's new:";
    std::string style = "Bold";
    Popup::TextStylized(msg, style, false);

    for (auto version = changeHistory.end() - 1; version != changeHistory.begin() - 1; version--)
    {
        Popup::TextStylized(version->version, style, false);
        ImGui::BeginChildFrame(ImGui::GetID(std::string("ChangeLogChildFrame" + version->version).c_str()),
                               ImVec2(0, 250));
        for (const std::string& i : version->changes)
        {
            ImGui::Bullet();
            ImGui::TextWrapped(i.c_str());
        }
        ImGui::EndChildFrame();

    }


}

void HandleNewVersionInputPopup()
{
    ImGui::InputTextMultiline("Changes", changes, sizeof(changes), ImVec2());
}

void HandleFeedbackPopup()
{
    ImGui::TextWrapped("Please enter your feedback or suggestion in the box bellow.");
    ImGui::TextWrapped("Be as precise as possible please.");

    ImGui::InputTextMultiline("", feedback, sizeof(feedback),
                              ImVec2(ImGui::GetWindowContentRegionWidth(), ImGui::GetWindowHeight() - 100));
    std::string size = feedback;
    size = StringUtils::NumToString(int(size.size())) + "/1000 Characters";
    static std::string style = "Light";
    Popup::TextStylized(size, style);
}

void SaveChanges()
{
    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::make_document;
    using bsoncxx::builder::basic::make_array;

    if (DB::HasUserWritePrivileges() == false)
    {
        Popup::Init("Unauthorized");
        Popup::AddCall(Popup::TextStylized, "You must be logged in to do this action", "Bold/4278190335", true);

        return;
    }

    bsoncxx::builder::basic::document builder = {};

    builder.append(kvp("Version", make_document(
        kvp("Major", VERSION_MAJOR),
        kvp("Minor", VERSION_MINOR),
        kvp("Revision", VERSION_REVISION),
        kvp("Build", VERSION_BUILD))));

    changeLog.clear();

    char* next_token;
    char* pch = strtok_s(changes, "\n", &next_token);
    while (pch != nullptr)
    {
        changeLog.emplace_back(pch);
        pch = strtok_s(nullptr, "\n", &next_token);
    }

    // If text box is empty, do not save as new version.
    if (changeLog.empty())
    {
        return;
    }

    auto array_builder = bsoncxx::builder::basic::array{};
    for (const auto& line : changeLog)
    {
        array_builder.append(line);
    }

    builder.append(kvp("ChangeLog", array_builder));

    DB::InsertDocument(builder.extract(), "CEP", "Version");
}

void SendFeedback()
{
    std::string fb = feedback;
    if (fb.empty() == true)
    {
        return;
    }

    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::make_document;
    using bsoncxx::builder::basic::make_array;

    bsoncxx::builder::basic::document builder = {};

    builder.append(kvp("Version", make_document(
        kvp("Major", VERSION_MAJOR),
        kvp("Minor", VERSION_MINOR),
        kvp("Revision", VERSION_REVISION),
        kvp("Build", VERSION_BUILD))));

    builder.append(kvp("FeedbackEntry", fb));

    DB::InsertDocument(builder.extract(), "CEP", "Feedback");

    Popup::Init("Thank you for your feedback!");
    Popup::AddCall(Popup::Text, "I will read it as soon as possible!");
}

void Refresh()
{
    static double elapsedTime = 0;
    static int frameCount = 0;
    // deltaTime is the time between two frames (e.g. deltaTime @ 60fps is ~16.667ms).
    const static float deltaTime = ImGui::GetIO().DeltaTime;

    // If we're at a new frame:
    if (frameCount != ImGui::GetFrameCount())
    {
        // Add the deltaTime to elapsed time.
        frameCount = ImGui::GetFrameCount();
        elapsedTime += deltaTime;
        // If it has been more than 5 minutes since the last refresh:
        if (elapsedTime >= 5000.f)
        {
            // Do the refresh.
            elapsedTime = 0;
            VerifySoftwareVersion();
        }
    }
}
