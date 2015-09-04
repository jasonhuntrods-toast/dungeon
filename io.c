#include <unistd.h>
#include <ncurses.h>

#include "io.h"
#include "move.h"
#include "object.h"
#include "pc.h"
#include "npc.h"
#include "character.h"
#include "descriptions.h"

/* We're going to be working in a standard 80x24 terminal, and, except when *
 * the PC is near the edges, we're going to restrict it to the centermost   *
 * 60x18, defining the center to be at (40,12).  So the PC can be in the    *
 * rectangle defined by the ordered pairs (11, 4) and (70, 21).  When the   *
 * PC leaves this zone, if already on the corresponding edge of the map,    *
 * nothing happens; otherwise, the map shifts by +-40 in x or +- 12 in y,   *
 * such that the PC is in the center 60x18 region.  Look mode will also     *
 * shift by 40x12 blocks.  Thus, the set of all possible dungeon positions  *
 * to correspond with the upper left corner of the dungeon are:             *
 *                                                                          *
 *   ( 0,  0), (40,  0), (80,  0)                                           *
 *   ( 0, 12), (40, 12), (80, 12)                                           *
 *   ( 0, 24), (40, 24), (80, 24)                                           *
 *   ( 0, 36), (40, 36), (80, 36)                                           *
 *   ( 0, 48), (40, 48), (80, 48)                                           *
 *   ( 0, 60), (40, 60), (80, 60)                                           *
 *   ( 0, 72), (40, 72), (80, 72)                                           */

void io_init_terminal(void)
{
  initscr();
  raw();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);
  start_color();
  init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
  init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
  init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
  init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
  init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
  init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
}

void io_reset_terminal(void)
{
  endwin();
}

void io_calculate_offset(dungeon_t *d)
{
  d->io_offset[dim_x] = ((d->pc.position[dim_x] - 20) / 40) * 40;
  if (d->io_offset[dim_x] < 0) {
    d->io_offset[dim_x] = 0;
  }
  if (d->io_offset[dim_x] > 80) {
    d->io_offset[dim_x] = 80;
  }
  d->io_offset[dim_y] = ((d->pc.position[dim_y] - 6) / 12) * 12;
  if (d->io_offset[dim_y] < 0) {
    d->io_offset[dim_y] = 0;
  }
  if (d->io_offset[dim_y] > 72) {
    d->io_offset[dim_y] = 72;
  }

#if 0
  uint32_t y;
  uint32_t min_diff, diff;

  min_diff = diff = abs(d->pc.position[dim_x] - 40);
  d->io_offset[dim_x] = 0;
  if ((diff = abs(d->pc.position[dim_x] - 80)) < min_diff) {
    min_diff = diff;
    d->io_offset[dim_x] = 40;
  }
  if ((diff = abs(d->pc.position[dim_x] - 120)) < min_diff) {
    min_diff = diff;
    d->io_offset[dim_x] = 80;
  }

  /* A lot more y values to deal with, so use a loop */

  for (min_diff = 96, d->io_offset[dim_y] = 0, y = 12; y <= 72; y += 12) {
    if ((diff = abs(d->pc.position[dim_y] - (y + 12))) < min_diff) {
      min_diff = diff;
      d->io_offset[dim_y] = y;
    }
  }
#endif
}

void io_update_offset(dungeon_t *d)
{
  int32_t x, y;

  x = (40 + d->io_offset[dim_x]) - d->pc.position[dim_x];
  y = (12 + d->io_offset[dim_y]) - d->pc.position[dim_y];

  if (x >= 30 && d->io_offset[dim_x]) {
    d->io_offset[dim_x] -= 40;
  }
  if (x <= -30 && d->io_offset[dim_x] != 80) {
    d->io_offset[dim_x] += 40;
  }
  if (y >= 8 && d->io_offset[dim_y]) {
    d->io_offset[dim_y] -= 12;
  }
  if (y <= -8 && d->io_offset[dim_y] != 72) {
    d->io_offset[dim_y] += 12;
  }
}

void io_display_tunnel(dungeon_t *d)
{
  uint32_t y, x;
  clear();
  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      mvprintw(y, x*2, "%02hhx",
               d->pc_tunnel[y][x] <= 255 ? d->pc_tunnel[y][x] : 255);
    }
  }
  refresh();
}

void io_display_distance(dungeon_t *d)
{
  uint32_t y, x;
  clear();
  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      mvprintw(y, x*2, "%02hhx", d->pc_distance[y][x]);
    }
  }
  refresh();
}

