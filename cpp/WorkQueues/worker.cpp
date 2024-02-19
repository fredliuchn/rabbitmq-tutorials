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
  //创建一个task_queue的队列
  amqp_bytes_t queueName(amqp_cstring_bytes("work_task"));
  //passive：表示是否以被动模式进行声明。在这里，设置为false表示以非被动模式进行声明，即创建新队列或获取现有队列的信息。
  //durable：表示队列是否持久化。在这里，设置为false表示队列是非持久化的。（不会在服务器重启后保留）
  //exclusive：表示队列是否为独占队列。在这里，设置为true表示队列是独占的。（仅对声明这个队列的连接可见，并在连接关闭时删除）
  //auto_delete：表示队列是否在不再使用时自动删除。在这里，设置为true表示队列会在不再使用时自动删除。（服务器不会在消费者断开后删除它）
  amqp_queue_declare(conn, KChannel, queueName, false, /*durable*/ true, false, true, amqp_empty_table);
  //prefetch_size：预取大小，用于限制服务器一次性发送给消费者的消息的总大小。在这里，设置为0表示不限制消息的总大小。
  //prefetch_count：预取计数，用于限制服务器一次性发送给消费者的消息的数量。在这里，设置为1表示每次只获取一条消息。
  //global：用于指定是否将预取限制应用到整个通道（global=true）还是仅应用于特定消费者（global=false）。在这里，设置为0表示仅应用于特定消费者。
  amqp_basic_qos(conn, KChannel, 0, /*prefetch_count*/1, 0);
  amqp_basic_consume(conn, KChannel, queueName, amqp_empty_bytes, false, /* auto ack*/false, false, amqp_empty_table);

  for (;;)
  {
    amqp_maybe_release_buffers(conn);
    amqp_envelope_t envelope;
    amqp_consume_message(conn, &envelope, nullptr, 0);

    std::string message((char *)envelope.message.body.bytes,(int)envelope.message.body.len);
    std::cout << " [x] Received " <<  message << std::endl;
    const int seconds = std::count(std::begin(message), std::end(message), '.');
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
    std::cout << " [x] Done" << std::endl;
    //向rabbitMQ服务器发送一条确认消息，表示已经接受并处理这条消息
    //delivery_tag：交付标识（delivery tag），它是一个标识特定消息的整数值
    amqp_basic_ack(conn, KChannel, envelope.delivery_tag, false);
    amqp_destroy_envelope(&envelope);
  }

  amqp_channel_close(conn, KChannel, AMQP_REPLY_SUCCESS);
  amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
  amqp_destroy_connection(conn);

  return 0;
}
