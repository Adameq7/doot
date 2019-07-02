#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include "doot.h"
#include <SOIL/SOIL.h>
#include <string.h>

bool info_dump = 0;

extern int fov;
extern int mouse_pos_x;
extern int mouse_pos_y;
extern int wall_height ;
extern float rotation_step;
extern float movement_step;
extern float gravity;

extern int camera_height;
extern float wall_bound;

extern sector s1, s2, s3, s4, s5;
extern color c1, c2, c3, c4;

extern character *player;
extern character *enemy;

extern int window_width, window_height;
extern unsigned char image_buffer[];
extern unsigned char* textures[];
extern unsigned char* char_textures[];
extern v_ray v_rays[resolution_width];

bool keys[256];

float disp_amount = 0.0f;
float text_corr = 0.0f;

void CleanSector(sector *s)
{
  for(int i = 0; i < MAX_CHARACTER_SECTOR_NUM; i++)
    s->characters[i] = NULL;
}

void AddCharacter(sector *s, character *chara)
{
  for(int i = 0; i < MAX_CHARACTER_SECTOR_NUM; i++)
  {
    if(s->characters[i] == NULL)
    {
      s->characters[i] = chara;
      return;
    }
  }

  printf("not enough space in sector\n");

  return;
}

void KeyUp(unsigned char c, int x, int y)
{
    keys[c] = false;
}

void KeyDown(unsigned char c, int x, int y)
{
    keys[c] = true;
}

float max(float a, float b)
{
    return a > b ? a : b;
}

float min(float a, float b)
{
    return a < b ? a : b;
}

float absf(float a)
{
    return a > 0 ? a : -a;
}

void intersect(float x11, float y11,
                float x12, float y12, 
                float x21, float y21, 
                float x22, float y22,
                float *x, float *y)
{
  float a1, a2, b1, b2, d_x, d_y;

  d_x = x12 - x11;
  d_y = y12 - y11;
  a1 = d_x != 0 ? d_y / d_x : 0;
  b1 = y11 - a1 * x11; 

  d_x = x22 - x21;
  d_y = y22 - y21;
  a2 = d_x != 0 ? d_y / d_x : 0;
  b2 = y21 - a2 * x21; 

  if(a1 - a2 == 0)
    return;

  *x = (b2 - b1) / (a1 - a2);
  *y = *x * a1 + b1;  
}

void ProcessKeys();

void switch_sector(character* player, int wall_index, sector* s, int a, bool vert)
{
                        wall* w = s->walls[wall_index];

                        float A = (vert ? 1 : a), B = (vert ? 0 : -1), C = w->y1 - A * w->x1, px, py, dist;

                        px = vert ? w->x1 : (B * (B * player->pos_x - A * player->pos_y) - A * C) / (A * A + B * B);
                        py = (A * (-B * player->pos_x + A * player->pos_y) - B * C) / (A * A + B * B);
                        dist = vert ? absf(player->pos_x - w->x1) : absf(A * player->pos_x + B * player->pos_y + C) / sqrt(A * A + B * B);

//                        printf("%d %d %d %d\n",w->x1, w->y1, w->x2, w->y2);

                        float n_p_x = px - w->x1;
                        float n_p_y = py - w->y1;
/*
                        if(vert)
                            n_p_x = 0;
*/
                        float rp = (n_p_x + absf(n_p_y)) / (w->x2 - w->x1 + absf(w->y2 - w->y1));

                        if(((w->angle <= 90 || w->angle > 270) &&
                            (w->s->walls[w->target_wall]->angle <= 90 || w->s->walls[w->target_wall]->angle > 270)) ||
                            ((w->angle > 90 && w->angle <= 270) &&
                            (w->s->walls[w->target_wall]->angle > 90 && w->s->walls[w->target_wall]->angle <= 270)))
                                rp = 1 - rp;

                        if(absf(w->s->walls[w->target_wall]->x1 - w->s->walls[w->target_wall]->x2) < 0.01)
                            rp = 1 - rp;
/*
                        if(vert)
                            rp += 1;
*/
//                        printf("%f %f %f %f %f \t %f\n", px, py, n_p_x, n_p_y, rp, dist);
/*
                        if(angle)
                            rp = 1 - rp;
*/
                        float w_x = w->s->walls[w->target_wall]->x2 - w->s->walls[w->target_wall]->x1;
                        float w_y = (w->s->walls[w->target_wall]->y2 - w->s->walls[w->target_wall]->y1);

//                        printf("b %f %f\n", player->pos_x, player->pos_y);

                        player->pos_x = w->s->walls[w->target_wall]->x1 + w_x * rp + sin((w->s->walls[w->target_wall]->angle + 180) * M_PI / 180) * dist;
                        player->pos_y = w->s->walls[w->target_wall]->y1 + w_y * rp + cos((w->s->walls[w->target_wall]->angle + 180) * M_PI / 180) * dist;

//                        printf("a %f %f\n", player->pos_x, player->pos_y);

                        float d_angle = w->angle - player->angle;

                        player->angle = w->s->walls[w->target_wall]->angle + 180 - d_angle;
                        player->sector_p = w->s;
}

