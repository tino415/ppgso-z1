/*
 OpenGL visualization skeleton for displaying bitmap images. Just provide a GenerateImage function.
 Good starting point for all image processing exercises for parallel programming.

 Example of generating bitmaps using GenerateImage and the prepared GLUT OpenGL visualization.
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

#ifdef __APPLE__
  #include <GLUT/glut.h>
#else
  #include <GL/freeglut.h>
  #include <GL/freeglut_ext.h>
#endif

#define TEX_SIZE 512
#define ALPHA_SIZE 128
#define C_RED 0
#define C_GREEN 1
#define C_BLUE 2

#define DISPLAY_EFFECT 0
#define DISPLAY_LAYERS 1
#define DISPLAY_FRACTAL 2
#define DISPLAY_TO_LESSBIT 3
#define BZ_STEP 5

typedef struct {
    GLubyte r;
    GLubyte g;
    GLubyte b;
} pixel;

typedef struct {
	int x;
	int y;
	int w;
} pt;

typedef struct {
	int x1;
	int y1;
	int x2;
	int y2;
	pt pt1;
	pt pt2;
	int min_x;
	int min_y;
	int max_x;
	int max_y;
	int len_x;
	int len_y;
	int len;
} line;

pixel image[TEX_SIZE][TEX_SIZE];

GLuint texture;

int pen_red = 0;
int pen_green = 0;
int pen_blue = 0;
int pen_width = 0;

void pen_set_color(int red, int green, int blue) {
	pen_red = red;
	pen_green = green;
	pen_blue = blue;
}

void pen_set_width(int width) {
	pen_width = width;
}

void pen_set(int red, int green, int blue, int width) {
	pen_set_color(red, green, blue);
	pen_set_width(width);
}

int get_between(int min, int max, int number) {
	if(number > max) return max;
	else if(number < min) return min; 
	else return number;
}

int truncate(int value, int size, int trunc_size) {
	float scale = (float)size / (float) trunc_size;
	return round(round(value/scale)*scale);
}

void set_color(pixel *pix, int red, int green, int blue) {
	pix->r = get_between(0, 255, red);	
	pix->g = get_between(0, 255, green);
	pix->b = get_between(0, 255, blue);
}

void bucket_fill(pixel canvas[TEX_SIZE][TEX_SIZE]) {
	int x, y;
	for(x = 0; x < TEX_SIZE; x++) {
		for(y = 0; y < TEX_SIZE; y++) {
			set_color(&canvas[x][y], pen_red, pen_green, pen_blue);
		}
	}
}

void point(pixel canvas[TEX_SIZE][TEX_SIZE], int x, int y) {
	int min_x, min_y, max_x, max_y, i_y;
	min_x = get_between(0, TEX_SIZE, floor(x-(pen_width/2)));
	min_y = get_between(0, TEX_SIZE, floor(y-(pen_width/2)));
	max_x = get_between(0, TEX_SIZE, ceil(x+(pen_width/2)));
	max_y = get_between(0, TEX_SIZE, ceil(y+(pen_width/2)));

	while(min_x++ < max_x) {
		for(i_y = min_y; i_y < max_y; i_y++) {
			set_color(&canvas[min_x][i_y], pen_red, pen_green, pen_blue);
		}
	}
}

pt new_pt(int x, int y, int w) {
	pt pt;
	pt.x = x;
	pt.y = y;
	pt.w = w;
	return pt;
}

void set_pt(pt *pt, int x, int y) {
	pt->x = x;
	pt->y = y;
}

line new_line(int x1, int y1, int x2, int y2) {
	line line;
	line.x1 = x1;
	line.y1 = y1;
	line.x2 = x2;
	line.y2 = y2;
	line.pt1 = new_pt(x1, y1, 1);
	line.pt2 = new_pt(x2, y2, 1);
	line.min_x = (x1<x2) ? x1 : x2;
	line.min_y = (y1<y2) ? y1 : y2;
	line.max_x = (x2<x1) ? x1 : x2;
	line.max_y = (y2<y1) ? y1 : y2;
	line.len_x = line.max_x - line.min_x;
	line.len_y = line.max_y - line.min_y;
	line.len = (line.len_x > line.len_y) ? line.len_x : line.len_y;
	return line;
}

line new_ln(pt *pt1, pt* pt2) {
	return new_line(pt1->x, pt1->y, pt2->x, pt2->y);
}

typedef struct {
	pt pt1;
	pt pt2;
	pt pt3;
	line ln1;
	line ln2;
} bzcv;

typedef struct {
	int step_1;
	int step_2;
	int num_of_steps;
} bzcv_desc;

bzcv new_bzcv(pt* pt1, pt* pt2, pt* pt3) {
	bzcv bzcv;
	bzcv.pt1 = *pt1;
	bzcv.pt2 = *pt2;
	bzcv.pt3 = *pt3;
	bzcv.ln1 = new_ln(pt1, pt2);
	bzcv.ln2 = new_ln(pt2, pt3);
	return bzcv;
}

bzcv_desc bzcv_get_desc(bzcv *bzcv) {
	bzcv_desc desc;

	if(bzcv->ln1.len > bzcv->ln2.len) {
		desc.step_1 = BZ_STEP;
		desc.num_of_steps = bzcv->ln1.len/BZ_STEP;
		desc.step_2 = bzcv->ln2.len/desc.num_of_steps;
	} else {
		desc.step_2 = BZ_STEP;
		desc.num_of_steps = bzcv->ln2.len/BZ_STEP;
		desc.step_1 = bzcv->ln1.len/desc.num_of_steps;
	}

	return desc;
}

pt ln_get_pt(line *line, int num) {
	pt point;
	float pomer;
	if(line->len_x > line->len_y) {
		pomer = (float)line->len_y/line->len_x;
		point.x = (line->min_x == line->x1) ? line->min_x + num : line->max_x - num;
		point.y = (line->min_y == line->y1) ? line->min_y + ceil(pomer*num) : line->max_y - ceil(pomer*num);
	} else {
		pomer = (float)line->len_x/line->len_y;
		point.x = (line->min_x == line->x1) ? line->min_x + ceil(pomer*num) : line->max_x - ceil(pomer*num);
		point.y = (line->min_y == line->y1) ? line->min_y + num : line->max_y - num;
	}

	return point;
}

pt line_midle_pt(line *line) {
	int step = line->len/2;
	return ln_get_pt(line, step);
}

void draw_line(line *line) {
	
	int i;
	pt pt;
	for(i=0; i<line->len;i++) {
		pt = ln_get_pt(line, i);
		point(image, pt.x, pt.y);
	} 
}

void draw_pt(pt* pt) {
	point(image, pt->x, pt->y);
}

void draw_ln_sub(line *line, int start, int end) {
	int i;
	pt pt;
	for(i=start; i<end; i++) {
		pt = ln_get_pt(line, i);
		draw_pt(&pt);
	}
}

void draw_bzcv(bzcv *bzcv) {
	bzcv_desc desc = bzcv_get_desc(bzcv);
	int i;
	pt prew = bzcv->ln1.pt1;
	pt actual;

	for(i=0; i<=desc.num_of_steps;i++) {
		pt a = ln_get_pt(&bzcv->ln1, i*desc.step_1);
		pt b = ln_get_pt(&bzcv->ln2, i*desc.step_2);
		line line_md = new_ln(&a, &b);
		pen_set(0, 125, 255, 3);
		actual = ln_get_pt(&line_md, ((float)i/desc.num_of_steps)*line_md.len);
		line ln3 = new_ln(&prew, &actual);	
		draw_line(&ln3);
		prew = actual;
	}
}

pt pt_tran(pt *pot, float tx, float ty) {
	pt result;
	result.x = pot->x + tx;
	result.y = pot->y + ty;
	return result;
}

void m_mul(float matrix_a[3][3], float matrix_b[3][3], float result[3][3]) {
	int row_a, coll_b, element;

	for(row_a = 0; row_a < 3; row_a++)
		for(coll_b = 0; coll_b < 3; coll_b++) {
			int sum = 0;

			for(element = 0; element < 3; element++)
				sum += matrix_a[row_a][element] * matrix_b[element][coll_b];

			result[row_a][coll_b] = sum;
		}
				
}

bzcv bzcv_tran(bzcv *curve, float tx, float ty) {
	pt pt1 = pt_tran(&curve->pt1, tx, ty);
	pt pt2 = pt_tran(&curve->pt2, tx, ty);
	pt pt3 = pt_tran(&curve->pt3, tx, ty);
	return new_bzcv(
		&pt1, &pt2, &pt3
	);
}

/*
 * Handles keyboard input, switch modes by char 'm' and 
 * controll chars for variations are q,w,e,r,t,z,u,i
 * in layers you can use a and d to move animation
 */
