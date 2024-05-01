// Last updated: <2024/05/02 04:04:25 +0900>
//
// Update objs and draw objs by OpenGL

#include "render.h"

// font data
#include "glbitmfont.h"

// roads data
#include "roads.h"
#include "motosuko.h"
#include "housakatouge.h"
#include "bandaiazumaskyline.h"
#include "yasyajintouge.h"

// object data
#include "car.h"
#include "scooter.h"

#include "settings.h"

// #if 0
#ifdef _WIN32
// Windows
#include <windows.h>
#include <mmsystem.h>
#define WINMM_TIMER
#else
// Linux
#undef WINMM_TIMER
#endif

#define SCRW 1280
#define SCRH 720
#define IDEAL_FRAMERATE (60.0)

#define FIXED_SPEED 0
#define IDX_SPD_MAX (0.25)
// #define IDX_SPD_MAX (2.0)

// globals for size of screen
int Width, Height;

#define deg2rad(x) ((x) * M_PI / 180.0)
#define rad2deg(x) ((x) / M_PI * 180.0)

// ----------------------------------------
// object data
typedef struct modeldata
{
    float scale;
    const int vtx_size;
    const float *vtx;
    const int nml_size;
    const float *nml;
    const int col_size;
    const float *col;
    const int uv_size;
    const float *uv;
} MODELDATA;

#define MODEL_MAX 2

MODELDATA models[MODEL_MAX] = {
    {// car
     12.0,
     car_obj_vtx_size, &car_obj_vtx[0][0],
     car_obj_nml_size, &car_obj_nml[0][0],
     car_obj_col_size, &car_obj_col[0][0],
     car_obj_uv_size, &car_obj_uv[0][0]},
    {// scooter
     16.0,
     scooter_obj_vtx_size, &scooter_obj_vtx[0][0],
     scooter_obj_nml_size, &scooter_obj_nml[0][0],
     scooter_obj_col_size, &scooter_obj_col[0][0],
     scooter_obj_uv_size, &scooter_obj_uv[0][0]},
};

// ----------------------------------------
// course data
#define COURSE_MAX 4

ROADDATA *course_data[COURSE_MAX] = {
    yasyajintouge_data,
    motosuko_data,
    bandaiazumaskyline_data,
    housakatouge_data,
};

const int course_size[COURSE_MAX] = {
    YASYAJINTOUGE_NUM,
    MOTOSUKO_NUM,
    BANDAIAZUMASKYLINE_NUM,
    HOUSAKATOUGE_NUM,
};

const char *course_name[COURSE_MAX] = {
    "To Yashajin Pass",
    "To Lake Motosu",
    "Bandai Azuma Skyline",
    "Hosaka Pass, Fukushima",
};

// ----------------------------------------
// stage type
#define STG_SUMMER 0
#define STG_AUTUMN 1
#define STG_WINTER 2
#define STG_NIGHT 3
#define STG_MAX 4

// ----------------------------------------
// stage background color
const float clear_colors[STG_MAX][3] = {
    {0.498, 0.720, 0.187}, // summer
    {0.990, 0.703, 0.129}, // autumn
    {0.911, 0.918, 0.920}, // winter
    {0.050, 0.300, 0.200}, // night
};

// ----------------------------------------
// trees color
const float tree_cols[STG_MAX][6][4] = {
    {
        {0.196, 0.580, 0.110, 1.0},
        {0.352, 0.890, 0.231, 1.0},
        {0.227, 0.400, 0.188, 1.0},
        {0.198, 0.600, 0.406, 1.0},
        {0.205, 0.410, 0.311, 1.0},
        {0.390, 0.560, 0.040, 1.0},
    },
    {
        {0.560, 0.373, 0.051, 1.0},
        {0.550, 0.459, 0.302, 1.0},
        {0.760, 0.237, 0.106, 1.0},
        {0.540, 0.305, 0.178, 1.0},
        {0.770, 0.000, 0.000, 1.0},
        {0.700, 0.490, 0.392, 1.0},
    },
    {
        {0.7, 0.7, 0.7, 1.0},
        {0.6, 0.6, 0.6, 1.0},
        {0.5, 0.5, 0.5, 1.0},
        {0.4, 0.4, 0.4, 1.0},
        {0.449, 0.591, 0.680, 1.0},
        {0.230, 0.329, 0.390, 1.0},
    },
    {
        {0.10, 0.30, 0.05, 1.0},
        {0.20, 0.45, 0.10, 1.0},
        {0.10, 0.20, 0.10, 1.0},
        {0.10, 0.30, 0.20, 1.0},
        {0.10, 0.20, 0.15, 1.0},
        {0.20, 0.25, 0.00, 1.0},
    },
};

