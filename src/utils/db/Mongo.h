/**
 ******************************************************************************
 * @addtogroup Mongo
 * @{
 * @file    Mongo
 * @author  Client Microdata
 * @brief   Header for the Mongo module.
 *
 * @date 1/16/2020 4:40:21 PM
 *
 ******************************************************************************
 */
#ifndef _Mongo
#define _Mongo

/*****************************************************************************/
/* Includes */
#include "bsoncxx\decimal128.hpp"
#include "bsoncxx\json.hpp"
#include "bsoncxx\oid.hpp"
#include "bsoncxx\types.hpp"
#include "bsoncxx\validate.hpp"
#include "bsoncxx\view_or_value.hpp"
#include "bsoncxx\array\element.hpp"
#include "bsoncxx\array\value.hpp"
#include "bsoncxx\array\view.hpp"
#include "bsoncxx\array\view_or_value.hpp"
#include "bsoncxx\builder\concatenate.hpp"
#include "bsoncxx\builder\core.hpp"
#include "bsoncxx\builder\basic\array.hpp"
#include "bsoncxx\builder\basic\document.hpp"
#include "bsoncxx\builder\basic\helpers.hpp"
#include "bsoncxx\builder\basic\impl.hpp"
#include "bsoncxx\builder\basic\kvp.hpp"
#include "bsoncxx\builder\basic\sub_array.hpp"
#include "bsoncxx\builder\basic\sub_document.hpp"
#include "bsoncxx\builder\stream\array.hpp"
#include "bsoncxx\builder\stream\array_context.hpp"
#include "bsoncxx\builder\stream\closed_context.hpp"
#include "bsoncxx\builder\stream\document.hpp"
#include "bsoncxx\builder\stream\helpers.hpp"
#include "bsoncxx\builder\stream\key_context.hpp"
#include "bsoncxx\builder\stream\single_context.hpp"
#include "bsoncxx\builder\stream\value_context.hpp"
#include "bsoncxx\config\compiler.hpp"
#include "bsoncxx\config\config.hpp"
#include "bsoncxx\config\export.hpp"
#include "bsoncxx\config\postlude.hpp"
#include "bsoncxx\config\prelude.hpp"
#include "bsoncxx\config\version.hpp"
#include "bsoncxx\document\element.hpp"
#include "bsoncxx\document\value.hpp"
#include "bsoncxx\document\view.hpp"
#include "bsoncxx\document\view_or_value.hpp"
#include "bsoncxx\exception\error_code.hpp"
#include "bsoncxx\exception\exception.hpp"
#include "bsoncxx\stdx\make_unique.hpp"
#include "bsoncxx\stdx\optional.hpp"
#include "bsoncxx\stdx\string_view.hpp"
#include "bsoncxx\string\to_string.hpp"
#include "bsoncxx\string\view_or_value.hpp"
#include "bsoncxx\types\value.hpp"
#include "bsoncxx\util\functor.hpp"

