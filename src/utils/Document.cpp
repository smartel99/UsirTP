﻿#include "Document.h"
#include "imgui/imgui.h"
#include "utils/StringUtils.h"
#include <string>
#include <fstream>
#include <filesystem>

/** @attention The functions of the Document class aren't commented because the class
 * is (probably) not used anywhere in this project */
Document::Document(const std::wstring path)
{
    m_FilePath = path;
    std::ifstream rawData;
    rawData.open(path);
    if (rawData.is_open() == false)
    {
        // File wasn't opened successfully.
        Open = false;
        return;
    }

    std::string line;
    while (std::getline(rawData, line))
    {
        // Read the entire file.
        m_FileData.push_back(line);
        std::cout << "Line " << m_FileData.size() - 1 << ": " <<
            line << std::endl;
    }

    rawData.close();

    m_FileName = StringUtils::GetFullNameFromPath(m_FilePath);
    Open = true;
}

/** @attention The functions of the Document class aren't commented because the class
 * is (probably) not used anywhere in this project */
Document::~Document() = default;

/** @attention The functions of the Document class aren't commented because the class
 * is (probably) not used anywhere in this project */
Document Document::DoOpen()
{
    return Document(m_FilePath);
}

/** @attention The functions of the Document class aren't commented because the class
 * is (probably) not used anywhere in this project */
void Document::DoClose()
{
    m_WantClose = true;
}

/** @attention The functions of the Document class aren't commented because the class
 * is (probably) not used anywhere in this project */
void Document::DoForceClose()
{
    // Empty the data buffer.
    m_FileData.clear();
    Open = false;
}

/** @attention The functions of the Document class aren't commented because the class
 * is (probably) not used anywhere in this project */
void Document::DoSave()
{
    std::ofstream file;
    file.open(m_FilePath);
    if (file.is_open() == false)
    {
        // Unable to open file.
        std::cout << "Unable to open file to save!" << std::endl;
        return;
    }

    for (const std::string& line : m_FileData)
    {
        // Copy all the modified data into the file.
        file << line;
    }

    file.close();
    m_Dirty = false;
}

/** @attention The functions of the Document class aren't commented because the class
 * is (probably) not used anywhere in this project */
void Document::DisplayContents()
{
    ImGui::PushID(this);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));

    for (std::string line : m_FileData)
    {
        line = StringUtils::ReplaceAll(line, "%", "%%");
        ImGui::Text(line.c_str());
    }

    ImGui::PopStyleColor();
    if (ImGui::Button("Modify", ImVec2(100, 100)))
    {
        m_Dirty = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Save", ImVec2(100, 100)))
    {
        DoSave();
    }
    ImGui::PopID();
}

/** @attention The functions of the Document class aren't commented because the class
 * is (probably) not used anywhere in this project */
void Document::DisplayContextMenu()
{
    if (ImGui::BeginPopupContextItem())
    {
        std::string save = "Save " + m_FileName;
        if (ImGui::MenuItem(save.c_str(), "CTRL+S", false, Open))
        {
            DoSave();
        }
        if (ImGui::MenuItem("Close", "CTRL+W", false, Open))
        {
            DoClose();
        }
        ImGui::EndPopup();
    }
}