// ----------------------------------------
// define global work
typedef struct gwk
{
    int scrw;
    int scrh;
    float framerate;
    float cfg_framerate;
    float zfar;
    float view_scale;
    float view_w;
    float view_h;

    int step;

    float ang;
    float idx;
    float idx_add;
    float spd;

    int roads_len;
    ROADDATA *roads;

    float fadev;
    int course_num;
    int stage_color_num;
    int model_kind;

    float course_name_timer;

    // FPS check
    float rec_time;
    float prev_time;
    float now_time;
    float delta;
    int count_frame;
    int count_fps;
    int use_waittime;
    float wait_time;
} GWK;

// reserve global work
static GWK gw;

// ----------------------------------------
// prototype declaration
void initCountFps(void);
void closeCountFps(void);
float countFps(void);
void init_work_first(int Width, int Height);
void init_work(void);
void update(float delta);
void init_gl(void);
void clear_screen(void);
void draw_gl(float delta);
void draw_roads(int idx, int num, double xb, double yb);
void draw_trees(int idx, int num, double xb, double yb);
void draw_obj(void);
double get_road_vec(float idx);
double get_curve_angle(float idx);
void get_road_pos(float idx, float p, double *x, double *y);
void draw_text(const char *buf, float x, float y, float sdw, int kind, float a);
void draw_fps(void);
void draw_course_name(float delta);
void draw_fadeout(float a);

// ========================================
// main loop. Screensaver version. Update objs and draw objs by OpenGL
void Render(void)
{
    gw.delta = countFps();
    update(gw.delta);
    draw_gl(gw.delta);
    // glFinish();
}

// setup animation
void SetupAnimation(int Width, int Height)
{
    init_work_first(Width, Height);
    init_gl();
    initCountFps();
}

// cleanup animation
void CleanupAnimation()
{
    closeCountFps();
}

void set_use_waittime(int fg)
{
    gw.use_waittime = fg;
}

void set_cfg_framerate(float fps)
{
    gw.cfg_framerate = fps;
}

float get_cfg_framerate(void)
{
    return gw.cfg_framerate;
}

void resize_window(int w, int h)
{
    Width = w;
    Height = h;
    gw.scrw = w;
    gw.scrh = h;
    init_gl();
}

// ========================================

// get random value. (0.0 - 1.0)
float getRand(void)
{
    return ((float)rand() / RAND_MAX); // retrun 0.0 - 1.0
}

float get_now_time(void)
{
#ifdef WINMM_TIMER
    // Windows
    return (float)(timeGetTime()) / 1000.0;
#else
    // Linux
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    float tn = ts.tv_sec;
    tn += (float)(ts.tv_nsec) / 1000000000.0;
    return tn;
#endif
}

void initCountFps(void)
{
#ifdef WINMM_TIMER
    // timeBeginPeriod(1);
#endif

    gw.rec_time = get_now_time();
    gw.prev_time = gw.rec_time;
    gw.count_fps = 0;
    gw.count_frame = 0;
}

void closeCountFps(void)
{
#ifdef WINMM_TIMER
    // timeEndPeriod(1);
#endif
}

void waitFrame(void)
{
#if 0
    if (gw.framerate == gw.cfg_framerate)
        return;
#endif

    // sleep
    float waittm = (gw.prev_time + (1.0 / gw.cfg_framerate)) - get_now_time();
    if (waittm > 0.0 && waittm < 1.0)
    {
#ifdef WINMM_TIMER
        // Windows
        waittm *= 1000;
        Sleep((DWORD)waittm);
#else
        // Linux
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = (long)(waittm * 1000000000);
        nanosleep(&ts, NULL);
#endif
    }
}

