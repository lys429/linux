#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <sys/types.h>

#include <assert.h>
#include <getopt.h>                    
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>         
#include <linux/videodev2.h>
#include <jpeglib.h>
#include <linux/fb.h>




int IC_TTY_addToLibIc()
{
	printf("IC_TTY_addToLibIc");
	
	return 0;
}
                                                   //校验位
int IC_TTY_Option_Set(int fd,int nSpeed,int nBits,char nEvent,int nStop)//设置方向，
{
	struct termios newtio,oldtio;

	if(tcgetattr(fd,&oldtio)!=0)// 读取当前串口参数到名为oldtio的 termio 类型结构体中
	{
		printf("SetupSerial 1");
		return -1;
	}

	bzero(&newtio,sizeof(newtio));//bzero() 会将内存块（字符串）的前n个字节清零

	newtio.c_cflag |= CLOCAL |CREAD;//控制模式
	newtio.c_iflag &= ~CSIZE;//输入模式

	//设置fd


	switch(nBits)//根据所读的bits来选取寄存器
	{
		case 5:
			newtio.c_cflag |= CS5;
			break;
		case 6:
			newtio.c_cflag |= CS6;
			break;
		case 7:
			newtio.c_cflag |= CS7;
			break;
		case 8:
			newtio.c_cflag |=CS8;
			break;
		default:
			newtio.c_cflag |=CS8;//默认8位
			break;
	}

	switch(nEvent)//设置奇偶校验位，检查数据完整性
	{
		case 'o':
			newtio.c_cflag |= PARENB;
			newtio.c_cflag |=PARODD;
			newtio.c_iflag |=(INPCK | ISTRIP);
			break;

		case 'E': //even
			newtio.c_iflag |=(INPCK | ISTRIP);
			newtio.c_cflag |=PARENB;
			newtio.c_cflag &= ~PARODD;
			break;

		case 'N':
			newtio.c_cflag &= ~PARENB;
			break;
		default:
			newtio.c_cflag &= ~PARENB;
			break;

	}

	switch(nSpeed)//控制上下传输速率
	{  
		case 2400:
			cfsetispeed(&newtio,B2400);//输入
			cfsetospeed(&newtio,B2400);//输出
			break;

		case 4800:
			cfsetispeed(&newtio,B4800);
			cfsetospeed(&newtio,B4800);
			break;

		case 9600:
			cfsetispeed(&newtio,B9600);
			cfsetospeed(&newtio,B9600);
			break;

		case 57600:
			cfsetispeed(&newtio,B57600);
			cfsetospeed(&newtio,B57600);
			break;

		case 115200:
			cfsetispeed(&newtio,B115200);
			cfsetospeed(&newtio,B115200);
			break;

		case 460800:
			cfsetispeed(&newtio,B460800);
			cfsetospeed(&newtio,B460800);
			break;

		default:
			cfsetispeed(&newtio,B9600);
			cfsetospeed(&newtio,B9600);
			break;

	}
	/////////////////////////设置数据结束标志位////////////////
	if(nStop == 1) 
	{
		newtio.c_cflag &= ~CSTOPB;
	}
	else if(nStop == 2)
	{
		newtio.c_cflag |= CSTOPB;
	}
	/////////////////////////////
	
	//VTIME = 0，VMIN = 0，不管能否读取到数据，read都会立即返回。
	newtio.c_cc[VTIME] = 0; 
	newtio.c_cc[VMIN] = 0;

	tcflush(fd,TCIFLUSH);//TCIFLUSH 清除正收到的数据，且不会读取出来

	//TCSANOW 不等数据传输完毕就立即改变属性
	if((tcsetattr(fd,TCSANOW,&newtio))!=0)//入新的结构体
	{
		printf("com set error");
		return -1;
	}

	return 0;
}

                //蓝牙名字
int IC_TTY_Init(const char* pTtyName,int nSpeed,int nBits,char nEvent,int nStop)
{
	if (NULL == pTtyName)
	{
		printf("pTtyName is NULL");
		return -1;
	}
	
	int nComFd = 0;

	nComFd = open(pTtyName, O_RDWR);
	if (nComFd <= 0 )
	{
		printf("Couldn't open %s", pTtyName);
		
		return -2;
	}
	else
	{
		printf("open %s success!", pTtyName);	
	}


	IC_TTY_Option_Set(nComFd,nSpeed,nBits,nEvent,nStop);

//	fcntl( nComFd, F_SETFL, FNDELAY );

	return nComFd;
}

int IC_TTY_Destroy(int nComFd)
{
	if (nComFd > 0)
	{
		close(nComFd);
	}

	return 0;
}


void led_contrl(int fd, int ser_fd)
{
	char cmd;
	char led;
	int nRead;
	fb_read_bmp("./hello.bmp", 800, 600);
	while (1) {
		led = '1';
		write (fd, &led, 1);
		sleep(1);
		led = '2';
		write (fd, &led, 1);
		sleep(1);
		nRead = read( ser_fd, &cmd, 1);
		if (nRead == 1) {
			if ('5' == cmd) {
				led = '1';
				write (fd, &led, 1);
				fb_read_bmp("./welcome.bmp", 800, 600);
				break;
			}
		}
	}
}


void beep_contrl(int fd, int ser_fd)
{
	char cmd;
	char beep;
	int nRead;
	fb_read_bmp("./license.bmp", 800, 600);
	while (1) {
		beep = '1';
		write (fd, &beep, 1);
		usleep(3000);
		beep = '2';
		write (fd, &beep, 1);
		usleep(3000);
		nRead = read( ser_fd, &cmd, 1);
		if (nRead == 1) {
			if ('5' == cmd) {
				beep = '1';
				write (fd, &beep, 1);
				fb_read_bmp("./welcome.bmp", 800, 600);
				break;
			}
		}
	}
}

int main(int argc,char**argv)
{
	int nSenDevFd = 0;
	char cmd = 0;
	int err = 0;
	int fd = -1;
	int fd_beep = -1;
	int len;
	char led;
	if(argc!=2)
	{
		printf("Usage:[cmd] [dev]\n");
		exit(1);
	}
	nSenDevFd = IC_TTY_Init(argv[1], 9600, 8, 'N', 1);
	if (nSenDevFd < 0)
	{
		printf("error: open\n");
	}

	char szBuf[1024] = {0};
	int nRead = 0;


	fd = open("/dev/myled", O_RDWR);
	if (fd < 0) {
		perror("open:");
		exit(1);
	}
	fd_beep = open("/dev/mybeep", O_RDWR);
	if (fd < 0) {
		perror("open:");
		exit(1);
	}

	//初始化 FRAMEBUFFER, 在显示之前必须先调用这个函数
	err = fb_driver_init ();//驱动初始化
	if (err) 
		exit (0) ;
	fb_read_bmp("./welcome.bmp", 800, 600);
	while (1)
	{

		nRead = read( nSenDevFd, &cmd, 1);
		if (nRead == 1) {
			printf("cmd = %c\n", cmd);
			if ('1' == cmd)
				led_contrl(fd, nSenDevFd);
			if ('2' == cmd)
				fb_read_bmp("./leijun.bmp", 800, 600);
			if ('3' == cmd)	
				fb_read_bmp("./mayun.bmp", 800, 600);
			if ('4' == cmd) 
				beep_contrl(fd_beep, nSenDevFd);
			if ('5' == cmd)
				get_pic("./3.jpg");
			if ('6' == cmd)
				show_jpg("./3.jpg");
		}
		
	}
	return 0;
}
