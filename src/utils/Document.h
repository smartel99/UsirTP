#pragma once
#include <iostream>
#include <vector>
#include <fstream>

#include "utils/CDialogEventHandler.h"

/**
 * @class   Document
 * @brief   I'm pretty sure this whole class is useless, but you should
 *          make sure of that by yourself before removing it.
 */
class Document
{
public:
    Document(const std::wstring path);
    ~Document();

    Document DoOpen();
    void DoClose();
    void DoForceClose();
    void DoSave();
    void DisplayContents();
    void DisplayContextMenu();

    inline void SetOpen(bool state)
    {
        Open = state;
    }

    inline void SetWantClose(bool state)
    {
        m_WantClose = state;
    }

    inline bool IsOpen()
    {
        return Open;
    }

    inline bool* GetIsOpen()
    {
        return &Open;
    }

    inline bool IsDirty()
    {
        return m_Dirty;
    }

    inline bool WantsToClose()
    {
        return m_WantClose;
    }

    inline std::string Name()
    {
        return m_FileName;
    }

    bool            Open = false;         // Set when the document is open.

private:
    std::wstring    m_FilePath = L"";       // The document's file path.
    std::string     m_FileName = "";        // The document's file name.
    std::vector<std::string > m_FileData;
    bool            m_OpenPrev = false;     // Copy of Open from last update.
    bool            m_Dirty = false;        // Set when the document has been modified.
    bool            m_WantClose = false;    // Set when a request to close the document
                                            // has been made.
};

/**
 * @namespace   File Document.h Document
 * @brief       The namespace that handles all things related to file operation
 */
namespace File
{
HRESULT OpenFile(std::wstring& filePath, FileTypeEnum_t type = INDEX_DEFAULT, LPCWSTR ext = L".*");
HRESULT SaveFile(std::wstring& filePath, FileTypeEnum_t type = INDEX_DEFAULT, LPCWSTR ext = L".*");
DWORD GetCurrentPath(std::string& path);
std::string GetCurrentPath();
std::string GetPathOfFile(const std::string& file);
bool IsEmpty(std::fstream& pFile);
std::vector<std::string> GetFilesInDir(const std::string& dirPath);
void GetFilesInDir(const std::string& dirPath, std::vector<std::string>& files);
}
