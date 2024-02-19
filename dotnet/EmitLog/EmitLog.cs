using System.Text;
using RabbitMQ.Client;

/// <summary>
/// 发布/订阅。使用广播形式。向多个消费者传递一条消息
/// </summary>
var factory = new ConnectionFactory { HostName = "localhost" };
using var connection = factory.CreateConnection();
using var channel = connection.CreateModel();

/// <summary>
/// ExchangeType.Fanout 将接收到的所有消息广播到它所知道的所有队列
/// </summary>
channel.ExchangeDeclare(exchange: "logs", type: ExchangeType.Fanout);

var message = GetMessage(args);
var body = Encoding.UTF8.GetBytes(message);
//Fanout不使用routingKey,将消息发送到exchange上的所有队列
channel.BasicPublish(exchange: "logs",
                     routingKey: string.Empty,
                     basicProperties: null,
                     body: body);
Console.WriteLine($" [x] Sent {message}");

Console.WriteLine(" Press [enter] to exit.");
Console.ReadLine();

static string GetMessage(string[] args)
{
    return ((args.Length > 0) ? string.Join(" ", args) : "info: Hello World!");
}