 #ifndef YUKINO_MULTIPARTPARSER_H_
 #define YUKINO_MULTIPARTPARSER_H_
 
 #ifdef __cplusplus
 extern "C"
 {
 #endif
 
 #include <stdlib.h>
 #include <ctype.h>
 
 // 定义一个多部分解析器的结构体
 typedef struct multipart_parser multipart_parser;
 
 // 定义一个多部分解析器设置的结构体
 typedef struct multipart_parser_settings multipart_parser_settings;
 
 // 定义一个多部分解析器状态的结构体
 typedef struct multipart_parser_state multipart_parser_state;
 
 // 定义一个多部分数据回调函数类型
 typedef int (*multipart_data_cb)(multipart_parser *, const char *at, size_t length);
 
 // 定义一个多部分通知回调函数类型
 typedef int (*multipart_notify_cb)(multipart_parser *);
 
 /**
  * @brief 定义了多部分解析器的设置结构体。
  *
  * 这个结构体包含了一系列回调函数，用于在解析过程中处理不同的事件。
  */
 struct multipart_parser_settings
 {
     multipart_data_cb on_header_field;  // 当解析到头部字段时调用的回调函数
     multipart_data_cb on_header_value;  // 当解析到头部值时调用的回调函数
     multipart_data_cb on_part_data;     // 当解析到部分数据时调用的回调函数
 
     multipart_notify_cb on_part_data_begin;  // 当部分数据开始时调用的回调函数
     multipart_notify_cb on_headers_complete; // 当头部解析完成时调用的回调函数
     multipart_notify_cb on_part_data_end;    // 当部分数据结束时调用的回调函数
     multipart_notify_cb on_body_end;         // 当整个请求体解析完成时调用的回调函数
 };
 
 /**
  * @brief 初始化一个多部分解析器。
  *
  * @param boundary 多部分数据的分隔符（boundary）。
  * @param settings 解析器的设置，包含回调函数。
  * @return 返回初始化后的解析器指针。
  */
 multipart_parser *multipart_parser_init(const char *boundary, const multipart_parser_settings *settings);
 
 /**
  * @brief 释放一个多部分解析器的资源。
  *
  * @param p 要释放的解析器指针。
  */
 void multipart_parser_free(multipart_parser *p);
 
 /**
  * @brief 执行解析操作。
  *
  * @param p 解析器指针。
  * @param buf 要解析的数据缓冲区。
  * @param len 缓冲区的长度。
  * @return 返回解析的数据长度。
  */
 size_t multipart_parser_execute(multipart_parser *p, const char *buf, size_t len);
 
 /**
  * @brief 设置解析器的用户数据。
  *
  * @param p 解析器指针。
  * @param data 要设置的用户数据。
  */
 void multipart_parser_set_data(multipart_parser *p, void *data);
 
 /**
  * @brief 获取解析器的用户数据。
  *
  * @param p 解析器指针。
  * @return 返回用户数据指针。
  */
 void *multipart_parser_get_data(multipart_parser *p);
 
 #ifdef __cplusplus
 } /* extern "C" */
 #endif
 
 #endif // YUKINO_MULTIPARTPARSER_H_