using System.Text;
using RabbitMQ.Client;
using RabbitMQ.Client.Events;

/// <summary>
/// NewTask_Worker���������С��������б���ļ����ǣ�ÿ������ֻ������һ������
/// </summary>

var factory = new ConnectionFactory { HostName = "localhost" };
using var connection = factory.CreateConnection();
using var channel = connection.CreateModel();

//�����ӳٵȴ�
channel.QueueDeclare(queue: "task_queue",
                     durable: true,
                     exclusive: false,
                     autoDelete: false,
                     arguments: null);

channel.BasicQos(prefetchSize: 0, prefetchCount: 1, global: false);

Console.WriteLine(" [*] Waiting for messages.");

var consumer = new EventingBasicConsumer(channel);
consumer.Received += (model, ea) =>
{
    byte[] body = ea.Body.ToArray();
    var message = Encoding.UTF8.GetString(body);
    Console.WriteLine($" [x] Received {message}");

    int dots = message.Split('.').Length - 1;
    Thread.Sleep(dots * 1000);

    Console.WriteLine(" [x] Done");

    //�����߷���һ��ack��ȷ�ϣ�������RabbitMQһ���ض�����Ϣ�Ѿ������ա�����RabbitMQ��������ɾ������
    channel.BasicAck(deliveryTag: ea.DeliveryTag, multiple: false);
};
//ȡ����Ϣ���Զ�ȷ��
channel.BasicConsume(queue: "task_queue",
                     autoAck: false,
                     consumer: consumer);

Console.WriteLine(" Press [enter] to exit.");
Console.ReadLine();