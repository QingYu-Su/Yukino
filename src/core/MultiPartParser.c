#include "MultiPartParser.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/**
 * @brief 日志函数，用于调试多部分解析器。
 *
 * @param format 格式化字符串。
 * @param ... 可变参数列表。
 *
 * @note 该函数仅在定义了 DEBUG_MULTIPART 宏时才会输出日志。
 */
static void multipart_log(const char *format, ...)
{
#ifdef DEBUG_MULTIPART
    va_list args;
    va_start(args, format);

    // 输出日志到标准错误输出
    fprintf(stderr, "[HTTP_MULTIPART_PARSER] %s:%d: ", __FILE__, __LINE__);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
#endif
}

/**
 * @brief 宏定义，用于调用通知回调函数。
 *
 * @param FOR 回调函数的名称（例如 on_part_data_begin）。
 *
 * @note 如果回调函数返回非零值，则停止解析并返回当前索引。
 */
#define NOTIFY_CB(FOR)                                                 \
do {                                                                   \
  if (p->settings->on_##FOR) {                                         \
    if (p->settings->on_##FOR(p) != 0) {                               \
      return i;                                                        \
    }                                                                  \
  }                                                                    \
} while (0)

/**
 * @brief 宏定义，用于调用数据回调函数。
 *
 * @param FOR 回调函数的名称（例如 on_header_field）。
 * @param ptr 数据指针。
 * @param len 数据长度。
 *
 * @note 如果回调函数返回非零值，则停止解析并返回当前索引。
 */
#define EMIT_DATA_CB(FOR, ptr, len)                                    \
do {                                                                   \
  if (p->settings->on_##FOR) {                                         \
    if (p->settings->on_##FOR(p, ptr, len) != 0) {                     \
      return i;                                                        \
    }                                                                  \
  }                                                                    \
} while (0)

#define LF 10  // 定义换行符（LF）的 ASCII 值
#define CR 13  // 定义回车符（CR）的 ASCII 值

/**
 * @brief 定义了多部分解析器的结构体。
 */
struct multipart_parser
{
    void *data;  // 用户自定义数据，用于存储解析过程中的上下文信息

    size_t index;  // 当前解析的索引位置
    size_t boundary_length;  // 边界字符串的长度

    unsigned char state;  // 当前解析器的状态

    const multipart_parser_settings *settings;  // 指向解析器设置的指针

    char *lookbehind;  // 用于存储解析过程中需要回溯的数据
    char multipart_boundary[1];  // 边界字符串（实际大小由用户设置）
};

/**
 * @brief 定义了多部分解析器的状态枚举。
 */
enum state
{
    s_uninitialized = 1,  // 未初始化状态
    s_start,              // 开始状态
    s_start_boundary,     // 开始边界状态
    s_header_field_start, // 头部字段开始状态
    s_header_field,       // 头部字段状态
    s_headers_almost_done,// 头部即将结束状态
    s_header_value_start, // 头部值开始状态
    s_header_value,       // 头部值状态
    s_header_value_almost_done, // 头部值即将结束状态
    s_part_data_start,    // 部分数据开始状态
    s_part_data,          // 部分数据状态
    s_part_data_almost_boundary, // 部分数据接近边界状态
    s_part_data_boundary, // 部分数据边界状态
    s_part_data_almost_end, // 部分数据即将结束状态
    s_part_data_end,      // 部分数据结束状态
    s_part_data_final_hyphen, // 部分数据最终短横线状态
    s_end                // 结束状态
};

/**
 * @brief 初始化一个多部分解析器。
 *
 * @param boundary 边界字符串（boundary），用于分隔多部分数据。
 * @param settings 解析器的设置，包含回调函数。
 * @return 返回初始化后的解析器指针。
 */
multipart_parser *multipart_parser_init(const char *boundary, const multipart_parser_settings *settings)
{
    // 分配内存空间，包括解析器结构体、边界字符串和额外的缓冲区
    multipart_parser *p = (multipart_parser *)malloc(sizeof(multipart_parser) +
                                                     strlen(boundary) +
                                                     strlen(boundary) + 9);

    // 复制边界字符串到解析器结构体中
    strcpy(p->multipart_boundary, boundary);
    p->boundary_length = strlen(boundary);  // 设置边界字符串的长度

    // 设置 lookbehind 缓冲区的位置
    p->lookbehind = (p->multipart_boundary + p->boundary_length + 1);

    // 初始化解析器的状态
    p->index = 0;  // 当前解析的索引位置
    p->state = s_start;  // 初始状态为 s_start
    p->settings = settings;  // 设置回调函数

    return p;  // 返回初始化后的解析器指针
}