void io_display_huge(dungeon_t *d)
{
  uint32_t y, x;

  clear();
  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      if (d->character[y][x]) {
        attron(COLOR_PAIR(d->character[y][x]->color));
        mvaddch(y, x, d->character[y][x]->symbol);
        attroff(COLOR_PAIR(d->character[y][x]->color));
      } else if (d->object[y][x]) {
        attron(COLOR_PAIR(get_color(d->object[y][x])));
        mvaddch(y, x, get_symbol(d->object[y][x]));
        attroff(COLOR_PAIR(get_color(d->object[y][x])));
      } else {
        switch (mapxy(x, y)) {
        case ter_wall:
        case ter_wall_no_room:
        case ter_wall_no_floor:
        case ter_wall_immutable:
          mvaddch(y, x, '#');
          break;
        case ter_floor:
        case ter_floor_room:
        case ter_floor_hall:
        case ter_floor_tunnel:
          mvaddch(y, x, '.');
          break;
        case ter_debug:
          mvaddch(y, x, '*');
          break;
        case ter_stairs_up:
          mvaddch(y, x, '<');
          break;
        case ter_stairs_down:
          mvaddch(y, x, '>');
          break;
        default:
 /* Use zero as an error symbol, since it stands out somewhat, and it's *
  * not otherwise used.                                                 */
          mvaddch(y, x, '0');
        }
      }
    }
  }
  refresh();
}

void io_display(dungeon_t *d)
{
  uint32_t y, x;

  if (d->render_whole_dungeon) {
    io_display_huge(d);
    return;
  }

  clear();
  
  for (y = 0; y < 24; y++) {
    for (x = 0; x < 80; x++) {
      if (d->character[d->io_offset[dim_y] + y]
                      [d->io_offset[dim_x] + x]) {
        attron(COLOR_PAIR(d->character[d->io_offset[dim_y] + y]
                                      [d->io_offset[dim_x] + x]->color));
        mvaddch(y, x, d->character[d->io_offset[dim_y] + y]
                                  [d->io_offset[dim_x] + x]->symbol);
        attroff(COLOR_PAIR(d->character[d->io_offset[dim_y] + y]
                                       [d->io_offset[dim_x] + x]->color));
      } else if (d->object[d->io_offset[dim_y] + y][d->io_offset[dim_x] + x]) {
        attron(COLOR_PAIR(get_color(d->object[d->io_offset[dim_y] + y]
                                             [d->io_offset[dim_x] + x])));
        mvaddch(y, x, get_symbol(d->object[d->io_offset[dim_y] + y]
                                          [d->io_offset[dim_x] + x]));
        attroff(COLOR_PAIR(get_color(d->object[d->io_offset[dim_y] + y]
                                              [d->io_offset[dim_x] + x])));
      } else {
        switch (mapxy(d->io_offset[dim_x] + x,
                      d->io_offset[dim_y] + y)) {
        case ter_wall:
        case ter_wall_no_room:
        case ter_wall_no_floor:
        case ter_wall_immutable:
          mvaddch(y, x, '#');
          break;
        case ter_floor:
        case ter_floor_room:
        case ter_floor_hall:
        case ter_floor_tunnel:
          mvaddch(y, x, '.');
          break;
        case ter_debug:
          mvaddch(y, x, '*');
          break;
        case ter_stairs_up:
          mvaddch(y, x, '<');
          break;
        case ter_stairs_down:
          mvaddch(y, x, '>');
          break;
        default:
 /* Use zero as an error symbol, since it stands out somewhat, and it's *
  * not otherwise used.                                                 */
          mvaddch(y, x, '0');
        }
      }
    }
  }
  mvprintw(0, 0, "PC position is (%3d,%2d); offset is (%3d,%2d).",
           d->pc.position[dim_x], d->pc.position[dim_y],
           d->io_offset[dim_x], d->io_offset[dim_y]);


  refresh();
}

