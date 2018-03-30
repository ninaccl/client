#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <netdb.h>
#include <time.h>
#include <wiringPi.h>

#define BUFSIZE 5 //��������С
#define RIGHT 26 //������output����GPIO.26

//ͨ�������ĸ������״̬����С��״̬
#define MOTOR_GO_FORWARD   digitalWrite(1,HIGH);digitalWrite(4,LOW);digitalWrite(5,HIGH);digitalWrite(6,LOW) 
#define MOTOR_GO_BACK	   digitalWrite(4,HIGH);digitalWrite(1,LOW);digitalWrite(6,HIGH);digitalWrite(5,LOW)
#define MOTOR_GO_RIGHT	   digitalWrite(1,HIGH);digitalWrite(4,LOW);digitalWrite(6,HIGH);digitalWrite(5,LOW)
#define MOTOR_GO_LEFT	   digitalWrite(4,HIGH);digitalWrite(1,LOW);digitalWrite(5,HIGH);digitalWrite(6,LOW)
#define MOTOR_GO_STOP	   digitalWrite(1, LOW);digitalWrite(4,LOW);digitalWrite(5, LOW);digitalWrite(6,LOW)

int main(int argc, char *argv[])
{
	struct timeval tv;//����ʱ����ṹtv
	char *servIP = argv[1];//��÷�����IP��ַ
	in_port_t servPort = atoi(argv[2]);//��÷������˿ں�

	//�����׽���
	int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	int z = -1;

	/*RPI*/
	wiringPiSetup();
	/*WiringPi GPIO*/
	pinMode(1, OUTPUT);	//IN1
	pinMode(4, OUTPUT);	//IN2
	pinMode(5, OUTPUT);	//IN3
	pinMode(6, OUTPUT);	//IN4
	pinMode(26, INPUT);

	/*Init output*/
	digitalWrite(1, HIGH);
	digitalWrite(4, HIGH);
	digitalWrite(5, HIGH);
	digitalWrite(6, HIGH);

	digitalWrite(1, HIGH);

	if (argc != 3)//��ϵͳ�����������������������趨
	{
		printf("Please add servIP or servPort!");
		exit(1);//ֱ���˳�����
	}
	if (servPort<8000)//��ϵͳ��������ȷ������������˳�����
	{
		printf("Enter Error");
		exit(1);
	}
	if (sockfd < 0)
	{
		printf("socket error");
		exit(1);
	}

	//���÷�������ַ
	struct sockaddr_in servAddr;
	memset(&servAddr, 0, sizeof(servAddr));//��addr���������㣬���������ýṹserver_addr������	
	servAddr.sin_family = AF_INET;//ָ����ַ��ΪAF_INET����ʾTCP/IPЭ��.
	servAddr.sin_addr.s_addr = inet_addr(servIP);//inet_addr�������õ��ʮ�����ַ�����ʾ��IPv4��ַת��Ϊ�������ֽ���������ʾ��IPv4��ַ 
	servAddr.sin_port = htons(servPort);//ָ���˿ں�

	//���ӷ�����
	int connectfd=connect(sockfd, (struct sockaddr *)&servAddr, sizeof(servAddr));
	if (connectfd < 0)
	{
		printf("connect error");
		exit(1);
	}
	printf("connecting to the server...\n");
	char buf[BUFSIZE]; //���岢��ʼ�����ռ�
	char backbuf[BUFSIZE]; //���岢��ʼ��д�ռ�
	memset(buf, 0, BUFSIZE);
	memset(backbuf, 0, BUFSIZE);
    int obstacleFlag=0;//����״̬���մ��������״̬
	int obstacle;
	fcntl(sockfd,F_SETFL,fcntl(sockfd,F_GETFL,0)|O_NONBLOCK);//���׽�������Ϊ������ģʽ
	while (1)
	{	
	//С�������ϰ���GPIO.26����͵�ƽ��ͷ����״̬���ظ�������
	if (obstacleFlag = 0)
	{
		obstacle = digitalRead(RIGHT);
		if (obstacle == LOW)
		{
			obstacleFlag = 2;
			MOTOR_GO_STOP;
			printf("obstacle\n"); 
			backbuf[1] = 0x05;
			write(sockfd, backbuf, 5);
			//usleep(500000);
		}
	}		
		if ((z = read(sockfd, buf, sizeof buf)) > 0)//��ȡ��������������ݣ��������ݳ���
		{
			if (z == 5)
			{
				if (buf[0] == 0x00)
				{
					switch (buf[1])
					{
					case 0x01:MOTOR_GO_FORWARD; printf("forward\n");memcpy(backbuf,buf,sizeof buf);break;
					case 0x02:MOTOR_GO_STOP;    printf("stop\n"); memcpy(backbuf,buf,sizeof buf);break;
					case 0x03:MOTOR_GO_LEFT;    printf("left\n"); memcpy(backbuf,buf,sizeof buf); break;
					case 0x04:MOTOR_GO_RIGHT;   printf("right\n"); memcpy(backbuf,buf,sizeof buf); break;
					default: break;
					}
					if (obstacleFlag)
					{
						obstacleFlag = obstacleFlag - 1;
					}
					digitalWrite(3, HIGH);
					write(sockfd, backbuf, 5);//ͷ������Ϣ���ظ�������
				}
			}
		}
		else if (z == 0)
		{
			printf("read is done\n");
			break;
		}
	}
	//�ر��׽���
	close(sockfd);
	return 0;
}
