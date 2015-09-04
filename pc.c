#include <stdlib.h>
#include <ncurses.h> /* for COLOR_WHITE */

#include "string.h"

#include "dungeon.h"
#include "pc.h"
#include "utils.h"
#include "move.h"
#include "io.h"
#include "object.h"
#include "descriptions.h"
#include "dice.h"

void pc_delete(pc_t *pc)
{
  if (pc) {
    free(pc);
  }
}

uint32_t pc_is_alive(dungeon_t *d)
{
  return d->pc.alive;
}

void place_pc(dungeon_t *d)
{
  d->pc.position[dim_y] = rand_range(d->rooms->position[dim_y],
                                     (d->rooms->position[dim_y] +
                                      d->rooms->size[dim_y] - 1));
  d->pc.position[dim_x] = rand_range(d->rooms->position[dim_x],
                                     (d->rooms->position[dim_x] +
                                      d->rooms->size[dim_x] - 1));
}

void config_pc(dungeon_t *d)
{
  d->pc.symbol = '@';
  d->pc.color = COLOR_WHITE;

  place_pc(d);
  
  d->pc.damage = new_dice(1, 2, 3);
  d->pc.hp = 500;
  d->pc.speed = PC_SPEED;
  d->pc.next_turn = 0;
  d->pc.alive = 1;
  d->pc.sequence_number = 0;
  d->pc.pc = malloc(sizeof (*d->pc.pc));
  strncpy(d->pc.pc->name, "Isabella Garcia-Shapiro", sizeof (d->pc.pc->name));
  strncpy(d->pc.pc->catch_phrase,
          "Whatcha doin'?", sizeof (d->pc.pc->name));
  d->pc.npc = NULL;

  d->character[d->pc.position[dim_y]][d->pc.position[dim_x]] = &d->pc;

  io_calculate_offset(d);

  dijkstra(d);
  dijkstra_tunnel(d);
}

uint32_t pc_next_pos(dungeon_t *d, pair_t dir)
{
  dir[dim_y] = dir[dim_x] = 0;

  /* Tunnel to the nearest dungeon corner, then move around in hopes *
   * of killing a couple of monsters before we die ourself.          */

  if (in_corner(d, &d->pc)) {
    /*
    dir[dim_x] = (mapxy(d->pc.position[dim_x] - 1,
                        d->pc.position[dim_y]) ==
                  ter_wall_immutable) ? 1 : -1;
    */
    dir[dim_y] = (mapxy(d->pc.position[dim_x],
                        d->pc.position[dim_y] - 1) ==
                  ter_wall_immutable) ? 1 : -1;
  } else {
    dir_nearest_wall(d, &d->pc, dir);
  }

  return 0;
}

uint32_t carry(dungeon_t *d)
{
  int i;
  d->cc = 1;
    for(i = 0; i < 10; i++) {
      if(!d->pc.pc->carry[i]) {
        d->cslot = i;
        d->pc.pc->carry[i] = d->object[d->pc.position[dim_y]][d->pc.position[dim_x]];
        d->object[d->pc.position[dim_y]][d->pc.position[dim_x]] = NULL;
        return 1;
      }
    }
    d->cslot = 11;
    return 0;
}

uint32_t drop(dungeon_t *d, char index)
{
  uint32_t spot;
  spot = index - '0';
  if(spot < 10) {
    if(d->pc.pc->carry[spot]) {
      d->object[d->pc.position[dim_y]][d->pc.position[dim_x]] = d->pc.pc->carry[spot];
      d->pc.pc->carry[spot] = NULL;
      return 1;
    }
  }
  return 0;
}

uint32_t expunge(dungeon_t *d, char index)
{
  uint32_t spot;
  spot = index - '0';
  if(spot < 10) {
    if(d->pc.pc->carry[spot]) {
      d->pc.pc->carry[spot] = NULL;
      return 1;
    }
  }
  return 0;
}

