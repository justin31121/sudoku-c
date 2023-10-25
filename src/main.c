#include <stdio.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "..\thirdparty\stb_truetype.h"

#define WINDOW_IMPLEMENTATION
#define WINDOW_STB_TRUETYPE
#include "window.h"

#define SUDOKU_IMPLEMENTATION
#include "sudoku.h"

#include "..\rsc\sudokus.h"

s32 main() {

  Sudoku origin = s2;
  Sudoku s = s2;
  Sudoku solved = s2;
  sudoku_solve(&solved);

  Window window;
  if(!window_init(&window, 800, 800, "Sudoku", 0)) {
    return 1;
  }

#define FONT_HEIGHT 24
  if(!push_font("c:\\windows\\fonts\\arial.ttf", FONT_HEIGHT)) {
    return 1;
  }

  //Vec4f FOREGROUND = vec4f(0.73333f, 0.87058f, 0.9843f, 1.f);
  Vec4f FOREGROUND = GREEN ;

  u8 x_focus = 0;
  u8 y_focus = 0;

  window_renderer_set_color(vec4f(1, 1, 1, 1));
  
  Window_Event event;
  while(window.running) {

    bool clicked = false;
    bool pressed = false;
    
    while(window_peek(&window, &event)) {
      switch(event.type) {
      case WINDOW_EVENT_KEYPRESS: {
	
	switch(event.as.key) {
	case WINDOW_ESCAPE: {
	  window.running = false;
	} break;
	case 'a':
	case WINDOW_ARROW_LEFT: {
	  if(x_focus > 0) x_focus -= 1;
	} break;
	case 'w':
	case WINDOW_ARROW_UP: {
	  if(y_focus > 0) y_focus -= 1;
	} break;
	case 'd':
	case WINDOW_ARROW_RIGHT: {
	  if(x_focus < 8) x_focus += 1;
	} break;
	case 's':
	case WINDOW_ARROW_DOWN: {
	  if(y_focus < 8) y_focus += 1;
	} break;

	case WINDOW_BACKSPACE: {
	  u8 value = SUDOKU_GET(origin, x_focus, y_focus);
	  if( (value == 0 || 9 < value) ) {
	    SUDOKU_SET(s, x_focus, y_focus, 0);
	  }
	} break;

	default: {
	  s8 c = event.as.key;
	  u8 value = SUDOKU_GET(origin, x_focus, y_focus);
	  if((value == 0 || 9 < value) &&
	     '1' <= c &&
	     c <= '9') {
	    SUDOKU_SET(s, x_focus, y_focus, (u8) (c - '0'));
	  }

	} break;
	}
	
      } break;
      case WINDOW_EVENT_MOUSEPRESS: {
	clicked = true;
      }
      default: {
	
      } break;
      }
    }

    s32 _x, _y;
    window_get_mouse_position(&window, &_x, &_y);
    f32 mouse_x = (f32) _x;
    f32 mouse_y = (f32) _y;

    f32 width = (f32) window.width;
    f32 height = (f32) window.height;

#define CELL_SIZE 48
#define BORDER_WIDTH 4
    
    f32 grid_size = 9 * (BORDER_WIDTH + CELL_SIZE) + BORDER_WIDTH;
    Vec2f grid_pos = vec2f(width/2 - grid_size/2, height/2 - grid_size/2);
    draw_solid_rect(grid_pos,
		    vec2f(grid_size, grid_size),
		    BLACK);

    for(u8 y=0;y<9;y++) {
      for(u8 x=0;x<9;x++) {

	Vec2f cell_pos = vec2f(grid_pos.x + BORDER_WIDTH + (BORDER_WIDTH + CELL_SIZE) * x,
			       grid_pos.y + BORDER_WIDTH + (BORDER_WIDTH + CELL_SIZE) * y);

	if(clicked &&
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

	if(value == 0 || 9 < value) {
	  continue;
	}

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
    }

    window_swap_buffers(&window);
  }

  window_free(&window);
  
  return 0;
}
