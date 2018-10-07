#ifndef COLOR_H
#define COLOR_H
#include "common.h"
static void closest_vertice(Eigen::MatrixXf &T, float xworld, float yworld)
{
    T(2, 0) = 0; T(2, 1) = 0; T(2, 2) = 0; // reset z(tag) to zero
    float p0x = T(0, 0),
         p0y = T(1, 0),
         p1x = T(0, 1),
         p1y = T(1, 1),
         p2x = T(0, 2),
         p2y = T(1, 2);

    float d0 = sqrt(pow(p0x - xworld, 2) + pow(p0y - yworld, 2)),
          d1 = sqrt(pow(p1x - xworld, 2) + pow(p1y - yworld, 2)),
          d2 = sqrt(pow(p2x - xworld, 2) + pow(p2y - yworld, 2));
    float min = std::min({d0, d1, d2});
    // z row will act as flag (if -1, then this vertice should be colored)
    if (d0 == min) T(2, 0) = -1;
    else if (d1 == min) T(2, 1) = -1;
    else T(2, 2) = -1;

}
static void mouse_button_callback_c(GLFWwindow *window, int button, int action, int mods)
{
    double xworld, yworld;
    get_world_coordinates(window, xworld, yworld);
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        for (auto itr = V.begin(); itr != V.end(); ++itr)
        {
            if (is_intersection(itr->second, xworld, yworld))
            {
                //triangle has been hit, find which vertice mouse is closest to
                closest_vertice(itr->second, xworld, yworld);
                itr->first.color_changing = true;
                break; //only one triangle can be clicked on
            }
        }
    }
}
static void change_color_vertice(std::pair<VertexBufferObject, Eigen::MatrixXf> &T, float r, float g, float b)
{
    uint col;
    if(T.second(2, 0) < 0) col = 0;
    else if (T.second(2, 1) < 0) col = 1;
    else if (T.second(2, 2) < 0) col = 2;
         
    T.second(3, col) = r;
    T.second(4, col) = g;
    T.second(5, col) = b;

    T.first.update(T.second);
    T.first.color_changed = true;
}

#endif