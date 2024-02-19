#include <iostream>
#include <string.h>
#include <algorithm>
#include <thread>
#include <chrono>

#include <rabbitmq-c/amqp.h>
#include <rabbitmq-c/tcp_socket.h>

int main(int argc, char const *const *argv)
{
  //创建通道
  amqp_connection_state_t conn = amqp_new_connection();
  //创建连接
  amqp_socket_t *socket = amqp_tcp_socket_new(conn);
  amqp_socket_open(socket, "localhost", AMQP_PROTOCOL_PORT);
  //虚拟主机。要在代理上连接到的虚拟主机.默认值为“/”。
  //设置了AMQ默认的帧大小。
  //连接通道数的限制,0是不做限制。
  //代理请求的心跳帧之间的秒数。值为 0 将禁用检测信号。
  //身份验证方法。后面两个参数跟着身份和密钥。
  amqp_login(conn, "/", 0, AMQP_DEFAULT_FRAME_SIZE, 0, AMQP_SASL_METHOD_PLAIN, "guest", "guest");
  //通道编号
  const amqp_channel_t KChannel = 1;
  amqp_channel_open(conn, KChannel);
  //创建一个direct_logs的交换器
  amqp_bytes_t exchangeName(amqp_cstring_bytes("direct_logs"));
  //函数用于向 AMQP 服务器发送一个交换机声明请求，以创建一个新的交换机或获取现有交换机的信息。
  //direct意味着消息将被发送到路由键（routing key）匹配绑定键（binding key）的队列
  //exchange：要声明的交换机名称。
  //type：交换机的类型，可以是 "direct"、"fanout"、"topic" 或 "headers"。
  //passive：表示是否以被动模式进行声明。如果设置为非零值（例如 1），表示以被动模式进行声明，仅返回现有交换机的信息而不创建新交换机。如果设置为零值，则会创建新交换机或返回现有交换机的信息。
  //durable：表示交换机是否持久化。如果设置为非零值（例如 1），表示交换机是持久化的，即在服务器重启后仍然存在。如果设置为零值，则交换机是非持久化的，不会存储到磁盘上。
  //auto_delete：表示交换机是否在不再使用时自动删除。如果设置为非零值（例如 1），表示交换机在没有与之绑定的队列时会被自动删除。如果设置为零值，则交换机不会自动删除。
  //internal：表示交换机是否是内部的。如果设置为非零值（例如 1），表示交换机是内部的，只能通过其他交换机进行路由。如果设置为零值，则交换机可以直接接收消息。
  amqp_exchange_declare(conn, KChannel, exchangeName, amqp_cstring_bytes("direct"),
                        false, false, false, false, amqp_empty_table);
  //函数用于向AMQP服务器发送队列声明请求，并根据提供的参数创建一个新的消息队列或获取现有队列的信息。
  //passive：表示是否以被动模式进行声明。在这里，设置为false表示以非被动模式进行声明，即创建新队列或获取现有队列的信息。
  //durable：表示队列是否持久化。在这里，设置为false表示队列是非持久化的。（不会在服务器重启后保留）
  //exclusive：表示队列是否为独占队列。在这里，设置为true表示队列是独占的。（仅对声明这个队列的连接可见，并在连接关闭时删除）
  //auto_delete：表示队列是否在不再使用时自动删除。在这里，设置为true表示队列会在不再使用时自动删除。（服务器不会在消费者断开后删除它）
  amqp_queue_declare_ok_t *r = amqp_queue_declare(conn, KChannel, amqp_empty_bytes, false, false, /*exclusive*/ true, false, amqp_empty_table);
  //新的字节序列分配内存并复制给定的字节序列，这里是队列名称。
  amqp_bytes_t queueName = amqp_bytes_malloc_dup(r->queue);

  if (argc > 1)
  {
    for (int i = 1; i < argc; ++i)
      amqp_queue_bind(conn, KChannel, queueName, exchangeName, amqp_cstring_bytes(argv[i]), amqp_empty_table);
  }
  else
  {
    std::cout << "Usage: " << argv[0] << " [info] [warning] [error]" << std::endl;
    return 1;
  }

  std::cout << "[*] Waiting for logs. To exit press CTRL+C'" << std::endl;
  //函数用于向 AMQP 服务器发送一个消费者订阅请求，并开始接收指定队列中的消息。
  amqp_basic_consume(conn, KChannel, queueName, amqp_empty_bytes, false, /* auto ack*/ true, false, amqp_empty_table);

  for (;;)
  {
    amqp_maybe_release_buffers(conn);
    amqp_envelope_t envelope;
    amqp_consume_message(conn, &envelope, nullptr, 0);

    std::string message((char *)envelope.message.body.bytes, (int)envelope.message.body.len);
    std::cout << " [x] Received " << std::string((char*)envelope.routing_key.bytes, (int)envelope.routing_key.len) << ":" << message << std::endl;

    amqp_destroy_envelope(&envelope);
  }

  amqp_bytes_free(queueName);
  amqp_channel_close(conn, KChannel, AMQP_REPLY_SUCCESS);
  amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
  amqp_destroy_connection(conn);

  return 0;
}
