// TODO: Move origin of screen coordinates to left bottom, use normalized coordinates with SDL

#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream>
#include <queue>
#include <random>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>

const double WINDOW_WIDTH  = 800;
const double WINDOW_HEIGHT = 800;
const uint32_t DELAY = 250;

const double RADIUS = 3;
const uint16_t POINT_COUNT = 20;

// NOTE: Points are in normalized coordinates between 0 and 1
struct point {
    double x = 0;
    double y = 0;
};

SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;
bool running = true;

std::vector<point> points;
std::vector<size_t> convex_hull;

void render_point(SDL_Renderer *renderer, const point &p);

void update()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
            break;
        }
    }
}

void render()
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for (auto &p : points) {
        render_point(renderer, p);
    }

    point *p = &points[0];
    for (size_t i = 1; i < convex_hull.size(); i++) {
        point *q = &points[convex_hull[i]];
        SDL_RenderDrawLine(
            renderer,
            p->x * WINDOW_WIDTH, (1 - p->y) * WINDOW_HEIGHT,
            q->x * WINDOW_WIDTH, (1 - q->y) * WINDOW_HEIGHT
        );
        p = q;
    }

    SDL_RenderPresent(renderer);
}

void render_point(SDL_Renderer *renderer, const point &p)
{
    assert((p.x >= 0.0) && (p.x <= 1.0));
    assert((p.y >= 0.0) && (p.y <= 1.0));
    filledCircleColor(
        renderer,
        p.x * WINDOW_WIDTH,
        // NOTE: So that the bottom left corner of the window is (0, 0)
        (1 - p.y) * WINDOW_HEIGHT,
        RADIUS, 0xFFFFFFFF
    );
}

double orientation_test(const point &p1, const point &p2, const point &p3)
{
    /* See lecture notes, this is the determinant we talked about
     * Compare the average growth to the next point each
     * => If p2, p3 have less growth, its a right "knick"
     * Otherwise, no "knick" or left */
    // NOTE: No division, so even if we have points with same x coordinates it does not break the program
    double result = (p3.y - p2.y) * (p2.x - p1.x) - (p2.y - p1.y) * (p3.x - p2.x);
    return result;
}

int main(int argc, char *argv[])
{
    assert(POINT_COUNT >= 3);

    SDL_Init(SDL_INIT_EVENTS | SDL_INIT_TIMER | SDL_INIT_VIDEO);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    window = SDL_CreateWindow("convex-hull", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    /* NOTE: Example
     * points = {
     *    { 0.1, 0.1 },
     *    { 0.2, 0.9 },
     *    { 0.3, 0.2 },
     *    { 0.4, 0.3 },
     *    { 0.5, 0.7 },
     *    { 0.6, 0.2 },
     *    { 0.8, 0.4 }
     *}; */
    std::default_random_engine engine;
    engine.seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<double> distribution(0.05, 0.95);
    for (uint16_t i = 0; i < POINT_COUNT; i++) {
        points.push_back(
            {
                distribution(engine),
                distribution(engine)
            }
        );
    }
    
    // Sort by x coordinate
    std::sort(points.begin(), points.end(), [](const point &left, const point &right) {
        return left.x < right.x;
    });
    std::cout << "- Points -" << std::endl;
    for (auto &p : points) {
        std::cout << p.x << " " << p.y << std::endl;
    }

    // Initial draw
    update();
    render();
    SDL_Delay(DELAY);

    // Store the indices
    convex_hull = {
        0, 1
    };

    // Second draw
    update();
    render();
    SDL_Delay(DELAY);
    size_t l = 1;
    for (int k = 2; k < points.size(); k++) {
        convex_hull.push_back(k);
        update();
        render();
        SDL_Delay(DELAY);
        convex_hull.pop_back();

        while (l >= 1 && orientation_test(points[convex_hull[l-1]], points[convex_hull[l]], points[k]) < 0) {
            l = l - 1;
            convex_hull.pop_back();

            convex_hull.push_back(k);
            update();
            render();
            SDL_Delay(DELAY);
            convex_hull.pop_back();
        }
        l = l + 1;
        convex_hull.push_back(k);
    }

    convex_hull.push_back(points.size() - 2);
    l = convex_hull.size() - 1;
    size_t l_min = l;
    for (int k = points.size() - 3; k >= 0; k--) {
        convex_hull.push_back(k);
        update();
        render();
        SDL_Delay(DELAY);
        convex_hull.pop_back();

        while (l >= l_min && orientation_test(points[convex_hull[l-1]], points[convex_hull[l]], points[k]) < 0) {
            l = l - 1;
            convex_hull.pop_back();

            convex_hull.push_back(k);
            update();
            render();
            SDL_Delay(DELAY);
            convex_hull.pop_back();
        }
        l = l + 1;
        convex_hull.push_back(k);
    }

    std::cout << "- Convex Hull -" << std::endl;
    for (auto &i : convex_hull) {
        std::cout << points[i].x << " " << points[i].y << std::endl;
    }

    while (running) {
        update();
        render();
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
