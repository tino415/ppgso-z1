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
#define C_RED 0
#define C_GREEN 1
#define C_BLUE 2

#define BZ_STEP 5
#define BZ_PTS 3
#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))
#define ANGLE 0.01

int animation = 0;

typedef struct {
    GLubyte r;
    GLubyte g;
    GLubyte b;
} pixel;

typedef struct {
	float x;
	float y;
	float z;
	float w;
} pt;

typedef struct {
	pt pt1;
	pt pt2;
} ln;

typedef struct {
	float min_x;
	float min_y;
	float len_x;
	float len_y;
	float len;
	float pomer;
	short longer_x;
} ln_desc;

typedef struct {
	pt pt1;
	pt pt2;
	pt pt3;
} bz;

typedef struct {
	float step_1;
	float step_2;
	float len;
	ln *ln1;
	ln *ln;
	ln_desc ldc1;
	ln_desc ldc2;
} bz_desc;

typedef struct {
	int len;
	bz curves[];
} obj;

pixel image[TEX_SIZE][TEX_SIZE];

GLuint texture;

int pen_red = 0;
int pen_green = 0;
int pen_blue = 0;
int pen_width = 0;

//Basic functions
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

//pen functions
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

void dw_pt(pt* pt) {
	point(image, pt->x, pt->y);
}

ln_desc ln_describe(ln *ln) {
	ln_desc dc;
	dc.len_x = abs(ln->pt1.x - ln->pt2.x);
	dc.len_y = abs(ln->pt1.y - ln->pt2.y);
	dc.min_x = min(ln->pt1.x, ln->pt2.x);
	dc.min_y = min(ln->pt1.y, ln->pt2.y);
	dc.longer_x = (dc.len_x > dc.len_y);
	dc.len = (dc.len_x > dc.len_y) ? dc.len_x : dc.len_y;
	dc.pomer = (dc.longer_x) ? (float)dc.len_y/dc.len_x : (float)dc.len_x/dc.len_y;
	return dc;
}

ln new_ln(pt pt1, pt pt2) {
	ln line;
	line.pt1 = pt1;
	line.pt2 = pt2;
	return line;
}

pt ln_gpt(ln *ln, ln_desc *dc, int num) {
	pt result;
	int desc;
	if(dc->longer_x) {
		desc = ceil(num*dc->pomer);
		result.x = (ln->pt1.x == dc->min_x) ? dc->min_x + num : max(ln->pt1.x, ln->pt2.x) - num;
		result.y = (ln->pt1.y == dc->min_y) ? dc->min_y + desc : max(ln->pt1.y, ln->pt2.y) - desc;
	} else {
		desc = ceil(num*dc->pomer);
		result.x = (ln->pt1.x == dc->min_x) ? dc->min_x + desc : max(ln->pt1.x, ln->pt2.x) - desc;
		result.y = (ln->pt1.y == dc->min_y) ? dc->min_y + num : max(ln->pt1.y, ln->pt2.y) - num;
	}
	return result;
}

void ln_dw_sub(ln* ln, int start, int end) {
	int i;
	ln_desc dc = ln_describe(ln);
	pt pt;
	for(i=start; i<dc.len; i++) {
		pt = ln_gpt(ln, &dc, i);
		dw_pt(&pt);
	}
}

