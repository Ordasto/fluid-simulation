#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum {
    // using 0 for object makes the divergence calc easier
    OBJECT = 0, // "static" object
    FLUID = 1,  // normal cell
} ObjType;

// should probably rename this to "side"
typedef struct {
    float vel;
    ObjType obj_type;
} Velocity;

// four pointers to each velocity component (sides).
// edges are shared between Cells
typedef struct {
    Velocity* left;  // positive enters cell
    Velocity* right; // positive leaves cell
    Velocity* up;    // positive leaves cell
    Velocity* down;  // positive enters cell
} GridCell;

// should probably just use a rectangle struct
typedef struct {
    int width;
    int height;
    // int depth; // for 3D
} Size2D;

void update(Size2D size, GridCell grid[size.height][size.width], int iterations);
// is it even worth putting this in a function?
void draw(Size2D size, GridCell grid[size.height][size.width]);

// All grid Cells i, j
// add dt * g deltatime and -9.81 m/s
// divergence = sum up all velcoities adjacent
// positive = too much outflow
// negative = too much inflow
// zero = Incrompessible
//
// Incrompessible
// calc divergence
// subtract/add 1/4 of it from the four velcoities
//
// Obstacles/Walls
// fixed outflow, we use 1/n where n is the number of normal cells
// for divergence for the Incrompessible part
// Velocity for objects can be zero for a wall or something else to simulate an
// object like a turbine, but cannot be changed by the divergence solver.
int main(void) {
    // Vector2 center = {width / 2, height / 2};
    InitWindow(900, 500, "Fluid Dynamics Simulation");
    int grid_w = 300;
    int grid_h = 300;
    Size2D grid_size = {grid_w, grid_h};
    GridCell(*grid)[grid_w] = malloc(grid_w * grid_h * sizeof(GridCell));

    // instantiate grid
    // outer cells need to be objects
    // need to make (grid_w+1)*grid_h + grid_w*(grid_h+1) velcotiy "edges"
    // int edge_count = (grid_w + 1) * grid_h + grid_w * (grid_h + 1);

    long vel_size = sizeof(Velocity);

    for (int y = 0; y < grid_h; y++) {
        for (int x = 0; x < grid_w; x++) {
            // easier to read
            GridCell* cell = &grid[y][x];
            // turn this into a function
            cell->down = malloc(vel_size);
            cell->down->vel = 0;
            cell->down->obj_type = FLUID;
            cell->right = malloc(vel_size);
            cell->right->vel = 0;
            cell->right->obj_type = FLUID;

            // this is extra jank
            // top row
            if (y == 0) {
                cell->up = malloc(vel_size);
                cell->up->vel = 0;
                cell->up->obj_type = OBJECT;
            } else {
                cell->up = grid[y - 1][x].down;
            }
            // left column
            if (x == 0) {
                cell->left = malloc(vel_size);
                cell->left->vel = 0;
                cell->left->obj_type = OBJECT;
            } else {
                cell->left = grid[y][x - 1].right;
            }
            // bottom row
            if (y == grid_h - 1) {
                cell->down->obj_type = OBJECT;
                cell->down->vel = 0;
            }
            // right column
            if (x == grid_w - 1) {
                cell->right->obj_type = OBJECT;
                cell->right->vel = 0;
            }
        }
    }

    // 10 down far left column

    int iterations = 5;

    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        draw(grid_size, grid);
        update(grid_size, grid, iterations);
    }
    CloseWindow();

    return 0;
}

void update(Size2D size, GridCell grid[size.height][size.width], int iterations) {
    const float relaxation = 1.9; // const between 1 and 2
    const float dt = GetFrameTime();
    const float g = -9.81;
    for (int i = 0; i < iterations; i++) {
        for (int y = 0; y < size.height; y++) {
            for (int x = 0; x < size.width; x++) {
                GridCell* cell = &grid[y][x];

                // use this to colour cells?
                float divergence = cell->right->vel - cell->left->vel + cell->up->vel - cell->down->vel;
                divergence *= relaxation; // make it less bad

                // these two vars factor in objects
                float diverg_mod = cell->down->obj_type + cell->right->obj_type + cell->left->obj_type + cell->up->obj_type;
                float vel_div_mod = divergence / diverg_mod;

                // could just multiply by object type
                if (cell->left->obj_type == FLUID) {
                    cell->left->vel += vel_div_mod * dt;
                }
                if (cell->right->obj_type == FLUID) {
                    cell->right->vel -= vel_div_mod * dt;
                }
                if (cell->down->obj_type == FLUID) {
                    cell->down->vel += vel_div_mod * dt;
                }
                if (cell->up->obj_type == FLUID) {
                    cell->up->vel -= (vel_div_mod + g) * dt;
                }
            }
        }
    }
}

void draw(Size2D size, GridCell grid[size.height][size.width]) {
    BeginDrawing();
    ClearBackground(BLACK);

    Vector2 rect_size = {
        (float)GetScreenWidth() / (float)size.width,
        (float)GetScreenHeight() / (float)size.height,
    };

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        int x_cord = GetMouseX() / rect_size.x;
        int y_cord = GetMouseY() / rect_size.y;
        grid[y_cord][x_cord].down->vel += 100;
    }

    for (int y = 0; y < size.height; y++) {
        for (int x = 0; x < size.width; x++) {
            GridCell* cell = &grid[y][x];
            float div = cell->right->vel - cell->left->vel + cell->up->vel - cell->down->vel;
            Color col = {(int)(div * 1000) % 255, 0, 0, 255};
            Vector2 pos = {rect_size.x * x, rect_size.y * y};
            // DrawRectangle(x * rect_size.x, y * rect_size.y, rect_size.x, rect_size.y, col);
            DrawRectangleV(pos, rect_size, col);
        }
    }

    DrawText(TextFormat("x:%d. y:%d", GetMouseX(), GetMouseY()), GetMouseX(), GetMouseY(), 20, WHITE);
    EndDrawing();
}
