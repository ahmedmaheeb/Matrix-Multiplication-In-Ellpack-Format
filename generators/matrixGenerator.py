import os

import numpy
from random import seed
from random import randint
from random import random


def run(width, height, path):
    seed(1)
    nonZeros = int((height * width) / 1000)

    max = 1000000
    min = 0.1
    lis = []
    for _ in range(nonZeros):
        x = randint(0, width - 1)
        y = randint(0, height - 1)
        v = numpy.format_float_scientific(min + random() * (max - min), precision=9, exp_digits=9)
        tup = (x, y, v)
        lis.append(tup)

    lis.sort()
    try:
        os.remove(path)
    except OSError:
        pass
    f = open(path, "a")
    f.write(str(width) + "\n")
    f.write(str(height) + "\n")
    f.write("\n")
    for index, tuple in enumerate(lis):
        f.write(str(tuple[0]) + ";")
        f.write(str(tuple[1]) + ";")
        f.write(str(tuple[2]) + "\n")


if __name__ == '__main__':
    width = randint(10000, 10000)
    height = randint(10000, 10000)
    run(width, height, "../a.mat")
    run(height, width, "../b.mat")
