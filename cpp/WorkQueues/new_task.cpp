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
  //创建一个task_queue的队列
  amqp_bytes_t queueName(amqp_cstring_bytes("work_task"));
  //passive：表示是否以被动模式进行声明。在这里，设置为false表示以非被动模式进行声明，即创建新队列或获取现有队列的信息。
  //durable：表示队列是否持久化。在这里，设置为false表示队列是非持久化的。（不会在服务器重启后保留）
  //exclusive：表示队列是否为独占队列。在这里，设置为true表示队列是独占的。（仅对声明这个队列的连接可见，并在连接关闭时删除）
  //auto_delete：表示队列是否在不再使用时自动删除。在这里，设置为true表示队列会在不再使用时自动删除。（服务器不会在消费者断开后删除它）
  amqp_queue_declare(conn, KChannel, queueName, false, /*durable*/ true, false, true, amqp_empty_table);

  std::string message("Hello World!");
  if (argc > 1)
  {
    std::stringstream s;
    //从 argv [1] 到 argv [argc]）复制到 std::ostream_iterator<const char*> 对象 s
    copy(&argv[1], &argv[argc], std::ostream_iterator<const char*>(s, " "));
    message = s.str();
  }

  amqp_basic_properties_t props;
  props._flags = AMQP_BASIC_DELIVERY_MODE_FLAG;
  ////将消息标记为持久。即使RabbitMQ重启，task_queue队列也不会丢失。
  props.delivery_mode = AMQP_DELIVERY_PERSISTENT;

  amqp_basic_publish(conn, KChannel, amqp_empty_bytes, /* routing key*/ queueName, false, false, &props, amqp_cstring_bytes(message.c_str()));
  std::cout << " [x] Sent " << message << std::endl;

  amqp_channel_close(conn, KChannel, AMQP_REPLY_SUCCESS);
  amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
  amqp_destroy_connection(conn);
  return 0;
}
