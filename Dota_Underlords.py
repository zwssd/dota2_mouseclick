# _*_ coding:UTF-8 _*_

# 安装
# pip3 install pyautogui
# pip3 install keyboard
# sudo zypper in python3-devel
# sudo zypper in python3-tk

import pyautogui  # 控制鼠标和键盘
import keyboard  # 监听键盘

pyautogui.FAILSAFE = True

pyautogui.size()
# (1920, 1080)
width, height = pyautogui.size()


def test():
   for i in range(2):
      pyautogui.moveTo(300, 300, duration=0.25)
      pyautogui.moveTo(400, 300, duration=0.25)
      pyautogui.moveTo(400, 400, duration=0.25)
      pyautogui.moveTo(300, 400, duration=0.25)
      pyautogui.PAUSE = 2.5


def monitorKey():
   print('aaa')


if __name__ == "__main__":
    keyboard.add_hotkey('f10', monitorKey)
