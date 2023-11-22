#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <arpa/inet.h>
#include "bmp.h"
#include "linux/input.h"
#include "common.h"
#include "bmp.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <strings.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>

void music();
void picture();
int get_number();
#define REC_CMD  "arecord -d3 -c1 -r16000 -traw -fS16_LE cmd.pcm"
#define PCM_FILE "cmd.pcm"

int open_fifo() {
	if (access("tmp/fifo", F_OK))//判断fifo是否存在，不存在返回-1
	{
		mkfifo("/tmp/fifo", 0777);//创建fifo
	}
	 int f = open("/tmp/fifo", O_RDWR);
	if (f == -1)
	{
		printf("打开管道失败\n");
		return -1;
	}
	return f;
};

int open_file(char* x) {//打开一个文件，打开失败返回-1
	int fd = open(x, O_RDWR);   //以可读可写形式打开
	if (fd == -1) //判断打开文件是否成功
	{
		printf("Open fail\n");
		return -1;
	}
	return fd;
};

int read_file(int j, char* m) {//读一个文件，失败返回-1
	int n = read(j, m, 100);
	if (n == -1) //判断读文件是否成功
	{
		printf("Read fail\n");
		return -1;
	}
};
int write_file(int j, char* m) {//写一个文件，失败返回-1
	int n = write(j, m, 100);
	if (n == -1) //判断写文件是否成功
	{
		printf("Write fail\n");
		return -1;
	}

};

	int picture_show(int x, int y, char* bmpname)
	{
		
		int i, j;
		//打开bmp图片
		int bmp=open_file(bmpname);
		int fd = open_file("/dev/fb0");   //以可读可写形式打开led
		
		//建立映射
		int* p = mmap(NULL, 1024 * 600 * 4, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if (p == (int*)-1)
		{
			printf("reflect fail\n");
			return -1;
		}
		
		//读取头部信息
		struct bitmap_header head;
		struct bitmap_info  info;
		read(bmp, &head, sizeof(head));
		read(bmp, &info, sizeof(info));

		int w = info.width;//图片的宽
		int h = info.height;//图片的高
		char bmp_buf[w * h * 3];//bmp像素缓存
		//获取图片的像素信息
		read(bmp, bmp_buf, w * h * 3);
		//将bmp的像素信息放到lcd的像素点上
		for (i = 0; i < 600 - y && i < h; i++)
		{
			for (j = 0; j < 1024 - x && j < w; j++)
			{
				if (y + h - 1 - i < 599) {
					p[(y + h - 1 - i) * 1024 + j + x] = bmp_buf[w * i * 3 + j * 3] | bmp_buf[w * i * 3 + j * 3 + 1] << 8 | bmp_buf[w * i * 3 + j * 3 + 2] << 16;
				}
			}
		}
		//关闭图片
		munmap(p, 1024 * 600 * 4);
		close(bmp);
		close(fd);
		return 0;
};

	int white_show()//刷白
	{

		int buf [1024][600];
		int i, j;
		//打开bmp图片
		int fd = open_file("/dev/fb0");   //以可读可写形式打开led
		for (i = 0; i < 600; i++) {
			for (j = 0; j < 1024; j++) {
				buf[i][j] = 0xffffffff;

			}
		}
		write(fd, buf, 1024 * 600 * 4);
		close(fd);
		return 0;
	};

	void get_xy(int* x, int* y) {
		int ts = open_file("/dev/input/event1");
		struct input_event info;//获取触摸信息的结构体变量
		while (1)
		{
			read(ts, &info, sizeof(info));//读取输入信息
			if (info.type == EV_ABS)//绝对位移事件
			{
				if (info.code == ABS_X)//x轴事件
				{
					*x = info.value;//x的值
					printf("(%d,", info.value);
				}

				if (info.code == ABS_Y)//y轴事件
				{
					*y = info.value;
					printf("%d)\n", info.value);
				}

			}

			if (info.type == EV_KEY)//按键压力事件
			{
				if (info.code == BTN_TOUCH)//触摸按键
				{
					if (info.value == 0)//没有按下
					{
						break;
						
					}
				}
			}
		}
	}
	void picture() {       //相册功能部分代码
		char* picture[] = { "photo/002.bmp","photo/003.bmp", "photo/004.bmp", "photo/005.bmp", "photo/006.bmp" };
		int x;
		int y;
		int i = 0;
		
		picture_show(0, 0, "photo/white.bmp");
		picture_show(0, 500, "photo/last1.bmp");
		picture_show(924, 500, "photo/next1.bmp");
		picture_show(924, 0, "photo/play.bmp");
		picture_show(0, 0, "photo/x.bmp");
		picture_show(100, 0, picture[i]);
		while (1) {
			get_xy(&x, &y);
			if (x < 200 && y>500) {
				picture_show(100, 0, picture[i--]);

			}
			if (x > 824 && y > 500) {
				picture_show(100, 0, picture[i++]);
			}
			if (i >4) {
				i = 0;
			}
			if (i < 0) {
				i = 4;
			}
			if (x > 924 && y < 200) {
				int j = 0;
				while(j<5)
				{
					j++;
					picture_show(100, 0, picture[i++]);
					sleep(1);
					if (i > 4) {
						i = 0;
					}
					if (i < 0) {
						i = 4;
					}
				}
				
			}
			if (x < 100 && y < 100) {
				printf("exit\n");
				picture_show(0, 0, "photo/white.bmp");
				picture_show(0, 0, "photo/music.bmp");
				picture_show(0, 110, "photo/photo.bmp");
				break;
			}
		}
	}


	void set_cmd(int fifo, char* cmd)
	{
		write(fifo, cmd, strlen(cmd));
	}

	void play_music(char *music_name) {
		char s1 [200] = "mplayer -slave -quiet -input file=/tmp/fifo ";
		char s3 [] = " &";
		strcat(s1, music_name);
		strcat(s1, s3);
		system(s1);
	}

	void music() {     //音乐播放器部分代码
		int f = open_fifo();
		int x, y, i = 0;
		int index = 0, index1 = 0,sound=1;
		char* s1[] = { "music/1.mp3","music/2.mp3","music/3.mp3","music/4.mp3","music/5.mp3","music/6.mp3" };
		picture_show(0, 0, "photo/white.bmp");
		picture_show(924, 0, "photo/x.bmp");
		picture_show(0, 490, "photo/forward.bmp");
		picture_show(0, 550, "photo/backward.bmp");
		picture_show(300, 500, "photo/mute.bmp");
		picture_show(924, 500, "photo/sounddown .bmp");
		picture_show(824, 500, "photo/soundup.bmp");
		picture_show(487, 550, "photo/play1.bmp");
		picture_show(427, 550, "photo/last.bmp");
		picture_show(547, 550, "photo/next.bmp");
		picture_show(0, 0, "photo/x1.bmp");
		picture_show(250, 924, "photo/assistant.bmp");
		while (1) {
			get_xy(&x, &y);
			if (x > 487 && x < 536 && y>550) {         //播放（暂停）按钮
				if (index == 0) {
					play_music(s1[i]);
					picture_show(487, 550, "photo/stop1.bmp");
					index = 1;
				}
				if(index == 1) {
					index1++;
					if (index1 % 2 == 0) {
						set_cmd(f, "pause\n");
						picture_show(487, 550, "photo/play1.bmp");
					}
					else {
						set_cmd(f, "pause\n");
						picture_show(487, 550, "photo/stop1.bmp");
					}
				}
			
			}
			if (x > 427 && x < 476 && y>550) {     //上一首
				system("killall -9 mplayer");
				play_music(s1[i--]);
			}
			if (x > 547 && x < 596 && y>550) {   //下一首
				system("killall -9 mplayer");
				play_music(s1[i++]);
			}
			if (x < 100 && y < 100) {   //返回上一级
				printf("exit\n");
				picture_show(0, 0, "photo/white.bmp");
				picture_show(0, 0, "photo/music.bmp");
				picture_show(0, 110, "photo/photo.bmp");
				break;
			}
			if (x > 924 && y < 200) {  //停止播放
				system("killall -9 mplayer");
			}
			if (x < 100 && y > 490&&y<540) {  //快进
				set_cmd(f,"seek 10\n");
			}
			if (x < 100 && y > 550) {    //快退
				set_cmd(f,"seek -10\n");
			}
			if (x > 300 && x < 400 && y > 500) {    //声音
				if (sound == 1) {
					set_cmd(f, "mute 1 \n");
					sound = 0;
					picture_show(300, 500, "photo/sound.bmp");
				}
				else {
					set_cmd(f, "mute 0 \n");
					sound = 1;
					picture_show(300, 500, "photo/mute.bmp");
				}
			}
			if (x >924  && y > 550) {    //声音-5
				set_cmd(f, "volume -5\n");
			}
			if (x > 824 &&x<924&& y > 500) {    //声音+5
				set_cmd(f, "volume 5\n");
			}
			if (x > 250 && x < 350 && y > 924) {    //声音+5
				
			}

			if (i > 5) {
				i = 0;
			}
			if (i < 0) {
				i = 5;
			}
		}
		close(f);
	}



	void show_init() {
		int x, y;
		picture_show(0, 0, "photo/white.bmp");
		picture_show(0, 0, "photo/music.bmp");
		picture_show(0, 110, "photo/photo.bmp");
		while (1) {
			get_xy(&x, &y);
			if (x < 200 && y < 100) {
				printf("music\n");
				music();
			}
			if (x < 100 && y > 110 && y < 210) {
				printf("photo\n");
				picture();
			}

		}
	}
