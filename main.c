/*
 * Image Viewer written in C
 * build options:
 * gcc main.c -o test \
 * `pkg-config --cflags glfw3 glew` \
 * `pkg-config --libs glfw3 glew` \
 * -framework OpenGL \
 * -std=c99 -Wall
 */
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define WINDOW_WIDTH  900
#define WINDOW_HEIGHT 600

int main(){

    if (!glfwInit()){
        printf("Failed to initialize GLFW\n");
        return EXIT_FAILURE;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Image Viewer", NULL, NULL);
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
 
    return 0;
}
