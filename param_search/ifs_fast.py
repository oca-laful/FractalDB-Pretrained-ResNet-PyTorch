# -*- coding: utf-8 -*-
"""
Created on Sat Aug 05 23:55:12 2017
@author: Kazushige Okayasu, Hirokatsu Kataoka
"""

import argparse
from ifs_simple import ifs_function
from PIL import Image
import numpy as np
import random
import math
import time
import cv2
import os

random.seed(10)

params = [[0.58378481, 0.5710405, 0.74313123, -0.55115729, -0.06844673, 0.06882748, 0.63922813],
          [0.69770033, -0.0544094, -0.02932284, -0.60126404, 0.35812325, -0.38601142, 0.36077187]]


class ifs_function():
    def __init__(self, prev_x, prev_y):
        # previous (x, y)
        self.prev_x, self.prev_y = prev_x, prev_y
        # IFS function
        self.function = []
        # Iterative results
        self.xs, self.ys = [], []
        # Add initial value
        self.xs.append(prev_x), self.ys.append(prev_y)
        # Select function
        self.select_function = []
        # Calculate select function
        self.temp_proba = 0.0

    # パラメータをfunctionに突っ込む
    def set_param(self, a, b, c, d, e, f, proba, **kwargs):
        # Initial parameters & select function
        temp_function = {"a": a, "b": b, "c": c,
                         "d": d, "e": e, "f": f, "proba": proba}
        self.function.append(temp_function)
        # Plus probability when function is added
        self.temp_proba += proba
        self.select_function.append(self.temp_proba)

    def calculate(self, iteration):
        # 既にparamsをfunctionに読み込んでいる
        # Fix random seed
        # iteration=100000 なので 100000 の0~1範囲のベクトルが生成
        rand = np.random.random(iteration)
        # [0.1, 0.4, 0.5, 0.7, 0.9, 1] みたいなの
        select_function = self.select_function
        function = self.function
        prev_x, prev_y = self.prev_x, self.prev_y
        count = 0
        print(select_function)
        # 若干時間かかっている
        for i in range(iteration-1):   # 100000回実行
            # [0.1, 0.4, 0.5, 0.7, 0.9, 1] みたいなの 結局は0~7のランダムな関数を選んでnextを決定したい
            # 辞書形式なので定数時間で値が取り出せないので速度が落ちる
            # 1000回繰り返したのを100回コピペしても変わらなさそう
            for j in range(len(select_function)):  # 2~8回実行
                if rand[i] <= select_function[j]:
                    next_x = prev_x * \
                        function[j]["a"] + prev_y * \
                        function[j]["b"] + function[j]["e"]
                    next_y = prev_x * \
                        function[j]["c"] + prev_y * \
                        function[j]["d"] + function[j]["f"]
                    break
            self.xs.append(next_x), self.ys.append(next_y)
            prev_x = next_x
            prev_y = next_y
            count += 1
        print(count)

    # フラクタルが四角形の中の整数値に収まるように調整する
    def __rescale(self, image_x, image_y, pad_x, pad_y):
        # Scale adjustment
        xs = np.array(self.xs)
        ys = np.array(self.ys)
        # xやyに欠損値のある場合、それを含むrowを削除する
        if np.any(np.isnan(xs)):
            #print("x is nan")
            nan_index = np.where(np.isnan(xs))
            extend = np.array(range(nan_index[0][0]-100, nan_index[0][0]))
            delete_row = np.append(extend, nan_index)
            xs = np.delete(xs, delete_row, axis=0)
            ys = np.delete(ys, delete_row, axis=0)
            #print ("early_stop: %d" % len(xs))
        if np.any(np.isnan(ys)):
            #print("y is nan")
            nan_index = np.where(np.isnan(ys))
            extend = np.array(range(nan_index[0][0]-100, nan_index[0][0]))
            delete_row = np.append(extend, nan_index)
            xs = np.delete(xs, delete_row, axis=0)
            ys = np.delete(ys, delete_row, axis=0)
            #print ("early_stop: %d" % len(ys))
        if np.min(xs) < 0.0:
            xs -= np.min(xs)
        if np.min(ys) < 0.0:
            ys -= np.min(ys)
        xmax, xmin, ymax, ymin = np.max(xs), np.min(xs), np.max(ys), np.min(ys)
        self.xs = np.uint16(xs / (xmax-xmin) *
                            float(image_x-2*pad_x)+float(pad_x))
        self.ys = np.uint16(ys / (ymax-ymin) *
                            float(image_y-2*pad_y)+float(pad_y))

    def draw_point(self, image_x, image_y, pad_x, pad_y):
        self.__rescale(image_x, image_y, pad_x, pad_y)
        image = np.array(Image.new("RGB", (image_x, image_y)))
        for i in range(len(self.xs)):  # python3.x
            image[self.ys[i], self.xs[i], :] = 127, 127, 127
        return image


# 引数を入力するところ. argsに引数が渡される
parser = argparse.ArgumentParser(description='PyTorch fractal random search')
parser.add_argument('--rate', default=0.2, type=float,
                    help='filling rate: (fractal pixels) / (all pixels in the image)')
parser.add_argument('--category', default=1000, type=int, help='# of category')
parser.add_argument('--numof_point', default=100000,
                    type=int, help='# of point')
parser.add_argument('--save_dir', default='.', type=str, help='save directory')
args = parser.parse_args()


def cal_pix(gray):
    height, width = gray.shape
    num_pixels = np.count_nonzero(gray) / float(height * width)
    return num_pixels

# 画像を生成する


def generator(params):
    generators = ifs_function(prev_x=0.0, prev_y=0.0)  # 開始点設定
    print('start')
    for param in params:
        generators.set_param(float(param[0]), float(param[1]), float(param[2]), float(
            param[3]), float(param[4]), float(param[5]), float(param[6]))
    generators.calculate(args.numof_point)  # class
    print('to')
    img = generators.draw_point(512, 512, 6, 6)  # image (x,y pad x,y)
    print('end')
    return img  # return by cv2


if __name__ == '__main__':
    count = 0
    t_start = time.time()
    for i in range(1):
        print(i)
        fractal_img = generator(params)
    t_end = time.time()
    print('avg:' + str((t_end - t_start) / 1))
    print(np.shape(fractal_img))
    cv2.imwrite(os.path.join('output.png'), fractal_img)