float countFps(void)
{
    float delta, t;

    if (gw.use_waittime != 0)
        waitFrame();

    // get delta time (millisecond)
    gw.now_time = get_now_time();
    delta = gw.now_time - gw.prev_time;
    if (delta <= 0 || delta >= 1.0)
        delta = 1.0 / gw.framerate;
    gw.prev_time = gw.now_time;

    // check FPS
    gw.count_frame++;
    t = gw.now_time - gw.rec_time;
    if (t >= 1.0)
    {
        gw.rec_time += 1.0;
        gw.count_fps = gw.count_frame;
        gw.count_frame = 0;
    }
    else if (t < 0)
    {
        gw.rec_time = gw.now_time;
        gw.count_fps = 0;
        gw.count_frame = 0;
    }
    return delta;
}

void init_work_first(int Width, int Height)
{
    srand((unsigned)time(NULL));

    gw.scrw = Width;
    gw.scrh = Height;
    gw.framerate = 60.0;
    gw.cfg_framerate = (1000.0 / (float)waitValue);
    gw.zfar = 1000.0;
    gw.use_waittime = 0;
    gw.wait_time = 0.0;
    gw.fadev = 0.0;
    gw.step = 0;

    gw.course_num = rand() % COURSE_MAX;
    // gw.course_num = 0;
    gw.stage_color_num = rand() % STG_MAX;
    gw.model_kind = rand() % MODEL_MAX;
}

void init_work(void)
{
    gw.ang = 0.0;
    gw.idx = 0.0;
    gw.idx_add = IDX_SPD_MAX;
    gw.spd = 0.0;
    gw.fadev = 1.0;
    gw.roads = course_data[gw.course_num];
    gw.roads_len = course_size[gw.course_num];
    gw.course_name_timer = 5.0;
}

void update(float delta)
{
    if (delta <= 0.0 || delta >= 1.0)
        delta = 1.0 / gw.framerate;

    switch (gw.step)
    {
    case 0:
        init_work();
        delta = 1.0 / gw.framerate;
        gw.fadev = 1.0;
        gw.step++;
        break;
    case 1:
        // fadein
        gw.fadev -= ((1.0 / (gw.framerate * 1.3)) * gw.framerate * delta);
        if (gw.fadev <= 0.0)
        {
            gw.fadev = 0.0;
            gw.step++;
        }
        break;
    case 2:
        // main job
        if (gw.idx >= gw.roads_len - 10)
        {
            gw.fadev = 0.0;
            gw.step++;
        }
        break;
    case 3:
        // fadeout
        gw.fadev += ((1.0 / (gw.framerate * 2.0)) * gw.framerate * delta);
        if (gw.fadev >= 1.0)
        {
            gw.fadev = 1.0;
            gw.course_num = (gw.course_num + 1) % COURSE_MAX;
            gw.stage_color_num = (gw.stage_color_num + 1) % STG_MAX;
            gw.model_kind = (gw.model_kind + 1) % MODEL_MAX;
            gw.step = 0;
        }
        break;
    default:
        break;
    }

    // set view scale
    if (gw.model_kind == 0)
    {
        gw.view_scale = 0.6 + 0.4 * sin(deg2rad(gw.ang * 0.4));
    }
    else
    {
        gw.view_scale = 0.5 + 0.4 * sin(deg2rad(gw.ang * 0.3));
    }
    gw.view_h = (float(SCRH) / 2.0) * gw.view_scale;
    gw.view_w = gw.view_h * float(gw.scrw) / float(gw.scrh);

    // update index
    float spdmax = fabsf(gw.idx_add);
    if (gw.model_kind == 1)
        spdmax *= 0.7;

    if (gw.idx >= gw.roads_len - 10)
    {
        gw.spd -= 0.005;
        if (gw.spd <= (spdmax * 0.1))
            gw.spd = spdmax * 0.1;
    }
    else
    {
        if (FIXED_SPEED)
        {
            // fixed speed
            gw.spd = spdmax;
        }
        else
        {
            // With acceleration / deceleration
            float a = get_curve_angle(gw.idx);
            if (a < 20.0)
            {
                gw.spd += 0.0025;
                if (gw.spd >= spdmax)
                    gw.spd = spdmax;
            }
            else if (a > 30.0)
            {
                gw.spd -= 0.0025;
                if (gw.spd <= spdmax * 0.4)
                    gw.spd = spdmax * 0.4;
            }
        }
    }
    gw.idx += (((gw.idx_add > 0) ? gw.spd : -gw.spd) * gw.framerate * delta);

    if (gw.idx < 0)
        gw.idx = 0;
    if (gw.idx >= gw.roads_len - 3)
        gw.idx = gw.roads_len - 3;

    gw.ang += (1.0 * gw.framerate * delta);
}

