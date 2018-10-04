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
bool translation_mode_on = false;

bool left_mouse_down = false;
// VertexBufferObject wrapper
//std::vector<VertexBufferObject> VBO(2);
bool is_drawn = false;
bool tri_drawing_in_process = false;

//int current_tri_vertice = 0;
uint current_tri_index = 0;
int index_of_translating_triangle = -1;
// Contains the vertex positions
using namespace std;
vector<pair<VertexBufferObject, Eigen::MatrixXf>> V(MAX_TRIANGLES, make_pair(VertexBufferObject(), Eigen::MatrixXf::Zero(5, 1))); // no special std container care needed for non fixed size matrices
void get_world_coordinates(GLFWwindow *window, double &xworld, double &yworld)
{
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    // Get the size of the window
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Convert screen position to world coordinates
    xworld = ((xpos / double(width)) * 2) - 1;
    yworld = (((height - 1 - ypos) / double(height)) * 2) - 1; // NOTE: y axis is flipped in glfw
}
void change_color(pair<VertexBufferObject, Eigen::MatrixXf>& T, float r, float g, float b)
{
    T.second(2, 0) = r;
    T.second(2, 1) = r;
    T.second(2, 2) = r;
    T.second(3, 0) = g;
    T.second(3, 1) = g;
    T.second(3, 2) = g;
    T.second(4, 0) = b;
    T.second(4, 1) = b;
    T.second(4, 2) = b;

    T.first.update(T.second);
}
bool barycentric(const Eigen::MatrixXf& T, double xworld, double yworld)
{
    if(T.cols() != 3 || T.rows() != 5)
        return false;
    double p0x = T(0, 0),
           p0y = T(1, 0),
           p1x = T(0, 1),
           p1y = T(1, 1),
           p2x = T(2, 0),
           p2y = T(2, 2);
    double area = 0.5 * (-p1y * p2x + p0y * (-p1x + p2x) + p0x * (p1y - p2y) + p1x * p2y),
           s = 1 / (2 * area) * (p0y * p2x - p0x * p2y + (p2y - p0y) * xworld + (p0x - p2x) * yworld),
           t = 1 / (2 * area) * (p0x * p1y - p0y * p1x + (p0y - p1y) * xworld + (p1x - p0x) * yworld);
    if (s > 0 && t > 0 && 1 - s - t > 0)
        return true;
    return false;
}

static void cursor_position_callback_i(GLFWwindow *window, double xpos, double ypos)
{
    if (tri_drawing_in_process)
    {
        double xworld, yworld;
        get_world_coordinates(window, xworld, yworld);
        V[current_tri_index].second.col(V[current_tri_index].second.cols() - 1) << xworld, yworld, 0.0, 0.0, 0.0;
        V[current_tri_index].first.update(V[current_tri_index].second);
    }
}
static void cursor_position_callback_o(GLFWwindow *window, double xpos, double ypos)
{
    double xworld, yworld;
    get_world_coordinates(window, xworld, yworld);
    auto itr = V.begin();
    if(left_mouse_down){
        for (; itr != V.end(); ++itr)
        {
            if(itr->first.translating)
                break;

        }
        // translate with position of mouse itr.second
    }
}
void mouse_button_callback_o(GLFWwindow *window, int button, int action, int mods)
{
    double xworld, yworld;
    get_world_coordinates(window, xworld, yworld);
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if(action == GLFW_PRESS)
            left_mouse_down = true;
        else if (action == GLFW_RELEASE)
            left_mouse_down = false;
    }
    if(left_mouse_down){
        for (auto itr = V.begin(); itr != V.end(); ++itr)
        {
            if (barycentric(itr->second, xworld, yworld))
            {
                //triangle has been hit
                itr->first.translating = true;
                break; //only one triangle can be clicked on
            }
        }
    }
    else{
        for (auto itr = V.begin(); itr != V.end(); ++itr)
        {
            itr->first.translating = false; //set all back to false
        }
    }
}

void mouse_button_callback_i(GLFWwindow *window, int button, int action, int mods)
{
    double xworld, yworld;
    get_world_coordinates(window, xworld, yworld);

    // Update the position of the first vertex if the left button is pressed
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        is_drawn = true;
        tri_drawing_in_process = true;
        V[current_tri_index].second.col(V[current_tri_index].second.cols() - 1) << xworld, yworld, 0.0, 0.0, 0.0;

        if (V[current_tri_index].second.cols() < 3){
            V[current_tri_index].second.conservativeResize(Eigen::NoChange, V[current_tri_index].second.cols() + 1);
            V[current_tri_index].second.col(V[current_tri_index].second.cols() - 1) << xworld, yworld, 0.0, 0.0, 0.0;
            V[current_tri_index].first.update(V[current_tri_index].second);
        }
        else{
            tri_drawing_in_process = false;
            V[current_tri_index].first.done_drawing = true;
            V[current_tri_index].first.update(V[current_tri_index].second);

            current_tri_index++;
        }
    }
    // Upload the change to the GPU
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    // Update the position of the first vertex if the keys 1,2, or 3 are pressed
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
            else{
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
        default:
            break;
        }
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


    // Initialize the OpenGL Program
    // A program controls the OpenGL pipeline and it must contains
    // at least a vertex shader and a fragment shader to be valid
    Program program;
    const GLchar *vertex_shader =
        "#version 150 core\n"
        "in vec2 position;"
        "in vec3 color;"
        "out vec3 fColor;"
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
        //glUniform3f(program.uniform("triangleColor"), 0.0f, 0.0f, 0.0f);

        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        if (is_drawn){
            for (auto itr = V.begin(); itr != V.end(); ++itr){
                program.bindVertexAttribArray("position", itr->first, 2, 5, 0);
                program.bindVertexAttribArray("color", itr->first, 3, 5, 2);
                // Bind your program
                program.bind();
                // Draw a triangle or line based on num vertices

                if (itr->second.cols() == 2)
                    glDrawArrays(GL_LINES, 0, 2);
                else if (itr->second.cols() == 3 && !itr->first.done_drawing){
                    glDrawArrays(GL_LINE_LOOP, 0, 3);
                }
                else if (itr->second.cols() == 3 && itr->first.done_drawing){
                    if(itr->first.translating)
                        change_color(*itr, 0.0, 0.0, 1.0);
                    else
                        change_color(*itr, 1.0, 0.0, 0.0);

                    glDrawArrays(GL_TRIANGLES, 0, 3);

                    // wire loop should always be black 
                    change_color(*itr, 0.0, 0.0, 0.0);
                    glDrawArrays(GL_LINE_LOOP, 0, 3);
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