void ProcessCharacter(character* chara)
{
    if(chara->height > chara->sector_p->floor_height)
        chara->height -= gravity;

    if(chara->height < chara->sector_p->floor_height)
        chara->height = chara->sector_p->floor_height;

    if(chara->angle >= 360)
        chara->angle -= 360;
    if(chara->angle < 0)
        chara->angle += 360;

    sector* s = chara->sector_p;
    wall* w;
    int wall_index;

    for(int i = 0; i < chara->sector_p->n_walls; i++)
    {
        w = s->walls[i];

        float d_x = w->x2 - w->x1;
        float d_y = w->y2 - w->y1;

        float a = d_x != 0 ? d_y / d_x : 0;
        float b = w->y1 - a * w->x1;

        float a2 = 0;
        float b2 = 0;

        float t_x = 0;

        if(a == 0)
        {
            if(w->angle == 270.0 && chara->pos_x < 0)
            {
                if(w->s != NULL)
                    switch_sector(chara, i, s, a, 1);
                else
                    chara->pos_x = 0;
            }

            if(w->angle == 90.0 && chara->pos_x > w->x1)
            {
                if(w->s != NULL)
                    switch_sector(chara, i, s, a, 1);
                else
                    chara->pos_x = w->x1;
            }
        }

        if(d_x != 0 && ((w->angle > 270) || (w->angle <= 90)))
        {
            if(chara->pos_y > a * chara->pos_x + b)
            {
                if(w->s != NULL)
                {
                    switch_sector(chara, i, s, a, 0);

                    break;
                }

                if(a != 0)
                {
                    a2 = -1.0 / a;
                    b2 = chara->pos_y - a2 * chara->pos_x;

                    t_x = (b2 - b) / (a - a2);

                    chara->pos_x = t_x;
                    chara->pos_y = a * t_x + b;
                }
                else
                {
                    chara->pos_y = a * chara->pos_x + b;
                }
            }
        }
        if(d_x != 0 && ((w->angle <= 270) && (w->angle > 90)))
        {
            if(chara->pos_y < a * chara->pos_x + b)
            {
                    if(w->s != NULL)
                    {
                        switch_sector(chara, i, s, a, 0);

                        break;
                    }

                if(a != 0)
                {
                    a2 = -1.0 / a;
                    b2 = chara->pos_y - a2 * chara->pos_x;

                    t_x = (b2 - b) / (a - a2);

                    chara->pos_x = t_x;
                    chara->pos_y = a * t_x + b;
                }
                else
                {
                    chara->pos_y = a * chara->pos_x + b;
                }
            }
        }
    }

  AddCharacter(chara->sector_p, chara);

  if(chara != player)
  {
    chara->x22 = chara->pos_x + (chara->radius) * sin((player->angle - 90) * (M_PI / 180));
    chara->x21 = chara->pos_x + (chara->radius) * sin((player->angle + 90) * (M_PI / 180));
    chara->y22 = chara->pos_y + (chara->radius) * cos((player->angle - 90) * (M_PI / 180));
    chara->y21 = chara->pos_y + (chara->radius) * cos((player->angle + 90) * (M_PI / 180));
  }
}

