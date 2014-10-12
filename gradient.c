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

#define DISPLAY_LAYERS_AND_EFFECT 0
#define DISPLAY_FRACTAL 1
#define DISPLAY_TO_LESSBIT 2

typedef struct {
    GLubyte r;
    GLubyte g;
    GLubyte b;
} pixel;

typedef struct {
    GLubyte r;
    GLubyte g;
    GLubyte b;
    GLubyte a;
} rgba_pix;

pixel image[2*TEX_SIZE][TEX_SIZE];

pixel (*source)[TEX_SIZE] = (pixel(*)[TEX_SIZE])&image[0][0];

pixel (*result)[TEX_SIZE] = (pixel(*)[TEX_SIZE])&image[TEX_SIZE][0];

rgba_pix layer1[TEX_SIZE][TEX_SIZE];

rgba_pix layer2[TEX_SIZE][TEX_SIZE];

int alpha_x = 0;

int mode = DISPLAY_LAYERS_AND_EFFECT;

GLuint texture;

FILE *open_image_file(char *path) {
	FILE *image_file = fopen(path, "rb");
	if(image_file == NULL) {
		printf("Error openning %s\n", path);
		exit(1);
	}
	return image_file;
}

void load_rgb(pixel target[TEX_SIZE][TEX_SIZE], char *path, int size) {
	int x, y;
	FILE *image_file = open_image_file(path);
	for(x=0; x<size; x++) {
		for(y=0; y<size; y++) {
			fread(&target[x][y], sizeof(pixel), 1, image_file);
		}
	}
	fclose(image_file);
}

void load_rgba(rgba_pix target[TEX_SIZE][TEX_SIZE], char *path, int size) {
	int x, y;
	FILE *image_file = open_image_file(path);
	for(x=0; x<size; x++) {
		for(y=0; y<size; y++) {
			fread(&target[x][y], sizeof(rgba_pix), 1, image_file);
		}
	}
	fclose(image_file);
}

int get_between_0_255(int source) {
	if(source > 255) return 255;
	else if(source < 0) return 0;
	else return source;
}

int truncate(int value, int size, int trunc_size) {
	float scale = (float)size / (float) trunc_size;
	return round(round(value/scale)*scale);
}

void set_color(pixel target[TEX_SIZE][TEX_SIZE], int x, int y, int red, int green, int blue) {
	target[x][y].r = get_between_0_255(red);
	target[x][y].g = get_between_0_255(green);
	target[x][y].b = get_between_0_255(blue);
}

void set_pixel_color(pixel *pix, int red, int green, int blue) {
	pix->r = get_between_0_255(red);	
	pix->g = get_between_0_255(green);
	pix->b = get_between_0_255(blue);
}

void blend_layer_at(pixel image[TEX_SIZE][TEX_SIZE], rgba_pix layer[TEX_SIZE][TEX_SIZE], int start_x, int start_y) {
	int x,y;
	for(x=start_x;x < TEX_SIZE; x++) {
		for(y=start_y; y < TEX_SIZE; y++) {
			int lx = x - start_x;
			int ly = y - start_y;
			float alp = layer[lx][ly].a / 255;
			set_color(image, x, y,
				layer[lx][ly].r * alp + image[x][y].r*(1-alp),
				layer[lx][ly].g * alp + image[x][y].g*(1-alp),
				layer[lx][ly].b * alp + image[x][y].b*(1-alp)
			);
		}
	}
}

void convolution_transform_3x(int x, int y, float kernel[3][3], float dividor) {
	int kx, ky, red, green, blue;
	red = green = blue = 0;

	for(kx = x - 1; kx <= x+1; kx++) {
		if(kx >= 0 && kx < TEX_SIZE) {
			for(ky = y - 1; ky <= y+1; ky++) {
				if(ky >= 0 && ky < TEX_SIZE) {
					int kerneln = kernel[(kx-x)+1][(ky-y)+1];
					red += round(image[kx][ky].r*kerneln);
					green += round(image[kx][ky].g*kerneln);
					blue += round(image[kx][ky].b*kerneln);
				}
			}
		}
	} 
	set_color(result, x, y, red*dividor, green*dividor, blue*dividor);
}

