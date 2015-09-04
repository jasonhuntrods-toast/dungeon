#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>

#include "dungeon.h"
#include "character.h"
#include "utils.h"
#include "pc.h"
#include "npc.h"
#include "move.h"
#include "io.h"
#include "descriptions.h"

static void usage(char *name)
{
  fprintf(stderr,
          "Usage: %s [-r|--rand <seed>] [-h|--huge]\n"
          "%*c[-n|--nummon <number of monsters>]\n",
          name, (int) strlen(name) + 8, ' ');

  exit(-1);
}

int main(int argc, char *argv[])
{
  dungeon_t d;
  time_t seed;
  uint32_t nummon;
  struct timeval tv;
  uint32_t i;
  uint32_t do_seed;
  uint32_t long_arg;
  uint32_t do_huge;

  memset(&d, 0, sizeof (d));

  /* Default behavior: Seed with the time, generate a new dungeon, *
   * and don't write to disk.                                      */
  do_seed = 1;
  do_huge = 0;
  nummon = 10;

  /* The project spec requires '--load' and '--store'.  It's common *
   * to have short and long forms of most switches (assuming you    *
   * don't run out of letters).  For now, we've got plenty.  Long   *
   * forms use whole words and take two dashes.  Short forms use an *
   * abbreviation after a single dash.  We'll add '--rand' (to      *
   * specify a random seed), which will take an argument of its     *
   * own, and we'll add short forms for all three commands, '-l',   *
   * '-s', and '-r', respectively.  We're also going to allow an    *
   * optional argument to load to allow us to load non-default save *
   * files.  No means to store to non-default locations, however.   */
  if (argc > 1) {
    for (i = 1, long_arg = 0; i < argc; i++, long_arg = 0) {
      if (argv[i][0] == '-') { /* All switches start with a dash */
        if (argv[i][1] == '-') {
          argv[i]++;    /* Make the argument have a single dash so we can */
          long_arg = 1; /* handle long and short args at the same place.  */
        }
        switch (argv[i][1]) {
        case 'r':
          if ((!long_arg && argv[i][2]) ||
              (long_arg && strcmp(argv[i], "-rand")) ||
              argc < ++i + 1 /* No more arguments */ ||
              !sscanf(argv[i], "%lu", &seed) /* Argument is not an integer */) {
            usage(argv[0]);
          }
          do_seed = 0;
          break;
        case 'n':
          if ((!long_arg && argv[i][2]) ||
              (long_arg && strcmp(argv[i], "-nummon")) ||
              argc < ++i + 1 /* No more arguments */ ||
              !sscanf(argv[i], "%u", &nummon) /* Argument is not an int */) {
            usage(argv[0]);
          }
          break;
        case 'h':
          if ((!long_arg && argv[i][2]) ||
              (long_arg && strcmp(argv[i], "-huge"))) {
            usage(argv[0]);
          }
          do_huge = 1;
          break;
        default:
          usage(argv[0]);
        }
      } else { /* No dash */
        usage(argv[0]);
      }
    }
  }

  if (do_seed) {
    /* Allows me to generate more than one dungeon *
     * per second, as opposed to time().           */
    gettimeofday(&tv, NULL);
    seed = (tv.tv_usec ^ (tv.tv_sec << 20)) & 0xffffffff;
  }

  if (do_huge) {
    d.render_whole_dungeon = 1;
  } else {
    d.render_whole_dungeon = 0;
  }

  printf("Seed is %ld.\n", seed);
  srand(seed);

  parse_descriptions(&d);

  io_init_terminal();
  init_dungeon(&d);

  if (read_dungeon(&d)) {
    gen_dungeon(&d);
    config_pc(&d);
    gen_monsters(&d, nummon, 0);
    gen_objects(&d, 10);
  }

  io_display(&d);
  while (pc_is_alive(&d) && dungeon_has_npcs(&d) &&
         !(d.save_and_exit || d.quit_no_save)) {
    do_moves(&d);
    io_display(&d);
    if (!pc_is_alive(&d)) {
      break;
    }
    mvprintw(23, 0, "%s - HP: %d ", d.pc.pc->name, d.pc.hp);
    mvprintw(23, 67, " MONSTERS: %d ", d.num_monsters);
    if(d.gg > 0) {
      io_attack(&d, d.aa, d.vv, d.kk);
    }
    if(d.cc > 0) {
      if(d.cslot > 10) {
        mvprintw(22, 0, "CARRY SLOTS FULL");
      } else {
        mvprintw(22, 0, "PICKED UP A %s ", get_name(d.pc.pc->carry[d.cslot]));
      }
    } 
    d.cc = 0;
    d.gg = 0; d.kk = 1;
    io_handle_input(&d);
  }
  io_display(&d);

  io_reset_terminal();

  if (!(d.save_and_exit || d.quit_no_save)) {
    sleep(2);
    if (pc_is_alive(&d)) {
      printf("%s says, \"%s\"\n", d.pc.pc->name, d.pc.pc->catch_phrase);
    } else {
      printf("The monster hordes growl menacingly.\n");
    }
    unlink_dungeon();
  } else if (d.save_and_exit) {
    write_dungeon(&d);
  } else /* d.quit_no_save */ {
    unlink_dungeon();
  }

  pc_delete(d.pc.pc);
  destroy_descriptions(&d);
  delete_dungeon(&d);

  return 0;
}
