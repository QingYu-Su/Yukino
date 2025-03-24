#include "workflow/StringUtil.h"  // 包含字符串工具函数
#include "workflow/WFFacilities.h"  // 包含工作流相关设施
#include <cstring>  // 包含标准C字符串处理函数
#include <cstdlib>  // 包含标准C库函数
#include <unistd.h>  // 包含POSIX操作系统API
#include <fcntl.h>  // 包含文件控制选项
#include "StrUtil.h"  // 包含自定义字符串工具函数
#include "HttpContent.h"  // 包含HTTP内容处理相关类
#include "StringPiece.h"  // 包含字符串片段处理类
#include "PathUtil.h"  // 包含路径工具函数
#include "HttpDef.h"  // 包含HTTP相关定义

using namespace Yukino;  // 使用Yukino命名空间

// 定义MultiPartForm类的默认边界字符串
const std::string MultiPartForm::k_default_boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";

// 定义Urlencode类的静态方法，用于解析URL编码的POST请求体为键值对
std::map<std::string, std::string> Urlencode::parse_post_kv(const StringPiece &body)
{
    std::map<std::string, std::string> map;  // 定义一个map用于存储解析后的键值对

    if (body.empty())  // 如果请求体为空，直接返回空map
        return map;

    // 使用StrUtil::split_piece函数按'&'分割请求体，得到多个键值对
    std::vector<StringPiece> arr = StrUtil::split_piece<StringPiece>(body, '&');

    if (arr.empty())  // 如果分割结果为空，直接返回空map
        return map;

    // 遍历每个键值对
    for (const auto &ele : arr)
    {
        if (ele.empty())  // 如果当前键值对为空，跳过
            continue;

        // 使用StrUtil::split_piece函数按'='分割键值对，得到键和值
        std::vector<std::string> kv = StrUtil::split_piece<std::string>(ele, '=');
        size_t kv_size = kv.size();  // 获取键值对的大小
        std::string &key = kv[0];  // 获取键

        // 如果键为空或键已存在于map中，跳过
        if (key.empty() || map.count(key) > 0)
            continue;

        // 如果键值对中只有键没有值，将键值对插入map，值为空字符串
        if (kv_size == 1)
        {
            map.emplace(std::move(key), "");
            continue;
        }

        std::string &val = kv[1];  // 获取值

        // 如果值为空，将键值对插入map，值为空字符串
        if (val.empty())
            map.emplace(std::move(key), "");
        else
            map.emplace(std::move(key), std::move(val));  // 将键值对插入map
    }
    return map;  // 返回解析后的键值对map
}

// 定义多部分表单解析器的状态枚举
enum multipart_parser_state_e
{
    MP_START,  // 开始状态
    MP_PART_DATA_BEGIN,  // 部分数据开始状态
    MP_HEADER_FIELD,  // 头部字段状态
    MP_HEADER_VALUE,  // 头部值状态
    MP_HEADERS_COMPLETE,  // 头部完成状态
    MP_PART_DATA,  // 部分数据状态
    MP_PART_DATA_END,  // 部分数据结束状态
    MP_BODY_END  // 请求体结束状态
};

struct multipart_parser_userdata
{
    Form *mp;  // 指向表单数据的指针
    multipart_parser_state_e state;  // 当前解析器的状态
    std::string header_field;  // 当前处理的头部字段
    std::string header_value;  // 当前处理的头部值
    std::string part_data;  // 当前部分的数据
    std::string name;  // 当前字段的名称
    std::string filename;  // 当前字段的文件名

    void handle_header();  // 处理头部字段和值
    void handle_data();  // 处理部分数据
};

void multipart_parser_userdata::handle_header()
{
    if (header_field.empty() || header_value.empty()) return;  // 如果头部字段或值为空，直接返回

    if (strcasecmp(header_field.c_str(), "Content-Disposition") == 0)
    {
        // 处理 Content-Disposition 头部
        // Content-Disposition: attachment
        // Content-Disposition: attachment; filename="filename.jpg"
        // Content-Disposition: form-data; name="avatar"; filename="user.jpg"
        StringPiece header_val_piece(header_value);  // 将头部值转换为 StringPiece
        std::vector<StringPiece> dispo_list = StrUtil::split_piece<StringPiece>(header_val_piece, ';');  // 按分号分割头部值

        for (auto &dispo : dispo_list)
        {
            auto kv = StrUtil::split_piece<StringPiece>(StrUtil::trim(dispo), '=');  // 按等号分割键值对
            if (kv.size() == 2)
            {
                // name="file"
                // kv[0] 是键 (name)
                // kv[1] 是值 ("file")
                StringPiece value = StrUtil::trim_pairs(kv[1], R"(""'')");  // 去掉值的引号
                if (kv[0].starts_with(StringPiece("name")))
                {
                    name = value.as_string();  // 设置字段名称
                }
                else if (kv[0].starts_with(StringPiece("filename")))
                {
                    filename = value.as_string();  // 设置文件名
                }
            }
        }
    }
    header_field.clear();  // 清空头部字段
    header_value.clear();  // 清空头部值
}

void multipart_parser_userdata::handle_data()
{
    if (!name.empty())
    {
        std::pair<std::string, std::string> formdata;  // 创建表单数据对
        formdata.first = filename;  // 设置文件名
        formdata.second = part_data;  // 设置数据
        (*mp)[name] = formdata;  // 将表单数据添加到表单中
    }
    name.clear();  // 清空字段名称
    filename.clear();  // 清空文件名
    part_data.clear();  // 清空部分数据
}

