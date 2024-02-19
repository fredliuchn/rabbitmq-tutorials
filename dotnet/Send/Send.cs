using System.Text;
using RabbitMQ.Client;

//创建到服务器的连接
var factory = new ConnectionFactory { HostName = "localhost" };
using var connection = factory.CreateConnection();
//创建一个通道
using var channel = connection.CreateModel();
//创建幂等队列。幂等（idempotent、idempotence）是一个数学与计算机学概念，常见于抽象代数中，即f(f(x)) = f(x)。简单的来说就是一个操作多次执行产生的结果与一次执行产生的结果一致。
channel.QueueDeclare(queue: "hello",
                     durable: false,
                     exclusive: false,
                     autoDelete: false,
                     arguments: null);

var message = "Hello World!";
var body = Encoding.UTF8.GetBytes(message);

channel.BasicPublish(exchange: string.Empty,
                     routingKey: "hello",
                     basicProperties: null,
                     body: body);
Console.WriteLine($" [x] Sent {message}");

Console.WriteLine(" Press [enter] to exit.");
Console.ReadLine();