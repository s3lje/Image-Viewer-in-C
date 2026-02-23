/*
 * Image Viewer written in C
 *
 * The program allows the user to specify a .ppm image file
 * and displays it in a window.
 *
 * build options:
 *      gcc main.c -o test \
 *      `pkg-config --cflags glfw3 glew` \
 *      `pkg-config --libs glfw3 glew` \
 *      -framework OpenGL \
 *      -std=c99 -Wall
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define WINDOW_WIDTH  900
#define WINDOW_HEIGHT 600

typedef struct {
    int width, height;
    unsigned char* data;
} PPMImage;

// Vertex data for a quad
float vertices[] = {
    // Positions        // Texture Coords
    -1.0f,  1.0f, 0.0f,   0.0f, 0.0f,  // Top-left
    -1.0f, -1.0f, 0.0f,   0.0f, 1.0f,  // Bottom-left
     1.0f, -1.0f, 0.0f,   1.0f, 1.0f,  // Bottom-right
     1.0f,  1.0f, 0.0f,   1.0f, 0.0f   // Top-right
};

// Indices for the quad
unsigned int indices[] = {
    0, 1, 2,
    0, 2, 3
};

// Shader source code (simple)
const char* vertexShaderSource = 
"\n"
"#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec2 aTexCoord;\n"
"out vec2 TexCoord;\n"
"void main() {\n"
"    gl_Position = vec4(aPos, 1.0);\n"
"    TexCoord = aTexCoord;\n"
"}\n";

const char* fragmentShaderSource =
"\n"
"#version 330 core\n"
"out vec4 FragColor;\n"
"in vec2 TexCoord;\n"
"uniform sampler2D texture1;\n"
"void main() {\n"
"    FragColor = texture(texture1, TexCoord);\n"
"}\n";

PPMImage* loadPPM(const char*);
void freePPM(PPMImage*);
GLuint compileShaders();

int main(int argc, char* argv[]){
    PPMImage* image;

    if (argc < 2){
        printf("usage: %s [image-file] ...\n", argv[0]);
        return EXIT_FAILURE;
    } else {
        image = loadPPM(argv[1]);
        if (!image){
            printf("Could not load file: %s...\n", argv[1]);
            return EXIT_FAILURE;
        }
    }
    
    // Initialize glfw
    if (!glfwInit()){
        printf("Failed to initialize GLFW\n");
        return EXIT_FAILURE;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT,
            "Image Viewer", NULL, NULL);
    if (!window){
        printf("Failed to create window...\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK){
        printf("Failed to initialize GLEW...\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }

    printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
 
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->width, image->height, 
            0, GL_RGB, GL_UNSIGNED_BYTE, image->data);
    glGenerateMipmap(GL_TEXTURE_2D);

    GLuint shaderProgram = compileShaders();

    // Setup VAO and VBO
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
            (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Main Program Loop
    while (!glfwWindowShouldClose(window)){
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindTexture(GL_TEXTURE_2D, texture);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    freePPM(image);
    glfwDestroyWindow(window);
    glfwTerminate();
    
    glDeleteVertexArrays(1, &VAO);

    return 0;
}

PPMImage* loadPPM(const char* filename){
    char format[3];
    int width, height, maxval;
    unsigned char* data; 
    size_t read_size; 
    PPMImage* image; 

    FILE* f = fopen(filename, "rb");
    if (!f){
        fprintf(stderr, "Error: could not open file %s\n", filename);
        return NULL;
    }
    
    // Check valid format
    fscanf(f, "%2s", format);
    if (strcmp(format, "P6")){
        fprintf(stderr, "Error: not a P6 PPM file\n");
        fclose(f);
        return NULL;
    }
    
    // Skip comments
    while (1){
        int c = fgetc(f);
        if (c == '#'){
            while (fgetc(f) != '\n');
        } else {
            ungetc(c, f);
            break;
        }
    }
    
    fscanf(f, "%d %d", &width, &height);
    fscanf(f, "%d", &maxval);

    if (maxval != 255){
        fprintf(stderr, "Error: only maxval 255 supported...\n");
        fclose(f);
        return NULL;
    }
    fgetc(f);

    data = (unsigned char*)malloc(width * height * 3);
    if (!data){
        fprintf(stderr, "Error: memory allocation failed...\n");
        free(data);
        fclose(f);
        return NULL;
    }
    
    read_size = fread(data, 1, width * height * 3, f);
    if (read_size != width * height * 3){
        fprintf(stderr, "Error: failed to read image data...\n");
        free(data);
        fclose(f);
        return NULL;
    }

    fclose(f);

    image = (PPMImage*)malloc(sizeof(PPMImage));
    image->width = width;
    image->height = height;
    image->data = data;

    return image; 
}

void freePPM(PPMImage* image){
    if (image){
        free(image->data);
        free(image);
    }
}

GLuint compileShaders(){
    // Vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Cleanup shaders after linking
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}
