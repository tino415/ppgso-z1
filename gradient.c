/*
 OpenGL visualization skeleton for displaying bitmap images. Just provide a GenerateImage function.
 Good starting point for all image processing exercises for parallel programming.

 Example of generating bitmaps using GenerateImage and the prepared GLUT OpenGL visualization.
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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

typedef struct {
    GLubyte r;
    GLubyte g;
    GLubyte b;
} pixel;

pixel image[2*TEX_SIZE][TEX_SIZE];

float kernel2[5][5] = {
	{1,1,1,1,1},
	{1,1,1,1,1},
	{1,1,1,1,1},
	{1,1,1,1,1},
	{1,1,1,1,1}
};

GLuint texture;

void load_image() {
	FILE *f = fopen("image.rgb", "rb");
	if(f == NULL) {
		printf("Error opening image\n");
		exit(1);
	}
	fread(image, TEX_SIZE*TEX_SIZE*sizeof(pixel), 1, f);
	fclose(f);
}

void set_color(int x, int y, int red, int green, int blue) {
	if(red > 255) {
		image[x][y].r = 255;
	} else if(red < 0) {
		image[x][y].r = 0;
	} else {
		image[x][y].r = red;
	}

	if(green > 255) {
		image[x][y].g = 255;
	} else if(green < 0) {
		image[x][y].g = 0;
	} else {
		image[x][y].g = green;
	}

	if(blue > 255) {
		image[x][y].b = 255;
	} else if(blue < 0) {
		image[x][y].b = 0;
	} else {
		image[x][y].b = blue;
	}
}

void convolution_transform_3x(int x, int y, float kernel[3][3], float dividor) {
	int kx, ky, red, green, blue;
	red = green = blue = 0;
	int efx = TEX_SIZE+x;

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

	set_color(efx, y, red*dividor, green*dividor, blue*dividor);
}

void convolution_transform_5x(int x, int y, float kernel[5][5], float dividor) {
	int kx, ky, red, green, blue;
	red = green = blue = 0;
	int efx = TEX_SIZE+x;

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

	set_color(efx, y, red*dividor, green*dividor, blue*dividor);
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
}

// Generate and display the image.
void display() {
    // Call user image generation
    load_image();
	//blur();
	edge_detection3();
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
    glutMainLoop();
    return EXIT_SUCCESS;
}
