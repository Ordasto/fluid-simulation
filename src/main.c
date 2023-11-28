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
    // float pressure;
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
void draw(Texture2D* texture, Rectangle source, Rectangle dest, Vector2 origin);

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
    Size2D grid_size = {grid_w, grid_h};
    GridCell(*grid)[grid_w] = malloc(grid_w * grid_h * sizeof(GridCell));
    // instantiate grid
    // outer cells need to be objects
    // need to make (grid_w+1)*grid_h + grid_w*(grid_h+1) velcotiy "edges"
    int edge_count = (grid_w + 1) * grid_h + grid_w * (grid_h + 1);

    long vel_size = sizeof(Velocity);

    for (int y = 0; y < grid_w; y++) {
        for (int x = 0; x < grid_h; x++) {
            // easier to read
            GridCell* cell = &grid[y][x];
            // turn this into a function
            cell->down = malloc(vel_size);
            cell->down->vel = x * y;
            cell->down->obj_type = 1;
            cell->right = malloc(vel_size);
            cell->right->vel = x * y;
            cell->right->obj_type = 1;

            // this is extra jank
            // top row
            if (y == 0) {
                cell->up = malloc(vel_size);
                cell->up->vel = 0;
                cell->up->obj_type = 0;
            } else {
                cell->up = grid[y - 1][x].down;
            }
            // left column
            if (x == 0) {
                cell->left = malloc(vel_size);
                cell->left->vel = 0;
                cell->left->obj_type = 0;
            } else {
                cell->left = grid[y][x - 1].right;
            }
            // bottom row
            if (y == grid_h - 1) {
                cell->down->obj_type = 0;
                cell->down->vel = 0;
            }
            // right column
            if (x == grid_w - 1) {
                cell->right->obj_type = 0;
                cell->right->vel = 0;
            }
        }
    }
    int iterations = 2;
    Rectangle source = {0, 0, grid_w, grid_h};
    Rectangle dest = {0, 0, GetScreenWidth(), GetScreenHeight()};
    Image image = GenImageColor(grid_w, grid_h, WHITE);

    Vector2 origin = {0, 0};
    while (!WindowShouldClose()) {
        // update(grid_size, grid, 1);
        // probably bad
        Texture2D texture = LoadTextureFromImage(blank);
        draw(texture, source, dest, origin);
        UnloadTexture(texture);
    }
    CloseWindow();

    return 0;
}

void update(Size2D size, GridCell grid[size.height][size.width], int iterations) {
    const float relaxation = 1.5; // const between 1 and 2
    for (int i = 0; i < iterations; i++) {
        for (int y = 0; y < size.width; y++) {
            for (int x = 0; x < size.height; x++) {
                GridCell* cell = &grid[y][x];
                // cell->pressure = 0; // do we need this?

                // use this to colour cells?
                float divergence = cell->right->vel - cell->left->vel + cell->up->vel - cell->down->vel;
                divergence *= relaxation; // make it less bad

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

void draw(Texture2D* texture, Rectangle source, Rectangle dest, Vector2 origin) {
    BeginDrawing();
    ClearBackground(BLACK);
    DrawTexturePro(*texture, source, dest, origin, 0, WHITE);
    EndDrawing();
}
