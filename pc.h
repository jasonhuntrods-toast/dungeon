#ifndef PC_H
# define PC_H

# include <stdint.h>

# include "dims.h"
# include "object.h"

typedef struct dungeon dungeon_t;

typedef struct pc {
  char name[40];
  char catch_phrase[80];
  object_t *carry[10];
  object_t *equipment[12];
} pc_t;

void pc_delete(pc_t *pc);
uint32_t pc_is_alive(dungeon_t *d);
void config_pc(dungeon_t *d);
uint32_t pc_next_pos(dungeon_t *d, pair_t dir);
void place_pc(dungeon_t *d);
uint32_t carry(dungeon_t *d);
uint32_t drop(dungeon_t *d, char index);
uint32_t expunge(dungeon_t *d, char index);
uint32_t wear(dungeon_t *d, char index);
uint32_t takeoff(dungeon_t *d, char index);


#endif
