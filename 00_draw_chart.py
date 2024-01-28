# coding:utf-8

import matplotlib.pyplot as plt
import numpy as np

import os
from os import path

src_dir = 'data_output'

cmap = plt.get_cmap('flag', 10)
print(type(cmap), cmap)


def draw(xs, ys):
    plt.figure(figsize=(15, 10))
    plt.ylim(-160, 200)
    index = 0
    for x, y in zip(xs, ys):
        plt.plot(x, y, '-', color=cmap(index), linewidth=0.5)
        index += 1
    plt.show()


def readFile(file_path):
    print(file_path)
    x = []
    y = []
    index = 0
    with open(file_path, 'r') as f:
        for line in f.readlines():
            strs = line.strip().split(',')
            num = float(strs[0]) if len(strs) < 6 else float(strs[4])
            x.append(index)
            y.append(num)
            index += 1
    return x, y


if __name__ == '__main__':
    xs = []
    ys = []
    for it in os.listdir(src_dir):
        p = path.join(src_dir, it)
        x, y = readFile(p)
        xs.append(x)
        ys.append(y)
    draw(xs, ys)
