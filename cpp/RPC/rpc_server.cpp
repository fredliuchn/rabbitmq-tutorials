#include <string.h>
#include <iostream>

#include <rabbitmq-c/amqp.h>
#include <rabbitmq-c/tcp_socket.h>

int fib(int n)
{
  if (n == 0)
    return 0;
  else if (n == 1)
    return 1;
  else
    return fib(n-1) + fib(n-2);
}

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
  //创建一个rpc_queue的交换器
  amqp_bytes_t queueName(amqp_cstring_bytes("rpc_queue"));
  //函数用于向 AMQP 服务器发送一个交换机声明请求，以创建一个新的交换机或获取现有交换机的信息。
  //direct意味着消息将被发送到路由键（routing key）匹配绑定键（binding key）的队列
  //exchange：要声明的交换机名称。
  //type：交换机的类型，可以是 "direct"、"fanout"、"topic" 或 "headers"。
  //passive：表示是否以被动模式进行声明。如果设置为非零值（例如 1），表示以被动模式进行声明，仅返回现有交换机的信息而不创建新交换机。如果设置为零值，则会创建新交换机或返回现有交换机的信息。
  //durable：表示交换机是否持久化。如果设置为非零值（例如 1），表示交换机是持久化的，即在服务器重启后仍然存在。如果设置为零值，则交换机是非持久化的，不会存储到磁盘上。
  //auto_delete：表示交换机是否在不再使用时自动删除。如果设置为非零值（例如 1），表示交换机在没有与之绑定的队列时会被自动删除。如果设置为零值，则交换机不会自动删除。
  //internal：表示交换机是否是内部的。如果设置为非零值（例如 1），表示交换机是内部的，只能通过其他交换机进行路由。如果设置为零值，则交换机可以直接接收消息。
  amqp_queue_declare(conn, KChannel, queueName, false, false, false, false, amqp_empty_table);
  //prefetch_size：预取大小，用于限制服务器一次性发送给消费者的消息的总大小。在这里，设置为0表示不限制消息的总大小。
  //prefetch_count：预取计数，用于限制服务器一次性发送给消费者的消息的数量。在这里，设置为1表示每次只获取一条消息。
  //global：用于指定是否将预取限制应用到整个通道（global=true）还是仅应用于特定消费者（global=false）。在这里，设置为0表示仅应用于特定消费者。
  amqp_basic_qos(conn, KChannel, 0, /*prefetch_count*/1, 0);
  amqp_basic_consume(conn, KChannel, queueName, amqp_empty_bytes, false, /* auto ack*/false, false, amqp_empty_table);
  std::cout << " [x] Awaiting RPC requests" << std::endl;

  for (;;)
  {
    amqp_maybe_release_buffers(conn);
    amqp_envelope_t envelope;
    amqp_rpc_reply_t res = amqp_consume_message(conn, &envelope, nullptr, 0);

    int n = *(int *)envelope.message.body.bytes;
    std::cout << " [.] fib(" <<  n << ")" << std::endl;
    int response = ++n;//fib(n);
    amqp_bytes_t response_;
    response_.bytes = &response;
    response_.len = sizeof(response);

    amqp_basic_properties_t props;
    //设置消息属性的标志位，这个标志位表示这个消息属性包含了关联ID(correlation_id)。
    props._flags = AMQP_BASIC_CORRELATION_ID_FLAG;
    //获取消息的通道，该通道将用于发送响应。
    props.correlation_id = envelope.message.properties.correlation_id;

    amqp_channel_t replyChannel = envelope.channel;
    amqp_basic_publish(conn, replyChannel, amqp_empty_bytes, /* routing key*/ envelope.message.properties.reply_to, false, false, &props, response_);
    amqp_basic_ack(conn, replyChannel, envelope.delivery_tag, false);
    
    amqp_destroy_envelope(&envelope);
  }

  amqp_channel_close(conn, KChannel, AMQP_REPLY_SUCCESS);
  amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
  amqp_destroy_connection(conn);

  return 0;
}
