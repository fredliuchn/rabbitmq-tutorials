using System.Text;
using RabbitMQ.Client;

/// <summary>
/// NewTask_Worker���������С��������б���ļ����ǣ�ÿ������ֻ������һ������
/// </summary>

var factory = new ConnectionFactory { HostName = "localhost" };
using var connection = factory.CreateConnection();
using var channel = connection.CreateModel();

//�����ӳٵȴ�
channel.QueueDeclare(queue: "work1",
                     durable: true,
                     exclusive: false,
                     autoDelete: false,
                     arguments: null);

var message = GetMessage(args);
var body = Encoding.UTF8.GetBytes(message);

var properties = channel.CreateBasicProperties();
//����Ϣ���Ϊ�־á�.��ʹRabbitMQ������task_queue����Ҳ���ᶪʧ��
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