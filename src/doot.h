#include <stdbool.h>

#define resolution_width 256
#define resolution_height 256
//#define info_dump 0

void RenderScene(void);
void ChangeSize(int, int);
void ProcessGame(void);
void KeyUp(unsigned char, int, int);
void KeyDown(unsigned char, int, int);
void ProcessSpecialKeys(int, int, int);
void ProcessMouseMovement(int, int);

float absf(float);
float dist(float, float, float, float);

typedef struct sector sector;
typedef struct color color;
typedef struct character character;
typedef struct slice slice;
typedef struct wall wall;
typedef struct v_ray v_ray;
typedef struct v_ray_slice v_ray_slice;

struct color
{
    int r, g, b;
};

struct wall
{
    float angle;
    int x1, x2, y1, y2;
    int up_texture, mid_texture, down_texture;
    int target_wall;
    int height_offset;
    sector* s;
};

struct sector
{
    int width;
    int height;

    int floor_height;
    int ceiling_height;
/*
    sector* sectors[4];

    int texture[6];
    int up_texture[4];
    int down_texture[4];

    color colors[6];
*/

    int n_walls;
    wall* walls[16];
    color colors[6];
};

struct v_ray_slice
{
    int x, y, p_x, p_y;
};

struct v_ray
{
    int slices;
    v_ray_slice v_slices[2];
};

struct slice
{
//    int x, y;
    bool draw_middle;

    float x1, y1, p_x, p_y;
    float length;
    float angle;

    int floor_height, ceiling_height;

    char wall;
    sector* s;
};

struct character
{
    sector* sector_p;
    float pos_x, pos_y;
    float angle;
    float height;
    int texture;
};
