#ifndef YUKINO_URIUTIL_H_
#define YUKINO_URIUTIL_H_

#include "workflow/URIParser.h"
#include <unordered_map>
#include <map>
#include <string>

namespace Yukino
{
    class StringPiece;

    /**
     * @class UriUtil
     * @brief 提供 URI（统一资源标识符）相关的工具函数
     *
     * 该类继承自 URIParser，并扩展了 URI 查询字符串解析功能。
     */
    class UriUtil : public URIParser
    {
    public:
        /**
         * @brief 解析查询字符串为键值对
         *
         * 将查询字符串（如 "key1=value1&key2=value2"）解析为键值对的映射表。
         * 支持 URL 编码的键和值，并自动解码。
         *
         * @param query 查询字符串
         * @return 返回解析后的键值对映射表
         */
        static std::map<std::string, std::string>
        split_query(const StringPiece &query);
    };

}  // namespace Yukino

#endif // YUKINO_URIUTIL_H_