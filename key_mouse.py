#
# _*_ coding:UTF-8 _*_
__author__ = 'shanl'

import win32api
import win32con
import win32gui
from ctypes import *
import time
import pyHook
import pythoncom
import thread

RUN = 0

class POINT(Structure):
    _fields_ = [("x", c_ulong),("y", c_ulong)]

def get_mouse_point():
    po = POINT()
    windll.user32.GetCursorPos(byref(po))
    return int(po.x), int(po.y)

def mouse_click(x=None,y=None):
    if not x is None and not y is None:
        mouse_move(x,y)
        time.sleep(0.05)
    win32api.mouse_event(win32con.MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0)
    win32api.mouse_event(win32con.MOUSEEVENTF_LEFTUP, 0, 0, 0, 0)

def mouse_dclick(x=None,y=None):
    if not x is None and not y is None:
        mouse_move(x,y)
        time.sleep(0.05)
    win32api.mouse_event(win32con.MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0)
    win32api.mouse_event(win32con.MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0)

def mouse_move(x,y):
    windll.user32.SetCursorPos(x, y)

def onKeyboardEvent(event):
    global RUN
    if event.Key == "F10":
        RUN = 0
        thread.start_new_thread(t5, ())
        print "f10"
    if event.Key == "F12":
        RUN = 1
        print "f12"
    return True

def get_key_click():
    #创建hook句柄
    hm = pyHook.HookManager()
    #监控键盘
    hm.KeyDown = onKeyboardEvent
    hm.HookKeyboard()

def time_sleep(t=None):
    time.sleep(t)

def t0():
    pass

def t2():
    mouse_click(800,200)
    for c in 'hello':
        win32api.keybd_event(65,0,0,0) #a键位码是86
        win32api.keybd_event(65,0,win32con.KEYEVENTF_KEYUP,0)
    #print get_mouse_point()

def t1():
    #mouse_move(1024,470)aa
    #time.sleep(0.05)
    #mouse_dclick()HELLO

    mouse_dclick(1024,470)

def t3():
    mouse_click(1024,470)
    str = 'hello'
    for c in str:
        win32api.keybd_event(VK_CODE[c],0,0,0) #a键位码是86
        win32api.keybd_event(VK_CODE[c],0,win32con.KEYEVENTF_KEYUP,0)
        time.sleep(0.01)

def t4():
    mouse_click(1024,470)
    str = 'hello'

def t5():
    global RUN
    count = 0
    while (count < 10000):
        if RUN == 1:
            break
        mouse_move(747,509)
        mouse_click()
        time_sleep(2)
        count = count+1
        print count

def t6():
    global COUNT
    get_key_click()
    # 进入循环，如不手动关闭，程序将一直处于监听状态
    pythoncom.PumpMessages()

if __name__ == "__main__":
    t6()
    #t3()
    #t2()
    #t1()
    t0()