/**
 * @brief 释放一个多部分解析器的资源。
 *
 * @param p 要释放的解析器指针。
 */
void multipart_parser_free(multipart_parser *p)
{
    free(p);  // 释放解析器占用的内存
}

/**
 * @brief 设置解析器的用户数据。
 *
 * @param p 解析器指针。
 * @param data 要设置的用户数据。
 */
void multipart_parser_set_data(multipart_parser *p, void *data)
{
    p->data = data;  // 将用户数据存储到解析器结构体中
}

/**
 * @brief 获取解析器的用户数据。
 *
 * @param p 解析器指针。
 * @return 返回用户数据指针。
 */
void *multipart_parser_get_data(multipart_parser *p)
{
    return p->data;  // 返回存储在解析器结构体中的用户数据
}

/**
 * @brief 执行多部分解析器，解析输入缓冲区中的数据。
 *
 * @param p 指向多部分解析器的指针。
 * @param buf 输入缓冲区，包含要解析的数据。
 * @param len 输入缓冲区的长度。
 * @return 返回已处理的字节数。
 */
size_t multipart_parser_execute(multipart_parser *p, const char *buf, size_t len)
{
    size_t i = 0; // 当前处理的字符索引
    size_t mark = 0; // 标记位置，用于记录数据的起始位置
    char c, cl; // 当前处理的字符和小写字符
    int is_last = 0; // 是否是最后一个字符

    while (i < len)
    {
        c = buf[i]; // 获取当前字符
        is_last = (i == (len - 1)); // 判断是否是最后一个字符
        switch (p->state) // 根据当前状态进行处理
        {
            case s_start:
                multipart_log("s_start");
                p->index = 0; // 初始化索引
                p->state = s_start_boundary; // 切换到开始边界状态

                /* fallthrough */
            case s_start_boundary:
                multipart_log("s_start_boundary");
                if (p->index == p->boundary_length) // 如果索引等于边界长度
                {
                    if (c != CR) // 如果当前字符不是回车符
                    {
                        return i; // 返回当前索引
                    }
                    p->index++; // 增加索引
                    break;
                } else if (p->index == (p->boundary_length + 1)) // 如果索引等于边界长度加1
                {
                    if (c != LF) // 如果当前字符不是换行符
                    {
                        return i; // 返回当前索引
                    }
                    p->index = 0; // 重置索引
                    NOTIFY_CB(part_data_begin); // 调用回调函数
                    p->state = s_header_field_start; // 切换到头部字段开始状态
                    break;
                }
                if (c != p->multipart_boundary[p->index]) // 如果当前字符不等于边界字符
                {
                    return i; // 返回当前索引
                }
                p->index++; // 增加索引
                break;

            case s_header_field_start:
                multipart_log("s_header_field_start");
                mark = i; // 标记当前位置
                p->state = s_header_field; // 切换到头部字段状态

                /* fallthrough */
            case s_header_field:
                multipart_log("s_header_field");
                if (c == CR) // 如果当前字符是回车符
                {
                    p->state = s_headers_almost_done; // 切换到头部即将结束状态
                    break;
                }

                if (c == ':') // 如果当前字符是冒号
                {
                    EMIT_DATA_CB(header_field, buf + mark, i - mark); // 调用回调函数
                    p->state = s_header_value_start; // 切换到头部值开始状态
                    break;
                }

                cl = tolower(c); // 将当前字符转换为小写
                if ((c != '-') && (cl < 'a' || cl > 'z')) // 如果当前字符不是连字符且不是字母
                {
                    multipart_log("invalid character in header name");
                    return i; // 返回当前索引
                }
                if (is_last) // 如果是最后一个字符
                    EMIT_DATA_CB(header_field, buf + mark, (i - mark) + 1); // 调用回调函数
                break;

            case s_headers_almost_done:
                multipart_log("s_headers_almost_done");
                if (c != LF) // 如果当前字符不是换行符
                {
                    return i; // 返回当前索引
                }

                p->state = s_part_data_start; // 切换到部分数据开始状态
                break;

            case s_header_value_start:
                multipart_log("s_header_value_start");
                if (c == ' ') // 如果当前字符是空格
                {
                    break;
                }

                mark = i; // 标记当前位置
                p->state = s_header_value; // 切换到头部值状态

                /* fallthrough */
            case s_header_value:
                multipart_log("s_header_value");
                if (c == CR) // 如果当前字符是回车符
                {
                    EMIT_DATA_CB(header_value, buf + mark, i - mark); // 调用回调函数
                    p->state = s_header_value_almost_done; // 切换到头部值即将结束状态
                    break;
                }
                if (is_last) // 如果是最后一个字符
                    EMIT_DATA_CB(header_value, buf + mark, (i - mark) + 1); // 调用回调函数
                break;

            case s_header_value_almost_done:
                multipart_log("s_header_value_almost_done");
                if (c != LF) // 如果当前字符不是换行符
                {
                    return i; // 返回当前索引
                }
                p->state = s_header_field_start; // 切换到头部字段开始状态
                break;

            case s_part_data_start:
                multipart_log("s_part_data_start");
                NOTIFY_CB(headers_complete); // 调用回调函数
                mark = i; // 标记当前位置
                p->state = s_part_data; // 切换到部分数据状态

                /* fallthrough */
            case s_part_data:
                multipart_log("s_part_data");
                if (c == CR) // 如果当前字符是回车符
                {
                    EMIT_DATA_CB(part_data, buf + mark, i - mark); // 调用回调函数
                    mark = i; // 标记当前位置
                    p->state = s_part_data_almost_boundary; // 切换到部分数据接近边界状态
                    p->lookbehind[0] = CR; // 设置回溯字符
                    break;
                }
                if (is_last) // 如果是最后一个字符
                    EMIT_DATA_CB(part_data, buf + mark, (i - mark) + 1); // 调用回调函数
                break;

            case s_part_data_almost_boundary:
                multipart_log("s_part_data_almost_boundary");
                if (c == LF) // 如果当前字符是换行符
                {
                    p->state = s_part_data_boundary; // 切换到部分数据边界状态
                    p->lookbehind[1] = LF; // 设置回溯字符
                    p->index = 0; // 重置索引
                    break;
                }
                EMIT_DATA_CB(part_data, p->lookbehind, 1); // 调用回调函数
                p->state = s_part_data; // 切换到部分数据状态
                mark = i--; // 标记当前位置并减少索引
                break;

            case s_part_data_boundary:
                multipart_log("s_part_data_boundary");
                if (p->multipart_boundary[p->index] != c) // 如果当前字符不等于边界字符
                {
                    EMIT_DATA_CB(part_data, p->lookbehind, 2 + p->index); // 调用回调函数
                    p->state = s_part_data; // 切换到部分数据状态
                    mark = i--; // 标记当前位置并减少索引
                    break;
                }
                p->lookbehind[2 + p->index] = c; // 设置回溯字符
                if ((++p->index) == p->boundary_length) // 如果索引等于边界长度
                {
                    NOTIFY_CB(part_data_end); // 调用回调函数
                    p->state = s_part_data_almost_end; // 切换到部分数据即将结束状态
                }
                break;

            case s_part_data_almost_end:
                multipart_log("s_part_data_almost_end");
                if (c == '-') // 如果当前字符是短横线
                {
                    p->state = s_part_data_final_hyphen; // 切换到部分数据最终短横线状态
                    break;
                }
                if (c == CR) // 如果当前字符是回车符
                {
                    p->state = s_part_data_end; // 切换到部分数据结束状态
                    break;
                }
                return i; // 返回当前索引

            case s_part_data_final_hyphen:
                multipart_log("s_part_data_final_hyphen");
                if (c == '-') // 如果当前字符是短横线
                {
                    NOTIFY_CB(body_end); // 调用回调函数
                    p->state = s_end; // 切换到结束状态
                    break;
                }
                return i; // 返回当前索引

            case s_part_data_end:
                multipart_log("s_part_data_end");
                if (c == LF) // 如果当前字符是换行符
                {
                    p->state = s_header_field_start; // 切换到头部字段开始状态
                    NOTIFY_CB(part_data_begin); // 调用回调函数
                    break;
                }
                return i; // 返回当前索引

            case s_end:
                multipart_log("s_end: %02X", (int) c); // 记录结束状态
                break;

            default:
                multipart_log("Multipart parser unrecoverable error"); // 记录不可恢复的错误
                return 0; // 返回0
        }
        ++i; // 增加索引
    }

    return len; // 返回处理的长度
}
