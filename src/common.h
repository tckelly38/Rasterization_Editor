#ifndef COMMON_H
#define COMMON_H
#include <vector>
#include "Helpers.h"
#include <GLFW/glfw3.h>

extern std::vector<std::pair<VertexBufferObject, Eigen::MatrixXf>> V;
extern bool insertion_mode_on;
extern bool translation_mode_on;
extern bool delete_mode_on;
extern bool rotate_mode_on;
extern bool rotate_clockwise;

extern bool left_mouse_down;
extern bool is_drawn;
extern bool tri_drawing_in_process;

extern uint current_tri_index;
extern int index_of_translating_triangle;

extern double prev_xworld;
extern double prev_yworld;
extern float lateral_adj;
extern float longitudinal_adj;
extern float scale;
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
    xworld -= lateral_adj;
    yworld -= longitudinal_adj;
    xworld /= scale;
    yworld /= scale;
}

void get_barycenter(std::pair<VertexBufferObject, Eigen::MatrixXf> &T)
{
    double p0x = T.second(0, 0),
           p0y = T.second(1, 0),
           p1x = T.second(0, 1),
           p1y = T.second(1, 1),
           p2x = T.second(0, 2),
           p2y = T.second(1, 2);
    T.first.barycentric_x = (p0x + p1x + p2x) / 3;
    T.first.barycentric_y = (p0y + p1y + p2y) / 3;
}
void scale_point(float cx, float cy, float scale, float &px, float &py){
    px = cx + (px-cx) * scale;
    py = cy + (py-cy) * scale;
}
void rotate_point(float cx, float cy, float angle, float &px, float &py)
{
    float s = sin(angle);
    float c = cos(angle);

    // translate point back to origin:
    px -= cx;
    py -= cy;

    // rotate point
    float xnew = px * c - py * s;
    float ynew = px * s + py * c;

    // translate point back:
    px = xnew + cx;
    py = ynew + cy;
}
bool is_intersection(const Eigen::MatrixXf &T, double xworld, double yworld)
{
    if (T.cols() != 3 || T.rows() != 6)
        return false;
    double p0x = T(0, 0),
           p0y = T(1, 0),
           p1x = T(0, 1),
           p1y = T(1, 1),
           p2x = T(0, 2),
           p2y = T(1, 2);

    Eigen::Matrix3d tri_tot, tri0, tri1, tri2;
    tri_tot << p0x, p1x, p2x,
        p0y, p1y, p2y,
        1, 1, 1;
    tri0 << p0x, xworld, p1x,
        p0y, yworld, p1y,
        1, 1, 1;
    tri1 << p1x, xworld, p2x,
        p1y, yworld, p2y,
        1, 1, 1;
    tri2 << p2x, xworld, p0x,
        p2y, yworld, p0y,
        1, 1, 1;

    double area_tot = 0.5 * abs(tri_tot.determinant()),
           area0 = 0.5 * abs(tri0.determinant()),
           area1 = 0.5 * abs(tri1.determinant()),
           area2 = 0.5 * abs(tri2.determinant());
    double x = area0 + area1 + area2;
    double y = abs((area0 + area1 + area2) - area_tot);

    if (abs((area0 + area1 + area2) - area_tot) <= 0.00001)
        return true;
    return false;
}

#endif