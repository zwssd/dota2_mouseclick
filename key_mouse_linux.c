/************************************************************************
 *                      Linux 模拟 鼠标 键盘 事件
 * 说明：
 *     以前看到有些软件能够控制鼠标移动，键盘操作等功能，总想知道这些到底
 * 是怎么做到的，好像是2年前也尝试去做这件事，但那时候对知识的匮乏直接导致
 * 无法进行，早上突然想到这件事，于是又搜索了一下，鉴于目前经常接触Linux
 * 驱动，对这些东西的理解也就很容易。
 ***********************************************************************/
/**************************************************************************
一、参考文章：
    1. 交互系统的构建之（二）Linux下鼠标和键盘的模拟控制
        http://blog.csdn.net/zouxy09/article/details/7920253
    2. 在Linux中模拟击键和鼠标移动
        http://os.51cto.com/art/201409/450283.htm

二、cat /proc/bus/input/devices:
    ......
    I: Bus=0011 Vendor=0001 Product=0001 Version=ab41
    N: Name="AT Translated Set 2 keyboard"
    P: Phys=isa0060/serio0/input0
    S: Sysfs=/devices/platform/i8042/serio0/input/input1
    U: Uniq=
    H: Handlers=sysrq kbd event1        ---> keyboard
    B: PROP=0
    B: EV=120013
    B: KEY=402000000 3803078f800d001 feffffdfffefffff fffffffffffffffe
    B: MSC=10
    B: LED=7
    ......
    I: Bus=0003 Vendor=0e0f Product=0003 Version=0110
    N: Name="VMware VMware Virtual USB Mouse"
    P: Phys=usb-0000:02:00.0-1/input1
    S: Sysfs=/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb2/2-1/2-1:1.1/input/input3
    U: Uniq=
    H: Handlers=mouse1 event3           ---> mouse
    B: PROP=0
    B: EV=17
    B: KEY=ffff0000 0 0 0 0
    B: REL=143
    B: MSC=10
    ......

三、权限修改：
    myzr@myzr:/dev/input$ ls -al
    total 0
    drwxr-xr-x  4 root root    280 Feb 27 08:49 .
    drwxr-xr-x 15 root root   4260 Feb 27 08:49 ..
    drwxr-xr-x  2 root root    120 Feb 27 08:49 by-id
    drwxr-xr-x  2 root root    180 Feb 27 08:49 by-path
    crw-r-----  1 root root 13, 64 Feb 27 08:49 event0
    crw-r-----  1 root root 13, 65 Feb 27 08:49 event1
    crw-r-----  1 root root 13, 66 Feb 27 08:49 event2
    crw-r-----  1 root root 13, 67 Feb 27 08:49 event3
    crw-r-----  1 root root 13, 68 Feb 27 08:49 event4
    crw-r--r--  1 root root 13,  0 Feb 27 08:49 js0
    crw-r-----  1 root root 13, 63 Feb 27 08:49 mice
    crw-r-----  1 root root 13, 32 Feb 27 08:49 mouse0
    crw-r-----  1 root root 13, 33 Feb 27 08:49 mouse1
    crw-r-----  1 root root 13, 34 Feb 27 08:49 mouse2
    myzr@myzr:/dev/input$ sudo chmod 666 *
    ***************************************************************/
 
 #include <stdio.h>
    #include <linux/input.h>
    #include <fcntl.h>
    #include <sys/time.h>
    #include <unistd.h>
    #include <string.h>
    
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
     
    int main(int argc, char **argv)
    {
        int fd_mouse = -1;
        int fd_kbd = -1;
        int i = 0;
        
        // 请保证该设备节点有写的权限
        fd_kbd = open("/dev/input/event6", O_RDWR);
        if(fd_kbd <= 0) {
            printf("Can not open keyboard input file\n");
            return -1;
        }
        
        // 请保证该设备节点有写的权限
        fd_mouse = open("/dev/input/event17", O_RDWR);
        if(fd_mouse <= 0) {
            printf("Can not open mouse input file\n");
            return -1;
        }
        
        for (i = 0; i < 50; i++) {
            simulate_key(fd_mouse, BTN_LEFT);   //模拟按下鼠标左键
            //simulate_key(fd_mouse, BTN_RIGHT);  //模拟按下鼠标左键
            //if (i % 3 == 0)
            //    simulate_key(fd_kbd, KEY_A);    //模拟按下键盘A键
            //模拟鼠标相对上次x和y轴相应移动10个像素
            simulate_mouse(fd_mouse, 20, 20);
            sleep(1);
        }
        
        close(fd_kbd);
        close(fd_mouse);
    }