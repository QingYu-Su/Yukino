#ifndef YUKINO_JSON_H_
#define YUKINO_JSON_H_

#include "workflow/json_parser.h"  // 包含workflow的json解析器头文件
#include <algorithm>              // 包含标准库中的algorithm头文件，用于算法操作
#include <cassert>                // 包含标准库中的cassert头文件，用于断言
#include <cstdio>                 // 包含标准库中的cstdio头文件，用于C风格的输入输出
#include <fstream>                // 包含标准库中的fstream头文件，用于文件流操作
#include <functional>             // 包含标准库中的functional头文件，用于函数对象
#include <iostream>               // 包含标准库中的iostream头文件，用于输入输出流
#include <map>                    // 包含标准库中的map头文件，用于映射表
#include <sstream>                // 包含标准库中的sstream头文件，用于字符串流操作
#include <string>                 // 包含标准库中的string头文件，用于字符串操作
#include <vector>                 // 包含标准库中的vector头文件，用于动态数组
#include <initializer_list>       // 包含标准库中的initializer_list头文件，用于初始化列表

namespace Yukino
{

namespace detail
{

// 模板结构体，用于判断类型是否为字符串类型
// 默认情况下，value为false，表示该类型不是字符串类型
template <typename T>
struct is_string
{
    static constexpr bool value = false;
};

// 特化模板结构体，用于判断std::basic_string是否为字符串类型
// 如果T是std::basic_string类型，则value为true，表示它是字符串类型
template <class T, class Traits, class Alloc>
struct is_string<std::basic_string<T, Traits, Alloc>>
{
    static constexpr bool value = true;
};

// 模板结构体，用于判断类型是否为字符类型
// 继承自std::integral_constant，value为true表示是字符类型，否则为false
// 检查类型是否为char、char16_t、char32_t或wchar_t
template <typename C>
struct is_char
    : std::integral_constant<bool, std::is_same<C, char>::value ||
                                       std::is_same<C, char16_t>::value ||
                                       std::is_same<C, char32_t>::value ||
                                       std::is_same<C, wchar_t>::value>
{
};

// 模板结构体，用于判断类型是否为数值类型
// 继承自std::integral_constant，value为true表示是数值类型，否则为false
// 检查类型是否为算术类型（整数或浮点数），并且不是bool类型，也不是字符类型
template <typename T>
struct is_number
    : std::integral_constant<bool, std::is_arithmetic<T>::value &&
                                       !std::is_same<T, bool>::value &&
                                       !detail::is_char<T>::value>
{
};

} // namespace detail

class Object_S;  // 声明Object_S类，表示JSON对象
class Array_S;   // 声明Array_S类，表示JSON数组

class Json
{
public:
    // 使用别名，方便在Json类中使用Object和Array
    using Object = Object_S;
    using Array = Array_S;

public:
    // 静态函数，用于解析JSON字符串
    static Json parse(const std::string& str);

    // 静态函数，用于解析文件流中的JSON数据
    static Json parse(const std::ifstream& stream);

    // 静态函数，用于解析FILE指针中的JSON数据
    static Json parse(FILE* fp);

    // 将Json对象序列化为字符串
    std::string dump() const;

    // 将Json对象序列化为字符串，并指定缩进空格数
    std::string dump(int spaces) const;

    // 提供对JSON对象或数组的访问，返回对应的Json对象
    // 用于非const对象，允许修改
    Json operator[](const char* key);

    // 提供对JSON对象或数组的访问，返回对应的Json对象
    // 用于const对象，不允许修改
    Json operator[](const char* key) const;

    // 提供对JSON对象或数组的访问，返回对应的Json对象
    // 用于非const对象，允许修改
    Json operator[](const std::string& key);

    // 提供对JSON对象或数组的访问，返回对应的Json对象
    // 用于const对象，不允许修改
    Json operator[](const std::string& key) const;

    // 提供对JSON数组的访问，返回对应的Json对象
    // 用于非const对象，允许修改
    Json operator[](int index);

    // 提供对JSON数组的访问，返回对应的Json对象
    // 用于const对象，不允许修改
    Json operator[](int index) const;

    // 模板函数，用于更新JSON数组中的值
    template <typename T>
    void operator=(const T& val)
    {
        if (parent_ == nullptr)
            return;
        if (json_value_type(parent_) == JSON_VALUE_ARRAY)
        {
            update_arr(val);  // 更新数组中的值
        }
        else if (json_value_type(parent_) == JSON_VALUE_OBJECT)
        {
            push_back_obj(parent_key_, val);  // 更新对象中的值
        }
    }

    // 检查JSON对象中是否存在指定的键
    bool has(const std::string& key) const;

