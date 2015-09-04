#ifndef DUNGEON_H
# define DUNGEON_H

# ifdef __cplusplus
extern "C" {
# endif

# include <stdint.h>
# include <stdio.h>

# include "heap.h"
# include "dims.h"
# include "character.h"
# include "object.h"

#define DUNGEON_X              160
#define DUNGEON_Y              96
#define MIN_ROOMS              12
#define MAX_ROOMS              24
#define ROOM_MIN_X             10
#define ROOM_MIN_Y             8
#define ROOM_SEPARATION        5
#define MAX_PLACEMENT_ATTEMPTS 1000
#define SAVE_DIR               ".rlg229"
#define DUNGEON_SAVE_FILE      "dungeon"
#define MONSTER_DESC_FILE      "monster_desc.txt"
#define OBJECT_DESC_FILE      "object_desc.txt"
#define DUNGEON_SAVE_SEMANTIC  "RLG229"
#define DUNGEON_SAVE_VERSION   1U
#define VISUAL_RANGE           30
#define PC_SPEED               10

#define mappair(pair) (d->map[pair[dim_y]][pair[dim_x]])
#define mapxy(x, y) (d->map[y][x])
#define hardnesspair(pair) (d->hardness[pair[dim_y]][pair[dim_x]])
#define hardnessxy(x, y) (d->hardness[y][x])

typedef enum __attribute__ ((__packed__)) terrain_type {
  ter_debug,
  ter_wall,
  ter_wall_no_room,
  ter_wall_no_floor,
  ter_wall_immutable,
  ter_floor,
  ter_floor_room,
  ter_floor_hall,
  ter_floor_tunnel,
  ter_stairs,
  ter_stairs_up,
  ter_stairs_down
} terrain_type_t;

typedef struct room {
  pair_t position;
  pair_t size;
  uint32_t connected;
  uint8_t exists;
} room_t;

typedef struct npc npc_t;
typedef struct pc pc_t;
typedef struct character character_t;
typedef void *monster_description_t;
typedef void *object_description_t;

typedef struct dungeon {
  uint32_t render_whole_dungeon;
  uint32_t num_rooms;
  room_t rooms[MAX_ROOMS];
  terrain_type_t map[DUNGEON_Y][DUNGEON_X];
  /* Since hardness is usually not used, it would be expensive to pull it *
   * into cache every time we need a map cell, so we store it in a        *
   * parallel array, rather than using a structure to represent the       *
   * cells.  We may want a cell structure later, but from a performanace  *
   * perspective, it would be a bad idea to ever have the map be part of  *
   * that structure.  Pathfinding will require efficient use of the map,  *
   * and pulling in unnecessary data with each map cell would add a lot   *
   * of overhead to the memory system.                                    */
  uint8_t hardness[DUNGEON_Y][DUNGEON_X];
  uint8_t seen[DUNGEON_Y][DUNGEON_X];
  uint16_t num_monsters;
  uint16_t num_objects;
  uint32_t character_sequence_number;
  character_t *character[DUNGEON_Y][DUNGEON_X];
  object_t *object[DUNGEON_Y][DUNGEON_X];
  uint8_t pc_distance[DUNGEON_Y][DUNGEON_X];
  /* Need 16 bits because these can go to 4 each */
  uint16_t pc_tunnel[DUNGEON_Y][DUNGEON_X];
  /* pc can be statically allocated; however, that lead to special cases *
   * for the deallocator when cleaning up our turn queue.  Making it     *
   * dynamic simplifies the logic at the expense of one level of         *
   * indirection.                                                        */
  /* Changed my mind.  Make it an instance because there are special     *
   * cases for it no matter what.                                        */
  character_t pc;
  heap_t next_turn;
  pair_t io_offset;
  uint32_t save_and_exit;
  uint32_t quit_no_save;
  monster_description_t monster_descriptions;
  object_description_t object_descriptions;
  character_t *aa;
  character_t *vv;
  uint32_t gg;
  uint32_t kk;
  uint32_t cc;
  uint32_t cslot;
} dungeon_t;

int read_dungeon(dungeon_t *dungeon);
int gen_dungeon(dungeon_t *dungeon);
void render_dungeon(dungeon_t *dungeon);
int write_dungeon(dungeon_t *dungeon);
void init_dungeon(dungeon_t *d);
void delete_dungeon(dungeon_t *d);
void new_dungeon(dungeon_t *d);
void unlink_dungeon(void);

# ifdef __cplusplus
}
# endif

#endif
