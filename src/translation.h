#ifndef TRANSLATION_H
#define TRANSLATION_H
#include "common.h"
static void cursor_position_callback_o(GLFWwindow *window, double xpos, double ypos)
{
    double xworld, yworld;
    get_world_coordinates(window, xworld, yworld);
    auto itr = V.begin();
    if (left_mouse_down)
    {
        for (; itr != V.end(); ++itr)
        {
            if (itr->first.translating)
                break;
        }
        if (!itr->first.translating)
            return;
        // translate with position of mouse itr.second
        itr->second(0, 0) += xworld - prev_xworld;
        itr->second(1, 0) += yworld - prev_yworld;
        itr->second(0, 1) += xworld - prev_xworld;
        itr->second(1, 1) += yworld - prev_yworld;
        itr->second(0, 2) += xworld - prev_xworld;
        itr->second(1, 2) += yworld - prev_yworld;
        prev_xworld = xworld;
        prev_yworld = yworld;
    }
}
void mouse_button_callback_p(GLFWwindow *window, int button, int action, int mods)
{
    double xworld, yworld;
    get_world_coordinates(window, xworld, yworld);
    for (auto itr = V.begin(); itr != V.end(); ++itr)
    {
        if (is_intersection(itr->second, xworld, yworld))
        {
            //triangle has been hit
            itr->first.free();
            V.erase(itr);

            break; //only one triangle can be clicked on
        }
    }
}
void mouse_button_callback_r(GLFWwindow *window, int button, int action, int mods)
{
    double xworld, yworld;
    get_world_coordinates(window, xworld, yworld);
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        for (auto itr = V.begin(); itr != V.end(); ++itr)
        {
            if (is_intersection(itr->second, xworld, yworld))
            {
                //triangle has been hit
                itr->first.rotate = true;
                get_barycenter(*itr);
                break; //only one triangle can be clicked on
            }
        }
    }
}
void mouse_button_callback_n(GLFWwindow *window, int button, int action, int mods)
{
    double xworld, yworld;
    get_world_coordinates(window, xworld, yworld);
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        for (auto itr = V.begin(); itr != V.end(); ++itr)
        {
            if (is_intersection(itr->second, xworld, yworld))
            {
                //triangle has been hit
                itr->first.animate = !itr->first.animate;
                get_barycenter(*itr);
                break; //only one triangle can be clicked on
            }
        }
    }
}
void mouse_button_callback_o(GLFWwindow *window, int button, int action, int mods)
{
    double xworld, yworld;
    get_world_coordinates(window, xworld, yworld);
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
            left_mouse_down = true;
        else if (action == GLFW_RELEASE)
            left_mouse_down = false;
    }
    if (left_mouse_down)
    {
        for (auto itr = V.begin(); itr != V.end(); ++itr)
        {
            if (is_intersection(itr->second, xworld, yworld))
            {
                //triangle has been hit
                itr->first.translating = true;
                break; //only one triangle can be clicked on
            }
        }
        prev_xworld = xworld;
        prev_yworld = yworld;
    }
    else
    {
        for (auto itr = V.begin(); itr != V.end(); ++itr)
        {
            if (itr->first.translating)
            {
                itr->first.translating = false;
                break;
            }
        }
    }
}
#endif