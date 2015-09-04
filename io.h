#ifndef IO_H
# define IO_H

# include "dungeon.h"
# include "character.h"
# include "move.h"

void io_init_terminal(void);
void io_reset_terminal(void);
void io_calculate_offset(dungeon_t *d);
void io_display(dungeon_t *d);
void io_update_offset(dungeon_t *d);
void io_attack(dungeon_t *d, character_t *a, character_t *v, uint32_t kill);
void io_handle_input(dungeon_t *d);

#endif
