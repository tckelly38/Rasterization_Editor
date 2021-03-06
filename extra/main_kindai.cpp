// This example is heavily based on the tutorial at https://open.gl

// OpenGL Helpers to reduce the clutter
#include "common.h"
#include "insertion.h"
#include "translation.h"

// Linear Algebra Library
#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Geometry>

// Timer
#include <chrono>
#include <iostream>

#define MAX_TRIANGLES 10

bool insertion_mode_on = false;
bool translation_mode_on = false;
bool delete_mode_on = false;
bool rotate_mode_on = false;
bool rotate_clockwise = false;
bool rotate_counter_clockwise = false;
bool scale_up = false;
bool scale_down = false;

bool left_mouse_down = false;
bool is_drawn = false;
bool tri_drawing_in_process = false;

//int current_tri_vertice = 0;
uint current_tri_index = 0;
int index_of_translating_triangle = -1;

double prev_xworld;
double prev_yworld;
// Contains the vertex positions
using namespace std;
using namespace Eigen;
vector<pair<VertexBufferObject, Eigen::MatrixXf>> V(MAX_TRIANGLES, make_pair(VertexBufferObject(), Eigen::MatrixXf::Zero(6, 1))); // no special std container care needed for non fixed size matrices

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_RELEASE)
    {
        switch (key)
        {
        case GLFW_KEY_I:
            if (insertion_mode_on)
            {
                glfwSetMouseButtonCallback(window, NULL);
                glfwSetCursorPosCallback(window, NULL);
                insertion_mode_on = false;
            }
            else
            {
                glfwSetMouseButtonCallback(window, mouse_button_callback_i);
                glfwSetCursorPosCallback(window, cursor_position_callback_i);
                insertion_mode_on = true;
            }
            break;
        case GLFW_KEY_O:
            if (translation_mode_on)
            {
                glfwSetMouseButtonCallback(window, NULL);
                glfwSetCursorPosCallback(window, NULL);
                translation_mode_on = false;
            }
            else
            {
                glfwSetMouseButtonCallback(window, mouse_button_callback_o);
                glfwSetCursorPosCallback(window, cursor_position_callback_o);
                translation_mode_on = true;
            }
            break;
        case GLFW_KEY_P:
            if (delete_mode_on)
            {
                glfwSetMouseButtonCallback(window, NULL);
                delete_mode_on = false;
            }
            else
            {
                glfwSetMouseButtonCallback(window, mouse_button_callback_p);
                delete_mode_on = true;
            }
            break;
        case GLFW_KEY_R:
            if (rotate_mode_on)
            {
                glfwSetMouseButtonCallback(window, NULL);
                rotate_mode_on = false;
                for (auto itr = V.begin(); itr != V.end(); ++itr)
                    itr->first.rotate = false;
                rotate_clockwise = rotate_counter_clockwise = scale_down = scale_up = false;
            }
            else
            {
                glfwSetMouseButtonCallback(window, mouse_button_callback_r);
                rotate_mode_on = true;
            }
            break;
        case GLFW_KEY_H:
            if (rotate_mode_on)
            {
                rotate_clockwise = false;
            }
            break;
        case GLFW_KEY_J:
            if (rotate_mode_on)
            {
                rotate_counter_clockwise = false;
            }
            break;
        case GLFW_KEY_K:
            if (rotate_mode_on)
            {
                scale_up = false;
            }
            break;
        case GLFW_KEY_L:
            if (rotate_mode_on)
            {
                scale_down = false;
            }
            break;
        default:
            break;
        }
    }
    if (action == GLFW_PRESS)
    {
        switch (key)
        {
        case GLFW_KEY_H:
            if (rotate_mode_on)
            {
                rotate_clockwise = true;
            }
            break;
        case GLFW_KEY_J:
            if (rotate_mode_on)
            {
                rotate_counter_clockwise = true;
            }
            break;
        case GLFW_KEY_K:
            if (rotate_mode_on)
            {
                scale_up = true;
            }
            break;
        case GLFW_KEY_L:
            if (rotate_mode_on)
            {
                scale_down = true;
            }
            break;
        }
    }
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

    // Initialize the OpenGL Program
    // A program controls the OpenGL pipeline and it must contains
    // at least a vertex shader and a fragment shader to be valid
    Program program;
    const GLchar *vertex_shader =
        "#version 150 core\n"
        "in vec2 position;"
        "in vec3 color;"
        "out vec3 fColor;"
        "uniform mat4 trans;"
        "void main()"
        "{"
        "    gl_Position = vec4(position, 0.0, 1.0);"
        "    fColor = color;"
        "}";

    const GLchar *fragment_shader =
        "#version 150 core\n"
        "in vec3 fColor;"
        "out vec4 outColor;"
        "uniform vec3 triangleColor;"
        "void main()"
        "{"
        "    outColor = vec4(fColor, 1.0);"
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
        if (is_drawn)
        {
            for (auto itr = V.begin(); itr != V.end(); ++itr)
            {
                program.bindVertexAttribArray("position", itr->first, 3, 6, 0);
                program.bindVertexAttribArray("color", itr->first, 3, 6, 3);
                // Bind your program
                program.bind();
                // Draw a triangle or line based on num vertices
                if (itr->second.cols() == 2)
                    glDrawArrays(GL_LINES, 0, 2);
                else if (itr->second.cols() == 3 && !itr->first.done_drawing)
                {
                    glDrawArrays(GL_LINE_LOOP, 0, 3);
                }
                else if (itr->second.cols() == 3 && itr->first.done_drawing)
                {
                    if (itr->first.translating)
                        change_color(*itr, 0.0, 0.0, 1.0);
                    else
                        change_color(*itr, 1.0, 0.0, 0.0);

                    if (itr->first.rotate && (rotate_clockwise || rotate_counter_clockwise))
                    {
                        double r;
                        if (rotate_clockwise)
                            r = -10 * M_PI / 180;
                        else
                            r = 10 * M_PI / 180;

                        rotate_point(itr->first.barycentric_x, itr->first.barycentric_y, r, itr->second(0, 0), itr->second(1, 0));
                        rotate_point(itr->first.barycentric_x, itr->first.barycentric_y, r, itr->second(0, 1), itr->second(1, 1));
                        rotate_point(itr->first.barycentric_x, itr->first.barycentric_y, r, itr->second(0, 2), itr->second(1, 2));

                        itr->first.update(itr->second);
                        rotate_clockwise = rotate_counter_clockwise = false;
                    }
                    else if (itr->first.rotate && (scale_up || scale_down))
                    {
                        double s;
                        if (scale_up)
                            s = 1.25;
                        else
                            s = 0.75;
                        scale_point(itr->first.barycentric_x, itr->first.barycentric_y, s, itr->second(0, 0), itr->second(1, 0));
                        scale_point(itr->first.barycentric_x, itr->first.barycentric_y, s, itr->second(0, 1), itr->second(1, 1));
                        scale_point(itr->first.barycentric_x, itr->first.barycentric_y, s, itr->second(0, 2), itr->second(1, 2));

                        itr->first.update(itr->second);
                        scale_up = scale_down = false;
                    }

                    glDrawArrays(GL_TRIANGLES, 0, 3);

                    // wire loop should always be black
                    change_color(*itr, 0.0, 0.0, 0.0);
                    glDrawArrays(GL_LINE_LOOP, 0, 3);
                    //rotate_clockwise  = rotate_counter_clockwise = false;
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
    for (auto itr = V.begin(); itr != V.end(); ++itr)
        itr->first.free();

    // Deallocate glfw internals
    glfwTerminate();
    return 0;
}