namespace File
{
/**
 * @brief   Make the user choose a file.
 *          Beyond that, I have no idea what this does, it is an example
 *          provided by Microsoft, which I can't find the link to anymore.
 * @param   filePath: A reference to a wstring in which the chosen path will be stored.
 * @param   type: The file type that will be selected by default when the pop up opens.
 * @param   ext: I don't know the difference between that and `type`, but ext is the
 *          extension of the default file type.
 * @retval  The result of the operations I suppose. (https://en.wikipedia.org/wiki/HRESULT)
 */
// Suppress warning "The enum type '...' is unscoped. Prefer 'enum class' over 'enum'
#pragma warning( suppress : 26812 )
HRESULT OpenFile(std::wstring& filePath, FileTypeEnum_t type, LPCWSTR ext)
{
    // CoCreate the File Open Dialog object.
    IFileDialog* pfd = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr,
                                  CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    if (SUCCEEDED(hr))
    {
        // Create an event handling object, and hook it up to the dialog.
        IFileDialogEvents* pfde = nullptr;
        hr = CDialogEventHandler_CreateInstance(IID_PPV_ARGS(&pfde));
        if (SUCCEEDED(hr))
        {
            // Hook up the event handler.
            DWORD dwCookie;
            hr = pfd->Advise(pfde, &dwCookie);
            if (SUCCEEDED(hr))
            {
                // Set up the options on the dialog.
                DWORD dwFlags;

                // Before setting, always get the options first
                // to avoid overwriting existing options.
                hr = pfd->GetOptions(&dwFlags);
                if (SUCCEEDED(hr))
                {
                    // Set the file types to display only.
                    hr = pfd->SetFileTypes(ARRAYSIZE(c_rgSaveTypes),
                                           c_rgSaveTypes);
                    if (SUCCEEDED(hr))
                    {
                        // Set the selected file type index to Script.
                        hr = pfd->SetFileTypeIndex(type);
                        if (SUCCEEDED(hr))
                        {
                            // Set the default extension to be ".s"
                            hr = pfd->SetDefaultExtension(ext);
                            if (SUCCEEDED(hr))
                            {
                                // Show the dialog.
                                hr = pfd->Show(nullptr);
                                if (SUCCEEDED(hr))
                                {
                                    // Obtain the result, once the user
                                    // clicks the 'Open' button.
                                    // The result is an IShellItem object.
                                    IShellItem* psiResult;
                                    hr = pfd->GetResult(&psiResult);
                                    if (SUCCEEDED(hr))
                                    {
                                        // Do the things we want to do.
                                        PWSTR pszFilePath = nullptr;
                                        hr = psiResult->GetDisplayName(
                                            SIGDN_FILESYSPATH, &pszFilePath);
                                        if (SUCCEEDED(hr))
                                        {
                                            filePath = pszFilePath;
                                            CoTaskMemFree(pszFilePath);
                                        }
                                        psiResult->Release();
                                    }
                                }
                            }
                        }
                    }
                }
                // Unhook the event handler.
                pfd->Unadvise(dwCookie);
            }
            pfde->Release();
        }
        pfd->Release();
    }
    return hr;
}

HRESULT SaveFile(std::wstring& filePath, FileTypeEnum_t type, LPCWSTR ext)
{
    // CoCreate the File Open Dialog object.
    IFileDialog* pfd = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, nullptr,
                                  CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    if (SUCCEEDED(hr))
    {
        // Create an event handling object, and hook it up to the dialog.
        IFileDialogEvents* pfde = nullptr;
        hr = CDialogEventHandler_CreateInstance(IID_PPV_ARGS(&pfde));
        if (SUCCEEDED(hr))
        {
            // Hook up the event handler.
            DWORD dwCookie;
            hr = pfd->Advise(pfde, &dwCookie);
            if (SUCCEEDED(hr))
            {
                // Set up the options on the dialog.
                DWORD dwFlags;

                // Before setting, always get the options first
                // to avoid overwriting existing options.
                hr = pfd->GetOptions(&dwFlags);
                if (SUCCEEDED(hr))
                {
                    // Set the file types to display only.
                    hr = pfd->SetFileTypes(ARRAYSIZE(c_rgSaveTypes),
                                           c_rgSaveTypes);
                    if (SUCCEEDED(hr))
                    {
                        // Set the selected file type index to Script.
                        hr = pfd->SetFileTypeIndex(type);
                        if (SUCCEEDED(hr))
                        {
                            // Set the default extension to be ".s"
                            hr = pfd->SetDefaultExtension(ext);
                            if (SUCCEEDED(hr))
                            {
                                // Show the dialog.
                                hr = pfd->Show(nullptr);
                                if (SUCCEEDED(hr))
                                {
                                    // Obtain the result, once the user
                                    // clicks the 'Open' button.
                                    // The result is an IShellItem object.
                                    IShellItem* psiResult;
                                    hr = pfd->GetResult(&psiResult);
                                    if (SUCCEEDED(hr))
                                    {
                                        // Do the things we want to do.
                                        PWSTR pszFilePath = nullptr;
                                        hr = psiResult->GetDisplayName(
                                            SIGDN_FILESYSPATH, &pszFilePath);
                                        if (SUCCEEDED(hr))
                                        {
                                            filePath = pszFilePath;
                                            CoTaskMemFree(pszFilePath);
                                        }
                                        psiResult->Release();
                                    }
                                }
                            }
                        }
                    }
                }
                // Unhook the event handler.
                pfd->Unadvise(dwCookie);
            }
            pfde->Release();
        }
        pfd->Release();
    }
    return hr;
}

DWORD GetCurrentPath(std::string& path)
{
    char lpFilename[MAX_PATH];
    // MAX_PATH -> The maximum length for a path defined by the Windows API.
    DWORD nSize = GetModuleFileNameA(nullptr, lpFilename, MAX_PATH);
    path = lpFilename;
    return nSize;
}

std::string GetCurrentPath()
{
    std::string path = "";
    GetCurrentPath(path);
    return StringUtils::RemoveNameFromPath(path);
}

std::string GetPathOfFile(const std::string& file)
{
    std::string path = "";

    GetCurrentPath(path);

    path = StringUtils::RemoveNameFromPath(path);
    return path + file;
}

bool IsEmpty(std::fstream& pFile)
{
    return pFile.peek() == std::ifstream::traits_type::eof();
}


struct path_leaf_string
{
    std::string operator()(const std::filesystem::directory_entry& entry) const
    {
        return entry.path().string();
    }
};

std::vector<std::string> GetFilesInDir(const std::string& dirPath)
{
    std::vector<std::string> files;
    GetFilesInDir(dirPath, files);
    return files;
}

void GetFilesInDir(const std::string& dirPath, std::vector<std::string>& files)
{
    std::filesystem::path p(dirPath);
    try
    {
        std::filesystem::directory_iterator start(p);
        std::filesystem::directory_iterator end;
        std::transform(start, end, std::back_inserter(files), path_leaf_string());
    }
    catch (std::filesystem::filesystem_error)
    {
        files.clear();
    }
}
}