void CreateView()
{
    camera_height = player->height;

    sector* s = player->sector_p;

    float p_x = 0, p_y = 0;
    char s_wall;
    bool found_wall = false;
    bool hit_wall = false;

    float length = 0;
    float angle = player->angle - (fov / 2);

    float x1 = player->pos_x;
    float y1 = player->pos_y;
    float x2 = player->pos_x + sin(angle * M_PI / 180) * 10;
    float y2 = player->pos_y + cos(angle * M_PI / 180) * 10;

    float x3 = 0;
    float y3 = 0;
    float x4 = 0;
    float y4 = 0;

    float p_x_t = 0;
    float p_y_t = 0;

//    memset(image_buffer, resolution_height, sizeof(image_buffer));
//    memset(image_buffer, resolution_height / 2, sizeof(image_buffer) / 2);

    float cur_disp = disp_amount / 2;

    for(int i = 0; i < resolution_width; i++)
    {
        if(angle < 0)
            angle += 360;
        if(angle >= 360)
            angle -= 360;

        hit_wall = false;
        length = 0;

        x1 = player->pos_x + sin(player->angle * M_PI / 180) * cur_disp;
        y1 = player->pos_y + cos(player->angle * M_PI / 180) * cur_disp;

        s = player->sector_p; 

        int n_slices = 0;
        slice slices[16];

        float min_x, max_x, min_y, max_y, delta_angle;

        if(info_dump)
            printf("%d:\n", i);

        int delta_height = 0;

        while(!hit_wall)
        {
            if(length > 2048)
            {
                hit_wall = true;
            }

            if(angle < 0)
                angle += 360;
            if(angle >= 360)
                angle -= 360;

            x2 = x1 + sin(angle * M_PI / 180) * 10;
            y2 = y1 + cos(angle * M_PI / 180) * 10;

            p_x = 0;
            p_y = 0;

            found_wall = false;

            slices[n_slices].hit_character = 0;

            for(int k = 0; k < MAX_CHARACTER_SECTOR_NUM; k++)
            {
              if(s->characters[k] == NULL)
                break;

              if(s->characters[k] != player)
              {
//                printf("found %p\n", s->characters[i]);

                float ehx, ehy;

                intersect(x1, y1, x2, y2, 
                          s->characters[k]->pos_x, s->characters[k]->pos_y,
                          s->characters[k]->x21, s->characters[k]->y21,
                          &ehx, &ehy);

//                printf("%f %f %d\n", ehx, ehy, i);

                if(dist(s->characters[k]->pos_x, s->characters[k]->pos_y, ehx, ehy) < s->characters[k]->radius)
                {
//                  printf("hit");
//                  printf("hit %p %d\n", s->characters[k], k);

                  float ct_x = sqrt(absf(ehx - s->characters[k]->x21) * 
                                    absf(ehx - s->characters[k]->x21) +
                                    absf(ehy - s->characters[k]->y21) *
                                    absf(ehy - s->characters[k]->y21));

                  float angle2 = atan2((x1 - ehx), (y1 - ehy));
                  float angle_temp = angle * DEG_TO_RAD;
                  
//                  float dangle = absf(angle - ((180 / M_PI) * atan2((x1 - s->characters[k]->pos_x), (y1 - s->characters[k]->pos_y))));
                  float dangle = min((2 * M_PI) - absf(angle_temp - angle2), absf(angle_temp - angle2));

//                  dangle = 180 - dangle;

//                  printf("%f %f %f\n", angle_temp, angle2, dangle);

                  slices[n_slices].hit_character = (absf(dangle) > M_PI / 2);
                  slices[n_slices].chara = s->characters[k];
                  slices[n_slices].hit_x = ehx;
                  slices[n_slices].hit_y = ehy;
                  slices[n_slices].hit_tx = ct_x;
                  slices[n_slices].hit_dangle = dangle;
                  slices[n_slices].hit_length = length + dist(x1, y1, s->characters[k]->pos_x, s->characters[k]->pos_y);
                  slices[n_slices].hit_height = s->characters[k]->height + delta_height;
                }

//                printf("\n");

                break;
              }
            }

            if(info_dump)
                printf("angle: %f\n",angle);

            for(int l = 0; l < s->n_walls; l++)
            {
                s_wall = l;

                x3 = s->walls[l]->x1;
                y3 = s->walls[l]->y1;
                x4 = s->walls[l]->x2;
                y4 = s->walls[l]->y2;

                p_x_t = ((x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4));
                p_y_t = ((x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4));
                p_x = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4)) / ((p_x_t != 0) ? p_x_t : 1);
                p_y = ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4)) / ((p_y_t != 0) ? p_y_t : 1);

                if(x3 < x4)
                {
                    min_x = x3;
                    max_x = x4;
                }
                else
                {
                    min_x = x4;
                    max_x = x3;
                }

                if(y3 < y4)
                {
                    min_y = y3;
                    max_y = y4;
                }
                else
                {
                    min_y = y4;
                    max_y = y3;
                }

                p_x = (int)(p_x + 0.5f);
                p_y = (int)(p_y + 0.5f);

