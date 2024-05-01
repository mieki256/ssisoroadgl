#!python
# -*- mode: python; Encoding: utf-8; coding: utf-8 -*-
# Last updated: <2024/04/27 09:10:06 +0900>
"""
Drawing isometric roads in OpenGL

Usage:
    python draw_road_iso.py INPUT.csv

P key : Toggle between polygon and line drawingChange
Up, Down : Pause, Go, Back

Windows10 x64 22H2 + Python 3.10.10 64bit
 + PyOpenGL 3.1.6 + PyOpenGL-accelerate 3.1.6
 + glfw 2.7.0
 + PyWavefront 1.3.3

by mieki256
License: CC0 / Public Domain
"""

import sys
import math
import random
import glfw
from OpenGL.GL import *
from OpenGL.GLU import *
import pywavefront


SCRW, SCRH = 1280, 720
WDWTITLE = "Drawing isometric roads in OpenGL"

INFILE = "./motosuko_roads.csv"
# INFILE = "./housakatouge_roads.csv"

INOBJFILE = "./car.obj"

FIXED_SPEED = False
IDX_SPD_MAX = 0.25
ROADW = 40.0

winw, winh = SCRW, SCRH
pausefg = False
draw_poly = True

ang = 0.0
idx = 0.0
idx_add = IDX_SPD_MAX
spd = 0.0
centers = []  # road center line
roads = []  # road edge
wlines = []  # road white line
trees = []  # trees pos
obj = None

tree_cols = [
    [0.196, 0.580, 0.110, 1.0],
    [0.352, 0.890, 0.231, 1.0],
    [0.227, 0.400, 0.188, 1.0],
    [0.198, 0.600, 0.406, 1.0],
    [0.205, 0.410, 0.311, 1.0],
    [0.390, 0.560, 0.040, 1.0],
]


def render():
    global idx, spd, ang

    # clear screen
    glClearDepth(1.0)
    glClearColor(0.498, 0.720, 0.187, 1)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

    # set view scale
    # view_scale = 1.0
    view_scale = 0.6 + 0.4 * math.sin(math.radians(ang * 0.4))

    # set ortho
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    h = (SCRH / 2) * view_scale
    w = h * (winw / winh)
    glOrtho(-w, w, -h, h, -1000.0, 1000.0)

    glMatrixMode(GL_MODELVIEW)
    glLoadIdentity()

    glEnable(GL_CULL_FACE)
    glFrontFace(GL_CCW)
    # glCullFace(GL_FRONT)
    glCullFace(GL_BACK)

    glDepthFunc(GL_LESS)
    glEnable(GL_DEPTH_TEST)
    glEnable(GL_BLEND)
    glEnable(GL_NORMALIZE)

    # set lighting
    light_pos = [1.0, 1.0, 1.0, 0.0]
    # light_ambient = [0.2, 0.2, 0.2, 1.0]
    light_ambient = [0.5, 0.5, 0.5, 1.0]
    light_diffuse = [1.0, 1.0, 1.0, 1.0]
    light_specular = [0.8, 0.8, 0.8, 1.0]

    glLightfv(GL_LIGHT0, GL_POSITION, light_pos)
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient)
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse)
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular)
    glEnable(GL_LIGHTING)
    glEnable(GL_LIGHT0)

    # set material
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE)
    # glColorMaterial(GL_FRONT, GL_DIFFUSE)
    glEnable(GL_COLOR_MATERIAL)

    # get index
    frac, idxi = math.modf(idx)  # xxx.yyy -> idxi = xxx, frac = yyy
    i = int(idxi)

    # get center position
    if i < len(centers) - 1:
        x0, y0 = centers[i]
        x1, y1 = centers[i + 1]
        xb = x0 + (x1 - x0) * frac
        yb = -(y0 + (y1 - y0) * frac)
    else:
        x0, y0 = centers[i]
        xb = x0
        yb = -(y0)

    glPushMatrix()
    glRotate(30, 1, 0, 0)

    disp_num = 512
    draw_roads(i, disp_num, xb, yb)
    draw_trees(i, disp_num, xb, yb)

    # draw car object
    x, z = get_road_pos(idx, 0.75)
    x = x - xb
    z = -z - yb
    y = 0.0 if not draw_poly else 5.1
    glTranslatef(x, y, z)

    road_angle = get_road_vec(idx)
    glRotatef(road_angle + 90, 0, 1, 0)
    scale = 12.0
    glScalef(scale, scale, scale)

    draw_obj(obj)

    glPopMatrix()

    # update index
    if not pausefg:
        spdmax = math.fabs(idx_add)
        if idx >= len(roads) - 10:
            spd -= 0.005
            if spd <= (spdmax * 0.1):
                spd = spdmax * 0.1
        else:
            if FIXED_SPEED:
                # fixed speed
                spd = spdmax
            else:
                # With acceleration / deceleration
                a = get_curve_angle(idx)
                # print(a)
                if a < 20.0:
                    spd += 0.0025
                    if spd >= spdmax:
                        spd = spdmax
                if a > 30.0:
                    spd -= 0.0025
                    if spd <= (spdmax * 0.4):
                        spd = spdmax * 0.4

        idx += spd if idx_add > 0 else (-spd)

        if idx < 0:
            idx = 0
        if idx >= len(centers) - 3:
            idx = len(centers) - 3

    ang += 1.0