void init_gl(void)
{
    glViewport(0, 0, gw.scrw, gw.scrh);
    glShadeModel(GL_FLAT);
    // glShadeModel(GL_SMOOTH);
    glClearDepth(1.0);
}

void clear_screen(void)
{
    if (gw.fadev >= 1.0)
    {
        glClearColor(0, 0, 0, 1);
    }
    else
    {
        int n = gw.stage_color_num;
        glClearColor(clear_colors[n][0], clear_colors[n][1], clear_colors[n][2], 1.0);
    }
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void draw_gl(float delta)
{
    clear_screen();

    // set ortho
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-gw.view_w, gw.view_w, -gw.view_h, gw.view_h, -gw.zfar, gw.zfar);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    // glCullFace(GL_FRONT);
    glCullFace(GL_BACK);

    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_NORMALIZE);

    // set lighting
    GLfloat light_pos[4] = {1.0, 1.0, 1.0, 0.0};
    GLfloat light_ambient[4] = {0.5, 0.5, 0.5, 1.0};
    GLfloat light_diffuse[4] = {1.0, 1.0, 1.0, 1.0};
    GLfloat light_specular[4] = {0.8, 0.8, 0.8, 1.0};

    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // set material
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    // glColorMaterial(GL_FRONT, GL_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    // get index
    int i = static_cast<int>(gw.idx);
    double frac = gw.idx - static_cast<double>(i);

    // get center position
    double xb, yb;
    if (i < gw.roads_len - 1)
    {
        double x0, y0, x1, y1;
        x0 = gw.roads[i].cx;
        y0 = gw.roads[i].cy;
        x1 = gw.roads[i + 1].cx;
        y1 = gw.roads[i + 1].cy;
        xb = x0 + (x1 - x0) * frac;
        yb = -(y0 + (y1 - y0) * frac);
    }
    else
    {
        double x0, y0;
        x0 = gw.roads[i].cx;
        y0 = gw.roads[i].cy;
        xb = x0;
        yb = -y0;
    }

    if (gw.fadev < 1.0)
    {
        // draw roads and trees
        glPushMatrix();

        glRotatef(30, 1, 0, 0);
        int disp_num = 600;
        draw_roads(i, disp_num, xb, yb);
        draw_trees(i, disp_num, xb, yb);

        // draw car
        {
            double x, y, z;
            float road_angle, scale;

            get_road_pos(gw.idx, 0.75, &x, &z);
            x = x - xb;
            z = -z - yb;
            y = 5.1;
            glTranslatef(x, y, z);

            road_angle = get_road_vec(gw.idx);
            glRotatef(road_angle + 90.0, 0, 1, 0);

            scale = models[gw.model_kind].scale;
            glScalef(scale, scale, scale);

            draw_obj();
        }

        glPopMatrix();
    }

    glLoadIdentity();

    draw_fadeout(gw.fadev);

    if (fps_display != 0)
        draw_fps();

    draw_course_name(delta);
}

