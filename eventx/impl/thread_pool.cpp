#include "thread_pool.h"

#include <tbox/base/log.h>
#include <tbox/event/loop.h>

namespace tbox {
namespace eventx {
namespace impl {

ThreadPool::ThreadPool(event::Loop *main_loop) :
    main_loop_(main_loop)
{ }

ThreadPool::~ThreadPool()
{
    if (is_ready_)
        cleanup();
}

bool ThreadPool::initialize(int min_thread_num, int max_thread_num)
{
    if (is_ready_) {
        LogWarn("it has ready, cleanup() first");
        return false;
    }

    if (max_thread_num < 0 || min_thread_num < 0 ||
        (max_thread_num != 0 && min_thread_num > max_thread_num)) {
        LogWarn("max_thread_num or min_thread_num invalid, max:%d, min:%d", max_thread_num, min_thread_num);
        return false;
    }

    {
        std::lock_guard<std::mutex> lg(lock_);
        min_thread_num_ = min_thread_num;
        max_thread_num_ = max_thread_num;

        for (int i = 0; i < min_thread_num; ++i)
            if (!createWorker())
                return false;
    }

    threads_stop_flag_ = false;
    is_ready_ = true;

    return true;
}

int ThreadPool::execute(const NonReturnFunc &backend_task, int prio)
{
    return execute(backend_task, NonReturnFunc(), prio);
}

int ThreadPool::execute(const NonReturnFunc &backend_task, const NonReturnFunc &main_cb, int prio)
{
    if (!is_ready_) {
        LogWarn("need initialize() first");
        return -1;
    }

    if (prio < -2)
        prio = -2;
    else if (prio > 2)
        prio = 2;

    int level = prio + 2;

    int curr_task_id = 0;

    {
        std::lock_guard<std::mutex> lg(lock_);
        curr_task_id = ++task_id_alloc_;
    }

    TaskItem *item = new TaskItem(curr_task_id, backend_task, main_cb);
    if (item == nullptr)
        return -1;

    {
        std::lock_guard<std::mutex> lg(lock_);
        undo_tasks_array_.at(level).push_back(item);
        //! ����ռ��̲߳������һ������ٴ����µ��߳�
        if (idle_thread_num_ == 0 &&
            (max_thread_num_ == 0 || threads_.size() < max_thread_num_))
            createWorker();
    }

    cond_var_.notify_one();
    LogInfo("task_id:%d", curr_task_id);

    return curr_task_id;
}

/**
 * ����ֵ���£�
 * 0: ȡ���ɹ�
 * 1: û���ҵ�������
 * 2: ����������ִ��
 */
int ThreadPool::cancel(int task_id)
{
    std::lock_guard<std::mutex> lg(lock_);

    //! �������ִ��
    if (doing_tasks_set_.find(task_id) != doing_tasks_set_.end())
        return 2;   //! ��������ִ��

    //! �Ӹ����ȼ�������ȼ��������ҳ����ȼ���ߵ�����
    for (size_t i = 0; i < undo_tasks_array_.size(); ++i) {
        auto &tasks = undo_tasks_array_.at(i);
        if (!tasks.empty()) {
            for (auto iter = tasks.begin(); iter != tasks.end(); ++iter) {
                TaskItem *item = *iter;
                if (item->task_id == task_id) {
                    tasks.erase(iter);
                    delete item;
                    return 0;   //! ����ȡ���ɹ�
                }
            }
        }
    }

    return 1;   //! ����û���ҵ�
}

void ThreadPool::cleanup()
{
    if (!is_ready_)
        return;

    {
        std::lock_guard<std::mutex> lg(lock_);
        //! ���task�е�����
        for (size_t i = 0; i < undo_tasks_array_.size(); ++i) {
            auto &tasks = undo_tasks_array_.at(i);
            while (!tasks.empty()) {
                TaskItem *item = tasks.front();
                delete item;
                tasks.pop_front();
            }
        }
    }

    threads_stop_flag_ = true;
    cond_var_.notify_all();

    //! �ȴ����е��߳��˳�
    for (auto item : threads_) {
        std::thread *t = item.second;
        t->join();
        delete t;
    }
    threads_.clear();

    is_ready_ = false;
}

void ThreadPool::threadProc(int id)
{
    LogInfo("thread %d start", id);

    while (true) {
        TaskItem* item = nullptr;
        {
            std::unique_lock<std::mutex> lk(lock_);

            /**
             * Ϊ��ֹ���������̣߳��˴����Ż���
             * �����ǰ���п��е��߳��ڵȴ����ҵ�ǰ���̸߳����ѳ�����פ�߳�����˵���߳���������������Ҫ��
             * ���˳���ǰ�߳�
             */
            if (idle_thread_num_ > 0 && threads_.size() > min_thread_num_) {
                LogDbg("thread %d will exit, no more work.", id);
                //! ���߳�ȡ����������main_loopȥjoin()��Ȼ��delete
                auto iter = threads_.find(id);
                if (iter != threads_.end()) {
                    std::thread *t = iter->second;
                    threads_.erase(iter);
                    main_loop_->runInLoop([t]{ t->join(); delete t; });
                }
                break;
            }

            //! �ȴ�����
            ++idle_thread_num_;
            cond_var_.wait(lk, std::bind(&ThreadPool::shouldThreadExitWaiting, this));
            --idle_thread_num_;

            /**
             * ������������ cond_var_.wait() �˳�
             * 1. �����������������Ҫִ��ʱ
             * 2. �̳߳� cleanup() ʱҪ�����й����߳��˳�ʱ
             *
             * ���ԣ������� threads_stop_flag_ ���ǲ��������˳�
             */
            if (threads_stop_flag_) {
                LogDbg("thread %d will exit, stop flag.", id);
                break;
            }

            item = popOneTask();    //! �����������ȡ�����ȼ���ߵ�����
        }

        //! �������ȥִ�����񣬲���Ҫ�ټ�����
        if (item != nullptr) {
            {
                std::lock_guard<std::mutex> lg(lock_);
                doing_tasks_set_.insert(item->task_id);
            }

            LogDbg("thread %d pick task %d", id, item->task_id);
            item->backend_task();
            main_loop_->runInLoop(item->main_cb);
            LogDbg("thread %d finish task %d", id, item->task_id);

            {
                std::lock_guard<std::mutex> lg(lock_);
                doing_tasks_set_.erase(item->task_id);
            }
            delete item;
        }
    }

    LogInfo("thread %d exit", id);
}

bool ThreadPool::createWorker()
{
    int curr_thread_id = ++thread_id_alloc_;
    std::thread *new_thread = new std::thread(std::bind(&ThreadPool::threadProc, this, curr_thread_id));
    if (new_thread != nullptr) {
        threads_.insert(std::make_pair(curr_thread_id, new_thread));
        return true;

    } else {
        LogErr("new thread fail");
        return false;
    }
}

bool ThreadPool::shouldThreadExitWaiting() const
{
    if (threads_stop_flag_)
        return true;

    for (size_t i = 0; i < undo_tasks_array_.size(); ++i) {
        const auto &tasks = undo_tasks_array_.at(i);
        if (!tasks.empty()) {
            return true;
        }
    }

    return false;
}

TaskItem* ThreadPool::popOneTask()
{
    //! �Ӹ����ȼ�������ȼ��������ҳ����ȼ���ߵ�����
    for (size_t i = 0; i < undo_tasks_array_.size(); ++i) {
        auto &tasks = undo_tasks_array_.at(i);
        if (!tasks.empty()) {
            TaskItem* ret = tasks.front();
            tasks.pop_front();
            return ret;
        }
    }
    return nullptr;
}

}
}
}
