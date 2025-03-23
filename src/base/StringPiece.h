// Taken from PCRE pcre_stringpiece.h
//
// Copyright (c) 2005, Google Inc.
// All rights reserved.

#ifndef YUKINO_STRINGPIECE_H_
#define YUKINO_STRINGPIECE_H_

#include <cstring>  // 包含标准库中的cstring头文件，用于字符串操作
#include <string>   // 包含标准库中的string头文件，用于std::string类型

namespace Yukino
{
class StringPiece
{
private:
    const char *ptr_;  // 指向字符串的指针
    size_t length_;    // 字符串的长度

public:
    // 默认构造函数，初始化为空字符串
    StringPiece()
            : ptr_(nullptr), length_(0)
    {}

    // 从C风格字符串构造
    StringPiece(const char *str)
            : ptr_(str), length_(strlen(ptr_))
    {}

    // 从std::string构造
    StringPiece(const std::string &str)
            : ptr_(str.data()), length_(str.size())
    {}

    // 从指针和长度构造
    StringPiece(const char *offset, size_t len)
            : ptr_(offset), length_(len)
    {}

    // 从void指针和长度构造
    StringPiece(const void *str, size_t len)
            : ptr_(static_cast<const char *>(str)), length_(len)
    {}

    // 获取指向字符串的指针
    const char *data() const
    { return ptr_; }

    // 获取字符串的长度
    size_t size() const
    { return length_; }

    // 检查字符串是否为空
    bool empty() const
    { return length_ == 0; }

    // 获取字符串的起始指针
    const char *begin() const
    { return ptr_; }

    // 获取字符串的结束指针
    const char *end() const
    { return ptr_ + length_; }

    // 清空StringPiece
    void clear()
    {
        ptr_ = nullptr;
        length_ = 0;
    }

    // 设置指针和长度
    void set(const char *buffer, int len)
    {
        ptr_ = buffer;
        length_ = len;
    }

    // 设置指针并计算长度
    void set(const char *str)
    {
        ptr_ = str;
        length_ = strlen(str);
    }

    // 设置指针和长度
    void set(const char *buffer, size_t len)
    {
        ptr_ = buffer;
        length_ = len;
    }

    // 设置指针和长度
    void set(const void *buffer, size_t len)
    {
        ptr_ = static_cast<const char *>(buffer);
        length_ = len;
    }

    // 通过索引访问字符
    char operator[](int i) const
    { return ptr_[i]; }

    // 移除前缀
    void remove_prefix(size_t n)
    {
        ptr_ += n;
        length_ -= n;
    }

    // 移除后缀
    void remove_suffix(size_t n)
    {
        length_ -= n;
    }

    // 同时移除前缀和后缀
    void shrink(size_t prefix, size_t suffix)
    {
        ptr_ += prefix;
        length_ -= prefix;
        length_ -= suffix;
    }

    // 比较两个StringPiece是否相等
    bool operator==(const StringPiece &x) const
    {
        return ((length_ == x.length_) &&
                (memcmp(ptr_, x.ptr_, length_) == 0));
    }

    // 比较两个StringPiece是否不相等
    bool operator!=(const StringPiece &x) const
    {
        return !(*this == x);
    }

    // 定义比较操作符
#define STRINGPIECE_BINARY_PREDICATE(cmp, auxcmp)                             \
    bool operator cmp (const StringPiece& x) const {                           \
        int r = memcmp(ptr_, x.ptr_, length_ < x.length_ ? length_ : x.length_); \
        return ((r auxcmp 0) || ((r == 0) && (length_ cmp x.length_)));          \
    }

    // 定义小于操作符
    STRINGPIECE_BINARY_PREDICATE(<, <);

    // 定义小于等于操作符
    STRINGPIECE_BINARY_PREDICATE(<=, <);

    // 定义大于等于操作符
    STRINGPIECE_BINARY_PREDICATE(>=, >);

    // 定义大于操作符
    STRINGPIECE_BINARY_PREDICATE(>, >);
#undef STRINGPIECE_BINARY_PREDICATE

    // 比较两个StringPiece
    int compare(const StringPiece &x) const
    {
        int r = memcmp(ptr_, x.ptr_, length_ < x.length_ ? length_ : x.length_);
        if (r == 0)
        {
            if (length_ < x.length_) r = -1;
            else if (length_ > x.length_) r = +1;
        }
        return r;
    }

    // 将StringPiece转换为std::string
    std::string as_string() const
    {
        return std::string(data(), size());
    }

    // 将StringPiece的内容复制到目标std::string
    void CopyToString(std::string *target) const
    {
        target->assign(ptr_, length_);
    }

    // 检查StringPiece是否以指定的前缀开始
    bool starts_with(const StringPiece &x) const
    {
        return ((length_ >= x.length_) && (memcmp(ptr_, x.ptr_, x.length_) == 0));
    }
};

}  // namespace Yukino

// ------------------------------------------------------------------
// 用于创建使用StringPiece的STL容器的函数
// 注意：StringPiece的生命周期最好小于底层std::string或char*的生命周期。
// 如果不是这样，那么你不能安全地将StringPiece存储到STL容器中
// ------------------------------------------------------------------

#ifdef HAVE_TYPE_TRAITS
// 这使得某些STL实现中的vector<StringPiece>非常快速
template<> struct __type_traits<Yukino::StringPiece> {
    typedef __true_type    has_trivial_default_constructor;  // 具有平凡的默认构造函数
    typedef __true_type    has_trivial_copy_constructor;     // 具有平凡的拷贝构造函数
    typedef __true_type    has_trivial_assignment_operator;  // 具有平凡的赋值运算符
    typedef __true_type    has_trivial_destructor;           // 具有平凡的析构函数
    typedef __true_type    is_POD_type;                      // 是POD类型
};
#endif

// STL的std::hash<>特化的替代实现。
template<typename StringPieceType>
struct StringPieceHashImpl
{
    // 这是一个自定义的哈希函数。我们不直接使用已经为string和std::u16string定义的哈希函数，
    // 因为这将需要调用string的构造函数，而我们不希望这样。
    std::size_t operator()(StringPieceType sp) const
    {
        std::size_t result = 0;  // 初始化哈希结果为0
        for (auto c: sp)  // 遍历StringPiece中的每个字符
            result = (result * 131) + c;  // 使用简单的哈希算法计算哈希值
        return result;  // 返回最终的哈希值
    }
};

// 定义一个类型别名，方便在代码中使用自定义的哈希函数
using StringPieceHash = StringPieceHashImpl<Yukino::StringPiece>;

#endif // YUKINO_STRINGPIECE_H_