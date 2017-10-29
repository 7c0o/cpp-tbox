#ifdef  ENABLE_TEST
#include <gtest/gtest.h>

//! �ſ�Ȩ�ޣ��������
#define protected public
#define private public

#endif //ENABLE_TEST

#include <cassert>
#include <cstring>
#include <utility>

#include "buffer.h"

namespace tbox {
namespace network {

Buffer::Buffer(size_t reverse_size)
{
    //! ���Ԥ���ռ��С����ôҪΪ�������ռ�
    if (reverse_size > 0) {
        uint8_t *p_buff = new uint8_t [reverse_size];
        assert(p_buff != NULL);
        buffer_ptr_ = p_buff;
        buffer_size_ = reverse_size;
    }
}

Buffer::Buffer(const Buffer &other)
{
    cloneFrom(other);
}

Buffer::~Buffer()
{
    if (buffer_size_ > 0) {
        delete [] buffer_ptr_;
        buffer_ptr_ = NULL;
    }
}

void Buffer::swap(Buffer &other)
{
    if (&other == this)
        return;

    std::swap(other.buffer_ptr_,  buffer_ptr_);
    std::swap(other.buffer_size_, buffer_size_);
    std::swap(other.read_index_,  read_index_);
    std::swap(other.write_index_, write_index_);
}

Buffer& Buffer::operator = (const Buffer &other)
{
    if (this != &other) {
        reset();
        cloneFrom(other);
    }

    return *this;
}

void Buffer::reset()
{
    Buffer tmp(0);
    swap(tmp);
}

bool Buffer::ensureWritableSize(size_t write_size)
{
    if (write_size == 0)
        return true;

    //! �ռ��㹻
    if (writableSize() >= write_size)
        return true;

    //! ����Ƿ����ͨ����������ǰŲ�Ƿ��������ռ�Ҫ��
    if ((writableSize() + read_index_) >= write_size) {
        //! �� readable ����������ǰ��
        ::memmove(buffer_ptr_, (buffer_ptr_ + read_index_), (write_index_ - read_index_));
        write_index_ -= read_index_;
        read_index_ = 0;
        return true;

    } else {    //! ֻ�����·������Ŀռ�ſ���
        size_t new_size = (write_index_ + write_size) * 3 / 2;
        uint8_t *p_buff = new uint8_t[new_size];
        if (p_buff == NULL)
            return false;

        if (buffer_ptr_ != NULL) {
            //! ֻ��Ҫ���� readable ��������
            ::memcpy((p_buff + read_index_), (buffer_ptr_ + read_index_), (write_index_ - read_index_));
            delete [] buffer_ptr_;
        }

        buffer_ptr_  = p_buff;
        buffer_size_ = new_size;

        return true;
    }
}

void Buffer::hasWritten(size_t write_size)
{
    if (write_index_ + write_size > buffer_size_) {
        write_index_ = buffer_size_;
    } else {
        write_index_ += write_size;
    }
}

size_t Buffer::append(const void *p_data, size_t data_size)
{
    if (ensureWritableSize(data_size)) {
        ::memcpy(writableBegin(), p_data, data_size);
        hasWritten(data_size);
        return data_size;
    }
    return 0;
}

void Buffer::hasRead(size_t read_size)
{
    if (read_index_ + read_size > write_index_) {
        read_index_ = write_index_ = 0;
    } else {
        read_index_ += read_size;
        if (read_index_ == write_index_)
            read_index_ = write_index_ = 0;
    }
}

void Buffer::hasReadAll()
{
    read_index_ = write_index_ = 0;
}

size_t Buffer::fetch(void *p_buff, size_t buff_size)
{
    size_t read_size = (buff_size > readableSize()) ? readableSize() : buff_size;
    ::memcpy(p_buff, readableBegin(), read_size);
    hasRead(read_size);
    return read_size;
}

void Buffer::cloneFrom(const Buffer &other)
{
    if (other.buffer_size_ > 0) {
        uint8_t *p_buff = new uint8_t[other.buffer_size_];
        assert(p_buff != NULL);
        ::memcpy(p_buff, other.buffer_ptr_, other.buffer_size_);
        buffer_ptr_ = p_buff;
    }

    buffer_size_  = other.buffer_size_;
    read_index_   = other.read_index_;
    write_index_  = other.write_index_;
}

/////////////////////
// unit test below
/////////////////////
#ifdef  ENABLE_TEST

namespace {

TEST(network_buffer, constuct) {
    Buffer b1;
    EXPECT_EQ(b1.readableSize(), 0u);

    Buffer b2(b1);
    EXPECT_EQ(b2.readableSize(), 0u);
}

TEST(network_buffer, append_and_fetch) {
    Buffer b1(0);
    //! �Ȳ���4���ֽ�
    b1.append("abcd", 4);
    EXPECT_EQ(b1.readableSize(), 4u);
    //! �ٲ���4���ֽ�
    b1.append("efgh", 4);
    EXPECT_EQ(b1.readableSize(), 8u);
    //! �����е����ݶ�����
    char buff1[10] = { 0 };
    EXPECT_EQ(b1.fetch(buff1, 10), 8u);
    EXPECT_STREQ(buff1, "abcdefgh");
    //! ���ڻ���������
    EXPECT_EQ(b1.readableSize(), 0u);
}

/**
 * ���û����ʼ����Ϊ8����д��7�ֽڣ��ٶ���5�ֽڣ���д��5�ֽ�
 * ��黺���������Ƿ���8�ֽڡ�
 * �����Ե�Ŀ���ǹ۲���д��ʱ���������Ȳ���ʱ��Buffer�᲻���ƶ��ɶ�ȡ������
 */
TEST(network_buffer, append_and_move_data) {
    Buffer b1(8);
    b1.append("1234567", 7);    //! �����ֻʣ1�ֽ�
    b1.hasRead(5);              //! ����5�ֽ�
    b1.append("89abc", 5);      //! ����5�ֽ�
    EXPECT_EQ(b1.buffer_size_, 8u);
    char buff[10] = { 0 };
    EXPECT_EQ(b1.fetch(buff, 10), 7u);
    EXPECT_STREQ(buff, "6789abc");
    EXPECT_EQ(b1.readableSize(), 0u);
}

/**
 * ���ͨ��readBegin()��ȡ�����Ƿ�����
 */
TEST(network_buffer, readBegin_hasRead) {
    Buffer b(8);
    const char *str = "hello world, my name is Sid Lee";
    size_t str_size = strlen(str) + 1;
    b.append(str, str_size);
    EXPECT_EQ(b.readableSize(), str_size);
    EXPECT_STREQ((const char*)b.readableBegin(), str);
    b.hasRead(14);
    EXPECT_STREQ((const char*)b.readableBegin(), (str + 14));

    b.hasRead(500);
    EXPECT_EQ(b.readableSize(), 0u);
}

/**
 * ���ͨ��writeBegin()д�����Ƿ�����
 */
TEST(network_buffer, writeBegin_hasWriten) {
    Buffer b(0);
    const char *str = "hello world";
    size_t str_size = strlen(str) + 1;
    b.ensureWritableSize(str_size);
    strcpy((char*)b.writableBegin(), str);
    b.hasWritten(str_size);

    EXPECT_STREQ((const char*)b.readableBegin(), str);
}

TEST(network_buffer, swap) {
    Buffer b1(8);
    b1.append("abc", 4);

    Buffer b2(2);
    b2.append("hello", 6);

    b1.swap(b2);
    EXPECT_EQ(b1.readableSize(), 6u);
    EXPECT_EQ(b2.readableSize(), 4u);
    EXPECT_STREQ((const char*)b1.readableBegin(), "hello");
    EXPECT_STREQ((const char*)b2.readableBegin(), "abc");
}

TEST(network_buffer, reset) {
    Buffer b1(8);
    b1.append("abc", 4);
    b1.reset();

    EXPECT_EQ(b1.readableSize(), 0u);
    EXPECT_TRUE(b1.buffer_ptr_ == NULL);
}

TEST(network_buffer, copy_construct) {
    Buffer b1(8);
    b1.append("abc", 4);
    Buffer b2(b1);
    EXPECT_EQ(b2.readableSize(), 4u);
    EXPECT_STREQ((const char*)b2.readableBegin(), "abc");
}

TEST(network_buffer, assign) {
    Buffer b1(8);
    b1.append("abc", 4);
    Buffer b2(43);
    b2.append("1234567890", 10);
    b2 = b1;
    EXPECT_EQ(b2.readableSize(), 4u);
    EXPECT_STREQ((const char*)b2.readableBegin(), "abc");
}

TEST(network_buffer, huge_data) {
    Buffer b;
    int orig_data[100] = { 0 };
    for (int i = 0; i < 100; ++i) {
        orig_data[i] = i;
    }

    //! ����10000������
    for (int i = 0; i < 10000; ++i) {
        EXPECT_EQ(b.append(orig_data, sizeof(orig_data)), sizeof(orig_data));
    }

    EXPECT_EQ(b.readableSize(), (sizeof(orig_data) * 10000));
    size_t max_size = b.buffer_size_;

    //! ȡ��9000������
    for (int i = 0; i < 9000; ++i) {
        int read_data[100] = { 0 };
        EXPECT_EQ(b.fetch(read_data, sizeof(read_data)), sizeof(read_data));
        EXPECT_EQ(memcmp(read_data, orig_data, sizeof(read_data)), 0);
    }

    //! ��д��9000������
    for (int i = 0; i < 9000; ++i) {
        EXPECT_EQ(b.append(orig_data, sizeof(orig_data)), sizeof(orig_data));
    }

    //! ��������Ŀռ��Сû�б仯
    EXPECT_EQ(max_size, b.buffer_size_);

    //! ȡ��10000������
    for (int i = 0; i < 10000; ++i) {
        int read_data[100] = { 0 };
        EXPECT_EQ(b.fetch(read_data, sizeof(read_data)), sizeof(read_data));
        EXPECT_EQ(memcmp(read_data, orig_data, sizeof(read_data)), 0);
    }

    //! ��������Ŀռ��Сû�б仯
    EXPECT_EQ(max_size, b.buffer_size_);
    EXPECT_EQ(b.readableSize(), 0u);
}

TEST(network_buffer, read_all_except_index_reset) {
    Buffer b;

    b.append("123456789", 10);
    char read_data[10] = { 0 };
    b.fetch(read_data, 10);

    EXPECT_EQ(b.read_index_, 0u);
    EXPECT_EQ(b.write_index_, 0u);
}

}
#endif //ENABLE_TEST

}
}