//                max_x += 1;
//                max_y += 1;

//                printf("%d %d %f %f\n", i, wall, p_x, p_y);

                delta_angle = min(360 - absf(s->walls[l]->angle - angle), absf(s->walls[l]->angle - angle));

                if( (p_x >= min_x) && (p_x <= max_x) && (p_y >= min_y) && (p_y <= max_y) && (delta_angle <= 90) )
                {
//                    if(info_dump)
//                        printf("\tf: %d %d %f %f %f %f %f %f\n", n_slices, l, p_x, p_y, min_x, max_x, min_y, max_y);
                    break;
                }
                else
                {
                    if(info_dump && absf(p_x) > 100000)
                        printf("%f %f %f %f\n%f %f %f %f\n\n", x1, y1, x2, y2, x3, y3, x4, y4);
//                        printf("\t%d %d %f %f %f %f %f %f\n", n_slices, l, p_x, p_y, min_x, max_x, min_y, max_y);
                }
            }

//            printf("%f %f %d\n", p_x, p_y, wall);

            length += sqrt(absf(x1 - p_x) * absf(x1 - p_x) + absf(y1 - p_y) * absf(y1 - p_y));// * cos(absf(player->angle - angle) * M_PI / 180);

            slices[n_slices].s = s;
            slices[n_slices].p_x = p_x;
            slices[n_slices].p_y = p_y;
            slices[n_slices].x1 = x1;
            slices[n_slices].y1 = y1;
            slices[n_slices].length = length;
            slices[n_slices].wall = s_wall;
            slices[n_slices].draw_middle = (s->walls[s_wall]->s == NULL);
            slices[n_slices].angle = angle;
            slices[n_slices].floor_height = s->floor_height + delta_height;
            slices[n_slices].ceiling_height = s->ceiling_height + delta_height;

            delta_height += s->walls[s_wall]->height_offset;

            n_slices++;

            if(s->walls[s_wall]->s == NULL)
                hit_wall = true;
            else
            {
//                delta_angle = min(360 - absf(s->walls[l]->angle - angle), absf(s->walls[l]->angle - angle));
                float x3 = s->walls[s_wall]->x1;
                float y3 = s->walls[s_wall]->y1;
                float x4 = s->walls[s_wall]->x2 - x3;
                float y4 = s->walls[s_wall]->y2 - y3;

                float n_p_x = p_x - x3;
                float n_p_y = p_y - y3;

//                float c_x = (s->walls[wall]->angle >= 90 && s->walls[wall]->angle < 270) ? x4 : x3,
//                      c_y = (s->walls[wall]->angle >= 90 && s->walls[wall]->angle < 270) ? y4 : y3;

                float t_x = (n_p_x + absf(n_p_y)) / (x4 + absf(y4));

//                printf("b: %f\n",t_x);

                if(((s->walls[s_wall]->angle > 90 && s->walls[s_wall]->angle <= 270) &&
                    (s->walls[s_wall]->s->walls[s->walls[s_wall]->target_wall]->angle > 90 && s->walls[s_wall]->s->walls[s->walls[s_wall]->target_wall]->angle <= 270)) ||
                    ((s->walls[s_wall]->angle <= 90 || s->walls[s_wall]->angle > 270) &&
                    (s->walls[s_wall]->s->walls[s->walls[s_wall]->target_wall]->angle <= 90 || s->walls[s_wall]->s->walls[s->walls[s_wall]->target_wall]->angle > 270)))
                        t_x = 1 - t_x;

/*
                    if(s->walls[s_wall]->angle >= 90 && s->walls[s_wall]->angle <= 270)
                        t_x = 1 - t_x;
*/
//                printf("a: %f\n",t_x);

//                float t_x = (int)(sqrt(absf(p_x - c_x) * absf(p_x - c_x) + absf(p_y - c_y) * absf(p_y - c_y)));

                if(info_dump)
                    printf("\t%f\n", t_x);

                float d_angle = s->walls[s_wall]->angle - angle;

                if(info_dump)
                    printf("\t%f\n", d_angle);

                wall w = *(s->walls[s_wall]->s->walls[s->walls[s_wall]->target_wall]);
/*
                x1 = w.x1 + absf(w.x2 - w.x1) * (w.angle >= 90 && w.angle < 270 ? t_x : 1 - t_x);
                y1 = w.y1 + absf(w.y1 - w.y2) * (w.angle >= 90 && w.angle < 270 ? t_x : 1 - t_x);
*/
                if(absf(w.x1 - w.x2) < 0.01)
                    t_x = 1 - t_x;

                x1 = w.x1 + absf(w.x2 - w.x1) * t_x;
                y1 = w.y1 + (w.y2 - w.y1) * t_x;

                angle = w.angle + 180 - d_angle;

                if(angle < 0)
                    angle += 360;
                if(angle >= 360)
                    angle -= 360;

                sector* new_s = s->walls[s_wall]->s;
                s = new_s;

                x1 += sin(angle * M_PI / 180) * 1;
                y1 += cos(angle * M_PI / 180) * 1;
            }
        }

        bool is_drawn[resolution_height];

        memset(is_drawn, false, resolution_height);

        v_rays[i].slices = n_slices > 2 ? 2 : n_slices;

        for(int k = 0; k < n_slices; k++)
        {
            if(k < 2)
            {
                v_rays[i].v_slices[k].x = slices[k].x1;
                v_rays[i].v_slices[k].y = slices[k].y1;
                v_rays[i].v_slices[k].p_x = slices[k].p_x;
                v_rays[i].v_slices[k].p_y = slices[k].p_y;
            }

            float n_length = cos(absf(player->angle - slices[0].angle) * M_PI / 180) * slices[k].length / 24;

            float dist = (1 * (n_length > 0.01 ? n_length : 0.01));
            float room_height = (float)((slices[k].ceiling_height) - (slices[k].floor_height)) / wall_height;
            float proj_size = (float)resolution_height / dist;
            int camera_offset = - 8 * camera_height / dist;

            int tv = camera_offset + 8 * (float)(slices[k].floor_height) / dist + (float)resolution_height / 2;

            int x = tv - proj_size + 0.5f;
            int y = tv + proj_size + wall_height * 8 * (room_height - 1) / dist; 

            //-----------------------------------------

//            printf("%d\n", i);
//            printf("-\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n", 
//                   n_length,dist,proj_size,camera_offset,tv,x,y);

            if(slices[k].hit_character)
            {
              float n_length_char = /*cos(absf(player->angle - slices[0].angle) * M_PI / 180) */ slices[k].hit_length / 12;

              float dist_char = (1 * (n_length_char > 0.01 ? n_length_char : 0.01));
              float proj_size_char = (float)resolution_height / dist_char;
              int camera_offset_char = 8 * (camera_height - 32) / dist_char;

              int tv_char = camera_offset_char + 8 * (float)(slices[k].hit_height) / dist_char + (float)resolution_height / 2;

              int x_char = tv_char - proj_size_char + 0.5f;
              int y_char = tv_char + proj_size_char + wall_height * 8 / dist_char; 

//              float ratio_char = (y - x) / slices[k].hit_height;
              float ratio_char = 64.0 / (y_char - x_char);

//              x_char = x_char > 0 ? x_char : 0;
//              y_char = y_char < resolution_height ? y_char : resolution_height - 1;
              slices[k].hit_tx = (slices[k].hit_tx >= 0 ? slices[k].hit_tx : 0);
              slices[k].hit_tx = (slices[k].hit_tx < 64 ? slices[k].hit_tx : 63);

//              z = x_char;

              for(int j = (x_char > 0 ? x_char : 0); j < (y_char < resolution_height ? y_char : resolution_height - 1); j++)
              {
                  if(!is_drawn[j])
                  {
                      is_drawn[j] = true;

                      image_buffer[(resolution_width * j + i) * 4 + 0] = char_textures[slices[k].chara->texture][((int)slices[k].hit_tx + (int)((j - x_char) * ratio_char) * 64) * 3 + 0];
                      image_buffer[(resolution_width * j + i) * 4 + 1] = char_textures[slices[k].chara->texture][((int)slices[k].hit_tx + (int)((j - x_char) * ratio_char) * 64) * 3 + 1];
                      image_buffer[(resolution_width * j + i) * 4 + 2] = char_textures[slices[k].chara->texture][((int)slices[k].hit_tx + (int)((j - x_char) * ratio_char) * 64) * 3 + 2];
                      image_buffer[(resolution_width * j + i) * 4 + 3] = 255;

//                      z += ratio_char;
                  }
              }

//            printf("---\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n", 
//                   n_length_char,dist_char,proj_size_char,camera_offset_char,tv_char,x_char,y_char);
//              printf("%d %d %f %d\n", x_char, y_char, ratio_char, i);
            }
                        

            if(info_dump)
            {
                printf("%d: %f %f+%f %d %d\r\n", i, tv - proj_size, tv + proj_size, wall_height * 8 * (room_height - 1) / dist, x, y);
            }
//            int t_x = slices[k].wall % 2 ? (int)slices[k].p_y % wall_height : (int)slices[k].p_x % wall_height;

            float x3 = slices[k].s->walls[slices[k].wall]->x1;
            float y3 = slices[k].s->walls[slices[k].wall]->y1;
            float x4 = slices[k].s->walls[slices[k].wall]->x2;
            float y4 = slices[k].s->walls[slices[k].wall]->y2;

            float c_x = (slices[k].s->walls[slices[k].wall]->angle >= 90 && slices[k].s->walls[slices[k].wall]->angle < 270) ? x4 : x3,
                  c_y = (slices[k].s->walls[slices[k].wall]->angle >= 90 && slices[k].s->walls[slices[k].wall]->angle < 270) ? y4 : y3;

            int t_x = (int)(sqrt(absf(slices[k].p_x - c_x) * absf(slices[k].p_x - c_x) + absf(slices[k].p_y - c_y) * absf(slices[k].p_y - c_y))) % wall_height;

//            if(k == 0)
//              printf("%d %d\t%f %f\t%f %f\t\t%f %f\n", i, t_x, x3, y3, x4, y4, p_x, p_y);

            float stripe_dist = sqrt(absf(slices[k].x1 - slices[k].p_x) * absf(slices[k].x1 - slices[k].p_x) + absf(slices[k].y1 - slices[k].p_y) * absf(slices[k].y1 - slices[k].p_y));

            float l = (float)(slices[k].ceiling_height - slices[k].floor_height) / (float)(y - x);

            float pos_x = slices[k].x1;
            float pos_y = slices[k].y1;
            int f_t_x, f_t_y;
            float step = 0;

            float h = (x < resolution_height ? x : resolution_height);

            float weight = 0;
            float start_dist = wall_height;

            for(int j = 0; j < h; j++)
            {
                if(j < 0 || j > resolution_height - 1)
                    break;
/*
                weight = (float)(j + 1) / h;

                pos_x = slices[k].x1 +
                        sin(slices[k].angle * M_PI / 180) * start_dist +
                        weight * sin(slices[k].angle * M_PI / 180) * (stripe_dist - start_dist);

                pos_y = slices[k].y1 +
                        cos(slices[k].angle * M_PI / 180) * start_dist +
                        weight * cos(slices[k].angle * M_PI / 180) * (stripe_dist - start_dist);


                f_t_x = (int)pos_x % wall_height;
                f_t_y = (int)pos_y % wall_height;

                if(f_t_x < 0)
                    f_t_x += wall_height;
                if(f_t_y < 0)
                    f_t_y += wall_height;

                if(info_dump)
                {
                    printf("%d %f\n", i, slices[k].length);
                }

                if(!is_drawn[j])
                {
                    is_drawn[j] = true;

                    image_buffer[(resolution_width * j + i) * 4 + 0] = textures[slices[k].s->texture[5]][(wall_height * f_t_y + f_t_x) * 3 + 0];
                    image_buffer[(resolution_width * j + i) * 4 + 1] = textures[slices[k].s->texture[5]][(wall_height * f_t_y + f_t_x) * 3 + 1];
                    image_buffer[(resolution_width * j + i) * 4 + 2] = textures[slices[k].s->texture[5]][(wall_height * f_t_y + f_t_x) * 3 + 2];
                    image_buffer[(resolution_width * j + i) * 4 + 3] = 255;
                }
*/
//                weight += stripe_dist / h;
                if(!is_drawn[j])
                {
                    is_drawn[j] = true;

                    image_buffer[(resolution_width * j + i) * 4 + 0] = slices[k].s->colors[5].r;
                    image_buffer[(resolution_width * j + i) * 4 + 1] = slices[k].s->colors[5].g;
                    image_buffer[(resolution_width * j + i) * 4 + 2] = slices[k].s->colors[5].b;
                    image_buffer[(resolution_width * j + i) * 4 + 3] = 255;
                }
/*
                image_buffer[(resolution_width * j + i) * 4 + 0] = slices[k].s->colors[5].r;
                image_buffer[(resolution_width * j + i) * 4 + 1] = slices[k].s->colors[5].g;
                image_buffer[(resolution_width * j + i) * 4 + 2] = slices[k].s->colors[5].b;
                image_buffer[(resolution_width * j + i) * 4 + 3] = 255;
*/
            }

            for(int j = y; j < resolution_height; j++)
            {
                if(j < 0 || j > resolution_height - 1)
                    break;

                if(!is_drawn[j])
                {
                    is_drawn[j] = true;

                    image_buffer[(resolution_width * j + i) * 4 + 0] = slices[k].s->colors[4].r;
                    image_buffer[(resolution_width * j + i) * 4 + 1] = slices[k].s->colors[4].g;
                    image_buffer[(resolution_width * j + i) * 4 + 2] = slices[k].s->colors[4].b;
                    image_buffer[(resolution_width * j + i) * 4 + 3] = 255;
                }
            }

            int l_y = 0, u_y = 0;
            float ratio = (float)((slices[k].ceiling_height) - (slices[k].floor_height)) / (y - x);

            if(slices[k].s->walls[slices[k].wall]->s != NULL)
            {
//                l_y = (slices[k].s->walls[slices[k].wall]->s->floor_height) - (slices[k].s->floor_height);
//                u_y = (slices[k].s->ceiling_height) - (slices[k].s->walls[slices[k].wall]->s->ceiling_height);
                l_y = (slices[k + 1].floor_height) - (slices[k].floor_height);
                u_y = (slices[k].ceiling_height) - (slices[k + 1].ceiling_height);
            }

//            printf("%d %d %d\n", (slices[k].s->ceiling_height), (slices[k].s->sectors[slices[k].wall]->ceiling_height), u_y);

            l_y = l_y > 0 ? l_y : 0;
            u_y = u_y > 0 ? u_y : 0;

            l_y /= ratio;
            u_y /= ratio;

//            printf("%d: %d %d %d %d\n", i, x, l_y, u_y, y);
                for(int j = (x >= 0 ? x : 0); j < (x >= 0 ? x : 0) + (l_y  + (x < 0 ? x : 0) < resolution_height ? l_y  + (x < 0 ? x : 0) : resolution_height); j++)
                {
                    if(j < 0 || j > resolution_height - 1)
                        break;

                    if(!is_drawn[j])
                    {
                        is_drawn[j] = true;

                        image_buffer[(resolution_width * j + i) * 4 + 0] = textures[slices[k].s->walls[slices[k].wall]->down_texture][(((int)((j - x) * l) * wall_height + t_x) % (64 * 64)) * 3 + 0];
                        image_buffer[(resolution_width * j + i) * 4 + 1] = textures[slices[k].s->walls[slices[k].wall]->down_texture][(((int)((j - x) * l) * wall_height + t_x) % (64 * 64)) * 3 + 1];
                        image_buffer[(resolution_width * j + i) * 4 + 2] = textures[slices[k].s->walls[slices[k].wall]->down_texture][(((int)((j - x) * l) * wall_height + t_x) % (64 * 64)) * 3 + 2];
                        image_buffer[(resolution_width * j + i) * 4 + 3] = 255;
                    }
                }

            if(slices[k].draw_middle)
                for(int j = (x >= 0 ? x : 0) + (l_y < resolution_height ? l_y : resolution_height); j < (y - u_y < resolution_height ? y - u_y : resolution_height); j++)
                {
                    if(j < 0 || j > resolution_height - 1)
                        break;

                    if(!is_drawn[j])
                    {
                        is_drawn[j] = true;

                        image_buffer[(resolution_width * j + i) * 4 + 0] = textures[slices[k].s->walls[slices[k].wall]->mid_texture][(((int)((j - x) * l) * wall_height + t_x) % (64 * 64)) * 3 + 0];
                        image_buffer[(resolution_width * j + i) * 4 + 1] = textures[slices[k].s->walls[slices[k].wall]->mid_texture][(((int)((j - x) * l) * wall_height + t_x) % (64 * 64)) * 3 + 1];
                        image_buffer[(resolution_width * j + i) * 4 + 2] = textures[slices[k].s->walls[slices[k].wall]->mid_texture][(((int)((j - x) * l) * wall_height + t_x) % (64 * 64)) * 3 + 2];
                        image_buffer[(resolution_width * j + i) * 4 + 3] = 255;
                    }
                }

                for(int j = (y - u_y < resolution_height ? y - u_y : resolution_height); j < (y < resolution_height ? y : resolution_height); j++)
                {
                    if(j < 0 || j > resolution_height - 1)
                        break;

                    if(!is_drawn[j])
                    {
                        is_drawn[j] = true;

                        image_buffer[(resolution_width * j + i) * 4 + 0] = textures[slices[k].s->walls[slices[k].wall]->up_texture][(((int)((j - x) * l) * wall_height + t_x) % (64 * 64)) * 3 + 0];
                        image_buffer[(resolution_width * j + i) * 4 + 1] = textures[slices[k].s->walls[slices[k].wall]->up_texture][(((int)((j - x) * l) * wall_height + t_x) % (64 * 64)) * 3 + 1];
                        image_buffer[(resolution_width * j + i) * 4 + 2] = textures[slices[k].s->walls[slices[k].wall]->up_texture][(((int)((j - x) * l) * wall_height + t_x) % (64 * 64)) * 3 + 2];
                        image_buffer[(resolution_width * j + i) * 4 + 3] = 255;
                    }
                }

            if(info_dump)
                printf("%d %f %f %f %f %f\n", slices[k].wall, slices[k].x1, slices[k].y1, slices[k].p_x, slices[k].p_y, slices[k].length);
        }

        if(info_dump)
            printf("\n");

        angle = player->angle - (fov / 2) + fov * (float)i / resolution_width;

        cur_disp += disp_amount / resolution_width;
    }
}

