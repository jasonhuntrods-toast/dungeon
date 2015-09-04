#ifndef MOVE_H
# define MOVE_H

# include <stdint.h>

# include "dims.h"

typedef struct dungeon dungeon_t;
typedef struct character character_t;

void next_move(dungeon_t *d,
               character_t *c,
               pair_t goal_pos,
               pair_t next_pos);
void do_moves(dungeon_t *d);
void dir_nearest_wall(dungeon_t *d, character_t *c, pair_t dir);
uint32_t in_corner(dungeon_t *d, character_t *c);
uint32_t move_pc(dungeon_t *d, uint32_t dir);
uint32_t calculate_damage(dungeon_t *d, character_t *player);
uint32_t calculate_speed(dungeon_t *d, character_t *player);
uint32_t attack(dungeon_t *d, character_t *attacker, character_t *victim);
void move_character(dungeon_t *d, character_t *c, pair_t next);
void dijkstra(dungeon_t *d);
void dijkstra_tunnel(dungeon_t *d);

#endif