void draw_roads(int i, int n, double xb, double yb)
{
    int nmax;
    double px0, py0, px1, py1;
    double lx0, ly0, lx1, ly1;

    px0 = 0.0;
    py0 = 0.0;
    px1 = 0.0;
    py1 = 0.0;
    lx0 = 0.0;
    ly0 = 0.0;
    lx1 = 0.0;
    ly1 = 0.0;

    nmax = gw.roads_len;

    glBegin(GL_QUADS);

    for (int j = -n; j < n; j++)
    {
        double x0, y0, x1, y1;
        double px2, py2, px3, py3;
        double lx2, ly2, lx3, ly3;
        int i0, i1;

        i0 = i + j;
        i1 = i0 + 1;
        if (i0 < 0 || i1 < 0)
            continue;
        if (i0 >= nmax || i1 >= nmax)
            break;

        // get center line position
        x0 = gw.roads[i0].cx - xb;
        x1 = gw.roads[i1].cx - xb;
        y0 = -(gw.roads[i0].cy) - yb;
        y1 = -(gw.roads[i1].cy) - yb;

        // get road edge
        px2 = gw.roads[i0].rx0 - xb;
        px3 = gw.roads[i0].rx1 - xb;
        py2 = -(gw.roads[i0].ry0) - yb;
        py3 = -(gw.roads[i0].ry1) - yb;

        // get white line edge
        lx2 = gw.roads[i0].lx0 - xb;
        lx3 = gw.roads[i0].lx1 - xb;
        ly2 = -(gw.roads[i0].ly0) - yb;
        ly3 = -(gw.roads[i0].ly1) - yb;

        // draw polygons
        if (px0 != 0.0 && py0 != 0.0)
        {
            double z;

            // draw shadow polygon
            glColor4f(0.2, 0.2, 0.2, 1);
            z = 0.0;
            glVertex3d(px0, z, py0);
            glVertex3d(px1, z, py1);
            glVertex3d(px3, z, py3);
            glVertex3d(px2, z, py2);

            // draw road polygon
            if (i0 % 2 == 0)
            {
                glColor4f(0.3, 0.4, 0.45, 1);
            }
            else
            {
                glColor4f(0.35, 0.45, 0.50, 1);
            }
            z = 5.0;
            glVertex3d(px0, z, py0);
            glVertex3d(px1, z, py1);
            glVertex3d(px3, z, py3);
            glVertex3d(px2, z, py2);

            // draw white line polygon
            if (i0 % 2 == 0)
            {
                z = 5.01;
                glColor4f(1.0, 1.0, 1.0, 1.0);
                glVertex3d(lx0, z, ly0);
                glVertex3d(lx1, z, ly1);
                glVertex3d(lx3, z, ly3);
                glVertex3d(lx2, z, ly2);
            }
        }

        px0 = px2;
        py0 = py2;
        px1 = px3;
        py1 = py3;
        lx0 = lx2;
        ly0 = ly2;
        lx1 = lx3;
        ly1 = ly3;
    }
    glEnd();
}

void draw_trees(int idx, int num, double xb, double yb)
{
    int n = gw.stage_color_num;
    glBegin(GL_TRIANGLES);
    for (int a = -num; a < num; a++)
    {
        int i = idx + a;
        if (i < 0)
            continue;
        if (i >= gw.roads_len)
            break;
        if (gw.roads[i].tfg == 0)
            continue;

        double x, y, r;
        int c;
        x = gw.roads[i].tx;
        y = gw.roads[i].ty;
        r = gw.roads[i].r;
        c = gw.roads[i].col;
        x = x - xb;
        y = -y - yb;

        glColor4fv(tree_cols[n][c]);
        glVertex3d(x, r * 0.866 * 2, y);
        glVertex3d(x - r, 0.0, y);
        glVertex3d(x + r, 0.0, y);
    }
    glEnd();
}

