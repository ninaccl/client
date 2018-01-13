#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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

#define BUFSIZE 512

#define MOTOR_GO_FORWARD   digitalWrite(1,HIGH);digitalWrite(4,LOW);digitalWrite(5,HIGH);digitalWrite(6,LOW)
#define MOTOR_GO_BACK	   digitalWrite(4,HIGH);digitalWrite(1,LOW);digitalWrite(6,HIGH);digitalWrite(5,LOW)
#define MOTOR_GO_RIGHT	   digitalWrite(1,HIGH);digitalWrite(4,LOW);digitalWrite(6,HIGH);digitalWrite(5,LOW)
#define MOTOR_GO_LEFT	   digitalWrite(4,HIGH);digitalWrite(1,LOW);digitalWrite(5,HIGH);digitalWrite(6,LOW)
#define MOTOR_GO_STOP	   digitalWrite(1, LOW);digitalWrite(4,LOW);digitalWrite(5, LOW);digitalWrite(6,LOW)

int main(int argc, char *argv[])
{
	struct timeval tv;
	char *servIP = argv[1];
	in_port_t servPort = atoi(argv[2]);
	int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	int z = -1;

	/*RPI*/
	wiringPiSetup();
	/*WiringPi GPIO*/
	pinMode(1, OUTPUT);	//IN1
	pinMode(4, OUTPUT);	//IN2
	pinMode(5, OUTPUT);	//IN3
	pinMode(6, OUTPUT);	//IN4

	pinMode(3, OUTPUT);	//beed

	/*Init output*/
	digitalWrite(1, HIGH);
	digitalWrite(4, HIGH);
	digitalWrite(5, HIGH);
	digitalWrite(6, HIGH);

	digitalWrite(1, HIGH);

	if (argc != 3)
	{
		printf("Please add servIP or servPort!");
		exit(1);
	}
	if (servPort<8000)
	{
		printf("Enter Error");
		exit(1);
	}
	if (sockfd < 0)
	{
		printf("socket error");
		exit(1);
	}
	struct sockaddr_in servAddr;
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = inet_addr(servIP);//inet_addr函数将用点分十进制字符串表示的IPv4地址转化为用网络字节序整数表示的IPv4地址 
	servAddr.sin_port = htons(servPort);

	int connectfd=connect(sockfd, (struct sockaddr *)&servAddr, sizeof(servAddr));
	if (connectfd < 0)
	{
		printf("connect error");
		exit(1);
	}
	printf("connecting to the server...\n");
	char buf[BUFSIZE] = { 0xff, 0x00, 0x00, 0x00, 0xff };
	char backbuf[BUFSIZE] = { 0xff, 0x00, 0x00, 0x00, 0xff };
	while (1)
	{
		if ((z = read(sockfd, buf, sizeof buf)) > 0)//读取传输的数据，返回数据长度
		{

			buf[z] = '\0';
			if (z == 5)
			{
				if (buf[1] == 0x00)
				{
					switch (buf[2])
					{
					case 0x01:MOTOR_GO_FORWARD; printf("forward\n"); backbuf = "go forward"; break;
					case 0x02:MOTOR_GO_BACK;    printf("back\n"); backbuf = "go back"; break;
					case 0x03:MOTOR_GO_LEFT;    printf("left\n"); backbuf = "go left"; break;
					case 0x04:MOTOR_GO_RIGHT;   printf("right\n"); backbuf = "go right"; break;
					case 0x00:MOTOR_GO_STOP;    printf("stop\n"); backbuf = "go stop"; break;
					default: break;
					}
					digitalWrite(3, HIGH);
				}
				else
				{
					digitalWrite(3, LOW);
					MOTOR_GO_STOP;
				}
			}
			else if (z == 6)
			{
				if (buf[2] == 0x00)
				{
					switch (buf[3])
					{
					case 0x01:MOTOR_GO_FORWARD; printf("forward\n"); backbuf = "go forward"; break;
					case 0x02:MOTOR_GO_BACK;    printf("back\n"); backbuf = "go back"; break;
					case 0x03:MOTOR_GO_LEFT;    printf("left\n"); backbuf = "go left"; break;
					case 0x04:MOTOR_GO_RIGHT;   printf("right\n"); backbuf = "go right"; break;
					case 0x00:MOTOR_GO_STOP;    printf("stop\n"); backbuf = "go stop"; break;
					default: break;
					}
					digitalWrite(3, HIGH);
				}
				else
				{
					digitalWrite(3, LOW);
					MOTOR_GO_STOP;
				}
			}
			else
			{
				digitalWrite(3, LOW);
				MOTOR_GO_STOP;
			}

		}
		else if (z == 0)
		{
			printf("read is done\n");
			break;
		}
		else
		{
			printf("read error");
			continue;
		}
		write(sockfd, backbuf, strlen(backbuf));
	}
	close(sockfd);
	return 0;
}