    // 模板函数，用于获取JSON值为bool类型的值
    template <typename T>
    typename std::enable_if<std::is_same<T, bool>::value, T>::type get() const
    {
        return json_value_type(node_) == JSON_VALUE_TRUE ? true : false;
    }

    // 模板函数，用于获取JSON值为数值类型的值
    template <typename T>
    typename std::enable_if<detail::is_number<T>::value, T>::type get() const
    {
        return static_cast<T>(json_value_number(node_));
    }

    // 模板函数，用于获取JSON值为字符串类型的值
    template <typename T>
    typename std::enable_if<detail::is_string<T>::value, T>::type get() const
    {
        return std::string(json_value_string(node_));
    }

    // 模板函数，用于获取JSON值为Object类型的值
    template <typename T>
    typename std::enable_if<std::is_same<T, Object>::value, T>::type
    get() const;

    // 模板函数，用于获取JSON值为Array类型的值
    template <typename T>
    typename std::enable_if<std::is_same<T, Array>::value, T>::type get() const;

    // 模板函数，用于获取JSON值为nullptr类型的值
    template <typename T>
    typename std::enable_if<std::is_same<T, std::nullptr_t>::value, T>::type
    get() const
    {
        return nullptr;
    }

    // 模板函数，用于将Json对象隐式转换为指定类型
    template <typename T>
    operator T()
    {
        return get<T>();
    }

public:
    // 获取JSON值的类型
    int type() const
    {
        return json_value_type(node_);  // 调用底层函数获取节点的类型
    }

    // 获取JSON值类型的字符串描述
    std::string type_str() const;

    // 检查JSON值是否为null
    bool is_null() const
    {
        return type() == JSON_VALUE_NULL;  // 检查类型是否为JSON_VALUE_NULL
    }

    // 检查JSON值是否为数值类型
    bool is_number() const
    {
        return type() == JSON_VALUE_NUMBER;  // 检查类型是否为JSON_VALUE_NUMBER
    }

    // 检查JSON值是否为布尔类型
    bool is_boolean() const
    {
        int type = this->type();  // 获取当前节点的类型
        return type == JSON_VALUE_TRUE || type == JSON_VALUE_FALSE;  // 检查类型是否为JSON_VALUE_TRUE或JSON_VALUE_FALSE
    }

    // 检查JSON值是否为对象类型
    bool is_object() const
    {
        return type() == JSON_VALUE_OBJECT;  // 检查类型是否为JSON_VALUE_OBJECT
    }

    // 检查JSON值是否为数组类型
    bool is_array() const
    {
        return type() == JSON_VALUE_ARRAY;  // 检查类型是否为JSON_VALUE_ARRAY
    }

    // 检查JSON值是否为字符串类型
    bool is_string() const
    {
        return type() == JSON_VALUE_STRING;  // 检查类型是否为JSON_VALUE_STRING
    }

    // 检查JSON值是否有效
    bool is_valid() const
    {
        return node_ != nullptr;  // 检查节点指针是否为空
    }

    // 获取JSON数组或对象的大小
    int size() const;

    // 检查JSON数组或对象是否为空
    bool empty() const;

    // 清空JSON数组或对象
    void clear();

    // 创建当前JSON对象的副本
    Json copy() const;

    // 获取当前JSON对象的父级键名
    std::string key() const
    {
        return parent_key_;  // 返回父级键名
    }

    // 获取当前JSON对象的值（返回自身引用）
    const Json& value() const
    {
        return *this;  // 返回当前对象的引用
    }

public:
    // 为对象添加键值对，数值类型
    template <typename T, typename std::enable_if<detail::is_number<T>::value,
                                                  bool>::type = true>
    void push_back(const std::string& key, const T& val)
    {
        if (is_placeholder())  // 如果当前对象是占位符
        {
            *this = Json::Object{{key, val}};  // 初始化为包含一个键值对的对象
            return;
        }
        if (!can_obj_push_back())  // 如果不能向对象中添加键值对
        {
            return;  // 直接返回
        }
        json_object_t* obj = json_value_object(node_);  // 获取当前对象的底层对象指针
        json_object_append(obj, key.c_str(), JSON_VALUE_NUMBER,  // 向对象中添加键值对
                           static_cast<double>(val));  // 将数值类型转换为double
    }

    // 为对象添加键值对，对象或数组类型
    template <typename T,
              typename std::enable_if<std::is_same<T, Object>::value ||
                                          std::is_same<T, Array>::value,
                                      bool>::type = true>
    void push_back(const std::string& key, const T& val)
    {
        if (is_placeholder())  // 如果当前对象是占位符
        {
            *this = Json::Object{{key, val}};  // 初始化为包含一个键值对的对象
            return;
        }
        if (!can_obj_push_back())  // 如果不能向对象中添加键值对
        {
            return;  // 直接返回
        }
        json_object_t* obj = json_value_object(node_);  // 获取当前对象的底层对象指针
        Json copy_json = val.copy();  // 创建值的副本
        json_object_append(obj, key.c_str(), 0, copy_json.node_);  // 向对象中添加键值对
        copy_json.reset();  // 重置副本
    }