void handle_keyboard(unsigned char ch, int x, int y) {
    switch(ch) {
	}
}

void draw_r() {
	pt r1 = new_pt(50, 50, 1);
	pt r2 = new_pt(50, 300, 1);
	pt r3 = new_pt(150, 50, 1);
	pt r4 = new_pt(250, 50, 1);
	pt r5 = new_pt(250, 300, 1);
	pt r6 = new_pt(375, 175, 1);
	pt r7 = new_pt(500, 50, 1);
	pt r8 = new_pt(500, 300, 1);

	pen_set(175, 255, 0, 8);
	draw_pt(&r1);
	draw_pt(&r2);
	draw_pt(&r3);
	draw_pt(&r4);
	draw_pt(&r5);
	draw_pt(&r6);
	draw_pt(&r7);
	draw_pt(&r8);

	pen_set(255, 0, 175, 3);
	bzcv bzcv1 = new_bzcv(&r2, &r1, &r3);
	bzcv bzcv2 = new_bzcv(&r3, &r4, &r5);
	bzcv bzcv3 = new_bzcv(&r2, &r5, &r8);
	bzcv bzcv4 = new_bzcv(&r5, &r6, &r7);
	draw_bzcv(&bzcv1);
	draw_bzcv(&bzcv2);
	draw_bzcv(&bzcv3);
	draw_bzcv(&bzcv4);

	pen_set(175, 255, 0, 8);
	draw_pt(&r1);
	draw_pt(&r2);
	draw_pt(&r3);
	draw_pt(&r4);
	draw_pt(&r5);
	draw_pt(&r6);
	draw_pt(&r7);
	draw_pt(&r8);

	pen_set(255, 0, 175, 3);
	bzcv1 = new_bzcv(&r2, &r1, &r3);
	bzcv2 = new_bzcv(&r3, &r4, &r5);
	bzcv3 = new_bzcv(&r2, &r5, &r8);
	bzcv4 = new_bzcv(&r5, &r6, &r7);
	draw_bzcv(&bzcv1);
	draw_bzcv(&bzcv2);
	draw_bzcv(&bzcv3);
	draw_bzcv(&bzcv4);
	pen_set(125, 125, 255, 6);
	bzcv bzcv_t1 = bzcv_tran(&bzcv3, 10, 10);
	draw_bzcv(&bzcv_t1);

	pen_set(124, 43, 234, 7);
	pt pt1 = new_pt(250, 250, 1);
	pt pt2 = pt_tran(&pt1, 8, 0);
	draw_pt(&pt1);
	draw_pt(&pt2);
}