void io_look_mode(dungeon_t *d)
{
  int32_t key;

  do {
    if ((key = getch()) == 27 /* ESC */) {
      io_calculate_offset(d);
      io_display(d);
      return;
    }
    
    switch (key) {
    case '1':
    case 'b':
    case KEY_END:
    case '2':
    case 'j':
    case KEY_DOWN:
    case '3':
    case 'n':
    case KEY_NPAGE:
      if (d->io_offset[dim_y] != 72) {
        d->io_offset[dim_y] += 12;
      }
      break;
    case '4':
    case 'h':
    case KEY_LEFT:
    case '5':
    case ' ':
    case KEY_B2:
    case '6':
    case 'l':
    case KEY_RIGHT:
      break;
    case '7':
    case 'y':
    case KEY_HOME:
    case '8':
    case 'k':
    case KEY_UP:
    case '9':
    case 'u':
    case KEY_PPAGE:
      if (d->io_offset[dim_y]) {
        d->io_offset[dim_y] -= 12;
      }
      break;
    }
    switch (key) {
    case '1':
    case 'b':
    case KEY_END:
    case '4':
    case 'h':
    case KEY_LEFT:
    case '7':
    case 'y':
    case KEY_HOME:
      if (d->io_offset[dim_x]) {
        d->io_offset[dim_x] -= 40;
      }
      break;
    case '2':
    case 'j':
    case KEY_DOWN:
    case '5':
    case ' ':
    case KEY_B2:
    case '8':
    case 'k':
    case KEY_UP:
      break;
    case '3':
    case 'n':
    case KEY_NPAGE:
    case '6':
    case 'l':
    case KEY_RIGHT:
    case '9':
    case 'u':
    case KEY_PPAGE:
      if (d->io_offset[dim_x] != 80) {
        d->io_offset[dim_x] += 40;
      }
      break;
    }
    io_display(d);
  } while (1);
}

void io_attack(dungeon_t *d, character_t *a, character_t *v, uint32_t kill) {
  if(a->pc) {
    if(kill > 0) {
      mvprintw(22, 0, "YOU KILLED A %s ", v->npc->name);
    } else {
      mvprintw(21, 0, "YOU ATTACKED A %s ", v->npc->name);
      mvprintw(22, 0, "%s - HP: %d ", v->npc->name, v->hp);
    }
  } else {
    mvprintw(21, 0, "YOU WERE ATTACKED BY A %s ", a->npc->name);
    mvprintw(22, 0, "%s - HP: %d ", a->npc->name, a->hp);
  }
}

