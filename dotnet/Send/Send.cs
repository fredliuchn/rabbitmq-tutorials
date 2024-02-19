using System.Text;
using RabbitMQ.Client;

//������������������
var factory = new ConnectionFactory { HostName = "localhost" };
using var connection = factory.CreateConnection();
//����һ��ͨ��
using var channel = connection.CreateModel();
//�����ݵȶ��С��ݵȣ�idempotent��idempotence����һ����ѧ������ѧ��������ڳ�������У���f(f(x)) = f(x)���򵥵���˵����һ���������ִ�в����Ľ����һ��ִ�в����Ľ��һ�¡�
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