    // 为对象添加键值对，布尔类型
    void push_back(const std::string& key, bool val);

    // 为对象添加键值对，nullptr类型
    void push_back(const std::string& key, std::nullptr_t val);

    // 为对象添加键值对，字符串类型
    void push_back(const std::string& key, const std::string& val);

    // 为对象添加键值对，C风格字符串类型
    void push_back(const std::string& key, const char* val);

    // 为对象添加键值对，字符串向量类型
    void push_back(const std::string& key, const std::vector<std::string>& val);

    // 为对象添加键值对，字符串初始化列表类型
    void push_back(const std::string& key, const std::initializer_list<std::string>& val)
    {
        push_back(key, std::vector<std::string>(val));  // 将初始化列表转换为向量并调用对应的push_back方法
    }

    // 为对象添加键值对，Json类型
    void push_back(const std::string& key, const Json& val);

private:
    // 为对象添加键值对
    template <typename T>
    void push_back_obj(const std::string& key, const T& val)
    {
        if (!can_obj_push_back())  // 如果不能向对象中添加键值对
        {
            return;  // 直接返回
        }
        if (is_placeholder())  // 如果当前对象是占位符
        {
            placeholder_push_back(key, val);  // 调用占位符版本的添加函数
        }
        else  // 否则
        {
            normal_push_back(key, val);  // 调用普通版本的添加函数
        }
    }

    // 占位符版本的添加函数，数值类型
    template <typename T, typename std::enable_if<detail::is_number<T>::value,
                                                  bool>::type = true>
    void placeholder_push_back(const std::string& key, const T& val)
    {
        json_object_t* obj = json_value_object(parent_);  // 获取父级对象的底层对象指针
        destroy_node(&node_);  // 销毁当前节点
        node_ = const_cast<json_value_t*>(json_object_append(  // 添加键值对
            obj, key.c_str(), JSON_VALUE_NUMBER, static_cast<double>(val)));
    }

    // 占位符版本的添加函数，对象或数组类型
    template <typename T,
              typename std::enable_if<std::is_same<T, Object>::value ||
                                          std::is_same<T, Array>::value,
                                      bool>::type = true>
    void placeholder_push_back(const std::string& key, const T& val)
    {
        json_object_t* obj = json_value_object(parent_);  // 获取父级对象的底层对象指针
        destroy_node(&node_);  // 销毁当前节点
        Json copy_json = val.copy();  // 创建值的副本
        node_ = const_cast<json_value_t*>(  // 添加键值对
            json_object_append(obj, key.c_str(), 0, copy_json.node_));
        copy_json.reset();  // 重置副本
    }

    // 占位符版本的添加函数，布尔类型
    void placeholder_push_back(const std::string& key, bool val);

    // 占位符版本的添加函数，nullptr类型
    void placeholder_push_back(const std::string& key, std::nullptr_t val);

    // 占位符版本的添加函数，字符串类型
    void placeholder_push_back(const std::string& key, const std::string& val);

    // 占位符版本的添加函数，C风格字符串类型
    void placeholder_push_back(const std::string& key, const char* val);

    // 占位符版本的添加函数，字符串向量类型
    void placeholder_push_back(const std::string& key,
                               const std::vector<std::string>& val);

    // 占位符版本的添加函数，Json类型
    void placeholder_push_back(const std::string& key, const Json& val);

    // 普通版本的添加函数，数值类型
    template <typename T, typename std::enable_if<detail::is_number<T>::value,
                                                  bool>::type = true>
    void normal_push_back(const std::string& key, const T& val)
    {
        json_object_t* obj = json_value_object(parent_);  // 获取父级对象的底层对象指针
        const json_value_t* find = json_object_find(key.c_str(), obj);  // 查找键是否已存在
        if (find == nullptr)  // 如果键不存在
        {
            json_object_append(obj, key.c_str(), JSON_VALUE_NUMBER,  // 添加键值对
                               static_cast<double>(val));
            return;
        }
        json_object_insert_before(find, obj, key.c_str(), JSON_VALUE_NUMBER,  // 在现有键前插入新键值对
                                  static_cast<double>(val));
        json_value_t* remove_val = json_object_remove(find, obj);  // 移除旧的键值对
        json_value_destroy(remove_val);  // 销毁旧的键值对
    }

