#include <GL/gl.h>
#include <GL/glut.h>
#include <math.h>
#include "doot.h"

extern int window_width, window_height;
extern unsigned char image_buffer[];
extern unsigned char* textures[];
extern character* player;
extern v_ray v_rays[resolution_width];

void DrawSector(sector* s, int x, int y)
{
    glDisable(GL_TEXTURE_2D);

	glColor4f(0.0f, 1.0f, 0.0f, 1.0f);

    glBegin(GL_LINES);
        for(int i = 0; i < s->n_walls; i++)
        {
            glVertex2f(s->walls[i]->x1, s->walls[i]->y1);
            glVertex2f(s->walls[i]->x2, s->walls[i]->y2);
        }
    glEnd();
}

void DrawPlayer(int x, int y)
{
    glDisable(GL_TEXTURE_2D);

	glColor4f(0.0f, 1.0f, 0.0f, 1.0f);

    glBegin(GL_LINES);
        glVertex2f(player->pos_x, player->pos_y);
        glVertex2f(player->pos_x + sin(player->angle * M_PI / 180) * 10,
                   player->pos_y + cos(player->angle * M_PI / 180) * 10);
    glEnd();
}

void DrawLine(int i, int j)
{
    glDisable(GL_TEXTURE_2D);

	glColor4f(0.0, !j ? 1.0 : 0.0, !j ? 0.0 : 1.0, 1.0f);

    glBegin(GL_LINES);
        glVertex2f(v_rays[i].v_slices[j].x, v_rays[i].v_slices[j].y);
        glVertex2f(v_rays[i].v_slices[j].p_x, v_rays[i].v_slices[j].p_y);
    glEnd();
}

void RenderScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glEnable( GL_ALPHA_TEST );
    glAlphaFunc(GL_GREATER, 0.4);
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glLineWidth(3);

//    glDepthFunc(GL_NEVER);

//    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_TEXTURE_2D);

//    glBindTexture(GL_TEXTURE_2D, NULL);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST); // ( NEW )
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST); // ( NEW )

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, resolution_width, resolution_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_buffer);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 128, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_buffer);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, textures[0]);

    glViewport(0, 0, window_width, window_height);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, window_height);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(window_width, window_height);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(window_width, 0.0f);
    glEnd();

    DrawSector(player->sector_p, 0, 0);
    DrawPlayer(0, 0);

//    glLineWidth(1);

    for(int i = 0; i < resolution_width; i++)
        if(v_rays[i].slices > 1)
            DrawLine(i, 1);

    for(int i = 0; i < resolution_width; i++)
        DrawLine(i, 0);

	glutSwapBuffers();
}

void ChangeSize(int w, int h)
{
	if (h == 0)
		h = 1;

    window_width = w;
    window_height = h;

	float ratio =  (float)w / (float)h;

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

//	gluPerspective(60.0f, ratio, 0.1f, 1000.0f);
    glOrtho(0.0f, window_width, 0.0f, window_height, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
}