def setup_animation():
    global idx, spd, ang
    global obj

    random.seed()

    idx = 0.0
    spd = 0.0
    ang = 0.0

    # load wavefront obj
    obj = pywavefront.Wavefront(INOBJFILE)


def draw_roads(i, n, xb, yb):
    px0, py0, px1, py1 = 0.0, 0.0, 0.0, 0.0
    lx0, ly0, lx1, ly1 = 0.0, 0.0, 0.0, 0.0
    nmax = len(centers)

    if draw_poly:
        glBegin(GL_QUADS)
    else:
        glBegin(GL_LINES)
        glColor4f(0, 0, 0, 1)

    for j in range(-n, n):
        i0 = i + j
        i1 = i0 + 1
        if i0 < 0 or i1 < 0:
            continue
        if i0 >= nmax or i1 >= nmax:
            break

        # get center line position
        x0, y0 = centers[i0][0] - xb, -(centers[i0][1]) - yb
        x1, y1 = centers[i1][0] - xb, -(centers[i1][1]) - yb

        # get road edge
        px2, py2, px3, py3 = roads[i0]
        px2, py2 = px2 - xb, -py2 - yb
        px3, py3 = px3 - xb, -py3 - yb

        # get white line edge
        lx2, ly2, lx3, ly3 = wlines[i0]
        lx2, ly2 = lx2 - xb, -ly2 - yb
        lx3, ly3 = lx3 - xb, -ly3 - yb

        if draw_poly:
            if px0 != 0.0 and py0 != 0.0:
                # draw shadow polygon
                glColor4f(0.2, 0.2, 0.2, 1)
                z = 0.0
                glVertex3f(px0, z, py0)
                glVertex3f(px1, z, py1)
                glVertex3f(px3, z, py3)
                glVertex3f(px2, z, py2)

                # draw road polygon
                if i0 % 2 == 0:
                    glColor4f(0.3, 0.4, 0.45, 1)
                else:
                    glColor4f(0.35, 0.45, 0.50, 1)
                z = 5.0
                glVertex3f(px0, z, py0)
                glVertex3f(px1, z, py1)
                glVertex3f(px3, z, py3)
                glVertex3f(px2, z, py2)

                # draw white line polygon
                if i0 & 0x01 == 0:
                    z = 5.01
                    glColor4f(1.0, 1.0, 1.0, 1.0)
                    glVertex3f(lx0, z, ly0)
                    glVertex3f(lx1, z, ly1)
                    glVertex3f(lx3, z, ly3)
                    glVertex3f(lx2, z, ly2)
        else:
            # draw lines
            z = 0.0
            glVertex3f(px2, z, py2)
            glVertex3f(px3, z, py3)
            glVertex3f(x0, z, y0)
            glVertex3f(x1, z, y1)

        px0, py0 = px2, py2
        px1, py1 = px3, py3
        lx0, ly0 = lx2, ly2
        lx1, ly1 = lx3, ly3

    glEnd()


def get_road_vec(idx):
    if idx < 0.0:
        idx = 0.0
    if idx >= len(centers) - 3:
        idx = len(centers) - 3

    f0, ii0 = math.modf(idx)
    i0 = int(ii0)
    cx = centers[i0][0] + (centers[i0 + 1][0] - centers[i0][0]) * f0
    cy = centers[i0][1] + (centers[i0 + 1][1] - centers[i0][1]) * f0
    tx = centers[i0 + 1][0] + (centers[i0 + 2][0] - centers[i0 + 1][0]) * f0
    ty = centers[i0 + 1][1] + (centers[i0 + 2][1] - centers[i0 + 1][1]) * f0
    xd, yd = tx - cx, ty - cy
    angle = math.degrees(math.atan2(yd, xd))
    return angle


def get_curve_angle(idx):
    sumangle = 0.0
    a0 = get_road_vec(idx)
    for i in range(8):
        a1 = get_road_vec(idx + i + 1)
        a = a1 - a0
        if a > 180.0:
            a = a1 - (a0 + 360.0)
        elif a < -180.0:
            a = (a1 + 360.0) - a0
        sumangle += math.fabs(a)
        a0 = a1
    return sumangle


