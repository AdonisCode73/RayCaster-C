#include <stdio.h>
#include <SDL.h>
#include <math.h>

#define WIN_HEIGHT 720
#define WIN_WIDTH 1280
#define COLOUR_BLACK 0x000000
#define COLOUR_GREEN 0x008000
#define COLOUR_YELLOW 0xffd43b
#define COLOUR_SUN 0xfffbd4

#define TABLE_SIZE 361

struct Circle {
	double radius;
	double x;
	double y;
};

struct Rectangle_ {
	SDL_Rect* rect;

	int posOrNegMovement;
};

double cosTable[TABLE_SIZE];
double sinTable[TABLE_SIZE];

// any pixel that is located within the circle is drawn
// calculated by getting the equation of the circle and checking that the distance squared of co-ords X,Y is less than radius squared
// if the distance squared is greater then these co-ords are beyond the circumference of the circle
void drawCircle(SDL_Surface* surf, struct Circle circle) {

	if (SDL_MUSTLOCK(surf)) {
		if (SDL_LockSurface(surf) < 0) {
			fprintf(stderr, "Failed to lock surface: %s\n", SDL_GetError());
			return;
		}
	}

	double radiusSquared = circle.radius * circle.radius;

	int xStart = (int)(circle.x - circle.radius);
	int xEnd = (int)(circle.x + circle.radius);
	int yStart = (int)(circle.y - circle.radius);
	int yEnd = (int)(circle.y + circle.radius);

	Uint32* pixels = (Uint32*)surf->pixels;
	int pitch = surf->pitch / 4;

	for (int x = xStart; x <= xEnd; x++) {
		if (x < 0 || x >= WIN_WIDTH) continue;

		for (int y = yStart; y <= yEnd; y++) {
			if (y < 0 || y >= WIN_HEIGHT) continue;

			double dx = x - circle.x;
			double dy = y - circle.y;

			double distanceSquared = dx * dx + dy * dy;

			if (distanceSquared < radiusSquared) {
				pixels[y * pitch + x] = COLOUR_SUN;
			}
		}
	}

	if (SDL_MUSTLOCK(surf)) {
		SDL_UnlockSurface(surf);
	}
}

void drawRectangle(SDL_Surface* surf, struct Rectangle_* rect, Uint32 colour, double speed, struct Circle circle, double max_distance) {
	if (rect->rect->y < 0) {
		rect->posOrNegMovement = 1;
	}
	if (rect->rect->y + rect->rect->h > WIN_HEIGHT) {
		rect->posOrNegMovement = -1;
	}
	rect->rect->y += rect->posOrNegMovement * speed;

	if (SDL_MUSTLOCK(surf)) {
		if (SDL_LockSurface(surf) < 0) {
			fprintf(stderr, "Failed to lock surface: %s\n", SDL_GetError());
			return;
		}
	}

	double distance = 0.0;

	int xStart = (int)(rect->rect->x);
	int xEnd = (int)(rect->rect->x + rect->rect->w);
	int yStart = (int)(rect->rect->y);
	int yEnd = (int)(rect->rect->y + rect->rect->h);

	Uint32* pixels = (Uint32*)surf->pixels;
	int pitch = surf->pitch / 4;

	for (int x = xStart; x < xEnd; x++) {
		if (x < 0 || x >= WIN_WIDTH) continue;
		for (int y = yStart; y < yEnd; y++) {
			if (y < 0 || y >= WIN_HEIGHT) continue;

			double dx = x - circle.x;
			double dy = y - circle.y;

			double distance = sqrt(dx * dx + dy * dy);

			Uint32 degraded_colour = calculateColourDegradation(distance, colour, max_distance);

			pixels[y * pitch + x] = degraded_colour;
		}
	}

	if (SDL_MUSTLOCK(surf)) {
		SDL_UnlockSurface(surf);
	}
}

