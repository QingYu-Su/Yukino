#include "MysqlUtil.h"

using namespace Yukino;
using namespace protocol;

// 获取查询结果的字段名列表
std::vector<std::string> MySQLUtil::fields(const MySQLResultCursor &cursor)
{
    std::vector<std::string> fields_name; // 用于存储字段名
    // 如果字段数量小于 0，表示查询结果无效，直接返回空列表
    if (cursor.get_field_count() < 0) 
    {
        return fields_name;
    }

    // 预分配字段名列表的大小，提高性能
    fields_name.reserve(cursor.get_field_count());

    // 获取查询结果的所有字段信息
    const MySQLField *const *fields = cursor.fetch_fields();

    // 遍历每个字段，提取字段名
    for (int i = 0; i < cursor.get_field_count(); i++) {
        fields_name.emplace_back(fields[i]->get_name()); // 将字段名添加到列表中
    }

    return fields_name; // 返回字段名列表
}

// 获取查询结果的数据类型列表
std::vector<std::string> MySQLUtil::data_type(const protocol::MySQLResultCursor &cursor)
{
    std::vector<std::string> data_type; // 用于存储数据类型
    // 如果字段数量小于 0，表示查询结果无效，直接返回空列表
    if (cursor.get_field_count() < 0) 
    {
        return data_type;
    }

    // 预分配数据类型列表的大小，提高性能
    data_type.reserve(cursor.get_field_count());

    // 获取查询结果的所有字段信息
    const MySQLField *const *fields = cursor.fetch_fields();

    // 遍历每个字段，提取数据类型
    for (int i = 0; i < cursor.get_field_count(); i++)
    {
        // 使用 datatype2str 函数将数据类型转换为字符串
        data_type.emplace_back(datatype2str(fields[i]->get_data_type()));
    }

    return data_type; // 返回数据类型列表
}

// 将 MySQL 单元格数据转换为字符串
std::string MySQLUtil::to_string(const MySQLCell &cell)
{
    // 如果单元格数据为空，返回空字符串
    if(cell.is_null())
    {
        return "";
    }
    // 如果单元格数据是整数类型，转换为字符串
    else if(cell.is_int())
    {
        return std::to_string(cell.as_int());
    } 
    // 如果单元格数据是字符串类型，直接返回字符串
    else if(cell.is_string())
    {
        return cell.as_string();
    }
    // 如果单元格数据是浮点数类型，转换为字符串
    else if(cell.is_float())
    {
        return std::to_string(cell.as_float());
    } 
    // 如果单元格数据是双精度浮点数类型，转换为字符串
    else if(cell.is_double())
    {
        return std::to_string(cell.as_double());
    } 
    // 如果单元格数据是无符号长整数类型，转换为字符串
    else if(cell.is_ulonglong())
    {
        return std::to_string(cell.as_ulonglong());
    }
    // 如果单元格数据是日期类型，返回日期字符串
    else if(cell.is_date())
    {
        return cell.as_date();
    } 
    // 如果单元格数据是时间类型，返回时间字符串
    else if(cell.is_time())
    {
        return cell.as_time();
    } 
    // 如果单元格数据是日期时间类型，返回日期时间字符串
    else if(cell.is_datetime())
    {
        return cell.as_datetime();
    }
    // 如果单元格数据类型未知，返回空字符串
    return "";
}