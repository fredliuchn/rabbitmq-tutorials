#include <string.h>
#include <iostream>

#include <rabbitmq-c/amqp.h>
#include <rabbitmq-c/tcp_socket.h>

class FibonacciRpcClient
{
public:
  FibonacciRpcClient()
  {
    //创建通道
    m_conn = amqp_new_connection();
    //创建连接
    amqp_socket_t *socket = amqp_tcp_socket_new(m_conn);
    amqp_socket_open(socket, "localhost", AMQP_PROTOCOL_PORT);
    //虚拟主机。要在代理上连接到的虚拟主机.默认值为“/”。
    //设置了AMQ默认的帧大小。
    //连接通道数的限制,0是不做限制。
    //代理请求的心跳帧之间的秒数。值为 0 将禁用检测信号。
    //身份验证方法。后面两个参数跟着身份和密钥。
    amqp_login(m_conn, "/", 0, AMQP_DEFAULT_FRAME_SIZE, 0, AMQP_SASL_METHOD_PLAIN, "guest", "guest");

    amqp_channel_open(m_conn, KChannel);
    //函数用于向 AMQP 服务器发送一个交换机声明请求，以创建一个新的交换机或获取现有交换机的信息。
    //direct意味着消息将被发送到路由键（routing key）匹配绑定键（binding key）的队列
    //exchange：要声明的交换机名称。
    //type：交换机的类型，可以是 "direct"、"fanout"、"topic" 或 "headers"。
    //passive：表示是否以被动模式进行声明。如果设置为非零值（例如 1），表示以被动模式进行声明，仅返回现有交换机的信息而不创建新交换机。如果设置为零值，则会创建新交换机或返回现有交换机的信息。
    //durable：表示交换机是否持久化。如果设置为非零值（例如 1），表示交换机是持久化的，即在服务器重启后仍然存在。如果设置为零值，则交换机是非持久化的，不会存储到磁盘上。
    //auto_delete：表示交换机是否在不再使用时自动删除。如果设置为非零值（例如 1），表示交换机在没有与之绑定的队列时会被自动删除。如果设置为零值，则交换机不会自动删除。
    //internal：表示交换机是否是内部的。如果设置为非零值（例如 1），表示交换机是内部的，只能通过其他交换机进行路由。如果设置为零值，则交换机可以直接接收消息。
    amqp_queue_declare_ok_t *r = amqp_queue_declare(m_conn, KChannel, amqp_empty_bytes, false, false, false, /*auto delete*/true, amqp_empty_table);
    m_callbackQueue = amqp_bytes_malloc_dup(r->queue);
  }

  int call(int n)
  {
    m_corr_id = std::to_string(++m_requestCount);

    amqp_basic_properties_t props;
    props._flags = AMQP_BASIC_CORRELATION_ID_FLAG | AMQP_BASIC_REPLY_TO_FLAG;
    props.correlation_id = amqp_cstring_bytes(m_corr_id.c_str());
    props.reply_to = m_callbackQueue;

    amqp_bytes_t n_;
    n_.bytes = &n;
    n_.len = sizeof(n);
    amqp_basic_publish(m_conn, KChannel, amqp_empty_bytes, /* routing key*/ amqp_cstring_bytes("rpc_queue"), false, false, &props, n_);

    amqp_basic_consume(m_conn, KChannel, m_callbackQueue, amqp_empty_bytes, false, /* auto ack*/ true, false, amqp_empty_table);

    int response = 0;
    bool keepProcessing = true;
    while (keepProcessing)
    {
      amqp_maybe_release_buffers(m_conn);
      amqp_envelope_t envelope;
      amqp_consume_message(m_conn, &envelope, nullptr, 0);

      std::string correlation_id((char *)envelope.message.properties.correlation_id.bytes, (int)envelope.message.properties.correlation_id.len);
      if (correlation_id == m_corr_id)
      {
        response = *(int *)envelope.message.body.bytes;
        keepProcessing = false;
      }

      amqp_destroy_envelope(&envelope);
    }

    return response;
  }

  ~FibonacciRpcClient()
  {
    amqp_bytes_free(m_callbackQueue);
    amqp_channel_close(m_conn, KChannel, AMQP_REPLY_SUCCESS);
    amqp_connection_close(m_conn, AMQP_REPLY_SUCCESS);
    amqp_destroy_connection(m_conn);
  }

private:
  amqp_connection_state_t m_conn;
  const amqp_channel_t KChannel = 1;
  amqp_bytes_t m_callbackQueue;
  //关联RPC的响应和请求
  std::string m_corr_id;
  //请求计数器
  int m_requestCount = 0;
};

int main(int argc, char const *const *argv)
{
  int n = 30;
  if (argc > 1)
  {
    n = std::stoi(argv[1]);
  }

  FibonacciRpcClient fibonacciRpcClient;
  std::cout << " [x] Requesting fib(" << n << ")" << std::endl;
  int response = fibonacciRpcClient.call(n);
  std::cout << " [.] Got " << response << std::endl;
  return 0;
}
