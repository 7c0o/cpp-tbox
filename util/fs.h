#ifndef TBOX_UTIL_FS_H_20220103
#define TBOX_UTIL_FS_H_20220103

#include <string>

namespace tbox::util::fs {

////////////////////////////////////////////////////////////////////
// �ļ����
////////////////////////////////////////////////////////////////////

/**
 * ����ļ��Ƿ����
 *
 * \param filename      �ļ���
 *
 * \return true     �ļ�����
 * \return false    �ļ�������
 */
bool IsFileExist(const std::string &filename);

/**
 * ���ļ��ж�ȡ�ִ�
 *
 * \param filename      �ļ���
 * \param content       ��ȡ���ı������std::string
 *
 * \return true     �ɹ�
 * \return false    ʧ��
 */
bool ReadStringFromTextFile(const std::string &filename, std::string &content);

/**
 * ���ִ�д�뵽�ļ�
 */
bool WriteStringToTextFile(const std::string &filename, const std::string &content);

/**
 * ���ļ��ж�ȡ����
 *
 * \param filename      �ļ���
 * \param content       ��ȡ�����ݣ����ܷ��ִ�
 *
 * \return true     �ɹ�
 * \return false    ʧ��
 */
bool ReadBinaryFromFile(const std::string &filename, std::string &content);

/**
 * ������д�뵽�ļ�
 */
bool WriteBinaryToFile(const std::string &filename, const std::string &content);

/**
 * ɾ���ļ�
 *
 * \param filename      ��ɾ�����ļ���
 *
 * \return true �ɹ�
 * \return false ʧ��
 */
bool RemoveFile(const std::string &filename);

////////////////////////////////////////////////////////////////////
// Ŀ¼���
////////////////////////////////////////////////////////////////////

/**
 * Ŀ¼�Ƿ����
 *
 * \param dir               Ŀ¼
 *
 * \return true     Ŀ¼����
 * \return false    Ŀ¼������
 */
bool IsDirectoryExist(const std::string &dir);

/**
 * ����Ŀ¼
 * �ȼ���shell���� "mkdir -p xxx"
 *
 * \param dir               Ŀ¼
 *
 * \return true     Ŀ¼�����ɹ�
 * \return false    Ŀ¼����ʧ��
 */
bool MakeDirectory(const std::string &dir);

/**
 * ɾ��Ŀ¼
 *
 * \param dir               Ŀ¼
 *
 * \return true     Ŀ¼ɾ���ɹ�
 * \return false    Ŀ¼ɾ��ʧ��
 */
bool RemoveDirectory(const std::string &dir);

////////////////////////////////////////////////////////////////////
// ����
////////////////////////////////////////////////////////////////////

/**
 * ����·����ȡ�ļ���
 */
std::string Basename(const std::string &full_path);

/**
 * ����·����ȡĿ¼��
 */
std::string Dirname(const std::string &full_path);

}

#endif //TBOX_UTIL_FS_H_20220103
