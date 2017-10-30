#ifndef TBOX_BASE_DEFINES_H_20171030
#define TBOX_BASE_DEFINES_H_20171030

//! ��ȡ�̶������Ա����
#define NUMBER_OF_ARRAY(arr)    (sizeof(arr) / sizeof(*arr))

//! ��Դ�ͷ���غ�
#define CHECK_DELETE_RESET_OBJ(obj) do { if (obj != NULL) { delete obj; obj = NULL; } } while (0)
#define CHECK_DELETE_OBJ(obj)   do { if (obj != NULL) { delete obj; } } while (0)
#define CHECK_DELETE_RESET_ARRAY(arr)   do { if (arr != NULL) { delete [] arr; arr = NULL; } } while (0)
#define CHECK_DELETE_ARRAY(arr) do { if (arr != NULL) { delete [] arr; } } while (0)
#define CHECK_FREE_RESET_PTR(ptr)   do { if (ptr != NULL) { free(ptr); ptr = NULL; } } while (0)
#define CHECK_FREE_PTR(ptr) do { if (ptr != NULL) { free(ptr); } } while (0)
#define CHECK_CLOSE_RESET_FD(fd)    do { if (fd != -1) { close(fd); fd = -1; } } while (0)
#define CHECK_CLOSE_FD(fd)  do { if (fd != -1) { close(fd); } } while (0)

//! �����н��ø�������
#define NONCOPYABLE(class_name) \
    class_name(const class_name&) = delete; \
    class_name& operator = (const class_name &) = delete; \

//! �����н����ƶ�����
#define IMMOVABLE(class_name) \
    class_name(class_name &&) = delete; \
    class_name& operator = (class_name &&) = delete; \

#endif //TBOX_BASE_DEFINES_H_20171030
