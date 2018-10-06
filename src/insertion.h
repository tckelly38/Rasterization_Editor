#ifndef INSERTION_H
#define INSERTION_H
#include "common.h"
static void cursor_position_callback_i(GLFWwindow *window, double xpos, double ypos)
{
    if (tri_drawing_in_process)
    {
        double xworld, yworld;
        get_world_coordinates(window, xworld, yworld);
        V[current_tri_index].second.col(V[current_tri_index].second.cols() - 1) << xworld, yworld, 0.0, 0.0, 0.0, 0.0;
        V[current_tri_index].first.update(V[current_tri_index].second);
    }
}
static void mouse_button_callback_i(GLFWwindow *window, int button, int action, int mods)
{
    double xworld, yworld;
    get_world_coordinates(window, xworld, yworld);

    // Update the position of the first vertex if the left button is pressed
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        is_drawn = true;
        tri_drawing_in_process = true;
        V[current_tri_index].second.col(V[current_tri_index].second.cols() - 1) << xworld, yworld, 0.0, 0.0, 0.0, 0.0;

        if (V[current_tri_index].second.cols() < 3)
        {
            V[current_tri_index].second.conservativeResize(Eigen::NoChange, V[current_tri_index].second.cols() + 1);
            V[current_tri_index].second.col(V[current_tri_index].second.cols() - 1) << xworld, yworld, 0.0, 0.0, 0.0, 0.0;
            V[current_tri_index].first.update(V[current_tri_index].second);
        }
        else
        {
            tri_drawing_in_process = false;
            V[current_tri_index].first.done_drawing = true;
            V[current_tri_index].first.update(V[current_tri_index].second);
            current_tri_index++;
        }
    }
}
#endif