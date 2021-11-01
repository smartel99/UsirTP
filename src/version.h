/**
 ******************************************************************************
 * @addtogroup version
 * @{
 * @file    version
 * @author  Client Microdata
 * @brief   Header for the version module.
 *
 * @date 1/30/2020 2:31:05 PM
 *
 ******************************************************************************
 */
#ifndef _version_
#define _version_

/*****************************************************************************/
/* Includes */


/*****************************************************************************/
/* Exported defines */
#define STRINGIZE2(s) #s
#define STRINGIZE(s) STRINGIZE2(s)

#define VERSION_MAJOR               1
#define VERSION_MINOR               1
#define VERSION_REVISION            0
#define VERSION_BUILD               100
#define VER_FILE_DESCRIPTION_STR    "Frasy Universal Test-Bench Control Station"
#define VER_FILE_VERSION            VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION, VERSION_BUILD
#define VER_FILE_VERSION_STR        STRINGIZE(VERSION_MAJOR)        \
                                    "." STRINGIZE(VERSION_MINOR)    \
                                    "." STRINGIZE(VERSION_REVISION) \
                                    "." STRINGIZE(VERSION_BUILD)    \

#define VER_PRODUCTNAME_STR         "Frasy Universal Test-Bench Control Station"
#define VER_PRODUCT_VERSION         VER_FILE_VERSION
#define VER_PRODUCT_VERSION_STR     VER_FILE_VERSION_STR
#define VER_ORIGINAL_FILENAME_STR   VER_PRODUCTNAME_STR ".exe"
#define VER_INTERNAL_NAME_STR       VER_ORIGINAL_FILENAME_STR
#define VER_COPYRIGHT_STR           "Copyright Conception Électronique Privé (C) 2021"

#ifdef _DEBUG
#define VER_VER_DEBUG             VS_FF_DEBUG
#else
#define VER_VER_DEBUG             0
#endif

#define VER_FILEOS                  VOS_NT_WINDOWS32
#define VER_FILEFLAGS               VER_VER_DEBUG
#define VER_FILETYPE                VFT_APP

/*****************************************************************************/
/* Exported macro */


/*****************************************************************************/
/* Exported types */


/*****************************************************************************/
/* Exported functions */


/* Have a wonderful day :) */
#endif /* _version_ */
/**
 * @}
 */
/****** END OF FILE ******/