void ln_dw(ln* ln) {
	int i;
	ln_desc dc = ln_describe(ln);
	pt pt;
	for(i=0; i<dc.len; i++) {
		pt = ln_gpt(ln, &dc, i);
		dw_pt(&pt);
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

bz new_bz(pt* pt1, pt* pt2, pt* pt3) {
	bz bz;
	bz.pt1 = *pt1;
	bz.pt2 = *pt2;
	bz.pt3 = *pt3;
	return bz;
}

bz_desc bz_get_desc(bz *bz) {
	bz_desc desc;
	desc.ln1 = (ln*)bz;
	desc.ln = (ln*)&bz->pt2;
	desc.ldc1 = ln_describe(desc.ln1);
	desc.ldc2 = ln_describe(desc.ln);

	if(desc.ldc1.len > desc.ldc2.len) {
		desc.step_1 = BZ_STEP;
		desc.len = desc.ldc1.len/BZ_STEP;
		desc.step_2 = desc.ldc2.len/desc.len;
	} else {
		desc.step_2 = BZ_STEP;
		desc.len = desc.ldc2.len/BZ_STEP;
		desc.step_1 = desc.ldc1.len/desc.len;
	}

	return desc;
}

void bz_dw(bz *bz) {
	bz_desc desc = bz_get_desc(bz);
	int i;
	pt prew = desc.ln1->pt1;
	pt actual;

	for(i=0; i<=desc.len;i++) {
		ln line_md;
		line_md.pt1 = ln_gpt(desc.ln1, &desc.ldc1, i*desc.step_1);
		line_md.pt2 = ln_gpt(desc.ln, &desc.ldc2, i*desc.step_2);
		ln_desc dc_md = ln_describe(&line_md);
		actual = ln_gpt(&line_md, &dc_md, ((float)i/desc.len)*dc_md.len);
		ln ln3 = new_ln(prew, actual);	
		ln_dw(&ln3);
		prew = actual;
	}
}

void m_mul(float matrix_a[4][4], float matrix_b[4][4], float result[4][4]) {
	int row_a, coll_b, element;

	for(row_a = 0; row_a < 4; row_a++)
		for(coll_b = 0; coll_b < 4; coll_b++) {
			float sum = 0;
			for(element = 0; element < 4; element++)
				sum += matrix_a[row_a][element] * matrix_b[element][coll_b];

			result[row_a][coll_b] = sum;
		}
				
}

obj r = {
	4,
	{
		{{50, 300, 512, 1}, {50, 50, 512, 1}, {150, 50, 512, 1}},
		{{150, 50, 512, 1}, {250, 50, 512, 1}, {250, 300, 512, 1}},
		{{50, 300, 512, 1}, {250, 300, 512, 1}, {500, 300, 512, 1}},
		{{250, 300, 512, 1}, {375, 175, 512, 1}, {500, 50, 512, 1}}
	}
};

obj *out_r;

pt mrx_pt(pt *point, float matrix[4][4]) {
	pt result;
	result.x = 
		point->x * matrix[0][0] + point->y * matrix[0][1] + point->z * matrix[0][2] + point->w * matrix[0][3];
	result.y = 
		point->x * matrix[1][0] + point->y * matrix[1][1] + point->z * matrix[1][2] + point->w * matrix[1][3];
	result.z =
		point->x * matrix[2][0] + point->y * matrix[2][1] + point->z * matrix[2][2] + point->w * matrix[2][3];
	result.w =
		point->x * matrix[3][0] + point->y * matrix[3][1] + point->z * matrix[3][2] + point->w * matrix[3][3];
	return result;
}

void obj_dw(obj *object) {
	int len = object->len;
	while(len-- > 0) {
		bz_dw(&object->curves[len]);
	}
}

obj* obj_copy(obj *object) {
	obj *res = malloc(sizeof(obj)+object->len*sizeof(bz));
	res->len = object->len;
	memcpy(res->curves, object->curves, object->len * sizeof(bz)); 
	return res;
}

void obj_trans(obj *object, int vx, int vy, int vz) {
	int len = object->len;
	float matrix[4][4] = {
		{1, 0, 0, vx},
		{0, 1, 0, vy},
		{0, 0, 1, vz},
		{0, 0, 0, 1 }
	};

	while(len-- > 0) {
		object->curves[len].pt1 = mrx_pt(&object->curves[len].pt1, matrix);
		object->curves[len].pt2 = mrx_pt(&object->curves[len].pt2, matrix);
		object->curves[len].pt3 = mrx_pt(&object->curves[len].pt3, matrix);
	}
}

void obj_rot_z(obj *object, float angle) {
	int len = object->len;
	float matrix[4][4] = {
		{cos(angle), -1*sin(angle), 0, 0},
		{sin(angle),    cos(angle), 0, 0},
		{0         ,             0, 1, 0},
		{0         ,             0, 0, 1}
	};

	while(len-- > 0) {
		object->curves[len].pt1 = mrx_pt(&object->curves[len].pt1, matrix);
		object->curves[len].pt2 = mrx_pt(&object->curves[len].pt2, matrix);
		object->curves[len].pt3 = mrx_pt(&object->curves[len].pt3, matrix);
	}
}

void obj_rot_x(obj *object, float angle) {
	int len = object->len;
	float matrix[4][4] = {
		{1,0,0,0},
		{0,cos(angle), -sin(angle), 0},
		{0,sin(angle), cos(angle), 0},
		{0,0,0,1}
	};

	while(len-- > 0) {
		object->curves[len].pt1 = mrx_pt(&object->curves[len].pt1, matrix);
		object->curves[len].pt2 = mrx_pt(&object->curves[len].pt2, matrix);
		object->curves[len].pt3 = mrx_pt(&object->curves[len].pt3, matrix);
	}
}

void obj_rot_y(obj *object, float angle) {
	int len = object->len;
	float matrix[4][4] = {
		{   cos(angle),   0,  sin(angle), 0},
		{   0         ,   1,           0, 0},
		{-1*sin(angle),   0,  cos(angle), 0},
		{   0         ,   0,           0, 1}
	};

	while(len-- > 0) {
		object->curves[len].pt1 = mrx_pt(&object->curves[len].pt1, matrix);
		object->curves[len].pt2 = mrx_pt(&object->curves[len].pt2, matrix);
		object->curves[len].pt3 = mrx_pt(&object->curves[len].pt3, matrix);
	}
}

void bz_divw(bz *bz) {
	int i;
	pt *start = (pt*) bz;
	
	for(i=0;i<3;i++) {
		start[i].x = start[i].x / start[i].w;
		start[i].y = start[i].y / start[i].w;
		start[i].z = start[i].z / start[i].w;
	}
}

void obj_persp(obj *object, float d) {
	int len = object->len;
	float matrix[4][4] = {
		{1, 0, 0, 0 },
		{0, 1, 0, 0 },
		{0, 0, 1, 0 },
		{0, 0, 1/d,0}
	};

	while(len-- > 0) {
		object->curves[len].pt1 = mrx_pt(&object->curves[len].pt1, matrix);
		object->curves[len].pt2 = mrx_pt(&object->curves[len].pt2, matrix);
		object->curves[len].pt3 = mrx_pt(&object->curves[len].pt3, matrix);
		bz_divw(&object->curves[len]);
	}
}

void obj_trot_z(obj *object, float angle, int vx, int vy, int vz) {
	int len = object->len;
	float trans1[4][4] = {
		{1, 0, 0, -1*vx},
		{0, 1, 0, -1*vy},
		{0, 0, 1, -1*vz}, 
		{0, 0, 0,     1}
	};
	float rot_z[4][4] = {
		{cos(angle), -1*sin(angle), 0, 0},
		{sin(angle),    cos(angle), 0, 0},
		{0         ,             0, 1, 0},
		{0         ,             0, 0, 1}
	};
	float trans2[4][4] = {
		{1, 0, 0, vx},
		{0, 1, 0, vy},
		{0, 0, 1, vz},
		{0, 0, 0, 1 }
	};
	float matrix[4][4];
	m_mul(trans2, rot_z, matrix);
	m_mul(matrix, trans1, matrix);

	while(len-- > 0) {
		object->curves[len].pt1 = mrx_pt(&object->curves[len].pt1, matrix);
		object->curves[len].pt2 = mrx_pt(&object->curves[len].pt2, matrix);
		object->curves[len].pt3 = mrx_pt(&object->curves[len].pt3, matrix);
	}
}

void obj_trot_y(obj *object, float angle, int vx, int vy, int vz) {
	int len = object->len;
	float trans1[4][4] = {
		{1, 0, 0, -1*vx},
		{0, 1, 0, -1*vy},
		{0, 0, 1, -1*vz}, 
		{0, 0, 0,     1}
	};
	float rot_y[4][4] = {
		{   cos(angle),   0,  sin(angle), 0},
		{   0         ,   1,           0, 0},
		{-1*sin(angle),   0,  cos(angle), 0},
		{   0         ,   0,           0, 1}
	};
	float trans2[4][4] = {
		{1, 0, 0, vx},
		{0, 1, 0, vy},
		{0, 0, 1, vz},
		{0, 0, 0, 1 }
	};
	float matrix[4][4];
	m_mul(trans2, rot_y, matrix);
	m_mul(matrix, trans1, matrix);

	while(len-- > 0) {
		object->curves[len].pt1 = mrx_pt(&object->curves[len].pt1, matrix);
		object->curves[len].pt2 = mrx_pt(&object->curves[len].pt2, matrix);
		object->curves[len].pt3 = mrx_pt(&object->curves[len].pt3, matrix);
	}
}

void obj_trot_x(obj *object, float angle, int vx, int vy, int vz) {
	int len = object->len;
	float trans1[4][4] = {
		{1, 0, 0, -1*vx},
		{0, 1, 0, -1*vy},
		{0, 0, 1, -1*vz}, 
		{0, 0, 0,     1}
	};
	float rot_x[4][4] = {
		{1,0,0,0},
		{0,cos(angle), -sin(angle), 0},
		{0,sin(angle), cos(angle), 0},
		{0,0,0,1}
	};
	float trans2[4][4] = {
		{1, 0, 0, vx},
		{0, 1, 0, vy},
		{0, 0, 1, vz},
		{0, 0, 0, 1 }
	};
	float matrix[4][4];
	m_mul(trans2, rot_x, matrix);
	m_mul(matrix, trans1, matrix);

	while(len-- > 0) {
		object->curves[len].pt1 = mrx_pt(&object->curves[len].pt1, matrix);
		object->curves[len].pt2 = mrx_pt(&object->curves[len].pt2, matrix);
		object->curves[len].pt3 = mrx_pt(&object->curves[len].pt3, matrix);
	}
}

void obj_trot_xyz(obj *object, float angle, int vx, int vy, int vz) {
	int len = object->len;
	float trans1[4][4] = {
		{1, 0, 0, -1*vx},
		{0, 1, 0, -1*vy},
		{0, 0, 1, -1*vz}, 
		{0, 0, 0,     1}
	};
	float rot_z[4][4] = {
		{cos(angle), -1*sin(angle), 0, 0},
		{sin(angle),    cos(angle), 0, 0},
		{0         ,             0, 1, 0},
		{0         ,             0, 0, 1}
	};
	float rot_y[4][4] = {
		{   cos(angle),   0,  sin(angle), 0},
		{   0         ,   1,           0, 0},
		{-1*sin(angle),   0,  cos(angle), 0},
		{   0         ,   0,           0, 1}
	};
	float rot_x[4][4] = {
		{1,0,0,0},
		{0,cos(angle), -sin(angle), 0},
		{0,sin(angle), cos(angle), 0},
		{0,0,0,1}
	};
	float trans2[4][4] = {
		{1, 0, 0, vx},
		{0, 1, 0, vy},
		{0, 0, 1, vz},
		{0, 0, 0, 1 }
	};
	float matrix[4][4];
	m_mul(trans2, rot_x, matrix);
	m_mul(matrix, rot_y, matrix);
	m_mul(matrix, rot_z, matrix);
	m_mul(matrix, trans1, matrix);

	while(len-- > 0) {
		object->curves[len].pt1 = mrx_pt(&object->curves[len].pt1, matrix);
		object->curves[len].pt2 = mrx_pt(&object->curves[len].pt2, matrix);
		object->curves[len].pt3 = mrx_pt(&object->curves[len].pt3, matrix);
	}
}

void animate() {
	obj_trans(&r, -200, -200, -512);
	obj_rot_z(&r, 0.2);
	obj_rot_y(&r, 0.2);
	obj_trans(&r, 200, 200, 512);
	free(out_r);
	out_r = obj_copy(&r);
}

/*
 * Handles keyboard input, switch modes by char 'm' and 
 * controll chars for variations are q,w,e,r,t,z,u,i
 * in layers you can use a and d to move animation
 */
void handle_keyboard(unsigned char ch, int x, int y) {
	switch(ch) {
		case 'r':
			obj_trans(&r, 0, 0, 10);
			free(out_r);
			out_r = obj_copy(&r);
			obj_persp(out_r, 512);
			break;
		case 'e':
			obj_trans(&r, 0, 0, -10);
			free(out_r);
			out_r = obj_copy(&r);
			obj_persp(out_r, 512);
			break;
		case 'y':
			obj_trot_y(&r, 0.3, 200, 200, 512);
			free(out_r);
			out_r = obj_copy(&r);
			obj_persp(out_r, 512);
			break;
		case 'z':
			obj_trot_z(&r, 0.3, 200, 200, 512);
			free(out_r);
			out_r = obj_copy(&r);
			obj_persp(out_r, 512);
			break;
		case 'x':
			obj_trot_x(&r, 0.3, 200, 200, 512);
			free(out_r);
			out_r = obj_copy(&r);
			obj_persp(out_r, 512);
			break;
		case 'a':
			animation = (animation) ? 0 : 1;
			break;
		case 'd':
			obj_trot_xyz(&r, 0.2,250, 250, 500);
			free(out_r);
			out_r = obj_copy(&r);
			obj_persp(out_r, 512);
			break;
	}
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
	out_r = obj_copy(&r);
	obj_persp(out_r, 512);
}

// Generate and display the image.
void display() {
	pen_set(255,255,255,5);
	bucket_fill(image);

    // Call user image generation
	if(animation) animate();
	pen_set(155, 255, 175, 10);
	obj_dw(out_r);
	
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