    // 普通版本的添加函数，对象或数组类型
    template <typename T,
              typename std::enable_if<std::is_same<T, Object>::value ||
                                          std::is_same<T, Array>::value,
                                      bool>::type = true>
    void normal_push_back(const std::string& key, const T& val)
    {
        json_object_t* obj = json_value_object(parent_);  // 获取父级对象的底层对象指针
        const json_value_t* find = json_object_find(key.c_str(), obj);  // 查找键是否已存在
        Json copy_json = val.copy();  // 创建值的副本
        if (find == nullptr)  // 如果键不存在
        {
            json_object_append(obj, key.c_str(), 0, copy_json.node_);  // 添加键值对
            copy_json.reset();  // 重置副本
            return;
        }
        json_object_insert_before(find, obj, key.c_str(), 0, copy_json.node_);  // 在现有键前插入新键值对
        copy_json.reset();  // 重置副本
        json_value_t* remove_val = json_object_remove(find, obj);  // 移除旧的键值对
        json_value_destroy(remove_val);  // 销毁旧的键值对
    }

    // 普通版本的添加函数，布尔类型
    void normal_push_back(const std::string& key, bool val);

    // 普通版本的添加函数，nullptr类型
    void normal_push_back(const std::string& key, std::nullptr_t val);

    // 普通版本的添加函数，字符串类型
    void normal_push_back(const std::string& key, const std::string& val);

    // 普通版本的添加函数，C风格字符串类型
    void normal_push_back(const std::string& key, const char* val);

    // 普通版本的添加函数，字符串向量类型
    void normal_push_back(const std::string& key,
                          const std::vector<std::string>& val);

    // 普通版本的添加函数，Json类型
    void normal_push_back(const std::string& key, const Json& val);

public:
    // 从JSON对象中移除指定的键
    void erase(const std::string& key);

    // 向JSON数组中添加数值类型元素
    template <typename T, typename std::enable_if<detail::is_number<T>::value,
                                                  bool>::type = true>
    void push_back(T val)
    {
        if (is_placeholder())  // 如果当前对象是占位符
        {
            *this = Json::Array{{val}};  // 初始化为包含一个元素的数组
            return;
        }
        if (!can_arr_push_back())  // 如果不能向数组中添加元素
        {
            return;  // 直接返回
        }
        json_array_t* arr = json_value_array(node_);  // 获取当前数组的底层数组指针
        json_array_append(arr, JSON_VALUE_NUMBER, static_cast<double>(val));  // 向数组中添加数值元素
    }

    // 向JSON数组中添加对象或数组类型元素
    template <typename T,
              typename std::enable_if<std::is_same<T, Json::Object>::value ||
                                          std::is_same<T, Json::Array>::value,
                                      bool>::type = true>
    void push_back(const T& val)
    {
        if (is_placeholder())  // 如果当前对象是占位符
        {
            *this = Json::Array{{val}};  // 初始化为包含一个元素的数组
            return;
        }
        if (!can_arr_push_back())  // 如果不能向数组中添加元素
        {
            return;  // 直接返回
        }
        json_array_t* arr = json_value_array(node_);  // 获取当前数组的底层数组指针
        Json copy_json = val.copy();  // 创建值的副本
        json_array_append(arr, 0, copy_json.node_);  // 向数组中添加对象或数组元素
        copy_json.reset();  // 重置副本
    }

    // 向JSON数组中添加布尔类型元素
    void push_back(bool val);

    // 向JSON数组中添加nullptr类型元素
    void push_back(std::nullptr_t val);

    // 向JSON数组中添加字符串类型元素
    void push_back(const std::string& val);

    // 向JSON数组中添加C风格字符串类型元素
    void push_back(const char* val);

    // 向JSON数组中添加字符串向量类型元素
    void push_back(const std::vector<std::string>& val);

    // 向JSON数组中添加字符串初始化列表类型元素
    void push_back(const std::initializer_list<std::string>& val)
    {
        push_back(std::vector<std::string>(val));  // 将初始化列表转换为向量并调用对应的push_back方法
    }

    // 向JSON数组中添加Json类型元素
    void push_back(const Json& val);

    // 从JSON数组中移除指定索引的元素
    void erase(int index);

private:
    // 更新JSON数组中的元素，数值类型
    template <typename T, typename std::enable_if<detail::is_number<T>::value,
                                                  bool>::type = true>
    void update_arr(T val)
    {
        json_array_t* arr = json_value_array(parent_);  // 获取父级数组的底层数组指针
        json_array_insert_before(node_, arr, JSON_VALUE_NUMBER,  // 在当前节点前插入新值
                                 static_cast<double>(val));
        json_value_t* remove_val = json_array_remove(node_, arr);  // 移除旧值
        json_value_destroy(remove_val);  // 销毁旧值
    }

