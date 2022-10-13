#include <genesis.h>

#include <resources.h>

#include <string.h>

#define SFX_LASER 65
#define SFX_BOOM 64
#define SFX_WIN 66

const int VELMOD_LEVEL = 3;

const int LEFT_EDGE = 0;
const int RIGHT_EDGE = 320;
const int TOP_EDGE = 0;
const int BOTTOM_EDGE = 224;

const int player_pos_y = 200;
const int player_width = 32;
const int player_height = 8;
const int win_points = 12;

int player_velocityModificator = 1;

int ball_pos_x = -1;
int ball_pos_y = -1;
int ball_vel_x = 1;
int ball_vel_y = 1;
int ball_width = 8;
int ball_height = 8;
int ball_velocityModificator = 1;

int player_pos_x = 144;
int player_vel_x = 0;

int score = 0;

Sprite * ball;
Sprite * player;

bool game_on = FALSE;

char msg_start[22] = "PRESS START TO BEGIN!\0";
char msg_reset[38] = "GAME OVER! PRESS START TO PLAY AGAIN.\0";

char label_score[6] = "SCORE\0";
char str_score[4] = "0";

void showText(char s[]) {
  VDP_drawText(s, 20 - strlen(s) / 2, 15);
}

int sign(int x) {
  return (x > 0) - (x < 0);
}

void endGame() {

  VDP_drawText(msg_reset, 20 - strlen(msg_reset) / 2, 12);
  game_on = FALSE;
}

void winGame() {
  VDP_clearTextArea(0, 0, 33, 33);
  XGM_startPlayPCM(SFX_WIN, 1, SOUND_PCM_CH2);
  VDP_drawText("CONGRATULATIONS! YOU WON!", 20 - strlen("CONGRATULATIONS! YOU WON!") / 2, 12);
  game_on = FALSE;
}

void updateScoreDisplay() {
  sprintf(str_score, "%d", score);
  VDP_clearText(1, 2, 3);
  VDP_drawText(str_score, 1, 2);
}

void startGame() {
  score = 0;
  ball_velocityModificator = 1;
  player_velocityModificator = 1;
  updateScoreDisplay();

  ball_pos_x = 0;
  ball_pos_y = 0;

  ball_vel_x = 1;
  ball_vel_y = 1;
  VDP_clearTextArea(0, 10, 40, 10);

  game_on = TRUE;
}

void moveBall() {
  if (ball_pos_x < LEFT_EDGE) {
    ball_pos_x = LEFT_EDGE;
    ball_vel_x = -ball_vel_x;
    XGM_startPlayPCM(SFX_LASER, 1, SOUND_PCM_CH2);
  } else if (ball_pos_x + ball_width > RIGHT_EDGE) {
    ball_pos_x = RIGHT_EDGE - ball_width;
    ball_vel_x = -ball_vel_x;
    XGM_startPlayPCM(SFX_LASER, 1, SOUND_PCM_CH2);
  }

  if (ball_pos_y < TOP_EDGE) {
    ball_pos_y = TOP_EDGE;
    ball_vel_y = -ball_vel_y;
    XGM_startPlayPCM(SFX_LASER, 1, SOUND_PCM_CH2);
  } else if (ball_pos_y + ball_height > BOTTOM_EDGE) {
    ball_pos_y = BOTTOM_EDGE - ball_height;
    ball_vel_y = -ball_vel_y;
    XGM_startPlayPCM(SFX_LASER, 1, SOUND_PCM_CH2);
  }

  if (ball_pos_y > player_pos_y + 15) {
    XGM_startPlayPCM(SFX_BOOM, 1, SOUND_PCM_CH2);
    endGame();
  }

  if (ball_pos_x < player_pos_x + player_width && ball_pos_x + ball_width > player_pos_x) {
    if (ball_pos_y < player_pos_y + player_height && ball_pos_y + ball_height >= player_pos_y) {
      ball_pos_y = player_pos_y - ball_height - 1;
      ball_vel_y = -ball_vel_y;
      XGM_startPlayPCM(SFX_LASER, 1, SOUND_PCM_CH2);
      score++;
      if (score % VELMOD_LEVEL == 0) {
        ball_velocityModificator++;
        player_velocityModificator++;
      }
      updateScoreDisplay();
      if (score == win_points) {
        winGame();
      }
    }

  }

  ball_pos_x += ball_vel_x * ball_velocityModificator;
  ball_pos_y += ball_vel_y * ball_velocityModificator;

  SPR_setPosition(ball, ball_pos_x, ball_pos_y);
}

void controllHandler(u16 joy, u16 changed, u16 state) {
  if (state & BUTTON_START) {
    if (!game_on) {
      startGame();
    }
  }

  if (joy == JOY_1) {

    if (state & BUTTON_RIGHT) {
      player_vel_x = 3 * player_velocityModificator;
    } else if (state & BUTTON_LEFT) {
      player_vel_x = -3 * player_velocityModificator;
    } else {
      if ((changed & BUTTON_RIGHT) | (changed & BUTTON_LEFT)) {
        player_vel_x = 0;
      }
    }
  }
}

void positionPlayer() {
  player_pos_x += player_vel_x;
  if (player_pos_x < LEFT_EDGE) player_pos_x = LEFT_EDGE;
  if (player_pos_x + player_width > RIGHT_EDGE) player_pos_x = RIGHT_EDGE - player_width;
  SPR_setPosition(player, player_pos_x, player_pos_y);
}

void setUpGraphix() {

  VDP_loadTileSet(bgtile.tileset, 1, DMA);
  PAL_setPaletteDMA(PAL0, bgtile.palette -> data);
  VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(PAL0, 0, FALSE, FALSE, 1), 0, 0, 40, 30);
  SYS_enableInts();

  VDP_setTextPalette(PAL3);
  PAL_setColor(63, RGB24_TO_VDPCOLOR(0xffffff));
  VDP_setTextPlane(BG_B);
  VDP_drawText(label_score, 1, 1);
  updateScoreDisplay();
  showText(msg_start);
  VDP_drawText("COLLECT 12 POINTS TO WIN!", 8, 9);

  SPR_init();
  player = SPR_addSprite( & paddle, player_pos_x, player_pos_y, TILE_ATTR(PAL0, 1, FALSE, FALSE));
  ball = SPR_addSprite( & imgball, 100, 100, TILE_ATTR(PAL0, 1, FALSE, FALSE));
}

void setUpPControll() {
  SYS_disableInts();
  JOY_init();
  JOY_setEventHandler( & controllHandler);
}

int main() {

  setUpPControll();
  XGM_setPCM(SFX_LASER, sfx_laser, sizeof(sfx_laser));
  XGM_setPCM(SFX_BOOM, sfx_boom, sizeof(sfx_boom));
  XGM_setPCM(SFX_WIN, sfx_win, sizeof(sfx_win));
  setUpGraphix();
  XGM_startPlay(xgm_music);
  while (1) {
    if (game_on == TRUE) {
      moveBall();
      positionPlayer();
    }

    SPR_update();
    SYS_doVBlankProcess();
  }
  return (0);
}
