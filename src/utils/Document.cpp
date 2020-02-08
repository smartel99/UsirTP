#include "Document.h"
#include "imgui/imgui.h"
#include "utils/StringUtils.h"
#include "widgets/Popup.h"
#include <string>
#include <fstream>
#include <filesystem>


/**
 * @brief   Load the file `path` and creates an instance of Document from it.
 * @param   path: The path of the Document to open.
 * @retval  An instance of Document.
 */
Document::Document(const std::wstring path)
{
    m_FilePath = path;

    // Load the file into a file buffer.
    std::ifstream rawData;
    rawData.open(path);

    // If the file couldn't be opened:
    if (rawData.is_open() == false)
    {
        // Mark the Document as closed and return.
        Open = false;
        return;
    }

    // Load the file's content into memory.
    std::string line;
    // For as long as there are lines of text to read in the file:
    while (std::getline(rawData, line))
    {
        // Add the newly read line to the Document's file buffer.
        m_FileData.emplace_back(line);
    }

    // Close the file.
    rawData.close();

    // Get the name of the file from its path.
    m_FileName = StringUtils::GetFullNameFromPath(m_FilePath);
    Open = true;
}


Document Document::DoOpen()
{
    return Document(m_FilePath);
}

void Document::DoClose()
{
    m_WantClose = true;
}

void Document::DoForceClose()
{
    // Empty the data buffer.
    m_FileData.clear();
    Open = false;
}

void Document::DoSave()
{
    // Open the file in output mode.
    std::ofstream file;
    file.open(m_FilePath);

    // If unable to open the file:
    if (file.is_open() == false)
    {
        // Warn the user.
        Popup::Init("Error");
        Popup::AddCall(Popup::Text, "Unable to open file to save!");
        return;
    }

    // For each lines in the Document's buffer:
    for (const std::string& line : m_FileData)
    {
        // Write that line into the file.
        file << line;
    }

    // Close the file.
    file.close();

    // Mark the Document as clean since all changes have been saved.
    m_Dirty = false;
}

void Document::DisplayContents()
{
    // Push the Document on the Imgui stack so it is its "own thing"
    // I'm honestly not too sure about this one.
    ImGui::PushID(this);

    // Change the color of the text to white on the Imgui stack.
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));

    // For each line in the Document's buffer:
    for (std::string line : m_FileData)
    {
        // Replace all `%` by `%%`.
        // We have to do this because `Imgui::Text` uses a printf-like
        // formatting to the string received in parameter.
        // Not changing those `%` causes the function to expects 
        // variadic arguments to be present on the stack, which there aren't,
        // causing undesired behavior when displaying the text.
        // 
        // Another solution would have been to just use `ImGui::TextUnformatted` instead.
        line = StringUtils::ReplaceAll(line, "%", "%%");
        ImGui::Text(line.c_str());
    }

    // Pop the text color we pushed on the Imgui stack previously.
    ImGui::PopStyleColor();

    // If the user has clicked on the `Modify` button:
    if (ImGui::Button("Modify", ImVec2(100, 100)))
    {
        // Mark the Document as dirty.
        m_Dirty = true;
    }
    // Draw the next ImGui item on the same line as the previous one.
    ImGui::SameLine();
    // If the user as clicked on the `Save` button:
    if (ImGui::Button("Save", ImVec2(100, 100)))
    {
        // Save the Document.
        DoSave();
    }

    // Pop the Document from the ImGui stack.
    ImGui::PopID();
}

void Document::DisplayContextMenu()
{
    // If the pop up should be rendered (the user has done the action that opens it):
    if (ImGui::BeginPopupContextItem())
    {
        // Create the save label.
        std::string save = "Save " + m_FileName;
        // If the user has clicked on the save option:
        if (ImGui::MenuItem(save.c_str(), "CTRL+S", false, Open))
        {
            // Save the Document.
            DoSave();
        }
        // If the user has clicked on the close option:
        if (ImGui::MenuItem("Close", "CTRL+W", false, Open))
        {
            // Close the Document.
            DoClose();
        }
        // End the pop up.
        ImGui::EndPopup();
    }
}

