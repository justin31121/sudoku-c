#include <stdio.h>
#include <time.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "..\thirdparty\stb_truetype.h"

#define STB_IMAGE_IMPLEMENTATION
#include "..\thirdparty\stb_image.h"

#define WINDOW_IMPLEMENTATION
#define WINDOW_STB_TRUETYPE
#define WINDOW_STB_IMAGE
#include "window.h"

#define AUDIO_IMPLEMENTATION
#include "audio.h"

#define WAV_IMPLEMENTATION
#include "wav.h"

#define SUDOKU_IMPLEMENTATION
#include "sudoku.h"

#include "..\rsc\sudokus.h"

Sudoku origin, s, solved;
u8 x_focus = 0;
u8 y_focus = 0;

Audio audio;
u8 *blob_data;
u64 blob_data_len;
u8 *erase_data;
u64 erase_data_len;
u8 *failure_data;
u64 failure_data_len;

void cell_place(u8 n) {
  u8 value = SUDOKU_GET(origin, x_focus, y_focus);
  u8 value_solved = SUDOKU_GET(solved, x_focus, y_focus);
  
  if( value == 0 || 9 < value ) {

    if(n == value_solved) {
      audio_play(&audio, blob_data, blob_data_len / audio.sample_size);
    } else {
      audio_play(&audio, failure_data, failure_data_len / audio.sample_size);
    }    

    SUDOKU_SET(s, x_focus, y_focus, n);
  }
}

void cell_remove() {
  u8 value_origin = SUDOKU_GET(origin, x_focus, y_focus);
  u8 value = SUDOKU_GET(s, x_focus, y_focus);
  
  if( (value_origin == 0 || 9 < value_origin)  &&
      (value != 0 && value <= 9) ) {
    audio_play(&audio, erase_data, erase_data_len / audio.sample_size);
    SUDOKU_SET(s, x_focus, y_focus, 0);
  }
}

s32 main () {

  srand(time(NULL));

  //Sudoku s = {0};

  u8 queue[81];
  for(u8 i=0;i<81;i++) queue[i] = i;
  __sudoku_shuffle_fisher_yates(queue, 81);

  u64 m = 40;

  for(u8 i=0;i<81;i++) {
    u8 j = queue[i];
    
    u8 y = j / 9;
    u8 x = j % 9;

    Sudoku_Table table = {0};
    sudoku_collect(&s, table, 0, y, 9, 1);
    sudoku_collect(&s, table, x, 0, 1, 9);
    sudoku_collect(&s, table, x - (x % 3), y - (y % 3), 3, 3);

    u8 random_start = (u8) (__sudoku_randf() * 8.f);
    printf("random_start: %u\n", random_start); fflush(stdout);
    u8 random_number = 0;
    for(u8 k=0;random_number==0 && k<9;k++) {
      if(table[random_start] == 0) {
	random_number = random_start + 1;
      }
      random_start = (random_start + 1) % 9;
    }
    assert(random_number != 0);

    SUDOKU_SET(s, x, y, random_number);

    sudoku_print(&s);
    printf("=====================================\n"); fflush(stdout);

    
    Sudoku copy = s;

    printf("Solving ... "); fflush(stdout);
    bool can_be_solved = sudoku_solve(&s);
    printf("Done\n"); fflush(stdout);
    
    s = copy;
    if(!can_be_solved) {
      SUDOKU_SET(s, x, y, 0);
    }      

  }

  sudoku_solve(&s);
  sudoku_print(&s);
  printf("solved: %s\n", sudoku_is_solved(&s) ? "true" : "false");
  printf("valid: %s\n", sudoku_is_valid(&s) ? "true" : "false");

  /* u8 queue[81]; */
  /* for(u8 i=0;i<81;i++) queue[i] = i; */
  /* shuffle_fisher_yates(queue, 81); */

  /* for(u8 i=0;i<81;i++) { */
  /*   u8 j = queue[i]; */
    
  /*   u8 y = j / 9; */
  /*   u8 x = j % 9; */

  /*   u8 value = SUDOKU_GET(s, x, y); */
  /*   SUDOKU_SET(s, x, y, 0); */
  /*   u64 solutions = sudoku_solutions(&s); */
  /*   assert(solutions != 0); */
  /*   if(solutions != 1) { */
  /*     SUDOKU_SET(s, x, y, value);  */
  /*   } */
    
  /* } */


  return 0;
}