def get_road_pos(idx, p):
    if idx < 0.0:
        idx = 0.0
    if idx >= len(roads) - 2:
        idx = len(roads) - 2

    f0, ii0 = math.modf(idx)
    i0 = int(ii0)
    x0r, y0r, x0l, y0l = roads[i0]
    x1r, y1r, x1l, y1l = roads[i0 + 1]
    pxr = x0r + (x1r - x0r) * f0
    pxl = x0l + (x1l - x0l) * f0
    pyr = y0r + (y1r - y0r) * f0
    pyl = y0l + (y1l - y0l) * f0
    x = pxl + (pxr - pxl) * p
    y = pyl + (pyr - pyl) * p
    return (x, y)


def draw_trees(idx, num, xb, yb):
    if draw_poly:
        glBegin(GL_TRIANGLES)
    else:
        glColor4f(0, 0, 0, 1)
        glBegin(GL_LINES)

    for a in range(-num, num):
        i = idx + a
        if i < 0:
            continue
        if i >= len(trees):
            break
        tree = trees[i]
        if tree["fg"] == 0:
            continue

        x, y, r, c = tree["x"], tree["y"], tree["r"], tree["col"]
        x = x - xb
        y = -y - yb
        if draw_poly:
            glColor4fv(tree_cols[c])
            glVertex3f(x, r * 0.866 * 2, y)
            glVertex3f(x - r, 0.0, y)
            glVertex3f(x + r, 0.0, y)
        else:
            glVertex3f(x - r, 0.0, y)
            glVertex3f(x + r, 0.0, y)

    glEnd()


def draw_obj(obj):
    for mesh in obj.mesh_list:
        for mat in mesh.materials:
            # glMaterialfv(GL_FRONT, GL_AMBIENT, mat.ambient)
            # glMaterialfv(GL_FRONT, GL_DIFFUSE, mat.diffuse)
            # glMaterialfv(GL_FRONT, GL_SPECULAR, mat.specular)
            glColor4fv(mat.diffuse)
            gl_floats = (GLfloat * len(mat.vertices))(*mat.vertices)
            count = len(mat.vertices) / mat.vertex_size
            glInterleavedArrays(GL_T2F_N3F_V3F, 0, gl_floats)
            glDrawArrays(GL_TRIANGLES, 0, int(count))


def load_csv(infile):
    global centers, roads, wlines, trees

    centers = []
    roads = []
    wlines = []
    trees = []

    with open(infile, "r") as file:
        for line in file:
            line = line.rstrip()
            if line[0] == "#":
                continue

            cx, cy, rx0, ry0, rx1, ry1, lx0, ly0, lx1, ly1, tfg, tx, ty, r, col = (
                line.split(",")
            )
            centers.append([float(cx), float(cy)])
            roads.append([float(rx0), float(ry0), float(rx1), float(ry1)])
            wlines.append([float(lx0), float(ly0), float(lx1), float(ly1)])
            trees.append(
                {
                    "fg": int(tfg),
                    "x": float(tx),
                    "y": float(ty),
                    "r": float(r),
                    "col": int(col),
                }
            )


def key_callback(window, key, scancode, action, mods):
    global pausefg
    global idx_add
    global draw_poly

    if action == glfw.PRESS:
        if key == glfw.KEY_ESCAPE or key == glfw.KEY_Q:
            glfw.set_window_should_close(window, True)
        if key == glfw.KEY_SPACE:
            pausefg = not pausefg
        if key == glfw.KEY_UP:
            pausefg = not pausefg
            if idx_add < 0:
                idx_add = IDX_SPD_MAX
        if key == glfw.KEY_DOWN:
            pausefg = not pausefg
            if idx_add > 0:
                idx_add = -IDX_SPD_MAX
        if key == glfw.KEY_P:
            draw_poly = not draw_poly


def set_view(w, h):
    global winw, winh
    winw, winh = w, h
    glViewport(0, 0, w, h)


def resize(window, w, h):
    if h == 0:
        return
    set_view(w, h)


def main():
    if not glfw.init():
        raise RuntimeError("Could not initialize GLFW3")
        return

    window = glfw.create_window(SCRW, SCRH, WDWTITLE, None, None)
    if not window:
        glfw.terminate()
        raise RuntimeError("Could not create an window")
        return

    glfw.set_key_callback(window, key_callback)
    glfw.set_window_size_callback(window, resize)
    glfw.make_context_current(window)
    glfw.swap_interval(1)

    glfw.window_hint(glfw.CONTEXT_VERSION_MAJOR, 1)
    glfw.window_hint(glfw.CONTEXT_VERSION_MINOR, 1)
    glfw.window_hint(glfw.OPENGL_PROFILE, glfw.OPENGL_CORE_PROFILE)
    glfw.window_hint(glfw.DEPTH_BITS, 24)

    set_view(SCRW, SCRH)

    # load csv and initialize work
    infile = INFILE if len(sys.argv) != 2 else sys.argv[1]
    load_csv(infile)
    setup_animation()

    # main loop
    while not glfw.window_should_close(window):
        render()
        glfw.swap_buffers(window)
        glfw.poll_events()

    glfw.terminate()


if __name__ == "__main__":
    main()