#include "mongocxx\bulk_write.hpp"
#include "mongocxx\change_stream.hpp"
#include "mongocxx\client.hpp"
#include "mongocxx\client_session.hpp"
#include "mongocxx\collection.hpp"
#include "mongocxx\cursor.hpp"
#include "mongocxx\database.hpp"
#include "mongocxx\hint.hpp"
#include "mongocxx\index_model.hpp"
#include "mongocxx\index_view.hpp"
#include "mongocxx\instance.hpp"
#include "mongocxx\logger.hpp"
#include "mongocxx\pipeline.hpp"
#include "mongocxx\pool.hpp"
#include "mongocxx\read_concern.hpp"
#include "mongocxx\read_preference.hpp"
#include "mongocxx\stdx.hpp"
#include "mongocxx\uri.hpp"
#include "mongocxx\validation_criteria.hpp"
#include "mongocxx\write_concern.hpp"
#include "mongocxx\write_type.hpp"
#include "mongocxx\config\compiler.hpp"
#include "mongocxx\config\config.hpp"
#include "mongocxx\config\export.hpp"
#include "mongocxx\config\postlude.hpp"
#include "mongocxx\config\prelude.hpp"
#include "mongocxx\config\version.hpp"
#include "mongocxx\events\command_failed_event.hpp"
#include "mongocxx\events\command_started_event.hpp"
#include "mongocxx\events\command_succeeded_event.hpp"
#include "mongocxx\events\heartbeat_failed_event.hpp"
#include "mongocxx\events\heartbeat_started_event.hpp"
#include "mongocxx\events\heartbeat_succeeded_event.hpp"
#include "mongocxx\events\server_changed_event.hpp"
#include "mongocxx\events\server_closed_event.hpp"
#include "mongocxx\events\server_description.hpp"
#include "mongocxx\events\server_opening_event.hpp"
#include "mongocxx\events\topology_changed_event.hpp"
#include "mongocxx\events\topology_closed_event.hpp"
#include "mongocxx\events\topology_description.hpp"
#include "mongocxx\events\topology_opening_event.hpp"
#include "mongocxx\exception\authentication_exception.hpp"
#include "mongocxx\exception\bulk_write_exception.hpp"
#include "mongocxx\exception\error_code.hpp"
#include "mongocxx\exception\exception.hpp"
#include "mongocxx\exception\gridfs_exception.hpp"
#include "mongocxx\exception\logic_error.hpp"
#include "mongocxx\exception\operation_exception.hpp"
#include "mongocxx\exception\query_exception.hpp"
#include "mongocxx\exception\server_error_code.hpp"
#include "mongocxx\exception\write_exception.hpp"
#include "mongocxx\gridfs\bucket.hpp"
#include "mongocxx\gridfs\downloader.hpp"
#include "mongocxx\gridfs\uploader.hpp"
#include "mongocxx\model\delete_many.hpp"
#include "mongocxx\model\delete_one.hpp"
#include "mongocxx\model\insert_one.hpp"
#include "mongocxx\model\replace_one.hpp"
#include "mongocxx\model\update_many.hpp"
#include "mongocxx\model\update_one.hpp"
#include "mongocxx\model\write.hpp"
#include "mongocxx\options\aggregate.hpp"
#include "mongocxx\options\apm.hpp"
#include "mongocxx\options\bulk_write.hpp"
#include "mongocxx\options\change_stream.hpp"
#include "mongocxx\options\client.hpp"
#include "mongocxx\options\client_session.hpp"
#include "mongocxx\options\count.hpp"
#include "mongocxx\options\create_collection.hpp"
#include "mongocxx\options\create_view.hpp"
#include "mongocxx\options\delete.hpp"
#include "mongocxx\options\distinct.hpp"
#include "mongocxx\options\estimated_document_count.hpp"
#include "mongocxx\options\find.hpp"
#include "mongocxx\options\find_one_and_delete.hpp"
#include "mongocxx\options\find_one_and_replace.hpp"
#include "mongocxx\options\find_one_and_update.hpp"
#include "mongocxx\options\find_one_common_options.hpp"
#include "mongocxx\options\index.hpp"
#include "mongocxx\options\index_view.hpp"
#include "mongocxx\options\insert.hpp"
#include "mongocxx\options\pool.hpp"
#include "mongocxx\options\replace.hpp"
#include "mongocxx\options\ssl.hpp"
#include "mongocxx\options\transaction.hpp"
#include "mongocxx\options\update.hpp"
#include "mongocxx\options\gridfs\bucket.hpp"
#include "mongocxx\options\gridfs\upload.hpp"
#include "mongocxx\result\bulk_write.hpp"
#include "mongocxx\result\delete.hpp"
#include "mongocxx\result\insert_many.hpp"
#include "mongocxx\result\insert_one.hpp"
#include "mongocxx\result\replace_one.hpp"
#include "mongocxx\result\update.hpp"
#include "mongocxx\result\gridfs\upload.hpp"

namespace DB
{
}
/*****************************************************************************/
/* Exported defines */


/*****************************************************************************/
/* Exported macro */


/*****************************************************************************/
/* Exported types */


/*****************************************************************************/
/* Exported functions */


/* Have a wonderful day :) */
#endif /* _Mongo */
/**
 * @}
 */
/****** END OF FILE ******/
