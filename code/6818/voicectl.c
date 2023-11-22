#include "common.h"
#include "apart.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <strings.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>


#define REC_CMD  "arecord -d3 -c1 -r16000 -traw -fS16_LE cmd.pcm"
#define PCM_FILE "cmd.pcm"



int main(int argc, char const *argv[]) // ./wav2pcm ubuntu-IP
{
	int id_num=0,num,index=0;
	if(argc != 2)
	{
		printf("Usage: %s <ubuntu-IP>\n", argv[0]);
		exit(0);
	}
	
	//int sockfd = init_sock(argv[1]); //由命令行传入一个对方的IP 等效于socket+bind+listen+accept
	
	struct sockaddr_in sin;
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	//连接到虚拟机中
	bzero((char *) &sin, sizeof(sin));
	sin.sin_family = AF_INET;
	inet_pton(AF_INET, argv[1], &sin.sin_addr);
	sin.sin_port = htons(54321);
	
	connect(sockfd, (struct sockaddr *)&sin, sizeof(sin));
	printf("waiting for result...\n");
		
	while(1)
	{
		// 1，调用arecord来录一段音频
		printf("input num to start REC in 3s...\n");
		scanf("%d",&num);
		
		//启动录音
		system(REC_CMD);

		// 2，将录制好的PCM音频发送给语音识别引擎

		// 打开PCM文件
		int fd = open(PCM_FILE, O_RDONLY);

		// 取得PCM数据的大小
		off_t pcm_size = lseek(fd, 0, SEEK_END);
		lseek(fd, 0, SEEK_SET);

		// 读取PCM数据
		char *pcm = calloc(1, pcm_size);
		Read(fd, pcm, pcm_size);

	// 将PCM发送给语音识别引擎系统
		int m = Write(sockfd, pcm, pcm_size);
		printf("%d bytes has been write into socket!\n", m);

		free(pcm);
		// 3，等待对方回送识别结果（字符串ID）
		xmlChar *id = wait4id(sockfd);
		if(id == NULL)
			continue;
		
		//将字符串的id转化成为整形的id
		id_num=atoi((char *)id);
	

		printf("id: %d\n", id_num);
		if (id_num == 1&&index==0) {
			picture_show(0, 0, "photo/white.bmp");
			picture_show(0, 0, "photo/music.bmp");
			picture_show(0, 110, "photo/photo.bmp");
			index == 1;
		}
		if (id_num == 3) {
			music();
		}
		if (id_num == 2) {
			picture();
		}
		
		
	
	}
	
	
	close(sockfd);
	return 0;
}

