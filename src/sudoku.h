#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

typedef uint8_t u8;
typedef char s8; // because of mingw warning not 'int8_t'
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;
typedef uint64_t u64;
typedef int64_t s64;

typedef float f32;
typedef double f64;

#define return_defer(n) do{			\
    result = (n);				\
    goto defer;					\
  }while(0)

#define da_append_many(n, xs, xs_len) do{				\
    u64 new_cap = (n)->cap;						\
    while((n)->len + xs_len >= new_cap) {				\
      new_cap *= 2;							\
      if(new_cap == 0) new_cap = 16;					\
    }									\
    if(new_cap != (n)->cap) {						\
      (n)->cap = new_cap;						\
      (n)->items = realloc((n)->items, (n)->cap * sizeof(*((n)->items))); \
      assert((n)->items);						\
    }									\
    memcpy((n)->items + (n)->len, xs, xs_len * sizeof(*((n)->items)));	\
    (n)->len += xs_len;							\
  }while(0)
    

#define da_append(n, x)	do{						\
    u64 new_cap = (n)->cap;						\
    while((n)->len >= new_cap) {					\
      new_cap *= 2;							\
      if(new_cap == 0) new_cap = 16;					\
    }									\
    if(new_cap != (n)->cap) {						\
      (n)->cap = new_cap;						\
      (n)->items = realloc((n)->items, (n)->cap * sizeof(*((n)->items))); \
      assert((n)->items);						\
    }									\
    (n)->items[(n)->len++] = x;						\
  }while(0)

#define errorf(...) do{						\
    fflush(stdout);						\
    fprintf(stderr, "%s:%d:ERROR: ", __FILE__, __LINE__);	\
    fprintf(stderr,  __VA_ARGS__ );				\
    fprintf(stderr, "\n");					\
    fflush(stderr);						\
  }while(0)

#define panicf(...) do{						\
    errorf(__VA_ARGS__);					\
    exit(1);							\
  }while(0)

#endif // TYPES_H


#ifndef SUDOKU_H
#define SUDOKU_H

#include <string.h>
#include <assert.h>

#ifndef SUDOKU_DEF
#  define SUDOKU_DEF static inline
#endif // SUDOKU_DEF

typedef struct {
  u8 grid[81];
}Sudoku;

typedef u8 Sudoku_Table[9];

#define SUDOKU_GET(s, x, y) (s).grid[(y) * 9 + (x)]
#define SUDOKU_SET(s, x, y, v) (s).grid[(y) * 9 + (x)] = (v)

SUDOKU_DEF void sudoku_print(Sudoku *s);
SUDOKU_DEF bool sudoku_is_solved(Sudoku *s);
SUDOKU_DEF bool sudoku_is_valid(Sudoku *s);
SUDOKU_DEF bool sudoku_solve(Sudoku *s);
SUDOKU_DEF u64 sudoku_solutions(Sudoku *s);

#ifdef SUDOKU_IMPLEMENTATION

SUDOKU_DEF void sudoku_collect(Sudoku *s, Sudoku_Table t, u8 px, u8 py, u8 w, u8 h) {
  for(u8 dy=0;dy<h;dy++) {
    for(u8 dx=0;dx<w;dx++) {

      u8 x = px + dx;
      u8 y = py + dy;
      assert(x < 9 && y < 9);
      
      u8 value = SUDOKU_GET(*s, x, y);
      if(value == 0 || 9 < value) {
	continue;
      }
      
      t[value - 1] += 1;
    }
  }
}

SUDOKU_DEF bool sudoku_solve(Sudoku *s) {

  // Find empty slot
  u8 i=0;
  for(;i<9*9;i++) {
    u8 value = s->grid[i];
    if(value == 0 || 9 < value) break;
  }

  // No empty slot found
  if(i == 9 * 9) return true;

  u8 y = i / 9;
  u8 x = i % 9;

  // Collect stats for (x, y)
  Sudoku_Table table = {0};
  sudoku_collect(s, table, 0, y, 9, 1);
  sudoku_collect(s, table, x, 0, 1, 9);
  sudoku_collect(s, table, x - (x % 3), y - (y % 3), 3, 3);

  for(u8 j=0;j<9;j++) {

    // If j was encountered, skip j
    if(table[j] > 0) continue;

    // Backtrack
    SUDOKU_SET(*s, x, y, j + 1);
    if(sudoku_solve(s)) return true;
    SUDOKU_SET(*s, x, y, 0);
  }

  return false;
}


SUDOKU_DEF u64 sudoku_solutions(Sudoku *s) {
  (void) s;
  u64 n = 0;
  
  return n;  
}

SUDOKU_DEF bool sudoku_is_valid(Sudoku *s) {

  Sudoku_Table table = {0};

  u8 x = 0;
  u8 y = 0;
  for(u8 i=0;i<9;i++) {

    memset(&table, 0, sizeof(table));
    
    sudoku_collect(s, table, 0, i, 9, 1);
    sudoku_collect(s, table, i, 0, 1, 9);
    sudoku_collect(s, table, x, y, 3, 3);

    for(u8 j=0;j<9;j++) if(table[j] > 3) return false;
    
    x += 3;
    if(x == 9) {
      x = 0;
      y += 3;
    }
  }

  return true;
}

SUDOKU_DEF bool sudoku_is_solved(Sudoku *s) {

  Sudoku_Table table = {0};

  u8 x = 0;
  u8 y = 0;
  for(u8 i=0;i<9;i++) {

    memset(&table, 0, sizeof(table));
    
    sudoku_collect(s, table, 0, i, 9, 1);
    sudoku_collect(s, table, i, 0, 1, 9);
    sudoku_collect(s, table, x, y, 3, 3);

    for(u8 j=0;j<9;j++) if(table[j] != 3) return false;
    
    x += 3;
    if(x == 9) {
      x = 0;
      y += 3;
    }
  }

  return true;
}

SUDOKU_DEF void sudoku_print(Sudoku *s) {
  for(u8 y=0;y<9;y++) {
    for(u8 x=0;x<9;x++) {
      u8 value = SUDOKU_GET(*s, x, y);
      if(value == 0 || value > 9) {
	printf("- ");
      } else {
	printf("%u ", value);
      }
      if((x + 1) % 3 == 0) printf(" ");
    }
    if((y + 1) % 3 == 0) printf("\n");
    printf("\n");
  }
}

#endif // SUDOKU_IMPLEMENTATION

#endif // SUDOKU_H