void io_handle_input(dungeon_t *d)
{
  uint32_t message;
  uint32_t yspot; 
  message = 0;
  uint32_t fail_code;
  const char* slots[12];
  slots[0] = "WEAPON"; 
  slots[1] = "OFFHAND"; 
  slots[2] = "RANGED"; 
  slots[3] = "ARMOR"; 
  slots[4] = "HELMET"; 
  slots[5] = "CLOAK"; 
  slots[6] = "GLOVES"; 
  slots[7] = "BOOTS"; 
  slots[8] = "AMULET"; 
  slots[9] = "LIGHT"; 
  slots[10] = "RING 1"; 
  slots[11] = "RING 2"; 
  int key;

  do {
    switch (key = getch()) {
    case '7':
    case 'y':
    case KEY_HOME:
      fail_code = move_pc(d, 7);
      break;
    case '8':
    case 'k':
    case KEY_UP:
      fail_code = move_pc(d, 8);
      break;
    case '9':
    case 'u':
    case KEY_PPAGE:
      fail_code = move_pc(d, 9);
      break;
    case '6':
    case 'l':
    case KEY_RIGHT:
      fail_code = move_pc(d, 6);
      break;
    case '3':
    case 'n':
    case KEY_NPAGE:
      fail_code = move_pc(d, 3);
      break;
    case '2':
    case 'j':
    case KEY_DOWN:
      fail_code = move_pc(d, 2);
      break;
    case '1':
    case 'b':
    case KEY_END:
      fail_code = move_pc(d, 1);
      break;
    case '4':
    case 'h':
    case KEY_LEFT:
      fail_code = move_pc(d, 4);
      break;
    case '5':
    case ' ':
    case KEY_B2:
      fail_code = 0;
      break;
    case '>':
      fail_code = move_pc(d, '>');
      break;
    case '<':
      fail_code = move_pc(d, '<');
      break;
    case 'L':
      io_look_mode(d);
      fail_code = 0;
      break;
    case 'S':
      d->save_and_exit = 1;
      d->pc.next_turn -= (1000 / d->pc.speed);
      fail_code = 0;
      break;
    case 'Q':
      /* Extra command, not in the spec.  Quit without saving.          */
      d->quit_no_save = 1;
      fail_code = 0;
      break;
    case 'H':
      /* Extra command, not in the spec.  H is for Huge: draw the whole *
       * dungeon, the pre-curses way.  Doesn't use a player turn.       */
      io_display_huge(d);
      fail_code = 1;
      break;
    case 'T':
      /* New command.  Display the distances for tunnelers.  Displays   *
       * in hex with two characters per cell.                           */
      io_display_tunnel(d);
      fail_code = 1;
      break;
    case 'D':
      /* New command.  Display the distances for non-tunnelers.         *
       *  Displays in hex with two characters per cell.                 */
      io_display_distance(d);
      fail_code = 1;
      break;
    case 's':
      /* New command.  Return to normal display after displaying some   *
       * special screen.                                                */
      io_display(d);
      fail_code = 1;
      break;
    case 'd':
      for(yspot = 0; yspot < 10; yspot++) {
        if(d->pc.pc->carry[yspot]) {
          attron(COLOR_PAIR(get_color(d->pc.pc->carry[yspot])));
          mvprintw(yspot + 12, 0, "SLOT %d: %c -> %s ", yspot, get_symbol(d->pc.pc->carry[yspot]), get_name(d->pc.pc->carry[yspot]));
          attroff(COLOR_PAIR(get_color(d->pc.pc->carry[yspot])));
        } else {
          mvprintw(yspot + 12, 0, "SLOT %d: EMPTY ", yspot);
        }
      }
      message = drop(d, getch());
      if(message == 1) {
        mvprintw(22, 0, "DROPPED ITEM ");
      } else {
        mvprintw(22, 0, "NOTHING TO DROP ");
      }
      break;
    case 'x':
      for(yspot = 0; yspot < 10; yspot++) {
        if(d->pc.pc->carry[yspot]) {
          attron(COLOR_PAIR(get_color(d->pc.pc->carry[yspot])));
          mvprintw(yspot + 12, 0, "SLOT %d: %c -> %s ", yspot, get_symbol(d->pc.pc->carry[yspot]), get_name(d->pc.pc->carry[yspot]));
          attroff(COLOR_PAIR(get_color(d->pc.pc->carry[yspot])));
        } else {
          mvprintw(yspot + 12, 0, "SLOT %d: EMPTY ", yspot);
        }
      }
      message = expunge(d, getch());
      if(message == 1) {
        mvprintw(22, 0, "EXPUNGED ITEM ");
      } else {
        mvprintw(22, 0, "NOTHING TO EXPUNGE ");
      }
      break;
    case 'w':
      for(yspot = 0; yspot < 10; yspot++) {
        if(d->pc.pc->carry[yspot]) {
          attron(COLOR_PAIR(get_color(d->pc.pc->carry[yspot])));
          mvprintw(yspot + 12, 0, "SLOT %d: %c -> %s ", yspot, get_symbol(d->pc.pc->carry[yspot]), get_name(d->pc.pc->carry[yspot]));
          attroff(COLOR_PAIR(get_color(d->pc.pc->carry[yspot])));
        } else {
          mvprintw(yspot + 12, 0, "SLOT %d: EMPTY ", yspot);
        }
      }
      wear(d, getch());
      mvprintw(22, 0, "EQUIPPED ITEM ");
      break;
    case 't':
      for(yspot = 0; yspot < 12; yspot++) {
        if(d->pc.pc->equipment[yspot]) {
          attron(COLOR_PAIR(get_color(d->pc.pc->equipment[yspot])));
          mvprintw(yspot + 10, 0, "SLOT[%c] %s: %c -> %s ", 'A' + yspot, slots[yspot], get_symbol(d->pc.pc->equipment[yspot]), get_name(d->pc.pc->equipment[yspot]));
          attroff(COLOR_PAIR(get_color(d->pc.pc->equipment[yspot])));
        } else {
          mvprintw(yspot + 10, 0, "SLOT[%c] %s: EMPTY ", 'A' + yspot, slots[yspot]);
        }
      }
      message = takeoff(d, getch());
      if(message == 1) {
        mvprintw(22, 0, "TOOK OFF ITEM ");
      } else if(message == 2) {
        mvprintw(22, 0, "TOOK OFF ITEM AND DROPPED TO FLOOR (CARRY IS FULL) ");
      } else {
        mvprintw(22, 0, "NOTHING TO TAKE OFF ");
      }
      break;
    default:
      /* Also not in the spec.  It's not always easy to figure out what *
       * key code corresponds with a given keystroke.  Print out any    *
       * unhandled key here.  Not only does it give a visual error      *
       * indicator, but it also gives an integer value that can be used *
       * for that key in this (or other) switch statements.  Printed in *
       * octal, with the leading zero, because ncurses.h lists codes in *
       * octal, thus allowing us to do reverse lookups.  If a key has a *
       * name defined in the header, you can use the name here, else    *
       * you can directly use the octal value.                          */
      mvprintw(0, 0, "Unbound key: %#o ", key);
      fail_code = 1;
    }
  } while (fail_code);
}
