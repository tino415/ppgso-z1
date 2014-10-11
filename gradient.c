/*
 OpenGL visualization skeleton for displaying bitmap images. Just provide a GenerateImage function.
 Good starting point for all image processing exercises for parallel programming.

 Example of generating bitmaps using GenerateImage and the prepared GLUT OpenGL visualization.
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

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

void set_color(pixel target[TEX_SIZE][TEX_SIZE], int x, int y, int red, int green, int blue) {
	target[x][y].r = get_between_0_255(red);
	target[x][y].g = get_between_0_255(green);
	target[x][y].b = get_between_0_255(blue);
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

void (*effect)() = blur;

void handle_keyboard(unsigned char ch, int x, int y) {
    switch(ch) {
        case 'b':
            effect = blur;
            break;
        case 's':
            effect = sharpen;
            break;
        case 'e':
            effect = edge_detection1;
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

    effect();
	blend_layer_at(result, layer1, alpha_x, 0);
	blend_layer_at(result, layer1, 50, 50);
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
