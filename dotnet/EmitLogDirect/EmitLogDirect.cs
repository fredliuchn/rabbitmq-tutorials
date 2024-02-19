using System.Text;
using RabbitMQ.Client;

/// <summary>
/// ·�ɡ�ֻ������Ϣ��һ���Ӽ���
/// </summary>
var factory = new ConnectionFactory { HostName = "localhost" };
using var connection = factory.CreateConnection();
using var channel = connection.CreateModel();

//ֱ�ӽ�����ֱ�ӽ��������·���㷨�ܼ�--��Ϣ����󶨼�����Ϣ��·�ɼ���ȫƥ��Ķ��С�
channel.ExchangeDeclare(exchange: "direct_logs", type: ExchangeType.Direct);

var severity = (args.Length > 0) ? args[0] : "info";
var message = (args.Length > 1)
                    ? string.Join(" ", args.Skip(1).ToArray())
                    : "Hello World!";
var body = Encoding.UTF8.GetBytes(message);
//����routingKey���͵�exchange�ϵ�ĳһ������
channel.BasicPublish(exchange: "direct_logs",
                     routingKey: severity,
                     basicProperties: null,
                     body: body);
Console.WriteLine($" [x] Sent '{severity}':'{message}'");

Console.WriteLine(" Press [enter] to exit.");
Console.ReadLine();