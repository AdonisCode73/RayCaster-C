#include <stdio.h>
#include <SDL.h>
#include <math.h>

#define WIN_HEIGHT 720
#define WIN_WIDTH 1280
#define COLOUR_GREEN 0x002000
#define COLOUR_YELLOW 0xFFDA03

struct Circle {
	double radius;
	double x;
	double y;
};

// any pixel that is located within the circle is drawn
// calculated by getting the equation of the circle and checking that the distance squared of co-ords X,Y is less than radius squared
// if the distance squared is greater then these co-ords are beyond the circumference of the circle
void drawCircle(SDL_Surface* surf, struct Circle circle) {
	double radius_squared = pow(circle.radius, 2);

	for (int x = circle.x - circle.radius; x <= circle.x + circle.radius; x++) {

		for (int y = circle.y - circle.radius; y <= circle.y + circle.radius; y++) {

			double distance_squared = pow(x - circle.x, 2) + pow(y - circle.y, 2);

			if (distance_squared < radius_squared) {
				SDL_Rect pixel = { x, y, 1, 1 };
				SDL_FillRect(surf, &pixel, COLOUR_YELLOW);
			}
		}
	}
}

int main() {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("Failed to init SDL2: %s", SDL_GetError());
		return;
	}

	SDL_Window* win = SDL_CreateWindow("Ray Caster", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIN_WIDTH, WIN_HEIGHT, SDL_WINDOW_SHOWN);

	if (!win) {
		printf("Failed to init the window: %s", SDL_GetError());
		return;
	}

	SDL_Surface* window_surface = SDL_GetWindowSurface(win);

	if (!window_surface) {
		printf("Failed to init the window surface: %s", SDL_GetError());
		return;
	}

	int width = 200;
	int height = 200;
	SDL_Rect rect = {(WIN_WIDTH / 2) - width / 2, (WIN_HEIGHT / 2) - height / 2, width, height};

	SDL_FillRect(window_surface, &rect, COLOUR_GREEN);

	struct Circle circle = { 50.0, 100, WIN_HEIGHT / 2 };
	drawCircle(window_surface, circle);
	SDL_UpdateWindowSurface(win);
	int windowAlive = 1; 

	while (windowAlive) {
		SDL_Event e;
		while (SDL_PollEvent(&e) > 0)
		{
			switch (e.type)
			{
			case SDL_QUIT:
				windowAlive = 0;
				break;
			}
			SDL_UpdateWindowSurface(win);
		}
	}
	return 0;
}