#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

//多线程
#include<pthread.h>
 
#include <linux/input.h>
#include <fcntl.h>
 
#define DBG_PRINTF(...)  
//#define DBG_PRINTF printf

int mouse_enable = 0;
 
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

void thread_print(char *message){
   		printf("[info] %s\n",message);

}

//按键模拟，按键包含按下和松开两个环节
void simulate_key(int fd, int kval)
    {
        struct input_event event;
        gettimeofday(&event.time, 0);
        
        //按下kval键
        event.type = EV_KEY;
        event.value = 1;
        event.code = kval;
        write(fd, &event, sizeof(event));
        
        //同步，也就是把它报告给系统
        event.type = EV_SYN;
        event.value = 0;
        event.code = SYN_REPORT;
        write(fd, &event, sizeof(event));
    
        memset(&event, 0, sizeof(event));
        gettimeofday(&event.time, 0);
        
        //松开kval键
        event.type = EV_KEY;
        event.value = 0;
        event.code = kval;
        write(fd, &event, sizeof(event));
        
        //同步，也就是把它报告给系统
        event.type = EV_SYN;
        event.value = 0;
        event.code = SYN_REPORT;
        write(fd, &event, sizeof(event));
    }
    
    //鼠标移动模拟
    void simulate_mouse(int fd, int rel_x, int rel_y)
    {
        struct input_event event;
        gettimeofday(&event.time, 0);
    
        //x轴坐标的相对位移
        event.type = EV_REL;
        event.value = rel_x;
        event.code = REL_X;
        write(fd, &event, sizeof(event));
    
        //y轴坐标的相对位移
        event.type = EV_REL;
        event.value = rel_y;
        event.code = REL_Y;
        write(fd, &event, sizeof(event));
    
        //同步
        event.type = EV_SYN;
        event.value = 0;
        event.code = SYN_REPORT;
        write(fd, &event, sizeof(event));
    }


void monitor_keyboard(void)
{
	int number,mouse_number;
	char *keyword="Handlers=sysrq kbd";
	char *mouse="mouse1";
	int keys_fd;
	int mouse_fd;
	struct input_event t;
	char dev_path[20];
	char dev_mouse_path[20];
	unsigned char ctrl_down_flag = 0, state = 0;
	unsigned func_enable = 1;
 
	number = find_event(keyword);
	mouse_number = find_event(mouse);
    printf("mouse_number>>%d\r\n",mouse_number);
	number = number - '0';
	mouse_number = mouse_number - '0';
    printf("%d\r\n",number);
    printf("mouse_number>>%d\r\n",mouse_number);
	DBG_PRINTF("found keyboard device:/dev/input/event%d\n",number-'0');
	
	sprintf(dev_path,"/dev/input/event%d",number);
	sprintf(dev_mouse_path,"/dev/input/event%d",mouse_number);
	//设定本机的usb外接键盘
	//sprintf(dev_path,"/dev/input/event12");

	/*int i=0;
	while(i<=100)
    {
        ++i;
		printf(">>>>>%d",i);
    }*/

	printf("%s\r\n",dev_path);
	
	keys_fd = open(dev_path, O_RDONLY);
	if(keys_fd <= 0) {
		printf("open /dev/input/event%d device error!\n",number);
		return;
	}

	printf("%s\r\n",dev_mouse_path);
	// 请保证该设备节点有写的权限
    //mouse_fd = open(dev_mouse_path, O_RDWR);
    mouse_fd = open("/dev/input/event17", O_RDWR);
    if(mouse_fd <= 0) {
		printf("open /dev/input/event%d mouse device error!\n",mouse_number);
        return;
    }

	while(1) {
			thread_print("while_start\n");
		if(read(keys_fd, &t, sizeof(t)) == sizeof(t)) {
			if(t.type == EV_KEY)
				if(t.value== 0 || t.value ==1 ) {
					DBG_PRINTF("key %d %s\n", t.code, (t.value) ? "Pressed" : "Released");
                    printf("key %d %s", t.code, (t.value) ? "Pressed" : "Released");
					thread_print("|a|");
 
					//F10按下，控制鼠标开始
					if(t.code == 68 && t.value == 1) {
						mouse_enable = 1;
    					//pthread_exit("Time thread finished!\n");
						//ctrl_down_flag = 1;
						//state = 1;
					}

					//F12按下，控制鼠标结束
					if(t.code == 88 && t.value == 1) {
						mouse_enable = 0;
					}
					
					//if(t.code == 102 && ctrl_down_flag)
					//	break;
			}
		}
		if(mouse_enable==1){
			thread_print("mouse_start\n");
		//移动鼠标函数
		simulate_mouse(mouse_fd,50,50);
		//鼠标按键函数
        simulate_key(mouse_fd, BTN_LEFT);   //模拟按下鼠标左键
		}

	}
	close(keys_fd);
	close(mouse_fd);
}
 
 
int main(int argc, char **argv)
{
	/*int number;
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
	//设定本机的usb外接键盘
	//sprintf(dev_path,"/dev/input/event12");

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
	}	*/

	//开始多线程
    int ret;
    void *thread_result;
    pthread_t new_thread;

    ret=pthread_create(&new_thread,NULL,(void*)monitor_keyboard,NULL);
    if(ret!=0){
        perror("thread creation failed!\n");
        exit(EXIT_FAILURE);
    }

    printf("waiting for new thread....\n");
    ret=pthread_join(new_thread,&thread_result);
    if(ret!=0){
        perror("thread join failed!\n");
        exit(EXIT_FAILURE);
    }

    printf("thread joined, returned: %s\n",(char*)thread_result);

	return 0;
}