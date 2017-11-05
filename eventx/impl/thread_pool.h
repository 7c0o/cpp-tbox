#ifndef TBOX_THREAD_POOL_IMP_H
#define TBOX_THREAD_POOL_IMP_H

#include <array>
#include <map>
#include <set>
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

#include <tbox/event/forward.h>

namespace tbox {
namespace eventx {
namespace impl {

using NonReturnFunc = std::function<void ()>;

/**
 * ������
 */
struct TaskEvent {
  int task_id;  //! �����
  NonReturnFunc backend_task;   //! �����ڹ����߳���ִ�к���
  NonReturnFunc main_cb;        //! ����ִ����ɺ���main_loopִ�еĻص�����

  TaskEvent(int id, const NonReturnFunc &task, const NonReturnFunc &cb) :
      task_id(id), backend_task(task), main_cb(cb)
  { }
};

/**
 * �̳߳�ʵ��
 */
class ThreadPool {
  public:
    explicit ThreadPool(event::Loop *main_loop);
    virtual ~ThreadPool();

    bool initialize(int min_thread_num, int max_thread_num);

    int execute(const NonReturnFunc &backend_task, int prio);
    int execute(const NonReturnFunc &backend_task, const NonReturnFunc &main_cb, int prio);

    int cancel(int task_id);

    void cleanup();

  protected:
    void threadProc(int id);
    bool createWorker();

    bool shouldThreadExitWaiting() const;   //! �ж����߳��Ƿ���Ҫ�˳�����������wait()����
    TaskEvent* popOneTask(); //! ȡ��һ�����ȼ���ߵ�����

  private:
    event::Loop *main_loop_ = nullptr; //!< ���߳�

    bool is_ready_ = false;     //! �Ƿ��Ѿ���ʼ����

    size_t min_thread_num_ = 0; //!< ���ٵ��̸߳���
    size_t max_thread_num_ = 0; //!< �����̸߳���

    std::mutex lock_;                //!< ������
    std::condition_variable cond_var_;   //!< ��������

    std::array<std::list<TaskEvent*>, 5> undo_tasks_array_;    //!< ���ȼ������б�5��
    std::set<int/*task_id*/> doing_tasks_set_;   //!< ��¼���ڴ��µ�����

    size_t idle_thread_num_ = 0;    //!< �ռ��̸߳���
    std::map<int/*thread_id*/, std::thread*> threads_;    //!< �̶߳���
    int thread_id_alloc_ = 0;       //!< �����̵߳�ID������
    bool threads_stop_flag_ = false;//!< �Ƿ����й����߳�����ֹͣ���

    int task_id_alloc_ = 0;         //!< ����id���������
};

}
}
}

#endif //TBOX_THREAD_POOL_IMP_H
