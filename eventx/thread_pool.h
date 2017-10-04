#ifndef TBOX_THREAD_POOL_H
#define TBOX_THREAD_POOL_H

#include <functional>
#include <tbox/event/forward.h>

namespace tbox {
namespace eventx {

namespace impl {
    class ThreadPool;
}

/**
 * �̳߳���
 */
class ThreadPool {
  public:
    /**
     * ���캯��
     *
     * \param main_loop         ���̵߳�Loop����ָ��
     */
    explicit ThreadPool(event::Loop *main_loop);
    virtual ~ThreadPool();

    /**
     * ��ʼ���̳߳أ�ָ����פ�߳���������߳���
     *
     * \param min_thread_num    ��פ�߳��������� >=0
     * \param max_thread_num    ����߳��������� >=0��=0��ʾ���޶�����߳�����
     *
     * \return bool     �Ƿ�ɹ�
     */
    bool initialize(int min_thread_num = 0, int max_thread_num = 0);

    using NonReturnFunc = std::function<void ()>;

    /**
     * ʹ��worker�߳�ִ��ĳ������
     *
     * \param backend_task      ��worker�߳�ִ�еĺ�������
     * \param prio              �������ȼ�[-2, -1, 0, 1, 2]��ԽС���ȼ�Խ��
     *
     * \return int  <0 ���񴴽�û�ɹ�
     *              >=0 ����id
     */
    int execute(const NonReturnFunc &backend_task, int prio = 0);

    /**
     * ʹ��worker�߳�ִ��ĳ���������������֮�������߳�ִ��ָ���Ļص�����
     *
     * \param backend_task      ��worker�߳�ִ�еĺ�������
     * \param main_cb           ������ɺ������߳�ִ�еĻص���������
     * \param prio              �������ȼ�[-2, -1, 0, 1, 2]��ԽС���ȼ�Խ��
     *
     * \return int  <0 ���񴴽�û�ɹ�
     *              >=0 ����id
     */
    int execute(const NonReturnFunc &backend_task, const NonReturnFunc &main_cb, int prio = 0);

    /**
     * ȡ������
     *
     * \param task_id   ����id
     *
     * \return  int     0: �ɹ�
     *                  1: û���ҵ���������ִ�У�
     *                  2: ����������ִ��
     */
    int cancel(int task_id);

    /**
     * ������Դ�����ȴ����е�worker�߳̽���
     */
    void cleanup();

  private:
    impl::ThreadPool *impl_;
};

}
}

#endif //TBOX_THREAD_POOL_H
