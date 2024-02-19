using System.Text;
using RabbitMQ.Client;

/// <summary>
/// NewTask_Worker。工作队列。工作队列背后的假设是，每个任务都只交付给一个工作
/// </summary>

var factory = new ConnectionFactory { HostName = "localhost" };
using var connection = factory.CreateConnection();
using var channel = connection.CreateModel();

//加入延迟等待
channel.QueueDeclare(queue: "work1",
                     durable: true,
                     exclusive: false,
                     autoDelete: false,
                     arguments: null);

var message = GetMessage(args);
var body = Encoding.UTF8.GetBytes(message);

var properties = channel.CreateBasicProperties();
//将消息标记为持久。.即使RabbitMQ重启，task_queue队列也不会丢失。
properties.Persistent = true;

channel.BasicPublish(exchange: string.Empty,
                     routingKey: "work1",
                     basicProperties: properties,
                     body: body);
Console.WriteLine($" [x] Sent {message}");

Console.WriteLine(" Press [enter] to exit.");
Console.ReadLine();

static string GetMessage(string[] args)
{
    return ((args.Length > 0) ? string.Join(" ", args) : "Hello World!");
}