    // 更新JSON数组中的元素，对象或数组类型
    template <typename T,
              typename std::enable_if<std::is_same<T, Json::Object>::value ||
                                          std::is_same<T, Json::Array>::value,
                                      bool>::type = true>
    void update_arr(const T& val)
    {
        json_array_t* arr = json_value_array(parent_);  // 获取父级数组的底层数组指针
        Json copy_json = val.copy();  // 创建值的副本
        json_array_insert_before(node_, arr, 0, copy_json.node_);  // 在当前节点前插入新值
        copy_json.reset();  // 重置副本
        json_value_t* remove_val = json_array_remove(node_, arr);  // 移除旧值
        json_value_destroy(remove_val);  // 销毁旧值
    }

    // 更新JSON数组中的元素，布尔类型
    void update_arr(bool val);

    // 更新JSON数组中的元素，nullptr类型
    void update_arr(std::nullptr_t val);

    // 更新JSON数组中的元素，字符串类型
    void update_arr(const std::string& val);

    // 更新JSON数组中的元素，C风格字符串类型
    void update_arr(const char* val);

    // 更新JSON数组中的元素，字符串向量类型
    void update_arr(const std::vector<std::string>& val);

    // 更新JSON数组中的元素，Json类型
    void update_arr(const Json& val);
    
public:
    class IteratorBase
    {
    public:
        friend class Json;  // 声明Json类为友元类，允许Json类访问IteratorBase的私有成员

        // 显式构造函数，初始化迭代器
        explicit IteratorBase(const json_value_t* val)
            : val_(val), json_(new Json(nullptr, nullptr, ""))  // 初始化json_为一个新的Json对象
        {
        }

        // 析构函数，释放动态分配的Json对象
        ~IteratorBase()
        {
            if (json_ != nullptr)  // 如果json_不为空
            {
                delete json_;  // 释放json_指向的Json对象
            }
        }

        // 拷贝构造函数，创建一个新的IteratorBase对象
        IteratorBase(const IteratorBase& iter)
        {
            val_ = iter.val_;  // 拷贝val_指针
            key_ = iter.key_;  // 拷贝key_指针
            json_ = new Json(iter.json_->node_, iter.json_->parent_,  // 创建一个新的Json对象
                            iter.json_->parent_key_);
        }

        // 拷贝赋值运算符，用于拷贝赋值
        IteratorBase& operator=(const IteratorBase& iter)
        {
            if (this == &iter)  // 如果是自赋值
            {
                return *this;  // 直接返回当前对象的引用
            }
            val_ = iter.val_;  // 拷贝val_指针
            key_ = iter.key_;  // 拷贝key_指针
            json_->reset(iter.json_->node_, iter.json_->parent_, false,  // 重置json_对象
                        iter.json_->parent_key_);
            return *this;  // 返回当前对象的引用
        }

        // 移动构造函数，用于移动语义
        IteratorBase(IteratorBase&& iter)
            : val_(iter.val_), key_(iter.key_), json_(iter.json_)  // 直接移动成员变量
        {
            iter.val_ = nullptr;  // 将原对象的成员置为空
            iter.key_ = nullptr;
            iter.json_ = nullptr;
        }

        // 移动赋值运算符，用于移动语义
        IteratorBase& operator=(IteratorBase&& iter)
        {
            if (this == &iter)  // 如果是自赋值
            {
                return *this;  // 直接返回当前对象的引用
            }
            val_ = iter.val_;  // 移动val_指针
            iter.val_ = nullptr;
            key_ = iter.key_;  // 移动key_指针
            iter.key_ = nullptr;
            delete json_;  // 释放当前对象的json_对象
            json_ = iter.json_;  // 移动json_指针
            iter.json_ = nullptr;
            return *this;  // 返回当前对象的引用
        }

        // 解引用运算符，返回当前迭代器所指向的Json对象的引用
        Json& operator*() const
        {
            if (key_ != nullptr)  // 如果key_不为空
            {
                json_->parent_key_ = std::string(key_);  // 更新json_的父级键名
            }
            else
            {
                json_->parent_key_ = "";  // 清空json_的父级键名
            }
            return *json_;  // 返回json_对象的引用
        }

        // 成员访问运算符，返回当前迭代器所指向的Json对象的指针
        Json* operator->() const
        {
            if (key_ != nullptr)  // 如果key_不为空
            {
                json_->parent_key_ = std::string(key_);  // 更新json_的父级键名
            }
            else
            {
                json_->parent_key_ = "";  // 清空json_的父级键名
            }
            return json_;  // 返回json_对象的指针
        }

        // 比较运算符，用于比较两个迭代器是否相等
        bool operator==(const IteratorBase& other) const
        {
            return json_->node_ == other.json_->node_;  // 比较两个迭代器所指向的节点是否相同
        }

        // 不等比较运算符，用于比较两个迭代器是否不相等
        bool operator!=(IteratorBase const& other) const
        {
            return !(*this == other);  // 调用==运算符并取反
        }

