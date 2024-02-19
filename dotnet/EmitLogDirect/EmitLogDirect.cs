using System.Text;
using RabbitMQ.Client;

/// <summary>
/// 路由。只订阅消息的一个子集。
/// </summary>
var factory = new ConnectionFactory { HostName = "localhost" };
using var connection = factory.CreateConnection();
using var channel = connection.CreateModel();

//直接交换。直接交换背后的路由算法很简单--消息进入绑定键与消息的路由键完全匹配的队列。
channel.ExchangeDeclare(exchange: "direct_logs", type: ExchangeType.Direct);

var severity = (args.Length > 0) ? args[0] : "info";
var message = (args.Length > 1)
                    ? string.Join(" ", args.Skip(1).ToArray())
                    : "Hello World!";
var body = Encoding.UTF8.GetBytes(message);
//根据routingKey发送到exchange上的某一个队列
channel.BasicPublish(exchange: "direct_logs",
                     routingKey: severity,
                     basicProperties: null,
                     body: body);
Console.WriteLine($" [x] Sent '{severity}':'{message}'");

Console.WriteLine(" Press [enter] to exit.");
Console.ReadLine();