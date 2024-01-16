#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_video.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdint.h>


#define SCREEN_WIDTH	256
#define SCREEN_HEIGHT	224

#define PLAYER_WIDTH 12
#define PLAYER_HEIGHT 16

#define LADDER_WIDTH 8
#define LADDER_HEIGHT 31

#define PLAT_WIDTH 64
#define PLAT_HEIGHT 8

#define START_POS SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2

#define JUMP_FORCE 200
#define GRAVITY 100

#define SPEEDX 50

void *err_;

#define ERR(cmp, fn, ...)						\
	(((err_ = fn(__VA_ARGS__)) cmp)					\
	 ? (fprintf(stderr, "%s: %s", #fn, SDL_GetError()), abort(), NULL) \
	 : err_)

#define ERRP(fn, ...) ERR(== NULL, fn, __VA_ARGS__)


#define nelem(x) (sizeof(x)/sizeof(*x))

typedef struct {
	double posx, posy;
	double speedx, speedy;
	SDL_Surface *sprite;
} Player;

typedef struct {
	int t1, t2, quit;
	int dark;
	double worldTime, delta;
	SDL_Surface *lader, *platform;
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Surface *screen, *charset;
	SDL_Texture *scrtex;
} Game;

const SDL_Point ladder_pos[] = {
	{145, 120},
	{145, 100},
	{115, 120},
	{72, 110},
};
const SDL_Point platform_pos[] = {
	{156, 125},
	{80, 112},
	{50, 140},
	{120, 120 + LADDER_HEIGHT},
};

void PlayerInit(Player *player, double posx, double posy) {
	player->posx = posx;
	player->posy = posy;
	player->speedx = player->speedy = 0;
	player->sprite = ERRP(IMG_Load, "./mario.png");
}

void GameInit(Game *game)
{
	ERR(!= 0, SDL_Init, SDL_INIT_EVERYTHING);
	ERR(!= 0, SDL_CreateWindowAndRenderer, SCREEN_WIDTH * 3, SCREEN_HEIGHT * 3, 0,
	    &game->window, &game->renderer);
	SDL_RenderSetLogicalSize(game->renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(game->window, "Donkey Kong 2022");
	game->screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
					    0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	game->scrtex = SDL_CreateTexture(game->renderer, SDL_PIXELFORMAT_ARGB8888,
					 SDL_TEXTUREACCESS_STREAMING,
					 SCREEN_WIDTH, SCREEN_HEIGHT);

	game->charset = ERRP(SDL_LoadBMP, "./cs8x8.bmp");
	SDL_SetColorKey(game->charset, 1, 0x000000);

	game->lader = ERRP(IMG_Load, "./lader.png");
	game->platform = ERRP(IMG_Load, "./platform.png");

	game->dark = SDL_MapRGB(game->screen->format, 0x00, 0x00, 0x00);

	game->t1 = SDL_GetTicks();
	game->quit = 0;
	game->delta = 0;
	game->t1 = game->t2 = 0;
	game->worldTime = 0;
}


void DrawString(SDL_Surface *screen, int x, int y, const char *text,
                SDL_Surface *charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while(*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
	}
}


void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x;
	dest.y = y;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
}


void playerOnLader(int x, int y, SDL_Point laddersa) {

}

void movePlayer(int speedx, int speedy, SDL_Point platforms) {

}

void GameUpdate(Game *game) {
	char text[128];
	game->t2 = SDL_GetTicks();
	game->delta = (game->t2 - game->t1) * 0.001;
	game->t1 = game->t2;
	game->worldTime += game->delta;

	SDL_FillRect(game->screen, NULL, game->dark);
	for (int i = 0; i < nelem(ladder_pos); i++) {
		DrawSurface(game->screen, game->lader, ladder_pos[i].x, ladder_pos[i].y);
	}
	for (int i = 0; i < nelem(platform_pos); i++) {
		DrawSurface(game->screen, game->platform, platform_pos[i].x, platform_pos[i].y);
	}
	sprintf(text, "TIME: %.1lf", game->worldTime);
	DrawString(game->screen, SCREEN_WIDTH / 2 - strlen(text) * 2, 10, text, game->charset);


}

void PlayerDraw(Player *player, Game *game)
{
	DrawSurface(game->screen, player->sprite, player->posx, player->posy);
}

void GameRender(Game *game)
{
	SDL_UpdateTexture(game->scrtex, NULL, game->screen->pixels, game->screen->pitch);
	SDL_RenderCopy(game->renderer, game->scrtex, NULL, NULL);
	SDL_RenderPresent(game->renderer);
}

SDL_Rect platformRect(SDL_Point pos) {
	return (SDL_Rect){pos.x, pos.y, PLAT_WIDTH, PLAT_HEIGHT};
}

SDL_Rect ladderRect(SDL_Point pos) {
	return (SDL_Rect){pos.x, pos.y, LADDER_WIDTH, LADDER_HEIGHT};
}

int PlayerIntersects(SDL_Rect player)
{
	SDL_Rect platform;
	for (int i = 0; i < nelem(platform_pos); i++) {
		platform = platformRect(platform_pos[i]);
		if (SDL_HasIntersection(&player, &platform)) {
			return i;
		}
	}
	return -1;
}

int IsPlayerOnLadder(SDL_Rect player) {
	SDL_Rect ladder;
	for (int i = 0; i < nelem(ladder_pos); i++) {
		ladder = ladderRect(ladder_pos[i]);
		if (SDL_HasIntersection(&player, &ladder)) {
			return i;
		}
	}
	return -1;
}

SDL_Rect PlayerRect(Player *player) {
	SDL_Rect prect;
	prect.x = player->posx;
	prect.y = player->posy;
	prect.w = player->sprite->w;
	prect.h = player->sprite->h;
	return prect;
}

int PlayerMove(Player *player, Game *game)
{
	SDL_Rect prect, platform;
	int res;
	player->posx += player->speedx * game->delta;\
	int grounded = 0;

	if ((res = PlayerIntersects(PlayerRect(player))) >= 0) {
		platform = platformRect(platform_pos[res]);
		if (player->speedx > 0) {
			player->posx = platform.x - PLAYER_WIDTH;
		} else {
			player->posx = platform.x + platform.w;
		}
	}

	player->posy += player->speedy * game->delta;
	if ((res = PlayerIntersects(PlayerRect(player))) >= 0) {
		platform = platformRect(platform_pos[res]);
		if (player->speedy > 0) {
			grounded = 1;
			player->posy = platform.y - PLAYER_HEIGHT;
		} else {
			player->posy = platform.y + platform.h;
		}
	}
	return grounded;
}

int PlayerUpdate(Player *player, Game *game) {
	return PlayerMove(player, game);
}

int isGrounded(Player *player) {
	SDL_Rect prect, platform;
	int res;
	prect = PlayerRect(player);
	prect.y += 5;
	if ((res = PlayerIntersects(prect)) >= 0) {
		return 1;
	}
	return 0;
}




// main

int main() {
	SDL_Event event;
	Game game;
	Player player;
	GameInit(&game);
	PlayerInit(&player, START_POS);
	int isUp = 0, isDown = 0, isJump = 0, isGround = 0;

	while(!game.quit) {
		GameUpdate(&game);
		isGround = PlayerUpdate(&player, &game);
		if (isGround) isJump = 0;
		PlayerDraw(&player, &game);
		if (!isJump) {
			if (IsPlayerOnLadder(PlayerRect(&player)) < 0) player.speedy = GRAVITY;
			else if (!isUp && !isDown){
				player.speedy = 0;
			}
		} else player.speedy += game.delta * GRAVITY * 10;
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
				case SDLK_ESCAPE: game.quit = 1; break;
				case SDLK_RIGHT: if (!player.speedx) player.speedx = SPEEDX; break;
				case SDLK_LEFT: if (!player.speedx) player.speedx = -SPEEDX; break;
				case SDLK_UP:
					if (IsPlayerOnLadder(PlayerRect(&player)) < 0) break;
					isUp = 1;
					player.speedy = -SPEEDX;
					break;
				case SDLK_DOWN:
					if (IsPlayerOnLadder(PlayerRect(&player)) < 0) break;
					isDown = 1;
					player.speedy = SPEEDX;
					break;
				case SDLK_n:
					PlayerInit(&player, START_POS);
                                        game.worldTime = 0;
					break;
			        case SDLK_SPACE:
					if (!isGrounded(&player)) break;
					player.speedy = -JUMP_FORCE;
					isJump = 1;
					break;
				}
				break;
			case SDL_KEYUP:
				switch (event.key.keysym.sym) {

				case SDLK_ESCAPE: game.quit = 1; break;
				case SDLK_LEFT: if (player.speedx > 0) break;
					player.speedx = 0;
					break;
				case SDLK_RIGHT: if(player.speedx < 0) break;
					player.speedx = 0;
					break;
				case SDLK_UP:
					isUp = 0;
					if (player.speedy > 0) break;
					player.speedy = 0;
					break;
				case SDLK_DOWN:
					isDown = 0;
					if (player.speedy < 0) break;
					player.speedy = 0;
					break;
				}
				break;
			case SDL_QUIT:
				game.quit = 1;
				break;
			};
		};
		GameRender(&game);
	};
	SDL_Quit();
}
