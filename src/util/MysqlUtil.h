#ifndef YUKINO_MYSQLUTIL_H_
#define YUKINO_MYSQLUTIL_H_

#include <string>
#include <vector>
#include "workflow/MySQLResult.h"

namespace Yukino
{
    /**
     * @class MySQLUtil
     * @brief 提供 MySQL 数据库相关工具函数
     *
     * 该类提供了一系列静态方法，用于从 MySQL 查询结果中提取字段信息、数据类型，以及将单元格数据转换为字符串。
     */
    class MySQLUtil
    {
    public:
        /**
         * @brief 获取查询结果的字段名列表
         *
         * 从 MySQL 查询结果游标中提取所有字段的名称。
         *
         * @param cursor MySQL 查询结果游标
         * @return 返回字段名列表
         */
        static std::vector<std::string> fields(const protocol::MySQLResultCursor &cursor);

        /**
         * @brief 获取查询结果的数据类型列表
         *
         * 从 MySQL 查询结果游标中提取每个字段的数据类型。
         *
         * @param cursor MySQL 查询结果游标
         * @return 返回数据类型列表
         */
        static std::vector<std::string> data_type(const protocol::MySQLResultCursor &cursor);

        /**
         * @brief 将 MySQL 单元格数据转换为字符串
         *
         * 将 MySQL 单元格中的数据转换为字符串形式。
         * 支持多种数据类型，包括整数、浮点数、字符串等。
         *
         * @param cell MySQL 单元格数据
         * @return 返回单元格数据的字符串表示
         */
        static std::string to_string(const protocol::MySQLCell &cell);
    };

} // namespace Yukino

#endif // YUKINO_MYSQLUTIL_H_