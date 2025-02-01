#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <math.h>

#define GAME_TITLE "Pong"
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define GAME_WIDTH 640
#define GAME_HEIGHT 360
#define FPS 60
#define PADDING 20
#define PADDLE_SPEED 200.f
#define BALL_SPEED 200.f

typedef struct  {
	Vector2 pos;
	Vector2 size;
	int score;
} Paddle;

typedef struct {
	Vector2 pos;
	float radius;
	float angle;
} Ball;

Rectangle get_rect(Paddle paddle) {
	return (Rectangle){paddle.pos.x-paddle.size.x/2,paddle.pos.y-paddle.size.y/2,paddle.size.x,paddle.size.y};
}

void player_movement(Paddle *player, float dt) {
	
	if(IsKeyDown(KEY_UP)) {
		player->pos.y -= PADDLE_SPEED * dt;
	}
	if(IsKeyDown(KEY_DOWN)) {
		player->pos.y += PADDLE_SPEED * dt;
	}

}

void enemy_movement(Paddle *enemy, Ball ball) {
	enemy->pos.y = ball.pos.y;
	enemy->pos.y = Clamp(enemy->pos.y+enemy->size.y/2, 0, GAME_HEIGHT-enemy->size.y/2);
}

void player_wall_collision(Paddle *player) {

	if(player->pos.y + player->size.y/2 >= GAME_HEIGHT) {
		player->pos.y = GAME_HEIGHT - player->size.y/2;
	}

	if(player->pos.y - player->size.y/2 <= 0) {
		player->pos.y = player->size.y/2;
	}
}

void ball_movement(Ball *ball, float dt) {
	float radian = ball->angle * DEG2RAD;
	Vector2 direction = {0};
	direction.x = cosf(radian);
	direction.y = sinf(radian);

	Vector2 movement = Vector2Scale(direction, BALL_SPEED * dt);

	ball->pos = Vector2Add(ball->pos, movement);
}


void ball_wall_collision(Ball *ball, Paddle *player, Paddle *enemy) {
	if (ball->pos.y <= 0 || ball->pos.y >= GAME_HEIGHT) {
		ball->angle = -ball->angle;
	}

	if (ball->pos.x <= 0) {
		enemy->score++;
		ball->angle = 150;
		ball->pos.x = GAME_WIDTH/2;
		ball->pos.y = GAME_HEIGHT/2;
	} else if (ball->pos.x >= GAME_WIDTH) {
		player->score++;
		ball->angle = 45;
		ball->pos.x = GAME_WIDTH/2;
		ball->pos.y = GAME_HEIGHT/2;
	}
}


void ball_paddle_collision(Ball *ball, Paddle player, Paddle enemy) {
	Rectangle player_rect = get_rect(player);
	
	Rectangle enemy_rect = get_rect(enemy);

	if (CheckCollisionCircleRec(ball->pos, ball->radius, player_rect) || CheckCollisionCircleRec(ball->pos, ball->radius, enemy_rect)) {
		ball->angle = 180 - ball->angle;	
	}

}


int main() {

	
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, GAME_TITLE);
	SetConfigFlags(FLAG_WINDOW_HIGHDPI);
	SetWindowState(FLAG_WINDOW_RESIZABLE);

	SetTargetFPS(FPS);

	RenderTexture2D render_texture = LoadRenderTexture(GAME_WIDTH, GAME_HEIGHT);
	SetTextureFilter(render_texture.texture, TEXTURE_FILTER_POINT);

	Paddle player = {0};
	player.pos = (Vector2){PADDING, GAME_HEIGHT/2};
	player.size = (Vector2){10, 50};

	Paddle enemy = {0};
	enemy.pos = (Vector2){GAME_WIDTH-PADDING, GAME_HEIGHT/2};
	enemy.size = (Vector2){10, 50};

	Ball ball = {0};
	ball.pos = (Vector2){GAME_WIDTH/2, GAME_HEIGHT/2};
	ball.radius = 6.f;
	ball.angle = 150;


	while(!WindowShouldClose()) {

		float dt = GetFrameTime();

		int screen_width = GetScreenWidth();
		int screen_height = GetScreenHeight();

		float scale = fminf(screen_width/(float)GAME_WIDTH, screen_height/(float)GAME_HEIGHT);

		player_movement(&player, dt);
		player_wall_collision(&player);

		enemy_movement(&enemy, ball);

		ball_movement(&ball, dt);
		ball_wall_collision(&ball, &player, &enemy);
		ball_paddle_collision(&ball, player, enemy);

		// to get correct mouse GetMousePosition
		// float offset_x = (screen_width - (GAME_WIDTH * scale)) * 0.5f;
		// float offset_y = (screen_height - (GAME_HEIGHT * scale)) * 0.5f;
		// Vector2 screen_mouse_pos = GetMousePosition();
		// Vector2 game_mouse_pos = (Vector2){
		// 	(screen_mouse_pos.x - offset_x) / scale,
		// 	(screen_mouse_pos.y - offset_y) / scale
		// };

		BeginTextureMode(render_texture);
		ClearBackground(RED);

		// draw bg
		DrawRectangleV((Vector2){0, 0}, (Vector2){GAME_WIDTH/2, GAME_HEIGHT}, GetColor(0x15a084ff));
		DrawRectangleV((Vector2){GAME_WIDTH/2, 0}, (Vector2){GAME_WIDTH/2, GAME_HEIGHT}, GetColor(0x26b999ff));

		// draw center circle
		DrawCircleV((Vector2){GAME_WIDTH/2, GAME_HEIGHT/2}, 50.f, GetColor(0xffffffBF));

		// draw center line
		DrawLine(GAME_WIDTH/2, 0, GAME_WIDTH/2, GAME_HEIGHT, WHITE);

		// draw player
		Rectangle player_rect = (Rectangle){player.pos.x-player.size.x/2, player.pos.y-player.size.y/2, player.size.x, player.size.y};
		DrawRectangleRounded(player_rect, 4.f, 10.f, WHITE);

		// draw enemy
		Rectangle enemy_rect = (Rectangle){enemy.pos.x-enemy.size.x/2, enemy.pos.y-enemy.size.y/2, enemy.size.x, enemy.size.y};
		DrawRectangleRounded(enemy_rect, 4.f, 10.f, WHITE);

		// draw score
		DrawText(TextFormat("%d", player.score), GAME_WIDTH/4, PADDING, 24, WHITE);
		DrawText(TextFormat("%d", enemy.score), GAME_WIDTH - GAME_WIDTH/4, PADDING, 24, WHITE);

		// draw ball
		DrawCircleV(ball.pos, ball.radius, YELLOW);

		EndTextureMode();


		// Render target
		Vector2 screen_center = (Vector2){ screen_width / 2, screen_height / 2 };
		Vector2 game_center = (Vector2){ GAME_WIDTH / 2, GAME_HEIGHT / 2 };

		Rectangle src_rect = (Rectangle){ 0, 0, render_texture.texture.width, -render_texture.texture.height };
		Rectangle dest_rect = (Rectangle){ screen_center.x - game_center.x * scale, screen_center.y - game_center.y * scale, GAME_WIDTH * scale, GAME_HEIGHT * scale };

		BeginDrawing();
		ClearBackground(BLACK);
		DrawTexturePro(render_texture.texture, src_rect, dest_rect, (Vector2){0}, 0.f, WHITE);
		EndDrawing();

	}

	CloseWindow();
	
	return 0;
}