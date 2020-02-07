#pragma once
#include <iostream>
#include <vector>
#include <fstream>

#include "utils/CDialogEventHandler.h"

/**
 * @class   Document
 * @brief   A class that handles most document operation.
 *
 * @note    Unused in UsirTP.
 * @note    I'm writing this documentation months after
 *          writing this code. It can possibly be erroneous, I'm sorry.
 */
class Document
{
public:
    Document(const std::wstring path);
    ~Document() = default;

    /**
     * @brief   Handles everything to safely open a document.
     * @param   None
     * @retval  An instance of the opened Document
     */
    Document DoOpen();

    /**
     * @brief   Safely closes a Document that is open.
     *          The document is saved before being closed.
     * @param   None
     * @retval  None
     */
    void DoClose();

    /**
     * @brief   Forcefully closes a Document, even if it has unsaved changes to it.
     *          The Document is **NOT** saved, every unsaved changes are lost.
     * @param   None
     * @retval  None
     */
    void DoForceClose();

    /**
     * @brief   Saves the Document into its file.
     * @param   None
     * @retval  None
     */
    void DoSave();

    /**
     * @brief   Display the content of the Document in a nice window,
     *          with no formatting applied to it.
     * @param   None
     * @retval  None
     */
    void DisplayContents();

    /**
     * @brief   Display a context menu containing a "Save" and a "Close" option
     * @param   None
     * @retval  None
     */
    void DisplayContextMenu();

    /**
     * @brief   Update the `open` state of the document.
     *          Setting it to `true` will load the Document,
     *          Setting it to `false` will gracefully close it.
     * @param   state: The desired state of the Document.
     * @retval  None
     */
    inline void SetOpen(bool state)
    {
        Open = state;
    }

    /**
     * @brief   Tell the module that we want to gracefully want to close the Document.
     *          The module will handle the graceful closure by itself.
     * @param   state: Setting it to `true` will gracefully close the Document.
     *          Setting it to `false` has no effects in most cases.
     * @retval  None
     *
     * @note    Since the handling of the graceful closure of Documents is handled by the
     *          module and not by the Documents themselves, it is undefined behavior to
     *          set this to `true` before instantly setting it to `false` to, for example,
     *          cancel the operation. The Document may or may not be closed if this is done.
     */
    inline void SetWantClose(bool state)
    {
        m_WantClose = state;
    }

    /**
     * @brief   Check if the Document is open.
     * @param   None
     * @retval  The openness of the Document.
     */
    inline bool IsOpen() const
    {
        return Open;
    }

    /**
     * @brief   Get a pointer to the variable containing the `open` status of the Document.
     * @param   None
     * @retval  A pointer to `Open`
     *
     * @note    I don't know why this returns a pointer in place of a reference. I think it
     *          is because the module uses a pointer to a bool to check if it should be rendered
     *          or not. (With `Imgui::Begin` and al.)
     */
    inline bool* GetIsOpen()
    {
        return &Open;
    }

    /**
     * @brief   Check if the Document is dirty.
     *          A Document is dirty if it has unsaved changes.
     * @param   None
     * @retval  `true` if the Document is dirty, `false` otherwise.
     */
    inline bool IsDirty() const
    {
        return m_Dirty;
    }

    /**
     * @brief   Check if the Document is waiting to be closed by the module.
     * @param   None
     * @retval  `true` if the Document is awaiting closure, `false` otherwise.
     */
    inline bool WantsToClose() const
    {
        return m_WantClose;
    }

    /**
     * @brief   Get the file name of the Document.
     * @param   None
     * @retval  The name of the Document.
     */
    inline const std::string& Name() const
    {
        return m_FileName;
    }

    bool            Open = false;           //!< Set when the document is open.

private:
    std::wstring    m_FilePath = L"";       /**< The document's file path. */
    std::string     m_FileName = "";        /**< The document's file name. */
    std::vector<std::string > m_FileData;   /**< The document's content. */
    bool            m_OpenPrev = false;     /**< Copy of Open from last update. */
    bool            m_Dirty = false;        /**< Set when the document has been modified. */
    bool            m_WantClose = false;    /**< Set when a request to close the document
                                                 has been made. */
};

/**
 * @namespace File File.h File
 * @brief   The namespace containing all functions related to file operations.
 */
namespace File
{
HRESULT OpenFile(std::wstring& filePath, FileType type = FileType::INDEX_DEFAULT, LPCWSTR ext = L".*");
DWORD GetCurrentPath(std::string& path);
std::string GetCurrentPath();
std::string GetPathOfFile(const std::string& file);
bool IsEmpty(std::fstream& pFile);
std::vector<std::string> GetFilesInDir(const std::string& dirPath);
void GetFilesInDir(const std::string& dirPath, std::vector<std::string>& files);
}