namespace File
{
/**
 * @brief   Open a file dialog made by the OS and let the user select a file to be opened.
 * @param   filePath: A reference to a `std::wstring` in which the chosen file's path will be stored.
 * @param   type: The default type of files to display in the dialog.
 * @param   ext: A const wchar* containing the default extension of the files to display by default.
 * @retval  The `HRESULT` of the operation.
 *
 * @note    This function is copied from an example provided by Microsoft, thus the abnormal structure.
 *          The comments are also copied from that example.
 *
 * @note    To construct a `LPCWSTR`, you can do `L"My LPCWSTR"`.
 */
HRESULT OpenFile(std::wstring& filePath, FileType type, LPCWSTR ext)
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
                        hr = pfd->SetFileTypeIndex(UINT(type));
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

/**
 * @brief   Open a file dialog made by the OS and let the user select a file to be used for the save operation.
 * @param   filePath: A reference to a `std::wstring` in which the chosen file's path will be stored.
 * @param   type: The default type of files to display in the dialog.
 * @param   ext: A const wchar* containing the default extension of the files to display by default.
 * @retval  The `HRESULT` of the operation.
 *
 * @note    This function is copied from an example provided by Microsoft, thus the abnormal structure.
 *          The comments are also copied from that example.
 *
 * @note    To construct a `LPCWSTR`, you can do `L"My LPCWSTR"`.
 */
HRESULT SaveFile(std::wstring& filePath, FileType type, LPCWSTR ext)
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
                        hr = pfd->SetFileTypeIndex(UINT(type));
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

/**
 * @brief   Get the path from which the program is currently running.
 *          This is the path where `UsirTP.exe` is being run from.
 * @param   path: A reference to a string in which the path will be stored.
 * @reval   The number of characters in `path`.
 */
DWORD GetCurrentPath(std::string& path)
{
    // MAX_PATH -> The maximum length for a path defined by the Windows API.
    char lpFilename[MAX_PATH];
    // Get the complete file name (path + file name) of the program.
    DWORD nSize = GetModuleFileNameA(nullptr, lpFilename, MAX_PATH);
    path = lpFilename;
    return nSize;
}

/**
 * @brief   Get the path from which the program is currently running from,
 *          with the name of the executable removed.
 * @param   None
 * @retval  The path.
 */
std::string GetCurrentPath()
{
    std::string path = "";
    // Get the complete path.
    GetCurrentPath(path);
    // Remove the name of the exe from that path.
    return StringUtils::RemoveNameFromPath(path);
}

/**
 * @brief   Create an absolute path for `file` by using the current path.
 * @param   file: The relative path of the desired file. This will be concatenated to the current path.
 * @retval  The absolute path of the file
 *
 * @note    This function only generates an absolute path. It is up to the caller to check if
 *          that path is valid.
 */
std::string GetPathOfFile(const std::string& file)
{
    std::string path = "";

    // Get the current path.
    GetCurrentPath(path);

    // Remove the name of the exe from the path.
    path = StringUtils::RemoveNameFromPath(path);
    // Concatenate `file` to the path.
    return path + file;
}

/**
 * @brief   Check if a file is empty.
 * @param   pFile: A file stream of the file to check.
 * @retval  `true` if the file is empty, `false` otherwise.
 */
bool IsEmpty(std::fstream& pFile)
{
    return pFile.peek() == std::ifstream::traits_type::eof();
}

/**
 * @struct  path_lead_string
 * @brief   Struct that does something with a path, I honestly don't remember what.
 *          It wouldn't be hard for you to check out what `directory_entry` is though ;)
 */
struct path_leaf_string
{
    std::string operator()(const std::filesystem::directory_entry& entry) const
    {
        return entry.path().string();
    }
};

/**
 * @brief   Get all the files contained in a directory and its directories.
 * @param   dirPath: The directory to get the files from.
 * @retval  A `std::vector<std::string>` containing all the files found.
 */
std::vector<std::string> GetFilesInDir(const std::string& dirPath)
{
    std::vector<std::string> files;
    GetFilesInDir(dirPath, files);
    return files;
}

/**
 * @brief   Get all the files contained in a directory and its directories.
 * @param   dirPath: The directory to get the files from.
 * @param   files: A reference where all the files found will be stored.
 * @retval  None
 */
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
