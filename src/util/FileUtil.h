#ifndef YUKINO_FILEUTIL_H_
#define YUKINO_FILEUTIL_H_

#include <cstddef>
#include <string>

namespace Yukino
{
    /**
     * @class FileUtil
     * @brief 提供文件相关的工具函数
     *
     * 该类包含静态方法，用于获取文件大小和检查文件是否存在。
     */
    class FileUtil
    {
    public:
        /**
         * @brief 获取文件的大小
         *
         * 通过路径获取文件的大小，并将结果存储在指定的指针中。
         * 如果文件不存在或无法访问，返回非零值。
         *
         * @param path 文件路径
         * @param size 指向存储文件大小的指针
         * @return 错误码
         */
        static int size(const std::string &path, size_t *size);

        /**
         * @brief 检查文件是否存在
         *
         * 检查指定路径的文件是否存在。
         *
         * @param path 文件路径
         * @return 如果文件存在，返回 true；否则返回 false
         */
        static bool file_exists(const std::string &path);
    };

} // namespace Yukino

#endif // YUKINO_FILEUTIL_H_