void draw_obj(void)
{
    // draw vertex array

    glEnableClientState(GL_VERTEX_ARRAY);
    // glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, models[gw.model_kind].vtx);
    // glTexCoordPointer(2, GL_FLOAT, 0, models[gw.model_kind].uv);
    glNormalPointer(GL_FLOAT, 0, models[gw.model_kind].nml);
    glColorPointer(4, GL_FLOAT, 0, models[gw.model_kind].col);

    glDrawArrays(GL_TRIANGLES, 0, models[gw.model_kind].vtx_size);

    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    // glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}

double get_road_vec(float idx)
{
    if (idx < 0.0)
        idx = 0.0;
    if (idx >= gw.roads_len - 3)
        idx = gw.roads_len - 3;

    int i0 = static_cast<int>(idx);
    double f0 = gw.idx - static_cast<double>(i0);

    double cx, cy, tx, ty, xd, yd, angle;
    cx = gw.roads[i0].cx + (gw.roads[i0 + 1].cx - gw.roads[i0].cx) * f0;
    cy = gw.roads[i0].cy + (gw.roads[i0 + 1].cy - gw.roads[i0].cy) * f0;
    tx = gw.roads[i0 + 1].cx + (gw.roads[i0 + 2].cx - gw.roads[i0 + 1].cx) * f0;
    ty = gw.roads[i0 + 1].cy + (gw.roads[i0 + 2].cy - gw.roads[i0 + 1].cy) * f0;
    xd = tx - cx;
    yd = ty - cy;
    angle = rad2deg(atan2f(yd, xd));
    return angle;
}

double get_curve_angle(float idx)
{
    double sumangle = 0.0;
    double a0 = get_road_vec(idx);
    for (int i = 0; i < 8; i++)
    {
        double a1 = get_road_vec(idx + i + 1);
        double a = a1 - a0;
        if (a > 180.0)
        {
            a = a1 - (a0 * 360.0);
        }
        else if (a < -180.0)
        {
            a = (a1 + 360.0) - a0;
        }
        sumangle += fabsf(a);
        a0 = a1;
    }
    return sumangle;
}

void get_road_pos(float idx, float p, double *x, double *y)
{
    if (idx < 0.0)
        idx = 0.0;
    if (idx >= gw.roads_len - 2)
        idx = gw.roads_len - 2;

    int i0 = static_cast<int>(idx);
    double f0 = idx - static_cast<double>(i0);

    double x0r, y0r, x0l, y0l;
    double x1r, y1r, x1l, y1l;
    double pxr, pxl, pyr, pyl;

    x0r = gw.roads[i0].rx0;
    y0r = gw.roads[i0].ry0;
    x0l = gw.roads[i0].rx1;
    y0l = gw.roads[i0].ry1;
    x1r = gw.roads[i0 + 1].rx0;
    y1r = gw.roads[i0 + 1].ry0;
    x1l = gw.roads[i0 + 1].rx1;
    y1l = gw.roads[i0 + 1].ry1;
    pxr = x0r + (x1r - x0r) * f0;
    pxl = x0l + (x1l - x0l) * f0;
    pyr = y0r + (y1r - y0r) * f0;
    pyl = y0l + (y1l - y0l) * f0;
    *x = pxl + (pxr - pxl) * p;
    *y = pyl + (pyr - pyl) * p;
}

void draw_text(const char *buf, float x, float y, float sdw, int kind, float a)
{
    float z;
    if (a < 0.0)
        a = 0.0;
    if (a >= 1.0)
        a = 1.0;

    // shadow
    if (a >= 1.0)
    {
        z = gw.zfar - 0.002;
        glColor4f(0, 0, 0, a);
        float d = sdw * gw.view_scale;
        glRasterPos3f(x + d, y - d, z);
        glBitmapFontDrawString(buf, kind);
    }

    // text
    z = gw.zfar - 0.001;
    glColor4f(1, 1, 1, a);
    glRasterPos3f(x, y, z);
    glBitmapFontDrawString(buf, kind);
}

void draw_fps(void)
{
    char buf[512];
    sprintf(buf, "FPS %d/%d", gw.count_fps, (int)gw.cfg_framerate);

    float x, y, sdw;
    x = gw.view_w * -0.05;
    y = gw.view_h * 0.9;
    sdw = 1.0;
    draw_text(buf, x, y, sdw, GL_FONT_PROFONT, 1.0);
}

void draw_course_name(float delta)
{
    if (gw.course_name_timer <= 0.0)
        return;

    float x, y, sdw, a;
    x = gw.view_w * -0.9;
    y = gw.view_h * -0.9;
    sdw = 1.0;
    a = (gw.course_name_timer >= 1.0) ? 1.0 : gw.course_name_timer;
    draw_text(course_name[gw.course_num], x, y, sdw, GL_FONT_PROFONT, a);

    gw.course_name_timer -= delta;
    if (gw.course_name_timer <= 0.0)
        gw.course_name_timer = 0.0;
}

void draw_fadeout(float a)
{
    if (a <= 0.0)
        return;

    glDisable(GL_TEXTURE_2D);

    if (a < 1.0)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
    {
        a = 1.0;
        glDisable(GL_BLEND);
    }

    float z, w, h;
    z = gw.zfar - 0.005;
    w = gw.view_w * 2.0;
    h = gw.view_h * 2.0;
    glColor4f(0.0, 0.0, 0.0, a);
    glBegin(GL_QUADS);
    glVertex3f(-w, h, z);
    glVertex3f(-w, -h, z);
    glVertex3f(+w, -h, z);
    glVertex3f(+w, h, z);
    glEnd();
    glDisable(GL_BLEND);
}
