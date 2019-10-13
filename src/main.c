#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include "doot.h"
#include <SOIL/SOIL.h>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

int window_width = 800;
int window_height = 800;
//int resolution_width = 128;
//int resolution_height = 96;
int mouse_pos_x;
int mouse_pos_y;
int fov = 60;

int wall_height = 64;
int wall_height_s = 16;
float rotation_step = 0.5f;
float movement_step = 5.5f;
float gravity = 5.0f;

int camera_height = 0;

float wall_bound = 4.0f;

unsigned char image_buffer[resolution_width * resolution_height * 4];
unsigned char* textures[16];
unsigned char* char_textures[16];

sector s1, s2, s3, s4, s5;
wall w1, w2, w3, w4, w5, w6, w7, w8, w9, w10, w11, w12;
color c1, c2, c3, c4;
//sector sectors[128];

character *player, *enemy, *enemy2;
v_ray v_rays[resolution_width];

void PrepareGame()
{
    player = (character*)malloc(sizeof(character));
    enemy  = (character*)malloc(sizeof(character));
    enemy2 = (character*)malloc(sizeof(character));

    c1.r = 64;
    c1.g = 64;
    c1.b = 64;
    c2.r = 96;
    c2.g = 96;
    c2.b = 96;
    c3.r = 128;
    c3.g = 128;
    c3.b = 128;
    c4.r = 160;
    c4.g = 160;
    c4.b = 160;

    textures[0] = SOIL_load_image("data/textures/floor.bmp", &wall_height, &wall_height, 0, SOIL_LOAD_RGB);
    textures[1] = SOIL_load_image("data/textures/grid.bmp", &wall_height, &wall_height, 0, SOIL_LOAD_RGB);

    char_textures[0] = SOIL_load_image("data/textures/char_1.bmp", &wall_height, &wall_height, 0, SOIL_LOAD_RGB);

    unsigned char temp1, temp2, temp3;

    for(int k = 0; k < 2; k++)
        for(int i = 0; i < wall_height / 2; i++)
        {
            for(int j = 0; j < wall_height; j++)
            {
                temp1 = textures[k][i * 3 * wall_height + j * 3 + 0];
                temp2 = textures[k][i * 3 * wall_height + j * 3 + 1];
                temp3 = textures[k][i * 3 * wall_height + j * 3 + 2];

                textures[k][i * 3 * wall_height + j * 3 + 0] = textures[k][(wall_height - i - 1) * 3 * wall_height + j * 3 + 0];
                textures[k][i * 3 * wall_height + j * 3 + 1] = textures[k][(wall_height - i - 1) * 3 * wall_height + j * 3 + 1];
                textures[k][i * 3 * wall_height + j * 3 + 2] = textures[k][(wall_height - i - 1) * 3 * wall_height + j * 3 + 2];

                textures[k][(wall_height - i - 1) * 3 * wall_height + j * 3 + 0] = temp1;
                textures[k][(wall_height - i - 1) * 3 * wall_height + j * 3 + 1] = temp2;
                textures[k][(wall_height - i - 1) * 3 * wall_height + j * 3 + 2] = temp3;
            }
        }

    for(int i = 0; i < 16; i++)
    {
      s1.characters[i] = NULL;
      s2.characters[i] = NULL;
    }

    s1.width = 384;
    s1.height = 384;
    s1.floor_height = 0;
    s1.ceiling_height = 128;
    s1.colors[4] = c3;
    s1.colors[5] = c1;

    s2.width = 128;
    s2.height = 192;
    s2.floor_height = 16;
    s2.ceiling_height = 144;
    s2.colors[4] = c1;
    s2.colors[5] = c3;

    w1.x1 = 0;
    w1.y1 = 256;
    w1.x2 = 128;
    w1.y2 = 384;
    w1.up_texture = 0;
    w1.mid_texture = 1;
    w1.down_texture = 0;
    w1.angle = 315;
//    w1.s = &s2;
    w1.target_wall = 0;
    w1.height_offset = 0;

    w2.x1 = 128;
    w2.y1 = 384;
    w2.x2 = 256;
    w2.y2 = 384;
    w2.up_texture = 0;
    w2.mid_texture = 1;
    w2.down_texture = 0;
    w2.angle = 0;
    w2.s = &s1;
    w2.target_wall = 5;
    w2.height_offset = -15;

    w3.x1 = 256;
    w3.y1 = 384;
    w3.x2 = 384;
    w3.y2 = 256;
    w3.up_texture = 0;
    w3.mid_texture = 1;
    w3.down_texture = 0;
    w3.angle = 45;
//    w3.s = &s1;
    w3.target_wall = 6;
    w3.height_offset = 0;

    w4.x1 = 384;
    w4.y1 = 128;
    w4.x2 = 384;
    w4.y2 = 256;
    w4.up_texture = 0;
    w4.mid_texture = 1;
    w4.down_texture = 0;
    w4.angle = 90;
//    w4.s = &s1;
    w4.target_wall = 7;
    w4.height_offset = 0;

    w5.x2 = 384;
    w5.y2 = 128;
    w5.x1 = 256;
    w5.y1 = 0;
    w5.up_texture = 0;
    w5.mid_texture = 1;
    w5.down_texture = 0;
    w5.angle = 135;
//    w5.s = &s1;
    w5.target_wall = 0;
    w5.height_offset = 0;

    w6.x2 = 256;
    w6.y2 = 0;
    w6.x1 = 128;
    w6.y1 = 0;
    w6.up_texture = 0;
    w6.mid_texture = 1;
    w6.down_texture = 0;
    w6.angle = 180;
//    w6.s = &s1;
    w6.target_wall = 1;
    w6.height_offset = 0;

    w7.x2 = 128;
    w7.y2 = 0;
    w7.x1 = 0;
    w7.y1 = 128;
    w7.up_texture = 0;
    w7.mid_texture = 1;
    w7.down_texture = 0;
    w7.angle = 225;
//    w7.s = &s1;
    w7.target_wall = 2;
    w7.height_offset = 0;

    w8.x1 = 0;
    w8.y1 = 128;
    w8.x2 = 0;
    w8.y2 = 256;
    w8.up_texture = 0;
    w8.mid_texture = 1;
    w8.down_texture = 0;
    w8.angle = 270;
//    w8.s = &s1;
    w8.target_wall = 3;
    w8.height_offset = 0;

    w9.x1 = 0;
    w9.y1 = 192;
    w9.x2 = 128;
    w9.y2 = 192;
    w9.up_texture = 1;
    w9.mid_texture = 1;
    w9.down_texture = 1;
    w9.angle = 0;
    w9.s = &s1;
    w9.target_wall = 0;
    w9.height_offset = 0;

    w10.x1 = 128;
    w10.y1 = 0;
    w10.x2 = 128;
    w10.y2 = 192;
    w10.up_texture = 1;
    w10.mid_texture = 1;
    w10.down_texture = 1;
    w10.angle = 90;
    w10.height_offset = 0;

    w11.x1 = 0;
    w11.y1 = 0;
    w11.x2 = 128;
    w11.y2 = 0;
    w11.up_texture = 1;
    w11.mid_texture = 1;
    w11.down_texture = 1;
    w11.angle = 180;
    w11.s = &s2;
    w11.target_wall = 1;
    w11.height_offset = 0;

    w12.x1 = 0;
    w12.y1 = 0;
    w12.x2 = 0;
    w12.y2 = 192;
    w12.up_texture = 1;
    w12.mid_texture = 1;
    w12.down_texture = 1;
    w12.angle = 270;
    w12.height_offset = 0;

    s1.walls[0] = &w1;
    s1.walls[1] = &w2;
    s1.walls[2] = &w3;
    s1.walls[3] = &w4;
    s1.walls[4] = &w5;
    s1.walls[5] = &w6;
    s1.walls[6] = &w7;
    s1.walls[7] = &w8;

    s1.n_walls = 8;

    s2.walls[0] = &w9;
    s2.walls[1] = &w10;
    s2.walls[2] = &w11;
    s2.walls[3] = &w12;

    s2.n_walls = 4;

    player->height = 0;
    player->angle = 0;
    player->sector_p = &s1;
    player->pos_x = 192;
    player->pos_y = 288;
    player->is_player = 1;

    enemy->height = 0;
    enemy->angle = 0;
    enemy->sector_p = &s1;
    enemy->pos_x = 192;
    enemy->pos_y = 192;
    enemy->is_player = 0;
    enemy->texture = 0;
    enemy->radius = 32;
 
    enemy2->height = 0;
    enemy2->angle = 0;
    enemy2->sector_p = &s1;
    enemy2->pos_x = 256;
    enemy2->pos_y = 200;
    enemy2->is_player = 0;
    enemy2->texture = 0;
    enemy2->radius = 32;
}

int main(int argc, char **argv)
{
    PrepareGame();

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(-1, -1);
	glutInitWindowSize(window_width, window_height);
    glutCreateWindow("DooT");
	glutDisplayFunc(RenderScene);
	glutReshapeFunc(ChangeSize);
	glutIdleFunc(ProcessGame);
//	glutKeyboardFunc(ProcessNormalKeys);
    glutKeyboardUpFunc(KeyUp);
    glutKeyboardFunc(KeyDown);
	glutSpecialFunc(ProcessSpecialKeys);
	glutPassiveMotionFunc(ProcessMouseMovement);

	glutMainLoop();

	return 1;
}
