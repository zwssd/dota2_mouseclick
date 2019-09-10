#include <termio.h>
#include <stdio.h>
 
//注：通过while循环读取键盘效率很低，参考使用异步事件或者多线程技术。单线程为阻塞式的
 
int scanKeyboard()
{
int in;
struct termios new_settings;
struct termios stored_settings;
tcgetattr(0,&stored_settings);
new_settings = stored_settings;
new_settings.c_lflag &= (~ICANON);
new_settings.c_cc[VTIME] = 0;
tcgetattr(0,&stored_settings);
new_settings.c_cc[VMIN] = 1;
tcsetattr(0,TCSANOW,&new_settings);

 
in = getchar();
 
tcsetattr(0,TCSANOW,&stored_settings);
return in;
}

//测试函数
int main(){
while(1){
printf(":%d",scanKeyboard());
}
}
