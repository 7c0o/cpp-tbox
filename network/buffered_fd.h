#ifndef TBOX_NETWORK_BUFFERED_FD_H_20171030
#define TBOX_NETWORK_BUFFERED_FD_H_20171030

#include <functional>
#include <tbox/event/forward.h>
#include <tbox/base/defines.h>

#include "fd.h"
#include "buffer.h"

namespace tbox {
namespace network {

class BufferedFd {
  public:
    explicit BufferedFd(event::Loop *wp_loop);
    virtual ~BufferedFd();

    NONCOPYABLE(BufferedFd);

  public:
    using ReceiveCallback       = std::function<void(Buffer&)>;
    using WriteCompleteCallback = std::function<void()>;
    using ReadZeroCallback      = std::function<void()>;
    using ErrorCallback         = std::function<void(int)>;

    enum {
        kReadOnly  = 0x01,
        kWriteOnly = 0x02,
        kReadWrite = 0x03,
    };
    //! ��ʼ������ָ�����ͻ��ǽ��չ���
    bool initialize(int fd, short events = kReadWrite);

    void setReceiveCallback(const ReceiveCallback &func, size_t threshold);
    void setSendCompleteCallback(const WriteCompleteCallback &func) { send_complete_cb_ = func; }
    void setReadZeroCallback(const ReadZeroCallback &func) { read_zero_cb_ = func; }
    void setErrorCallback(const ErrorCallback &func) { error_cb_ = func; }

    //! ��������
    bool send(const void *data_ptr, size_t data_size);

    //! ������ر��ڲ��¼���������
    bool enable();
    bool disable();

  private:
    event::Loop *wp_loop_ = nullptr;    //! �¼�����

    ReceiveCallback         receive_cb_;
    WriteCompleteCallback   send_complete_cb_;
    ReadZeroCallback        read_zero_cb_;
    ErrorCallback           error_cb_;

    size_t  receive_threshold_ = 0;
};

}
}

#endif //TBOX_NETWORK_BUFFERED_FD_H_20171030
