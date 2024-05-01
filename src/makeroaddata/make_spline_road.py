#!python
# -*- mode: python; Encoding: utf-8; coding: utf-8 -*-
# Last updated: <2024/04/29 19:19:46 +0900>
"""
Make spline road data csv

QGISからエクスポートした座標値データを
テキストエディタで編集して作成したcsvを読み込んで、
scipyでスプライン補間値を生成しつつ、
描画用のポリゴンデータも求めて、
C言語の構造体配列の形で出力する。

Usage:
    python make_spline_road.py -i INPUT.csv > OUTPUT.h

Windows10 x64 22H2 + Python 3.10.10 64bit + scipy 1.13.0 + numpy 1.26.4

by mieki256
License: CC0 / Public Domain
"""

import argparse
import sys
import math
import random
import numpy as np
from scipy import interpolate

SCALE = 1000000.0
ROADW = 40.0
LINEW = 2.0

tree_cols_len = 6


def calc_road_edge(xs, ys):
    """save road center and edge position."""
    global centers, roads, wlines

    centers = []
    roads = []
    wlines = []

    for i in range(len(xs)):
        # save road center
        x0, y0 = xs[i] * SCALE, ys[i] * SCALE
        centers.append([x0, y0])
        if i >= len(xs) - 1:
            break

        # save road edge
        x1, y1 = xs[i + 1] * SCALE, ys[i + 1] * SCALE
        xd, yd = x1 - x0, y1 - y0
        lg = math.sqrt(xd * xd + yd * yd)
        wx = xd / lg
        wy = yd / lg
        px2 = x0 - (wy * (ROADW * 1.0))
        py2 = y0 + (wx * (ROADW * 1.0))
        px3 = x0 + (wy * (ROADW * 1.0))
        py3 = y0 - (wx * (ROADW * 1.0))
        roads.append([px2, py2, px3, py3])

        # save white line edge
        lx2 = x0 - (wy * (LINEW * 1.0))
        ly2 = y0 + (wx * (LINEW * 1.0))
        lx3 = x0 + (wy * (LINEW * 1.0))
        ly3 = y0 - (wx * (LINEW * 1.0))
        wlines.append([lx2, ly2, lx3, ly3])

    return centers, roads, wlines


def create_trees(centers):
    trees = []
    for i in range(len(centers) - 1):
        fg = 0
        x, y, r = 0.0, 0.0, 0.0
        c = 0
        if random.random() < 0.4:
            x0, y0 = centers[i]
            x1, y1 = centers[i + 1]
            xd, yd = x1 - x0, y1 - y0

            # normalize vector
            lg = math.sqrt(xd * xd + yd * yd)
            wx = xd / lg
            wy = yd / lg

            w = random.randint(100, 400)
            xa = -wy * w
            ya = wx * w
            if i % 2 == 0:
                x = x0 + xa
                y = y0 + ya
            else:
                x = x0 - xa
                y = y0 - ya
            r = random.randint(25, 50)
            c = random.randint(0, tree_cols_len - 1)

            # Check for collisions between trees and the road. very slow
            fg = 1
            for pos in centers:
                cx, cy = pos
                if (((x - cx) ** 2) + ((y - cy) ** 2)) < ((r + ROADW) ** 2):
                    fg = 0
                    break

        if fg:
            trees.append({"fg": 1, "x": x, "y": y, "r": r, "col": c})
        else:
            trees.append({"fg": 0, "x": 0, "y": 0, "r": 0, "col": 0})

    return trees


def get_area_size(roads):
    xmin, ymin = roads[0][0], roads[0][1]
    xmax, ymax = xmin, ymin
    for i in range(len(roads)):
        x0, y0, x1, y1 = roads[i]
        if x0 < xmin:
            xmin = x0
        if x1 < xmin:
            xmin = x1
        if x0 > xmax:
            xmax = x0
        if x1 > xmax:
            xmax = x1
        if y0 < ymin:
            ymin = y0
        if y1 < ymin:
            ymin = y1
        if y0 > ymax:
            ymax = y0
        if y1 > ymax:
            ymax = y1

    return (xmin, ymin, xmax, ymax)


# B-Spline
def spline1(x, y, s):
    tck, u = interpolate.splprep([x, y], k=3, s=0)
    u = np.linspace(0, 1, num=len(x) * s, endpoint=True)
    spline = interpolate.splev(u, tck)
    return spline[0], spline[1]


def load_csv(infile):
    """load csv file."""

    ax = []
    ay = []
    with open(infile, "r") as file:
        for line in file:
            line = line.rstrip()
            x, y = line.split(",")
            ax.append(float(x))
            ay.append(float(y) * 1.3)
    return ax, ay


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", "--input", required=True, help="Input csv file")
    parser.add_argument("-n", "--num", type=int, default=10, help="number")
    args = parser.parse_args()

    if args.input is None:
        sys.exit()

    # load csv and initialize work
    x, y = load_csv(args.input)
    xs, ys = spline1(x, y, args.num)

    random.seed()
    centers, roads, wlines = calc_road_edge(xs, ys)
    trees = create_trees(centers)

    # stg_xmin, stg_ymin, stg_xmax, stg_ymax = get_area_size(roads)
    # stg_width, stg_height = stg_xmax - stg_xmin, stg_ymax - stg_ymin
    # print(f"area = ( {stage_xmin}, {stage_ymin} ) - ( {stage_xmax}, {stage_ymax} )")
    # print(f"area size w x h = {stage_width} x {stage_height}")

    symname = args.input.replace(".csv", "").replace(".CSV", "")
    valname = symname
    defname = symname.upper()

    print(f"#ifndef __{defname}__")
    print(f"#define __{defname}__")
    print()
    print('#include "roads.h"')
    print()
    print(f"#define {defname}_NUM {len(centers)}")
    print()

    print("static ROADDATA %s_data[%d] = {" % (valname, len(centers)))
    for i in range(len(centers)):
        cx, cy = 0.0, 0.0
        rx0, ry0, rx1, ry1 = 0.0, 0.0, 0.0, 0.0
        lx0, ly0, lx1, ly1 = 0.0, 0.0, 0.0, 0.0
        tfg = 0
        tx, ty, r = 0.0, 0.0, 0.0
        col = 0

        cx, cy = centers[i]
        if i < len(centers) - 1:
            rx0, ry0, rx1, ry1 = roads[i]
            lx0, ly0, lx1, ly1 = wlines[i]
            t = trees[i]
            tfg = t["fg"]
            tx, ty = t["x"], t["y"]
            r = t["r"]
            col = t["col"]

        print(
            f"  {{ {cx}, {cy}, {rx0}, {ry0}, {rx1}, {ry1}, {lx0}, {ly0}, {lx1}, {ly1}, {tfg}, {col}, {tx}, {ty}, {r} }},"
        )

    print("};")
    print()
    print("#endif")


if __name__ == "__main__":
    main()
