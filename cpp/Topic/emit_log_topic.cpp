#include <string.h>
#include <iostream>
#include <sstream>
#include <iterator>

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
  //创建一个topic_logs的交换器
  amqp_bytes_t exchangeName(amqp_cstring_bytes("topic_logs"));
  //direct意味着消息将被发送到路由键（routing key）匹配绑定键（binding key）的队列
  //exchange：要声明的交换机名称。
  //type：交换机的类型，可以是 "direct"、"fanout"、"topic" 或 "headers"。
  //passive：表示是否以被动模式进行声明。如果设置为非零值（例如 1），表示以被动模式进行声明，仅返回现有交换机的信息而不创建新交换机。如果设置为零值，则会创建新交换机或返回现有交换机的信息。
  //durable：表示交换机是否持久化。如果设置为非零值（例如 1），表示交换机是持久化的，即在服务器重启后仍然存在。如果设置为零值，则交换机是非持久化的，不会存储到磁盘上。
  //auto_delete：表示交换机是否在不再使用时自动删除。如果设置为非零值（例如 1），表示交换机在没有与之绑定的队列时会被自动删除。如果设置为零值，则交换机不会自动删除。
  //internal：表示交换机是否是内部的。如果设置为非零值（例如 1），表示交换机是内部的，只能通过其他交换机进行路由。如果设置为零值，则交换机可以直接接收消息。
  amqp_exchange_declare(conn, KChannel, exchangeName, amqp_cstring_bytes("topic"), 
                        false, false, false, false, amqp_empty_table);

  std::string routing_key = argc > 2 ? argv[1] : "anonymous.info";
  std::string message(" Hello World!");
  if (argc > 2)
  {
    std::stringstream s;
    copy(&argv[2], &argv[argc], std::ostream_iterator<const char*>(s, " "));
    message = s.str();
  }

  amqp_basic_publish(conn, KChannel, exchangeName, amqp_cstring_bytes(routing_key.c_str()), false, false, nullptr, amqp_cstring_bytes(message.c_str()));
  std::cout << " [x] Sent " << routing_key << ":" << message << std::endl;

  amqp_channel_close(conn, KChannel, AMQP_REPLY_SUCCESS);
  amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
  amqp_destroy_connection(conn);
  return 0;
}