    private:
        const json_value_t* val_ = nullptr;  // 指向当前节点的指针
        const char* key_ = nullptr;  // 指向当前键名的指针
        Json* json_;  // 指向当前Json对象的指针，作为游标
    };

    class iterator : public IteratorBase
    {
    public:
        friend class Json;  // 声明Json类为友元类，允许Json类访问iterator的私有成员

        // 显式构造函数，初始化迭代器
        explicit iterator(const json_value_t* val) : IteratorBase(val)
        {
        }

        // 前缀递增操作符
        iterator& operator++()
        {
            if (is_end())  // 如果已经是结束迭代器
            {
                return *this;  // 直接返回当前迭代器
            }
            forward();  // 向前移动到下一个元素
            return *this;  // 返回当前迭代器
        }

        // 后缀递增操作符
        iterator operator++(int)
        {
            iterator old = (*this);  // 保存当前迭代器的状态
            if (!is_end())  // 如果不是结束迭代器
            {
                ++(*this);  // 调用前缀递增操作符
            }
            return old;  // 返回旧的迭代器状态
        }

    private:
        // 设置迭代器为开始状态
        void set_begin()
        {
            json_->node_ = nullptr;  // 初始化节点指针为nullptr
            key_ = nullptr;  // 初始化键名指针为nullptr
            forward();  // 向前移动到第一个元素
        }

        // 设置迭代器为结束状态
        void set_end()
        {
            json_->node_ = nullptr;  // 设置节点指针为nullptr
            key_ = nullptr;  // 设置键名指针为nullptr
        }

        // 检查迭代器是否为结束状态
        bool is_end()
        {
            return json_->node_ == nullptr && key_ == nullptr;  // 检查节点和键名指针是否都为nullptr
        }

        // 向前移动迭代器
        void forward()
        {
            if (json_value_type(val_) == JSON_VALUE_OBJECT)  // 如果当前值是对象类型
            {
                json_object_t* obj = json_value_object(val_);  // 获取对象指针
                key_ = json_object_next_name(key_, obj);  // 获取下一个键名
                json_->node_ = const_cast<json_value_t*>(  // 获取下一个值节点
                    json_object_next_value(json_->node_, obj));
            }
            else if (json_value_type(val_) == JSON_VALUE_ARRAY)  // 如果当前值是数组类型
            {
                json_array_t* arr = json_value_array(val_);  // 获取数组指针
                json_->node_ = const_cast<json_value_t*>(  // 获取下一个值节点
                    json_array_next_value(json_->node_, arr));
            }
        }
    };

    class reverse_iterator : public IteratorBase
    {
    public:
        friend class Json;  // 声明Json类为友元类，允许Json类访问reverse_iterator的私有成员

        // 显式构造函数，初始化反向迭代器
        explicit reverse_iterator(const json_value_t* val) : IteratorBase(val)
        {
        }

        // 前缀递增操作符（实际上是向后移动）
        reverse_iterator& operator++()
        {
            if (is_rend())  // 如果已经是反向结束迭代器
            {
                return *this;  // 直接返回当前迭代器
            }
            backward();  // 向后移动到上一个元素
            return *this;  // 返回当前迭代器
        }

        // 后缀递增操作符（实际上是向后移动）
        reverse_iterator operator++(int)
        {
            reverse_iterator old = (*this);  // 保存当前迭代器的状态
            if (!is_rend())  // 如果不是反向结束迭代器
            {
                ++(*this);  // 调用前缀递增操作符
            }
            return old;  // 返回旧的迭代器状态
        }

    private:
        // 设置反向迭代器为反向开始状态
        void set_rbegin()
        {
            json_->node_ = nullptr;  // 初始化节点指针为nullptr
            key_ = nullptr;  // 初始化键名指针为nullptr
            backward();  // 向后移动到第一个元素
        }

        // 设置反向迭代器为反向结束状态
        void set_rend()
        {
            json_->node_ = nullptr;  // 设置节点指针为nullptr
            key_ = nullptr;  // 设置键名指针为nullptr
        }

        // 检查反向迭代器是否为反向结束状态
        bool is_rend()
        {
            return json_->node_ == nullptr && key_ == nullptr;  // 检查节点和键名指针是否都为nullptr
        }

        // 向后移动迭代器
        void backward()
        {
            if (json_value_type(val_) == JSON_VALUE_OBJECT)  // 如果当前值是对象类型
            {
                json_object_t* obj = json_value_object(val_);  // 获取对象指针
                key_ = json_object_prev_name(key_, obj);  // 获取上一个键名
                json_->node_ = const_cast<json_value_t*>(  // 获取上一个值节点
                    json_object_prev_value(json_->node_, obj));
            }
            else if (json_value_type(val_) == JSON_VALUE_ARRAY)  // 如果当前值是数组类型
            {
                json_array_t* arr = json_value_array(val_);  // 获取数组指针
                json_->node_ = const_cast<json_value_t*>(  // 获取上一个值节点
                    json_array_prev_value(json_->node_, arr));
            }
        }
    };

