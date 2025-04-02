#ifndef YUKINO_PATHUTIL_H_
#define YUKINO_PATHUTIL_H_

#include <string>

namespace Yukino
{
    /**
     * @class PathUtil
     * @brief 提供路径相关的工具函数
     *
     * 该类包含静态方法，用于处理文件路径和目录路径的常见操作，例如判断路径类型、拼接路径、获取文件名和扩展名等。
     */
    class PathUtil
    {
    public:
        /**
         * @brief 判断给定路径是否为目录
         *
         * @param path 要检查的路径
         * @return 如果路径是目录，返回 true；否则返回 false
         */
        static bool is_dir(const std::string &path);

        /**
         * @brief 判断给定路径是否为文件
         *
         * @param path 要检查的路径
         * @return 如果路径是文件，返回 true；否则返回 false
         */
        static bool is_file(const std::string &path);

    public:
        /**
         * @brief 拼接两个路径
         *
         * 将两个路径字符串拼接在一起，确保路径分隔符的正确性。
         * 例如，`concat_path("/usr/local", "image")` 返回 `"/usr/local/image"`。
         *
         * @param lhs 左侧路径
         * @param rhs 右侧路径
         * @return 返回拼接后的完整路径
         */
        static std::string concat_path(const std::string &lhs, const std::string &rhs);

        /**
         * @brief 获取路径中的文件名部分
         *
         * 提取路径中的最后一部分作为文件名。
         * 例如，`base("/usr/local/image/test.jpg")` 返回 `"test.jpg"`。
         *
         * @param filepath 完整路径
         * @return 返回路径中的文件名部分
         */
        static std::string base(const std::string &filepath);

        /**
         * @brief 获取路径中的文件扩展名
         *
         * 提取路径中的文件扩展名部分。
         * 例如，`suffix("/usr/local/image/test.jpg")` 返回 `"jpg"`。
         *
         * @param filepath 完整路径
         * @return 返回路径中的文件扩展名部分
         */
        static std::string suffix(const std::string& filepath);
    };

}  // namespace Yukino


#endif // YUKINO_PATHUTIL_H_