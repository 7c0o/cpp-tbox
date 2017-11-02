#ifndef TBOX_NETWORK_BUFFERED_FD_H_20171030
#define TBOX_NETWORK_BUFFERED_FD_H_20171030

#include <functional>
#include <tbox/event/forward.h>
#include <tbox/base/defines.h>

#include "byte_stream.h"
#include "fd.h"
#include "buffer.h"

namespace tbox {
namespace network {

class BufferedFd : public ByteStream {
  public:
    explicit BufferedFd(event::Loop *wp_loop);
    virtual ~BufferedFd();

    NONCOPYABLE(BufferedFd);

  public:
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


    //! ��������˵�ǰ���ݷ���ʱ�Ļص�����
    void setSendCompleteCallback(const WriteCompleteCallback &func) { send_complete_cb_ = func; }
    //! ���õ�����0�ֽ�����ʱ�ص�����
    void setReadZeroCallback(const ReadZeroCallback &func) { read_zero_cb_ = func; }
    //! ���õ���������ʱ�Ļص�����
    void setErrorCallback(const ErrorCallback &func) { error_cb_ = func; }

    //! ʵ�� ByteStream �Ľӿ�
    virtual void setReceiveCallback(const ReceiveCallback &func, size_t threshold) override;
    virtual bool send(const void *data_ptr, size_t data_size) override;
    virtual void bind(ByteStream *receiver) override { wp_receiver_ = receiver; }
    virtual void unbind() override { wp_receiver_ = nullptr; }

    //! ������ر��ڲ��¼���������
    bool enable();
    bool disable();

  private:
    void onReadCallback(short);
    void onWriteCallback(short);

  private:
    event::Loop *wp_loop_ = nullptr;    //! �¼�����

    enum class State {
        kEmpty,     //! δ��ʼ��
        kInited,    //! �ѳ�ʼ��
        kRunning    //! ��������
    };
    State state_ = State::kEmpty;

    Fd fd_;

    event::FdItem *sp_read_event_  = nullptr;
    event::FdItem *sp_write_event_ = nullptr;

    Buffer send_buff_;
    Buffer recv_buff_;

    ReceiveCallback         receive_cb_;
    WriteCompleteCallback   send_complete_cb_;
    ReadZeroCallback        read_zero_cb_;
    ErrorCallback           error_cb_;

    ByteStream  *wp_receiver_ = nullptr;

    size_t  receive_threshold_ = 0;
    int     cb_level_ = 0;
};

}
}

#endif //TBOX_NETWORK_BUFFERED_FD_H_20171030