void test_tran() {
	pen_set(125, 125, 125, 5);
	
	pt pt1 = new_pt(100, 100, 1);
	pt pt2 = new_pt(255, 255, 1);
	pt pt3 = new_pt(100, 496, 1);
	bzcv bz = new_bzcv(&pt1, &pt2, &pt3);
	pen_set(212, 124, 154, 10);
	draw_pt(&pt1);
	draw_pt(&pt2);
	draw_pt(&pt3);
	draw_bzcv(&bz);

	pen_set(255, 255, 255, 7);
	bzcv bz2 = bzcv_tran(&bz, 8, 8);
	pen_set(0, 0, 255, 10);
	draw_pt(&bz2.pt1);
	draw_pt(&bz2.pt2);
	draw_pt(&bz2.pt3);
	draw_bzcv(&bz2);
}

void test_mulm() {
	float m1[3][3] = {
		{1, 2, 3},
		{1, 2, 3},
		{1, 2, 3}
	};
	float m2[3][3] = {
		{1, 2, 3},
		{1, 2, 3},
		{1, 2, 3}
	};
	float m3[3][3];
	m_mul(m1, m2, m3);

	int x, y;
	for(x = 0; x < 3; x++)
		for(y = 0; y < 3; y++)
			printf(" | %f\n", m3[x][y]);
}