void convolution_transform_5x(int x, int y, float kernel[5][5], float dividor) {
	int kx, ky, red, green, blue;
	red = green = blue = 0;

	for(kx = x - 2; kx <= x+2; kx++) {
		if(kx >= 0 && kx < TEX_SIZE) {
			for(ky = y - 2; ky <= y+2; ky++) {
				if(ky >= 0 && ky < TEX_SIZE) {
					int kerneln = kernel[(kx-x)+2][(ky-y)+2];
					red += round(image[kx][ky].r*kerneln);
					green += round(image[kx][ky].g*kerneln);
					blue += round(image[kx][ky].b*kerneln);
				}
			}
		}
	}

	set_color(result, x, y, red*dividor, green*dividor, blue*dividor);
}

void convolution3x(float kernel[3][3], float dividor) {
	int x, y;

	for(x = 0; x < TEX_SIZE; x++) {
		for(y = 0; y < TEX_SIZE; y++) {
			convolution_transform_3x(x,y,kernel, dividor);
		}
	}
}

void convolution5x(float kernel[5][5], float dividor) {
	int x,y;
	for(x = 0; x < TEX_SIZE; x++) {
		for(y = 0; y < TEX_SIZE; y++) {
			convolution_transform_5x(x,y,kernel,dividor);
		}
	}
}

void blur() {
	float kernel[3][3] = {
		{1,1,1},
		{1,1,1},
		{1,1,1}
	};

	convolution3x(kernel, 1.00/9.00);
}

void blur5x() {
	float kernel[5][5] = {
		{ 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1},
		{ 1, 1, 1, 1, 1}
	};

	convolution5x(kernel, 1.00/25.0);
}

void edge_detection1() {
	float kernel[3][3] = {
		{1,0,-1},
		{0,0,0},
		{-1,0,1}
	};

	convolution3x(kernel, 1.00);
}

void edge_detection3() {
	float kernel[3][3] = {
		{-1, -1, -1},
		{-1,  8, -1},
		{-1, -1, -1}
	};

	convolution3x(kernel, 1.0);
}

void sharpen() {
    float kernel[3][3] = {
        {0,-1,0},
        {-1,5,-1},
        {0,-1,0}
    };

    convolution3x(kernel, 1.0);
}

void emboss() {
	float kernel[3][3] = {
		{-2, -2, 0},
		{-2, 6, 0},
		{0, 0, 0}
	};

	convolution3x(kernel, 1.0);
}

void (*effect)() = blur;

void print_fractal() {

	#define MaxIters 400
	#define LEFT     -2.0
	#define RIGHT    1.0
	#define TOP      1.0
	#define BOTTOM   -1.0

	short   x, y, count;
	long double zr, zi, cr, ci;
	long double rsquared, isquared;

	for (y = 0; y < TEX_SIZE; y++) {
		for (x = 0; x < TEX_SIZE; x++) {
			zr = 0.0;
			zi = 0.0;

			cr = LEFT + x * (RIGHT - LEFT) / TEX_SIZE;
			ci = TOP + y * (BOTTOM - TOP) / TEX_SIZE;

			rsquared = zr * zr;
			isquared = zi * zi;

			for (count = 0; rsquared + isquared <= 4.0 && count < MaxIters; count++) {
				zi = zr * zi * 2;
				zi += ci;
				zr = rsquared - isquared;
				zr += cr;
				rsquared = zr * zr;
				isquared = zi * zi;
			}
			if (rsquared + isquared <= 4.0)
				set_color(result,x,y,50,50,50);
			else
				set_color(result,x,y,0,0,0);
		}
	}
}

void to_3bit() {
	int x, y;
	for(x = 0; x < TEX_SIZE; x++) {
		for(y = 0; y < TEX_SIZE; y++) {
			result[x][y].r = (source[x][y].r > 125) ? 255 : 0;
			result[x][y].g = (source[x][y].g > 125) ? 255 : 0;
			result[x][y].b = (source[x][y].b > 125) ? 255 : 0;
		}
	}
}

void to_1bit() {
	int x, y;
	for(x = 0; x < TEX_SIZE; x++) {
		for(y = 0; y < TEX_SIZE; y++) {
			int power = (source[x][y].r + source[x][y].g + source[x][y].b);
			if(power > 380) {
				set_pixel_color(&result[x][y], 255,255,255);
			} else {
				set_pixel_color(&result[x][y], 0, 0, 0);
			}
		}
	}
}