    // 创建正向迭代器的开始位置
    iterator begin() const
    {
        iterator iter(node_);  // 创建一个迭代器，初始化为当前节点
        iter.set_begin();      // 将迭代器设置为开始状态
        return iter;           // 返回初始化后的迭代器
    }

    // 创建正向迭代器的结束位置
    iterator end() const
    {
        iterator iter(node_);  // 创建一个迭代器，初始化为当前节点
        iter.set_end();        // 将迭代器设置为结束状态
        return iter;           // 返回初始化后的迭代器
    }

    // 创建反向迭代器的开始位置
    reverse_iterator rbegin() const
    {
        reverse_iterator iter(node_);  // 创建一个反向迭代器，初始化为当前节点
        iter.set_rbegin();            // 将反向迭代器设置为反向开始状态
        return iter;                  // 返回初始化后的反向迭代器
    }

    // 创建反向迭代器的结束位置
    reverse_iterator rend() const
    {
        reverse_iterator iter(node_);  // 创建一个反向迭代器，初始化为当前节点
        iter.set_rend();               // 将反向迭代器设置为反向结束状态
        return iter;                   // 返回初始化后的反向迭代器
    }

private:
    // 检查是否可以向对象中添加键值对
    bool can_obj_push_back();

    // 检查是否可以向数组中添加元素
    bool can_arr_push_back();

    // 检查当前对象是否为占位符
    // 占位符是一个特殊的null对象，可以转换为任何类型
    bool is_placeholder() const
    {
        return is_null() && parent_ != nullptr;  // 检查是否为null且有父级节点
    }

    // 销毁指定的节点
    void destroy_node(json_value_t** node)
    {
        if (allocated_)  // 如果当前节点是动态分配的
        {
            json_value_destroy(*node);  // 调用底层函数销毁节点
            *node = nullptr;  // 将节点指针置为空
            allocated_ = false;  // 标记为未分配
        }
    }

    // 拷贝另一个Json对象的内容到当前对象
    void copy(const Json& other);

public:
    // 默认构造函数，创建一个空的Json对象
    Json();

    // 从std::string构造Json对象，将字符串作为JSON值
    Json(const std::string& str);

    // 从C风格字符串构造Json对象，将字符串作为JSON值
    Json(const char* str);

    // 创建一个表示null的Json对象
    Json(std::nullptr_t null);

    // 从数值类型构造Json对象
    template <typename T, typename std::enable_if<detail::is_number<T>::value,
                                                bool>::type = true>
    Json(T val)
        : node_(json_value_create(JSON_VALUE_NUMBER, static_cast<double>(val))),  // 创建数值类型的JSON节点
        parent_(nullptr),  // 父级节点指针初始化为nullptr
        allocated_(true)  // 标记为已分配
    {
    }

    // 从布尔类型构造Json对象
    Json(bool val);

    // 从字符串向量构造Json对象，将向量转换为JSON数组
    Json(const std::vector<std::string>& val);

    // 从字符串构造Json对象，并解析为JSON数据
    Json(const std::string& str, bool parse_flag);

    // 拷贝构造函数，创建一个新的Json对象，内容与另一个Json对象相同
    Json(const Json& json);

    // 拷贝赋值运算符，将当前Json对象的内容替换为另一个Json对象的内容
    Json& operator=(const Json& json);

    // 移动构造函数，将另一个Json对象的内容移动到当前对象
    Json(Json&& other);

    // 移动赋值运算符，将另一个Json对象的内容移动到当前对象
    Json& operator=(Json&& other);

    // 析构函数，释放动态分配的资源
    ~Json();

protected:
    // watcher
    Json(const json_value_t* node, const json_value_t* parent,
        std::string&& key);
    Json(const json_value_t* node, const json_value_t* parent,
        const std::string& key);

    // 枚举类，用于表示JSON类型
    enum class JsonType
    {
    Object,  // 表示JSON对象
    Array,   // 表示JSON数组
    };

    // 用于创建JSON对象或数组的构造函数
    Json(JsonType type);

    // 检查当前Json对象是否为根节点
    bool is_root() const
    {
    return parent_ == nullptr;  // 如果父级节点为空，则为根节点
    }

    // 将当前Json对象转换为JSON对象类型
    bool to_object();

    // 将当前Json对象转换为JSON数组类型
    bool to_array();

    // 重置当前Json对象，使其为空
    void reset()
    {
    node_ = nullptr;  // 将节点指针置为空
    parent_ = nullptr;  // 将父级节点指针置为空
    allocated_ = false;  // 标记为未分配
    parent_key_.clear();  // 清空父级键名
    }