void draw() {
	pen_set(0,0,0,5);
	bucket_fill(image);
	//test_tran();
	//test_mulm();

	//draw_r();
//	pt a = new_pt(50, 50);
//	pt a1 = new_pt(100, 100);
//	pt a2 = new_pt(150, 150);
//	pt a3 = new_pt(200, 200);
//	pt b = new_pt(250, 250);
//	pt b1 = new_pt(200, 300);
//	pt b2 = new_pt(150, 350);
//	pt b3 = new_pt(100, 400);
//	pt c = new_pt(50, 450);


//	pen_set(255, 125, 0, 8);
//	draw_pt(&a);
//	draw_pt(&a1);
//	draw_pt(&a2);
//	draw_pt(&a3);
//	draw_pt(&b);
//	draw_pt(&b1);
//	draw_pt(&b2);
//	draw_pt(&b3);
//	draw_pt(&c);

//	pen_set(125, 255, 0, 3);
//	line ln1 = new_ln(&a, &b);
//	line ln2 = new_ln(&b, &c);
//	draw_line(&ln1);
//	draw_line(&ln2);

//	pen_set(0, 125, 255, 2);
//	bzcv bzcv1 = new_bzcv(&a, &b, &c);
//	draw_bzcv(&bzcv1);

//	pt b1 = new_pt(250, 300);
//	pt b2 = new_pt(250, 350);
//	pt b3 = new_pt(250, 400);
//	pt c = new_pt(250, 450);
//	pt b1 = new_pt(300, 250);
//	pt b2 = new_pt(350, 250);
// 	pt b3 = new_pt(400, 250);
// 	pt c = new_pt(450, 250);
//
//	pen_set(255, 125, 0, 8);
//	draw_pt(&a);
//	draw_pt(&a1);
//	draw_pt(&a2);
//	draw_pt(&a3);
//	draw_pt(&b);
//	draw_pt(&b1);
//	draw_pt(&b2);
//	draw_pt(&b3);
//	draw_pt(&c);
//
//	pen_set(125, 255, 0, 3);
//	line ln1 = new_ln(&a, &b);
//	line ln2 = new_ln(&b, &c);
//	draw_line(&ln1);
//	draw_line(&ln2);
//	
//	pen_set(0, 125, 255, 2);
//	bzcv bzcv1 = new_bzcv(&a, &b, &c);
//	draw_bzcv(&bzcv1);
}

// Initialize OpenGL state
void init() {
	// Texture setup
    glEnable(GL_TEXTURE_2D);
    glGenTextures( 1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    // Other
    glClearColor(0,0,0,0);
    gluOrtho2D(-1,1,-1,1);
    glLoadIdentity();
    glColor3f(1,1,1);
	draw();
}

// Generate and display the image.
void display() {
    // Call user image generation
	
    // Copy image to texture memory
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEX_SIZE, TEX_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    // Clear screen buffer
    glClear(GL_COLOR_BUFFER_BIT);
    // Render a quad
    glBegin(GL_QUADS);
        glTexCoord2f(0,0); glVertex2f(1,1);
        glTexCoord2f(0,1); glVertex2f(1,-1);
        glTexCoord2f(1,1); glVertex2f(-1,-1);
        glTexCoord2f(1,0); glVertex2f(-1,1);
    glEnd();
    // Display result
    glFlush();
    glutPostRedisplay();
    glutSwapBuffers();
}

// Main entry function
int main(int argc, char ** argv) {
    // Init GLUT
    glutInit(&argc, argv);
    glutInitWindowSize(TEX_SIZE, TEX_SIZE);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
    glutCreateWindow("OpenGL Window");
    // Set up OpenGL state
    init();
    // Run the control loop
    glutDisplayFunc(display);
    glutKeyboardFunc(handle_keyboard);
    glutMainLoop();
    return EXIT_SUCCESS;
}