void random_dithering_1bit() {
	int x, y;
	for(x = 0; x < TEX_SIZE; x++) {
		for(y = 0; y < TEX_SIZE; y++) {
			int power = (source[x][y].r + source[x][y].g + source[x][y].b);
			if(power > 250 + (rand()%130)) {
				set_pixel_color(&result[x][y], 255,255,255);
			} else {
				set_pixel_color(&result[x][y], 0, 0, 0);
			}
		}
	}
}

void to_8bit() {
	int x, y;
	for(x = 0; x < TEX_SIZE; x++) {
		for(y = 0; y < TEX_SIZE; y++) {
			result[x][y].r = truncate(source[x][y].r, 255, 8);
			result[x][y].g = truncate(source[x][y].g, 255, 8);
			result[x][y].b = truncate(source[x][y].b, 255, 4); 
		}
	}
}

void random_dithering_8bit() {
	int x, y;
	for(x = 0; x < TEX_SIZE; x++) {
		for(y = 0; y < TEX_SIZE; y++) {
			set_pixel_color(&result[x][y],
				truncate((source[x][y].r - 20) + rand()%40, 255, 8),
				truncate((source[x][y].g - 20) + rand()%40, 255, 8),
				truncate((source[x][y].b - 20) + rand()%40, 255, 4)
			);
		}
	}
}

//void ordered_dithering_1bit() {
//	int x, y;
//	for(x = 0; x < TEX_SIZE; x++) {
//		for(y = 0; y < TEX_SIZE; y++) {
//			int power = (source[x][y].r + source[x][y].g + source[y][y].b);
//			int quantum;
//			if(power > 380) {
//				set_pixel_color(&result[x][y], 255, 255, 255);
//			} else {
//				set_pixel_color(&result[x][y], 0, 0, 0);
//			}
//			
//			result[x+1][y].r += (quantum*7.0/16.0)/3;
//			result[x+1][y].g += (quantum*7.0/16.0)/3;
//			result[x+1][y].b += (quantum*7.0/16.0)/3;
//			result[x-1][y-1].r += (quantum*3.0/16.0)/3;
//			result[x-1][y-1].g += (quantum*3.0/16.0)/3;
//			result[x-1][y-1].b += (quantum*3.0/16.0)/3;
//			result[x][y-1].r += (quantum*5.0/16.0)/3;
//			result[x][y-1].g += (quantum*5.0/16.0)/3;
//			result[x][y-1].b += (quantum*5.0/16.0)/3;
//			result[x+1][y-1].r += (quantum*1.0/16.0)/3;
//			result[x+1][y-1].g += (quantum*1.0/16.0)/3;
//			result[x+1][y-1].b += (quantum*1.0/16.0)/3;
//		}
//	}
//}

void (*reduce)() = to_1bit;

void handle_keyboard(unsigned char ch, int x, int y) {
    switch(ch) {
        case 'q':
            effect = blur;
			reduce = to_3bit;
            break;
        case 'w':
            effect = sharpen;
			reduce = to_1bit;
            break;
        case 'e':
            effect = edge_detection1;
			reduce = to_8bit;
            break;
		case 'r':
			effect = edge_detection3;
			reduce = random_dithering_1bit;
			break;
		case 't':
			effect = blur5x;
			reduce = random_dithering_8bit;
			break;
		case 'z':
			effect = emboss;
			//reduce = ordered_dithering_1bit;
			break;
		case 'a':
			if(alpha_x < 255) {
				alpha_x++;
			}
			break;
		case 'd':
			if(alpha_x > 0) {
				alpha_x--;
			}
			break;
		case 'm':
			if(mode == DISPLAY_TO_LESSBIT) {
				mode = DISPLAY_LAYERS_AND_EFFECT;
			} else {
				mode++;
			}
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
    load_rgb(source, "image.rgb", TEX_SIZE);
	load_rgba(layer1,"image.rgba", TEX_SIZE);
	load_rgba(layer2, "pentagram.rgba", TEX_SIZE);

}

// Generate and display the image.
void display() {
    // Call user image generation
	
	switch(mode) {
		case DISPLAY_LAYERS_AND_EFFECT:
			effect();
			blend_layer_at(result, layer1, alpha_x, 0);
			blend_layer_at(result, layer1, 50, 50);
			break;
		case DISPLAY_FRACTAL:
			print_fractal();
			break;
		case DISPLAY_TO_LESSBIT:
			reduce();
			break;
	}

    // Copy image to texture memory
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEX_SIZE, 2*TEX_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
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
    glutInitWindowSize(TEX_SIZE, 2*TEX_SIZE);
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