void ProcessGame()
{
    ProcessKeys();

    CleanSector(&s1);
    CleanSector(&s2);

    ProcessCharacter(enemy);
    ProcessCharacter(player);

    CreateView();

    glutPostRedisplay();

    if(info_dump)
        system("pause");
}

void ProcessKeys()
{
        if(keys['w'])
        {
            player->pos_x += sin(player->angle * M_PI / 180) * movement_step;
            player->pos_y += cos(player->angle * M_PI / 180) * movement_step;
        }
        if(keys['s'])
        {
            player->pos_x -= sin(player->angle * M_PI / 180) * movement_step;
            player->pos_y -= cos(player->angle * M_PI / 180) * movement_step;
        }
        if(keys['a'])
        {
            player->pos_x += sin((player->angle - 90) * M_PI / 180) * movement_step;
            player->pos_y += cos((player->angle - 90)* M_PI / 180) * movement_step;
        }
        if(keys['d'])
        {
            player->pos_x += sin((player->angle + 90) * M_PI / 180) * movement_step;
            player->pos_y += cos((player->angle + 90)* M_PI / 180) * movement_step;
        }
        if(keys['+'])
        {
            text_corr += 0.01f;
        }
        if(keys['-'])
        {
            text_corr -= 0.01f;
        }
        if(keys['z'])
        {
            printf("%f %f %ld\n", player->pos_x, player->pos_y, player->sector_p);
        }
        if(keys['o'])
        {
            info_dump = 0;
        }
        if(keys['p'])
        {
            info_dump = 1;
        }
}

void ProcessSpecialKeys(int key, int x, int y)
{
    switch(key)
    {
        case GLUT_KEY_UP:
            player->pos_x += sin(player->angle * M_PI / 180) * movement_step;
            player->pos_y += cos(player->angle * M_PI / 180) * movement_step;
        break;
        case GLUT_KEY_DOWN:
            player->pos_x -= sin(player->angle * M_PI / 180) * movement_step;
            player->pos_y -= cos(player->angle * M_PI / 180) * movement_step;
        break;
        case GLUT_KEY_RIGHT:
            player->angle += rotation_step;
        break;
        case GLUT_KEY_LEFT:
            player->angle -= rotation_step;
        break;
    }
}

void ProcessMouseMovement(int x, int y)
{
    player->angle += (x - window_width / 2) * rotation_step;

    if(x != window_width / 2)
        glutWarpPointer(window_width / 2, window_height / 2);

//    mouse_pos_x = x;
}

float dist(float x1, float y1, float x2, float y2)
{
    return sqrt( (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1) );
}