MultiPartForm::MultiPartForm()
{
    settings_ = {  // 初始化解析器设置
            .on_header_field = header_field_cb,  // 头部字段回调
            .on_header_value = header_value_cb,  // 头部值回调
            .on_part_data = part_data_cb,  // 部分数据回调
            .on_part_data_begin = part_data_begin_cb,  // 部分数据开始回调
            .on_headers_complete = headers_complete_cb,  // 头部完成回调
            .on_part_data_end = part_data_end_cb,  // 部分数据结束回调
            .on_body_end = body_end_cb  // 请求体结束回调
    };
}

int MultiPartForm::header_field_cb(multipart_parser *parser, const char *buf, size_t len)
{
    // 从解析器中获取用户数据
    auto *userdata = static_cast<multipart_parser_userdata *>(multipart_parser_get_data(parser));
    
    // 处理当前头部字段
    userdata->handle_header();
    
    // 更新状态为头部字段状态
    userdata->state = MP_HEADER_FIELD;
    
    // 将当前头部字段数据追加到 header_field
    userdata->header_field.append(buf, len);
    
    // 返回 0 表示成功
    return 0;
}

int MultiPartForm::header_value_cb(multipart_parser *parser, const char *buf, size_t len)
{
    // 从解析器中获取用户数据
    auto *userdata = static_cast<multipart_parser_userdata *>(multipart_parser_get_data(parser));
    
    // 更新状态为头部值状态
    userdata->state = MP_HEADER_VALUE;
    
    // 将当前头部值数据追加到 header_value
    userdata->header_value.append(buf, len);
    
    // 返回 0 表示成功
    return 0;
}

int MultiPartForm::part_data_cb(multipart_parser *parser, const char *buf, size_t len)
{
    // 从解析器中获取用户数据
    auto *userdata = static_cast<multipart_parser_userdata *>(multipart_parser_get_data(parser));
    
    // 更新状态为部分数据状态
    userdata->state = MP_PART_DATA;
    
    // 将当前部分数据追加到 part_data
    userdata->part_data.append(buf, len);
    
    // 返回 0 表示成功
    return 0;
}

int MultiPartForm::part_data_begin_cb(multipart_parser *parser)
{
    // 从解析器中获取用户数据
    auto *userdata = static_cast<multipart_parser_userdata *>(multipart_parser_get_data(parser));
    
    // 更新状态为部分数据开始状态
    userdata->state = MP_PART_DATA_BEGIN;
    
    // 返回 0 表示成功
    return 0;
}

int MultiPartForm::headers_complete_cb(multipart_parser *parser)
{
    // 从解析器中获取用户数据
    auto *userdata = static_cast<multipart_parser_userdata *>(multipart_parser_get_data(parser));
    
    // 处理当前头部字段
    userdata->handle_header();
    
    // 更新状态为头部完成状态
    userdata->state = MP_HEADERS_COMPLETE;
    
    // 返回 0 表示成功
    return 0;
}

// 处理部分数据结束事件的回调函数
int MultiPartForm::part_data_end_cb(multipart_parser *parser)
{
    // 从解析器中获取用户数据
    auto *userdata = static_cast<multipart_parser_userdata *>(multipart_parser_get_data(parser));
    // 更新状态为部分数据结束状态
    userdata->state = MP_PART_DATA_END;
    // 处理当前部分数据，例如将数据存储到表单中
    userdata->handle_data();
    // 返回 0 表示成功
    return 0;
}

// 处理请求体结束事件的回调函数
int MultiPartForm::body_end_cb(multipart_parser *parser)
{
    // 从解析器中获取用户数据
    auto *userdata = static_cast<multipart_parser_userdata *>(multipart_parser_get_data(parser));
    // 更新状态为请求体结束状态
    userdata->state = MP_BODY_END;
    // 返回 0 表示成功
    return 0;
}

// 解析多部分表单数据的方法
Form MultiPartForm::parse_multipart(const StringPiece &body) const
{
    // 创建一个空的表单对象用于存储解析结果
    Form form;
    // 构造边界字符串，通常以 "--" 开头
    std::string boundary = "--" + boundary_;
    // 初始化多部分解析器，传入边界字符串和解析器设置
    multipart_parser *parser = multipart_parser_init(boundary.c_str(), &settings_);
    // 创建用户数据对象，用于存储解析过程中的状态和数据
    multipart_parser_userdata userdata;
    // 设置用户数据的初始状态为开始状态
    userdata.state = MP_START;
    // 将表单对象的指针存储到用户数据中，以便在回调函数中使用
    userdata.mp = &form;
    // 将用户数据与解析器关联起来
    multipart_parser_set_data(parser, &userdata);
    // 执行解析操作，传入请求体数据和长度
    multipart_parser_execute(parser, body.data(), body.size());
    // 释放解析器资源
    multipart_parser_free(parser);
    // 返回解析后的表单数据
    return form;
}

// MultiPartEncoder 类的构造函数，初始化边界字符串为默认值
MultiPartEncoder::MultiPartEncoder()
    : boundary_(MultiPartForm::k_default_boundary)
{
}

// 添加表单参数的方法
void MultiPartEncoder::add_param(const std::string &name, const std::string &value) 
{
    // 将参数名和值作为一对存储到参数列表中
    params_.push_back({name, value});
}

// 添加文件的方法
void MultiPartEncoder::add_file(const std::string &file_name, const std::string &file_path)
{
    // 将文件名和文件路径作为一对存储到文件列表中
    files_.push_back({file_name, file_path});
}

// 设置边界字符串的方法（普通版本）
void MultiPartEncoder::set_boundary(const std::string &boundary)
{
    // 直接赋值新的边界字符串
    boundary_ = boundary;
}

// 设置边界字符串的方法（移动版本）
void MultiPartEncoder::set_boundary(std::string &&boundary) 
{
    // 使用移动语义赋值新的边界字符串，提高性能
    boundary_ = std::move(boundary);
}