s32 main1() {

  origin = s = solved = s2;
  sudoku_solve(&solved);

  f32 DT = 1000.f / 60.f;
  sudoku_solve(&solved);

  Window window;
  if(!window_init(&window, 800, 800, "Sudoku", 0)) {
    return 1;
  }

  // ASSSETS

#define FONT_HEIGHT 24
  if(!push_font("c:\\windows\\fonts\\arial.ttf", FONT_HEIGHT)) {
    return 1;
  }

  u32 tex;
  s32 width, height;
  if(!push_image("rsc\\eraser.png", &width, &height, &tex)) {
    return 1;
  }

  Wav blob_wav;
  if(!wav_slurp(&blob_wav, "rsc\\b1.wav", &blob_data, &blob_data_len)) {
    return 1;
  }
  assert(blob_wav.wFormatTag == WAV_FMT_PCM);

  Wav erase_wav;
  if(!wav_slurp(&erase_wav, "rsc\\erase.wav", &erase_data, &erase_data_len)) {
    return 1;
  }
  assert(erase_wav.wFormatTag == blob_wav.wFormatTag);
  assert(erase_wav.channels == blob_wav.channels);
  assert(erase_wav.sample_rate == blob_wav.sample_rate);

  Wav failure_wav;
  if(!wav_slurp(&failure_wav, "rsc\\failure.wav", &failure_data, &failure_data_len)) {
    return 1;
  }
  assert(failure_wav.wFormatTag == blob_wav.wFormatTag);
  assert(failure_wav.channels == blob_wav.channels);
  assert(failure_wav.sample_rate == blob_wav.sample_rate);

  if(!audio_init(&audio, AUDIO_FMT_S16, blob_wav.channels, blob_wav.sample_rate)) {
    return 1;
  }

  Vec4f FOREGROUND = vec4f(0.9f, 0.9f, 0.9f, 1.f);

  bool dragging = false;

  window_renderer_set_color(vec4f(1, 1, 1, 1));

#define LEFT 0
#define UP 1
#define RIGHT 2
#define DOWN 3

#define CONTROL_TRESHHOLD 112.f
  
  bool controls[4] = {0};
  f32 control_dt = 0.f;
  
  Window_Event event;
  while(window.running) {
    
    while(window_peek(&window, &event)) {
      switch(event.type) {

      case WINDOW_EVENT_KEYRELEASE: {
	switch(event.as.key) {
	case 'a':
	case WINDOW_ARROW_LEFT: {
	  controls[LEFT] = false;
	} break;
	case 'w':
	case WINDOW_ARROW_UP: {
	  controls[UP] = false;
	} break;
	case 'd':
	case WINDOW_ARROW_RIGHT: {
	  controls[RIGHT] = false;
	} break;
	case 's':
	case WINDOW_ARROW_DOWN: {
	  controls[DOWN] = false;
	} break;
	}
      } break;
	
      case WINDOW_EVENT_KEYPRESS: {	
	switch(event.as.key) {
	case WINDOW_ESCAPE: {
	  window.running = false;	  
	} break;
	case 'a':
	case WINDOW_ARROW_LEFT: {
	  controls[LEFT] = true;
	  control_dt = CONTROL_TRESHHOLD;
	} break;
	case 'w':
	case WINDOW_ARROW_UP: {
	  controls[UP] = true;
	  control_dt = CONTROL_TRESHHOLD;
	} break;
	case 'd':
	case WINDOW_ARROW_RIGHT: {
	  controls[RIGHT] = true;
	  control_dt = CONTROL_TRESHHOLD;
	} break;
	case 's':
	case WINDOW_ARROW_DOWN: {
	  controls[DOWN] = true;
	  control_dt = CONTROL_TRESHHOLD;
	} break;

	case WINDOW_BACKSPACE: {
	  cell_remove();
	} break;

	default: {
	  s8 c = event.as.key;
	  if('1' <= c && c <= '9') {
	    cell_place( (u8) (c - '0') );
	  }

	} break;
	}
	
      } break;

      case WINDOW_EVENT_MOUSERELEASE: {
	dragging = false;
      } break;
      case WINDOW_EVENT_MOUSEPRESS: {
	dragging = true;
      }
      default: {
	
      } break;
      }
    }


    if(control_dt >= CONTROL_TRESHHOLD) {
      if(controls[LEFT]  && x_focus > 0) x_focus -= 1;
      if(controls[UP]    && y_focus > 0) y_focus -= 1;
      if(controls[RIGHT] && x_focus < 8) x_focus += 1;
      if(controls[DOWN]  && y_focus < 8) y_focus += 1;
      control_dt = 0.f;
    }

    if(controls[LEFT] || controls[UP] || controls[RIGHT] || controls[DOWN]) control_dt += DT;
    else control_dt = 0.f;

    s32 _x, _y;
    window_get_mouse_position(&window, &_x, &_y);
    f32 mouse_x = (f32) _x;
    f32 mouse_y = (f32) _y;

    f32 width = (f32) window.width;
    f32 height = (f32) window.height;

#define CELL_SIZE 48
#define BORDER_WIDTH 2

    // ++ CELL_SIZE + CELL_SIZE + CELL_SIZE ++ CELL_SIZE + CELL_SIZE + CELL_SIZE ++ CELL_SIZE + CELL_SIZE + CELL_SIZE ++
    
    f32 grid_size = (4 * BORDER_WIDTH + 3 * CELL_SIZE) * 3 + 2 * BORDER_WIDTH;
    Vec2f grid_pos = vec2f(width/2 - grid_size/2, height/2 - grid_size/2);
    draw_solid_rect(grid_pos,
		    vec2f(grid_size, grid_size),
		    BLACK);

    Vec2f cell_pos = vec2f(grid_pos.x + 2 * BORDER_WIDTH,
			   grid_pos.y + 2 * BORDER_WIDTH);

    for(u8 y=0;y<9;y++) {
      for(u8 x=0;x<9;x++) {

	if(dragging &&
	   cell_pos.x <= mouse_x &&
	   mouse_x <= cell_pos.x + CELL_SIZE &&
	   cell_pos.y <= mouse_y &&
	   mouse_y <= cell_pos.y + CELL_SIZE) {
	  x_focus = x;
	  y_focus = y;
	}

	Vec4f color = WHITE;
	if(x == x_focus && (8 - y) == y_focus) {
	  color = FOREGROUND;
	}
	
	draw_solid_rect(cell_pos, vec2f(CELL_SIZE, CELL_SIZE), color);

	u8 value_origin = SUDOKU_GET(origin, x, 8 - y);
	u8 value_solved = SUDOKU_GET(solved, x, 8 - y);
	u8 value = SUDOKU_GET(s, x, 8 - y);

	if(value != 0 && value <= 9) {
	  Vec4f value_color = BLACK;
	  if(value_origin == 0 || 9 < value_origin) {

	    if(value_solved == value) {
	      value_color = BLUE;	    
	    } else {
	      value_color = RED;
	    }	  

	  }

	  Vec2f text_size;
	  s8 c = (s8) value + '0';
	  window_renderer_measure_text(&c, 1, 1.f, &text_size);	

	  window_renderer_text(&c, 1,
			       vec2f(cell_pos.x + CELL_SIZE/2 - text_size.x/2,
				     cell_pos.y + CELL_SIZE/2 - text_size.y/2),
			       1.f, value_color);

	}
	cell_pos.x += CELL_SIZE + BORDER_WIDTH;
	if(x % 3 == 2) cell_pos.x += BORDER_WIDTH;
	
      }
      
      cell_pos.x = grid_pos.x + 2 * BORDER_WIDTH;
      cell_pos.y += CELL_SIZE + BORDER_WIDTH;
      if(y % 3 == 2) cell_pos.y += BORDER_WIDTH;
    }

    Vec2f button_pos = vec2f(grid_pos.x, grid_pos.y/2);
    s8 c = '1';
    for(u8 i=0;i<9;i++) {

      if(window_renderer_text_button(&c, 1, 1.f, BLACK, button_pos, vec2f(CELL_SIZE, CELL_SIZE), FOREGROUND)) {

	cell_place( (u8) (c - '0') );
      }
      button_pos.x += CELL_SIZE + 2 * BORDER_WIDTH;
      c++;
    }

    button_pos.x += 4 * BORDER_WIDTH;
    if(window_renderer_button(button_pos, vec2f(CELL_SIZE, CELL_SIZE),
			      FOREGROUND)) {
      cell_remove();
    }
    f32 z = 0.125f;
    window_renderer_texture(tex,
			    vec2f(button_pos.x + z * CELL_SIZE,
				  button_pos.y + z * CELL_SIZE),
			    vec2f(CELL_SIZE * (1.f - 2*z),
				  CELL_SIZE * (1.f - 2*z)),
			    vec2f(0.f, 0.f), vec2f(1.f, 1.f));

    window_swap_buffers(&window);
  }

  
  window_free(&window);
  
  return 0;
}