    // 重置当前Json对象，设置新的节点、父级节点、分配状态和父级键名
    void reset(json_value_t* node, const json_value_t* parent, bool allocated,
            const std::string& parent_key)
    {
    node_ = node;  // 设置节点指针
    parent_ = parent;  // 设置父级节点指针
    allocated_ = allocated;  // 设置分配状态
    parent_key_ = parent_key;  // 设置父级键名
    }

    // 重置当前Json对象，设置新的节点、父级节点、分配状态和父级键名（移动语义）
    void reset(json_value_t* node, const json_value_t* parent, bool allocated,
            std::string&& parent_key)
    {
    node_ = node;  // 设置节点指针
    parent_ = parent;  // 设置父级节点指针
    allocated_ = allocated;  // 设置分配状态
    parent_key_ = std::move(parent_key);  // 移动父级键名
    }

    // 设置当前Json对象的父级节点和父级键名
    void set_parent(const json_value_t* parent, std::string&& parent_key)
    {
    parent_ = parent;  // 设置父级节点指针
    parent_key_ = std::move(parent_key);  // 移动父级键名
    }

private:
    json_value_t* node_ = nullptr;  // 当前节点指针
    const json_value_t* parent_ = nullptr;  // 父级节点指针
    bool allocated_ = false;  // 标记当前节点是否动态分配
    std::string parent_key_;  // 父级键名

private:
    // for parse
    // 将JSON值转换为字符串，支持缩进和深度控制
    static void value_convert(const json_value_t* val, int spaces, int depth,
                              std::string* out_str);

    // 将C风格字符串转换为JSON字符串格式
    static void string_convert(const char* raw_str, std::string* out_str);

    // 将数值转换为字符串
    static void number_convert(double number, std::string* out_str);

    // 将JSON数组转换为字符串，支持缩进和深度控制
    static void array_convert(const json_array_t* arr, int spaces, int depth,
                              std::string* out_str);

    // 将JSON数组转换为字符串，不支持格式化
    static void array_convert_not_format(const json_array_t* arr,
                                         std::string* out_str);

    // 将JSON对象转换为字符串，支持缩进和深度控制
    static void object_convert(const json_object_t* obj, int spaces, int depth,
                               std::string* out_str);

    // 将JSON对象转换为字符串，不支持格式化
    static void object_convert_not_format(const json_object_t* obj,
                                          std::string* out_str);

    // 重载输出运算符，方便将Json对象直接输出到ostream
    friend inline std::ostream& operator<<(std::ostream& os, const Json& json)
    {
        return (os << json.dump());  // 调用dump方法序列化Json对象
    }
};

class Object_S : public Json
{
public:
    // 默认构造函数，创建一个空的JSON对象
    Object_S() : Json(JsonType::Object)
    {
    }

    // 从节点和父级节点构造JSON对象
    Object_S(const json_value_t* node, const json_value_t* parent)
        : Json(node, parent, "")
    {
    }

    // 使用std::basic_string定义字符串类型
    using string_type = std::basic_string<char, std::char_traits<char>,
                                                std::allocator<char>>;

    // 定义键值对类型
    using pair_type = std::pair<string_type, Json>;

    // 从初始化列表构造JSON对象
    Object_S(std::initializer_list<pair_type> list)
    {
        // 遍历初始化列表，将每个键值对添加到JSON对象中
        std::for_each(list.begin(), list.end(),
                      [this](const pair_type& pair)
                      { this->push_back(pair.first, pair.second); });
    }
};

class Array_S : public Json
{
public:
    // 默认构造函数，创建一个空的JSON数组
    Array_S() : Json(JsonType::Array)
    {
    }

    // 从节点和父级节点构造JSON数组
    Array_S(const json_value_t* node, const json_value_t* parent)
        : Json(node, parent, "")
    {
    }

    // 从初始化列表构造JSON数组
    Array_S(std::initializer_list<Json> list)
    {
        // 遍历初始化列表，将每个元素添加到JSON数组中
        std::for_each(list.begin(), list.end(),
                      [this](const Json& js) { this->push_back(js); });
    }
};

// 特化实现，用于提取Json::Object类型的子对象
template <typename T>
typename std::enable_if<std::is_same<T, Json::Object>::value, T>::type
Json::get() const
{
    return Json::Object(node_, parent_);  // 使用node_和parent_构造一个Json::Object对象
}

// 特化实现，用于提取Json::Array类型的子对象
template <typename T>
typename std::enable_if<std::is_same<T, Json::Array>::value, T>::type
Json::get() const
{
    return Json::Array(node_, parent_);  // 使用node_和parent_构造一个Json::Array对象
}

} // namespace Yukino

#endif // YUKINO_JSON_H_
