// This example is heavily based on the tutorial at https://open.gl

// OpenGL Helpers to reduce the clutter
#include "Helpers.h"

// GLFW is necessary to handle the OpenGL context
#include <GLFW/glfw3.h>

// Linear Algebra Library
#include <Eigen/Core>

// Timer
#include <chrono>

#include <iostream>
#include <vector>

#define MAX_TRIANGLES 10

bool insertion_mode_on = false;

// VertexBufferObject wrapper
//std::vector<VertexBufferObject> VBO(2);
bool is_drawn = false;
bool tri_drawing_in_process = false;
//int current_tri_vertice = 0;
uint current_tri_index = 0;
// Contains the vertex positions
using namespace std;
vector<pair<VertexBufferObject, Eigen::MatrixXf>> V(MAX_TRIANGLES, make_pair(VertexBufferObject(), Eigen::MatrixXf::Zero(2, 1))); // no special std container care needed for non fixed size matrices
static void cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (tri_drawing_in_process)
    {
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        double xworld = ((xpos / double(width)) * 2) - 1;
        double yworld = (((height - 1 - ypos) / double(height)) * 2) - 1; // NOTE: y axis is flipped in glfw

        V[current_tri_index].second.col(V[current_tri_index].second.cols() - 1) << xworld, yworld;
        V[current_tri_index].first.update(V[current_tri_index].second);
    }
}
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    // Get the position of the mouse in the window
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    // Get the size of the window
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Convert screen position to world coordinates
    double xworld = ((xpos / double(width)) * 2) - 1;
    double yworld = (((height - 1 - ypos) / double(height)) * 2) - 1; // NOTE: y axis is flipped in glfw

    // Update the position of the first vertex if the left button is pressed
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        is_drawn = true;
        tri_drawing_in_process = true;
        V[current_tri_index].second.col(V[current_tri_index].second.cols() - 1) << xworld, yworld;

        if (V[current_tri_index].second.cols() < 3){
            V[current_tri_index].second.conservativeResize(Eigen::NoChange, V[current_tri_index].second.cols() + 1);
            V[current_tri_index].second.col(V[current_tri_index].second.cols() - 1) << xworld, yworld;
        }
        else{
            tri_drawing_in_process = false;
            current_tri_index++;
        }
    }
    // Upload the change to the GPU
    V[current_tri_index].first.update(V[current_tri_index].second);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    // Update the position of the first vertex if the keys 1,2, or 3 are pressed
    switch (key)
    {
    case GLFW_KEY_I:
        if(action == GLFW_RELEASE){
            if(insertion_mode_on)
            {
                glfwSetMouseButtonCallback(window, NULL);
                glfwSetCursorPosCallback(window, NULL);
                insertion_mode_on = false;
            }
            else{
                glfwSetMouseButtonCallback(window, mouse_button_callback);
                glfwSetCursorPosCallback(window, cursor_position_callback);
                insertion_mode_on = true;
            }
        }
        break;
    default:
        break;
    }

    // // Upload the change to the GPU
    // VBO.update(V);
}

int main(void)
{
    GLFWwindow *window;

    // Initialize the library
    if (!glfwInit())
        return -1;

    // Activate supersampling
    glfwWindowHint(GLFW_SAMPLES, 8);

    // Ensure that we get at least a 3.2 context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    // On apple we have to load a core profile with forward compatibility
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(800, 600, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

#ifndef __APPLE__
    glewExperimental = true;
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    }
    glGetError(); // pull and savely ignonre unhandled errors like GL_INVALID_ENUM
    fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

    int major, minor, rev;
    major = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR);
    minor = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR);
    rev = glfwGetWindowAttrib(window, GLFW_CONTEXT_REVISION);
    printf("OpenGL version recieved: %d.%d.%d\n", major, minor, rev);
    printf("Supported OpenGL is %s\n", (const char *)glGetString(GL_VERSION));
    printf("Supported GLSL is %s\n", (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION));

    // Initialize the VAO
    // A Vertex Array Object (or VAO) is an object that describes how the vertex
    // attributes are stored in a Vertex Buffer Object (or VBO). This means that
    // the VAO is not the actual object storing the vertex data,
    // but the descriptor of the vertex data.
    VertexArrayObject VAO;
    VAO.init();
    VAO.bind();

    // Initialize the VBO with the vertices data
    // A VBO is a data container that lives in the GPU memory
    for (auto itr = V.begin(); itr != V.end(); ++itr)
        itr->first.init();

    // V.resize(5, 4);
    // V <<
    //     -0.4, 0.4, -0.4, 0.4,             // x coordinates
    //     0.4, 0.4, -0.4, -0.4,              // y ccordinates
    //     0.0, 1.0, 0.0, 1.0,               // colors
    //     0.0, 0., 1.0, 1.0,
    //     1.0, 0., 0.0, 0.0;
    //VBO.update(V[0]);

    // Initialize the OpenGL Program
    // A program controls the OpenGL pipeline and it must contains
    // at least a vertex shader and a fragment shader to be valid
    Program program;
    const GLchar *vertex_shader =
        "#version 150 core\n"
        "in vec2 position;"
        "void main()"
        "{"
        "    gl_Position = vec4(position, 0.0, 1.0);"
        "}";

    const GLchar *fragment_shader =
        "#version 150 core\n"
        "out vec4 outColor;"
        "void main()"
        "{"
        "    outColor = vec4(0.0, 0.0, 0.0, 1.0);"
        "}";

    // Compile the two shaders and upload the binary to the GPU
    // Note that we have to explicitly specify that the output "slot" called outColor
    // is the one that we want in the fragment buffer (and thus on screen)
    program.init(vertex_shader, fragment_shader, "outColor");
    program.bind();

    // The vertex shader wants the position of the vertices as an input.
    // The following line connects the VBO we defined above with the position "slot"
    // in the vertex shader
    // program.bindVertexAttribArray("position", VBO);
    //program.bindVertexAttribArray("color", VBO, 3, VBO.rows, 2);

    // Register the keyboard callback
    glfwSetKeyCallback(window, key_callback);

   

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
        // Clear the framebuffer
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        if (is_drawn){
            for (auto itr = V.begin(); itr != V.end(); ++itr){
                program.bindVertexAttribArray("position", itr->first);      
                // Bind your program
                program.bind();
                // Draw a triangle or line based on num vertices
                if (itr->second.cols() == 2)
                    glDrawArrays(GL_LINES, 0, 2);
                else if (itr->second.cols() == 3){
                    glDrawArrays(GL_LINES, 0, 2);
                    glDrawArrays(GL_TRIANGLES, 0, 3);
                }
            }
        }

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    // Deallocate opengl memory
    program.free();
    VAO.free();
    V[0].first.free();

    // Deallocate glfw internals
    glfwTerminate();
    return 0;
}
