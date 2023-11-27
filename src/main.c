#include "raylib.h"
#include <math.h>
#include <stdio.h>

typedef enum {
    // using 0 for object makes the divergence calc easier
    OBJECT = 0, // "static" object
    FLUID = 1,  // normal cell
} ObjType;

typedef struct {
    float vel;
    ObjType obj_type;
} Velocity;

// four pointers to each velocity component (sides).
// edges are shared between Cells
typedef struct {
    float pressure;
    Velocity* left;  // positive enters cell
    Velocity* right; // positive leaves cell
    Velocity* up;    // positive leaves cell
    Velocity* down;  // positive enters cell
} GridCell;

typedef struct {
    int width;
    int height;
    // int depth; // for 3D
} Size2D;

void update(GridCell* grid[], Size2D size, int iterations);
void draw(GridCell* grid[]);

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
    int width = 800;
    int height = 450;
    // Vector2 center = {width / 2, height / 2};
    InitWindow(800, 450, "Fluid Dynamics Simulation");
    int grid_w = 20;
    int grid_h = 20;
    GridCell grid[grid_w][grid_h];
    // instantiate grid
    // outer cells need to be objects
    // need to make (grid_w+1)*grid_h + grid_w*(grid_h+1) velcotiy "edges"
    int edge_count = (grid_w + 1) * grid_h + grid_w * (grid_h + 1);
    // how the hell do i map these
    //
    // optimisation doesn't matter we only do this once (famous last words)
    for (int x = 0; x < grid_w; x++) {
        // x = 0 // top
        // x = grid_h-1 // bottom
        // y = 0 // left
        // y = grid_w-1 // right
        for (int y = 0; y < grid_h; y++) {
            // this is hard
        }
    }
    int iterations = 2;
    while (!WindowShouldClose()) {
    }

    CloseWindow();

    return 0;
}

void update(GridCell* grid[], Size2D size, int iterations) {
    const float relaxation = 1.5; // const between 1 and 2
    for (int i = 0; i < iterations; i++) {
        for (int x = 0; x < size.width; x++) {
            for (int y = 0; y < size.height; y++) {
                GridCell* cell = &grid[x][y];
                // cell->pressure = 0; // do we need this?

                // use this to colour cells?
                float divergence = cell->right->vel - cell->left->vel + cell->up->vel - cell->down->vel;
                divergence *= relaxation;

                // these two vars factor in objects
                float diverg_mod = cell->down->obj_type + cell->right->obj_type + cell->left->obj_type + cell->up->obj_type;
                float vel_div_mod = divergence / diverg_mod;

                cell->left->vel += vel_div_mod;
                cell->right->vel -= vel_div_mod;
                cell->down->vel += vel_div_mod;
                cell->up->vel -= vel_div_mod;
            }
        }
    }
}

void draw(GridCell* grid[]) {
    BeginDrawing();
    ClearBackground(BLACK);
    // draw grid
    EndDrawing();
}
