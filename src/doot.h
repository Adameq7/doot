#include <stdbool.h>

#define resolution_width 400
#define resolution_height 256
#define MAX_CHARACTER_SECTOR_NUM 16
#define RAD_TO_DEG 57.29577951308232
#define DEG_TO_RAD 0.017453292519943

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
void intersect(float, float, float, float, float, float, float, float, float*, float*);

typedef struct sector sector;
typedef struct color color;
typedef struct character character;
typedef struct slice slice;
typedef struct wall wall;
typedef struct v_ray v_ray;
typedef struct v_ray_slice v_ray_slice;
typedef struct character_slice character_slice;

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
    character* characters[MAX_CHARACTER_SECTOR_NUM];
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

struct character
{
    sector* sector_p;
    float pos_x, pos_y, x21, y21, x22, y22;
    float angle;
    float height;
    int radius;
    int texture;
    float dist;
    bool is_player;
};

struct character_slice
{
  character *chara;
//  float dist;
  float hit_x, hit_y, hit_tx, hit_dangle, hit_length, hit_height;
};

struct slice
{
//    int x, y;
    bool draw_middle;
    bool hit_character;

    float x1, y1, p_x, p_y;
    float length;
    float angle;

    character_slice character_slices[MAX_CHARACTER_SECTOR_NUM];
    int character_slices_ind;

    int floor_height, ceiling_height;

    char wall;
    sector* s;
};