uint32_t wear(dungeon_t *d, char index)
{
  uint32_t spot;
  object_t *temp;
  spot = index - '0';
  if(spot < 10) {
    if(d->pc.pc->carry[spot]) {
      temp = d->pc.pc->carry[spot];
      switch(get_type(d->pc.pc->carry[spot])) {
        case objtype_WEAPON :
          if(d->pc.pc->equipment[0]) {
            d->pc.pc->carry[spot] = d->pc.pc->equipment[0];
          } else {
            d->pc.pc->carry[spot] = NULL;
          }
          d->pc.pc->equipment[0] = temp;
          break;
        case objtype_OFFHAND :
          if(d->pc.pc->equipment[1]) {
            d->pc.pc->carry[spot] = d->pc.pc->equipment[1];
          } else {
            d->pc.pc->carry[spot] = NULL;
          }
          d->pc.pc->equipment[1] = temp;
          break;
        case objtype_RANGED :
          if(d->pc.pc->equipment[2]) {
            d->pc.pc->carry[spot] = d->pc.pc->equipment[2];
          } else {
            d->pc.pc->carry[spot] = NULL;
          }
          d->pc.pc->equipment[2] = temp;
          break;
        case objtype_ARMOR :
          if(d->pc.pc->equipment[3]) {
            d->pc.pc->carry[spot] = d->pc.pc->equipment[3];
          } else {
            d->pc.pc->carry[spot] = NULL;
          }
          d->pc.pc->equipment[3] = temp;
          break;
        case objtype_HELMET :
          if(d->pc.pc->equipment[4]) {
            d->pc.pc->carry[spot] = d->pc.pc->equipment[4];
          } else {
            d->pc.pc->carry[spot] = NULL;
          }
          d->pc.pc->equipment[4] = temp;
          break;
        case objtype_CLOAK :
          if(d->pc.pc->equipment[5]) {
            d->pc.pc->carry[spot] = d->pc.pc->equipment[5];
          } else {
            d->pc.pc->carry[spot] = NULL;
          }
          d->pc.pc->equipment[5] = temp;
          break;
        case objtype_GLOVES :
          if(d->pc.pc->equipment[6]) {
            d->pc.pc->carry[spot] = d->pc.pc->equipment[6];
          } else {
            d->pc.pc->carry[spot] = NULL;
          }
          d->pc.pc->equipment[6] = temp;
          break;
        case objtype_BOOTS :
          if(d->pc.pc->equipment[7]) {
            d->pc.pc->carry[spot] = d->pc.pc->equipment[7];
          } else {
            d->pc.pc->carry[spot] = NULL;
          }
          d->pc.pc->equipment[7] = temp;
          break;
        case objtype_AMULET :
          if(d->pc.pc->equipment[8]) {
            d->pc.pc->carry[spot] = d->pc.pc->equipment[8];
          } else {
            d->pc.pc->carry[spot] = NULL;
          }
          d->pc.pc->equipment[8] = temp;
          break;
        case objtype_LIGHT :
          if(d->pc.pc->equipment[9]) {
            d->pc.pc->carry[spot] = d->pc.pc->equipment[9];
          } else {
            d->pc.pc->carry[spot] = NULL;
          }
          d->pc.pc->equipment[9] = temp;
          break;
          case objtype_RING :
          if(!d->pc.pc->equipment[10]) {
            d->pc.pc->carry[spot] = NULL;
            d->pc.pc->equipment[10] = temp;
            break;
          } else if(!d->pc.pc->equipment[11]) {
            d->pc.pc->carry[spot] = NULL;
            d->pc.pc->equipment[11] = temp;
            break;
          } else {
            d->pc.pc->carry[spot] = d->pc.pc->equipment[10];
            d->pc.pc->equipment[10] = temp;
            break;
          }
        default:
          return 1;
      }
    }
  }
  d->pc.speed = calculate_speed(d, &d->pc);
  return 0;
}

uint32_t takeoff(dungeon_t *d, char index)
{
  int i;
  uint32_t spot;
  object_t *temp;
  switch(index) {
    case 'a' :
      spot = 0;
      break;
    case 'b' :
      spot = 1;
      break;
    case 'c' :
      spot = 2;
      break;
    case 'd' :
      spot = 3;
      break;
    case 'e' :
      spot = 4;
      break;
    case 'f' :
      spot = 5;
      break;
    case 'g' :
      spot = 6;
      break;
    case 'h' :
      spot = 7;
      break;
    case 'i' :
      spot = 8;
      break;
    case 'j' :
      spot = 9;
      break;
    case 'k' :
      spot = 10;
      break;
    case 'l' :
      spot = 11;
      break;
    default :
      spot = 12;
      break;
  }
  if(spot < 12) {
    if(d->pc.pc->equipment[spot]) {
      temp = d->pc.pc->equipment[spot];
      d->pc.pc->equipment[spot] = NULL;
      for(i = 0; i < 10; i++) {
        if(!d->pc.pc->carry[i]) {
          d->pc.pc->carry[i] = temp;
          return 1;
        }
      }
      d->object[d->pc.position[dim_y]][d->pc.position[dim_x]] = temp;
      return 2;
    } 
  }
  d->pc.speed = calculate_speed(d, &d->pc);
  return 0;
}
