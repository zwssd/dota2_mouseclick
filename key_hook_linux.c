
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
 
#include <linux/input.h>
#include <fcntl.h>
 
#define DBG_PRINTF(...)  
//#define DBG_PRINTF printf
 
int find_event(char *sub_buff)
{
	int iFd;
        FILE *tFp;
	struct stat tStat;
	char *command="cat /proc/bus/input/devices > log.txt" ;
	char *file_path="./log.txt";
 	int number;
	unsigned char  *file_buff;
	/* according  to mouse name find event number*/
	
	char *buff;
 
	tFp=fopen(file_path,"r+");    /* check if have log.txt file */
	
	if(NULL!=tFp) {
	  fclose(tFp);
	  system("rm log.txt");
	}
 
	system(command);
	/* 打开文件 */
	tFp = fopen(file_path, "r+");
	
	if (tFp == NULL) {
		DBG_PRINTF("can't open %s\n", file_path);
		return -1;
	}
	
        iFd = fileno(tFp);
 
	fstat(iFd, &tStat);
	 /* mmap the file to mem */
	file_buff = (unsigned char *)mmap(NULL , tStat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, iFd, 0);
 
	if (file_buff == (unsigned char *)-1) {
		DBG_PRINTF("mmap error!\n");
		return -1;
	}
 
	buff = strstr(file_buff,sub_buff);/* in the mem file_buff  find sub_buff name */
	
	if(NULL == buff) {
   		DBG_PRINTF("can't find %s\n",sub_buff);
   		munmap(file_buff, tStat.st_size);
		return -1;
	}
 
        number = *(buff+strlen(sub_buff)+6);/* 6== event */
	munmap(file_buff, tStat.st_size);
	fclose(tFp);
	return  number;
}
 
 
int main(int argc, char **argv)
{
	int number;
	char *keyword="Handlers=sysrq kbd";
	int keys_fd;
	struct input_event t;
	char dev_path[20];
	unsigned char ctrl_down_flag = 0, state = 0;
	unsigned func_enable = 1;
 
	number = find_event(keyword);
	number = number - '0';
    printf("%d\r\n",number);
	DBG_PRINTF("found keyboard device:/dev/input/event%d\n",number-'0');
	
	sprintf(dev_path,"/dev/input/event%d",number);
	printf("%s\r\n",dev_path);
	
	keys_fd = open(dev_path, O_RDONLY);
	if(keys_fd <= 0) {
		printf("open /dev/input/event%d device error!\n",number);
		return -1;
	}
 
	while(1) {
		if(read(keys_fd, &t, sizeof(t)) == sizeof(t)) {
			if(t.type == EV_KEY)
				if(t.value== 0 || t.value ==1 ) {
					DBG_PRINTF("key %d %s\n", t.code, (t.value) ? "Pressed" : "Released");
                    printf("key %d %s", t.code, (t.value) ? "Pressed" : "Released");
 
					if(t.code == 97 && t.value == 1) {
						ctrl_down_flag = 1;
						state = 1;
					} else if(state == 1) {
						state = 2;
					} else {
						ctrl_down_flag = 0;
						state = 0;
					}
					
					if(t.code == 102 && ctrl_down_flag)
						break;
 
					if(t.code == 102 && t.value == 0) {
						func_enable ^= 1;
						DBG_PRINTF("func_enable=%d \n",func_enable);
					}
					
					if(func_enable) {
						if(t.code == 70 && t.value == 1) {
							DBG_PRINTF("scp xxxx@192.168.x.x:/data/ \n");
							system("echo \"scp xxxx@192.168.x.x:/data/ \\c\" | xsel --clipboard");
						}
 
						if(t.code == 119 && t.value == 1) {
							DBG_PRINTF("xxxx@192.168.x.x\n");
							system("echo \"xxxx@192.168.x.x\\c\" | xsel --clipboard");
						}
					}
			}
		}
	}	
	close(keys_fd);
	return 0;
}