void drawRays(SDL_Surface* surf, struct Circle circle, SDL_Rect rect, Uint32 colour, double angleIncrement, double max_pixels) {

	if (SDL_MUSTLOCK(surf)) {
		if (SDL_LockSurface(surf) < 0) {
			fprintf(stderr, "Failed to lock surface: %s\n", SDL_GetError());
			return;
		}
	}


	for (double a = 0; a <= 360; a = a + angleIncrement) {
		double radians = a * (M_PI / 180);

		double x = circle.x;
		double y = circle.y;

		double dx = cosTable[(int)a];
		double dy = sinTable[(int)a];

		Uint32* pixels = (Uint32*)surf->pixels;
		int pitch = surf->pitch / 4;

		int insideCircle = 1;

		for (int i = 0; i < max_pixels; i++) {
			if (x < 0 || x >= WIN_WIDTH || y < 0 || y >= WIN_HEIGHT) {
				break;
			}

			double xSquared = (x - circle.x) * (x - circle.x);
			double ySquared = (y - circle.y) * (y - circle.y);

			if (insideCircle) {
				if (calculateCircleCollision(circle, x, y)) {
					x += dx;
					y += dy;
					continue;
				}
				insideCircle = 0;
			}

			if (calculateRectCollision(rect, x, y)) {
				break;
			}

			double distanceFromCircleEdge = sqrt(xSquared + ySquared) - circle.radius;

			Uint32 degraded_colour = calculateColourDegradation(distanceFromCircleEdge, colour, max_pixels);
			pixels[(int)y * pitch + (int)x] = degraded_colour;

			x += dx;
			y += dy;
		}
	}

	if (SDL_MUSTLOCK(surf)) {
		SDL_UnlockSurface(surf);
	}
}

int calculateRectCollision(SDL_Rect rect, double cx, double cy) {
	if ((cx > rect.x && cx < rect.x + rect.w) && (cy >= rect.y && cy < rect.y + rect.h)) {
		return 1;
	}
	return 0;
}

int calculateCircleCollision(struct Circle circle, double cx, double cy) {
	double xSquared = (cx - circle.x) * (cx - circle.x);
	double ySquared = (cy - circle.y) * (cy - circle.y);
	double radius = circle.radius * circle.radius;

	if (xSquared + ySquared <= radius) {
		return 1;
	}
	return 0;
}

int calculateColourDegradation(double distance, Uint32 colour, double max_pixels) {
	int R = (colour >> 16) & 0xFF;
	int G = (colour >> 8) & 0xFF;
	int B = colour & 0xFF;

	double brightness = 1.0 - (distance / max_pixels);
	if (brightness < 0) {
		brightness = 0;
	}

	R = (int)(R * brightness);
	G = (int)(G * brightness);
	B = (int)(B * brightness);

	return (Uint32)((R << 16) | (G << 8) | B);
}

void calculateTables(double angleIncrement) {
	for (double a = 0; a <= 360; a = a + angleIncrement) {
		double radians = a * (M_PI / 180);

		cosTable[(int)a] = cos(radians);
		sinTable[(int)a] = sin(radians);
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

	const int width = 200;
	const int height = 200;

	SDL_Rect rect = { (WIN_WIDTH / 2) - width / 2, (WIN_HEIGHT / 2) - height / 2, width, height };
	struct Rectangle_ rect_ = { &rect, 1 };

	SDL_Rect eraseScreen = { 0, 0, WIN_WIDTH, WIN_HEIGHT };

	struct Circle circle = { 50.0, 100, WIN_HEIGHT / 2 };
	int windowAlive = 1;

	double angleIncrement = 1.0;
	int calculatedTables = 0;

	while (windowAlive) {
		SDL_Event e;
		while (SDL_PollEvent(&e) > 0)
		{
			switch (e.type)
			{
			case SDL_QUIT:
				windowAlive = 0;
				break;
			case SDL_MOUSEMOTION:
				if (e.motion.state != 0) {
					circle.x = e.motion.x;
					circle.y = e.motion.y;
				}
			}
		}
		SDL_FillRect(window_surface, &eraseScreen, COLOUR_BLACK);

		if (!calculatedTables) {
			calculateTables(angleIncrement);
			calculatedTables = 1;
		}

		drawCircle(window_surface, circle);
		drawRays(window_surface, circle, rect, COLOUR_YELLOW, 1.0, 800.0);

		drawRectangle(window_surface, &rect_, COLOUR_GREEN, 2.5, circle, 800.0);

		SDL_UpdateWindowSurface(win);
		SDL_Delay(5);
	}
	return 0;
}