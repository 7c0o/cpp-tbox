#include "thread_pool.h"
#include "impl/thread_pool.h"

namespace tbox {
namespace eventx {

ThreadPool::ThreadPool(event::Loop *main_loop) :
    impl_(new impl::ThreadPool(main_loop))
{ }

ThreadPool::~ThreadPool()
{
    delete impl_;
}

bool ThreadPool::initialize(int min_thread_num, int max_thread_num)
{
    return impl_->initialize(min_thread_num, max_thread_num);
}

int ThreadPool::execute(const NonReturnFunc &backend_task, int prio)
{
    return impl_->execute(backend_task, prio);
}

int ThreadPool::execute(const NonReturnFunc &backend_task, const NonReturnFunc &main_cb, int prio)
{
    return impl_->execute(backend_task, main_cb, prio);
}

int ThreadPool::cancel(int task_id)
{
    return impl_->cancel(task_id);
}

void ThreadPool::cleanup()
{
    impl_->cleanup();
}

}
}

#ifdef ENABLE_TEST
#include <gtest/gtest.h>

#include <tbox/base/log.h>
#include <tbox/event/loop.h>

using namespace std;
using namespace tbox::event;
using namespace tbox::eventx;

namespace {

auto backend_func = \
        [](int id) {
            LogDbg("<<task %d run in back", id);
            std::this_thread::sleep_for(std::chrono::seconds(1));
            LogDbg("task %d run in back>>", id);
        };

/**
 * ��С�߳���Ϊ2������߳���Ϊ5
 */
TEST(ThreadPool, min2_max5) {
    Loop *loop = Loop::New();

    LogDbg("%d", 123);
    ThreadPool *tp = new ThreadPool(loop);
    ASSERT_TRUE(tp->initialize(2,5));

    for (int i = 0; i < 12; ++i) {
        ASSERT_NE(tp->execute(std::bind(backend_func, i)), -1);
    }

    LogDbg("run in main");
    loop->exitLoop(Timespan::Second(4));
    loop->runLoop();

    tp->cleanup();

    delete tp;
    delete loop;
}

/**
 * ��С�߳���Ϊ0������߳���Ϊ5
 */
TEST(ThreadPool, min0_max5) {
    Loop *loop = Loop::New();

    ThreadPool *tp = new ThreadPool(loop);
    ASSERT_TRUE(tp->initialize(0,5));

    for (int i = 0; i < 12; ++i) {
        ASSERT_NE(tp->execute(std::bind(backend_func, i)), -1);
    }

    LogDbg("run in main");
    loop->exitLoop(Timespan::Second(4));
    loop->runLoop();

    tp->cleanup();

    delete tp;
    delete loop;
}

/**
 * ��������ɹ������˳����߳�
 * ��Ҫ�Ǽ����û���ڴ�й©
 */
TEST(ThreadPool, exit_before_finish) {
    Loop *loop = Loop::New();

    LogDbg("%d", 123);
    ThreadPool *tp = new ThreadPool(loop);
    ASSERT_TRUE(tp->initialize(1,1));

    for (int i = 0; i < 3; ++i) {
        ASSERT_NE(tp->execute(std::bind(backend_func, i)), -1);
    }

    LogDbg("run in main");
    loop->exitLoop(Timespan::Second(1));
    loop->runLoop();

    tp->cleanup();

    delete tp;
    delete loop;
}

/**
 * ȡ������
 *
 * ��������������1.5��ʱȫ��ȡ����
 */
TEST(ThreadPool, cancel_task) {
    Loop *loop = Loop::New();

    LogDbg("%d", 123);
    ThreadPool *tp = new ThreadPool(loop);
    ASSERT_TRUE(tp->initialize(1,1));

    vector<int> task_ids;
    for (int i = 0; i < 3; ++i) {
        int tid = tp->execute(std::bind(backend_func, i));
        task_ids.push_back(tid);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    EXPECT_EQ(tp->cancel(task_ids[0]), 1);  //! ��һ�����������
    EXPECT_EQ(tp->cancel(task_ids[1]), 2);  //! �ڶ�����������ִ��
    EXPECT_EQ(tp->cancel(task_ids[2]), 0);  //! ���������������ȡ��
    EXPECT_EQ(tp->cancel(100), 1);  //! ���񲻴���

    loop->exitLoop(Timespan::Second(4));
    loop->runLoop();

    tp->cleanup();

    delete tp;
    delete loop;
}

/**
 * ���ȼ�
 *
 * �������ȼ���ߵ��ȱ�ִ��
 */
TEST(ThreadPool, prio) {
    Loop *loop = Loop::New();

    LogDbg("%d", 123);
    ThreadPool *tp = new ThreadPool(loop);
    ASSERT_TRUE(tp->initialize(1,1));

    vector<int> task_ids;
    auto backend_func = \
        [&task_ids](int id) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            LogDbg("task id: %d", id);
            task_ids.push_back(id);
        };

    tp->execute([] { std::this_thread::sleep_for(std::chrono::seconds(1)); });
    for (int i = 0; i < 5; ++i)
        tp->execute(std::bind(backend_func, i), 2-i);

    loop->exitLoop(Timespan::Second(2));
    loop->runLoop();

    ASSERT_EQ(task_ids.size(), 5);
    EXPECT_EQ(task_ids[0], 4);
    EXPECT_EQ(task_ids[1], 3);
    EXPECT_EQ(task_ids[2], 2);
    EXPECT_EQ(task_ids[3], 1);
    EXPECT_EQ(task_ids[4], 0);

    tp->cleanup();

    delete tp;
    delete loop;
}

}

#endif
