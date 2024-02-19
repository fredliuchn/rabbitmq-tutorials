using System.Text;
using RabbitMQ.Client;

/// <summary>
/// ����/���ġ�ʹ�ù㲥��ʽ�����������ߴ���һ����Ϣ
/// </summary>
var factory = new ConnectionFactory { HostName = "localhost" };
using var connection = factory.CreateConnection();
using var channel = connection.CreateModel();

/// <summary>
/// ExchangeType.Fanout �����յ���������Ϣ�㲥������֪�������ж���
/// </summary>
channel.ExchangeDeclare(exchange: "logs", type: ExchangeType.Fanout);

var message = GetMessage(args);
var body = Encoding.UTF8.GetBytes(message);
//Fanout��ʹ��routingKey,����Ϣ���͵�exchange�ϵ����ж���
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