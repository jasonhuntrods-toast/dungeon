#include <stdlib.h>

#include "utils.h"
#include "npc.h"
#include "dungeon.h"
#include "character.h"
#include "descriptions.h"
#include "move.h"

void npc_delete(npc_t *n)
{
  if (n) {
    free(n);
  }
}

void gen_monsters(dungeon_t *d, uint32_t nummon, uint32_t game_turn)
{
  uint32_t i;

  d->num_monsters = nummon;
  for (i = 0; i < nummon; i++) {
    /* Does all the necessary work inside, so ignoring the return value. */
    generate_monster(d);

    /*
    m = malloc(sizeof (*m));

    m->symbol = 'A' + rand_range(0, 25) + ((rand() & 1) ? 32 : 0);
    room = rand_range(1, d->num_rooms - 1);
    do {
      p[dim_y] = rand_range(d->rooms[room].position[dim_y],
                            (d->rooms[room].position[dim_y] +
                             d->rooms[room].size[dim_y] - 1));
      p[dim_x] = rand_range(d->rooms[room].position[dim_x],
                            (d->rooms[room].position[dim_x] +
                             d->rooms[room].size[dim_x] - 1));
    } while (d->character[p[dim_y]][p[dim_x]]);
    m->position[dim_y] = p[dim_y];
    m->position[dim_x] = p[dim_x];
    d->character[p[dim_y]][p[dim_x]] = m;
    m->speed = rand_range(5, 20);
    m->next_turn = game_turn;
    m->alive = 1;
    m->sequence_number = ++d->character_sequence_number;
    m->pc = NULL;
    m->npc = malloc(sizeof (*m->npc));
    m->npc->characteristics = rand();
    m->npc->have_seen_pc = 0;

    d->character[p[dim_y]][p[dim_x]] = m;

    heap_insert(&d->next_turn, m);
    */
  }
}

void npc_next_pos_rand(dungeon_t *d, character_t *c, pair_t next)
{
  pair_t n;
  union {
    uint32_t i;
    uint8_t a[4];
  } r;

  do {
    n[dim_y] = next[dim_y];
    n[dim_x] = next[dim_x];
    r.i = rand();
    if (r.a[0] > 85 /* 255 / 3 */) {
      if (r.a[0] & 1) {
        n[dim_y]--;
      } else {
        n[dim_y]++;
      }
    }
    if (r.a[1] > 85 /* 255 / 3 */) {
      if (r.a[1] & 1) {
        n[dim_x]--;
      } else {
        n[dim_x]++;
      }
    }
  } while (mappair(n) < ter_floor);

  next[dim_y] = n[dim_y];
  next[dim_x] = n[dim_x];
}

void npc_next_pos_line_of_sight(dungeon_t *d, character_t *c, pair_t next)
{
  pair_t dir;

  dir[dim_y] = d->pc.position[dim_y] - c->position[dim_y];
  dir[dim_x] = d->pc.position[dim_x] - c->position[dim_x];
  if (dir[dim_y]) {
    dir[dim_y] /= abs(dir[dim_y]);
  }
  if (dir[dim_x]) {
    dir[dim_x] /= abs(dir[dim_x]);
  }

  if ((c->npc->characteristics & NPC_TELEPATH) &&
      (c->npc->characteristics & NPC_PASS_WALL)) {
    next[dim_x] += dir[dim_x];
    next[dim_y] += dir[dim_y];
  } else {
    if (mapxy(next[dim_x] + dir[dim_x],
              next[dim_y] + dir[dim_y]) >= ter_floor) {
      next[dim_x] += dir[dim_x];
      next[dim_y] += dir[dim_y];
    } else if (mapxy(next[dim_x] + dir[dim_x], next[dim_y]) >= ter_floor) {
      next[dim_x] += dir[dim_x];
    } else if (mapxy(next[dim_x], next[dim_y] + dir[dim_y]) >= ter_floor) {
      next[dim_y] += dir[dim_y];
    }
  }
}

void npc_next_pos_gradient(dungeon_t *d, character_t *c, pair_t next)
{
  pair_t min_next;
  uint16_t min_cost;
  if (c->npc->characteristics & NPC_TUNNEL) {
    min_cost = d->pc_tunnel[next[dim_y] - 1][next[dim_x]];
    min_next[dim_x] = next[dim_x];
    min_next[dim_y] = next[dim_y] - 1;
    if (d->pc_tunnel[next[dim_y] + 1][next[dim_x]    ] < min_cost) {
      min_cost = d->pc_tunnel[next[dim_y] + 1][next[dim_x]];
      min_next[dim_x] = next[dim_x];
      min_next[dim_y] = next[dim_y] + 1;
    }
    if (d->pc_tunnel[next[dim_y]    ][next[dim_x] + 1] < min_cost) {
      min_cost = d->pc_tunnel[next[dim_y]][next[dim_x] + 1];
      min_next[dim_x] = next[dim_x] + 1;
      min_next[dim_y] = next[dim_y];
    }
    if (d->pc_tunnel[next[dim_y]    ][next[dim_x] - 1] < min_cost) {
      min_cost = d->pc_tunnel[next[dim_y]][next[dim_x] - 1];
      min_next[dim_x] = next[dim_x] - 1;
      min_next[dim_y] = next[dim_y];
    }
    if (d->pc_tunnel[next[dim_y] - 1][next[dim_x] + 1] < min_cost) {
      min_cost = d->pc_tunnel[next[dim_y] - 1][next[dim_x] + 1];
      min_next[dim_x] = next[dim_x] + 1;
      min_next[dim_y] = next[dim_y] - 1;
    }
    if (d->pc_tunnel[next[dim_y] + 1][next[dim_x] + 1] < min_cost) {
      min_cost = d->pc_tunnel[next[dim_y] + 1][next[dim_x] + 1];
      min_next[dim_x] = next[dim_x] + 1;
      min_next[dim_y] = next[dim_y] + 1;
    }
    if (d->pc_tunnel[next[dim_y] - 1][next[dim_x] - 1] < min_cost) {
      min_cost = d->pc_tunnel[next[dim_y] - 1][next[dim_x] - 1];
      min_next[dim_x] = next[dim_x] - 1;
      min_next[dim_y] = next[dim_y] - 1;
    }
    if (d->pc_tunnel[next[dim_y] + 1][next[dim_x] - 1] < min_cost) {
      min_cost = d->pc_tunnel[next[dim_y] + 1][next[dim_x] - 1];
      min_next[dim_x] = next[dim_x] - 1;
      min_next[dim_y] = next[dim_y] + 1;
    }
    if (hardnesspair(min_next) <= 60) {
      hardnesspair(min_next) = 0;
      mappair(min_next) = ter_floor_tunnel;
      next[dim_x] = min_next[dim_x];
      next[dim_y] = min_next[dim_y];

      /* Update distance maps because map has changed. */
      dijkstra(d);
      dijkstra_tunnel(d);
    } else {
      hardnesspair(min_next) -= 60;
    }
  } else {
    /* Make monsters prefer cardinal directions */
    if (d->pc_distance[next[dim_y] - 1][next[dim_x]    ] <
        d->pc_distance[next[dim_y]][next[dim_x]]) {
      next[dim_y]--;
      return;
    }
    if (d->pc_distance[next[dim_y] + 1][next[dim_x]    ] <
        d->pc_distance[next[dim_y]][next[dim_x]]) {
      next[dim_y]++;
      return;
    }
    if (d->pc_distance[next[dim_y]    ][next[dim_x] + 1] <
        d->pc_distance[next[dim_y]][next[dim_x]]) {
      next[dim_x]++;
      return;
    }
    if (d->pc_distance[next[dim_y]    ][next[dim_x] - 1] <
        d->pc_distance[next[dim_y]][next[dim_x]]) {
      next[dim_x]--;
      return;
    }
    if (d->pc_distance[next[dim_y] - 1][next[dim_x] + 1] <
        d->pc_distance[next[dim_y]][next[dim_x]]) {
      next[dim_y]--;
      next[dim_x]++;
      return;
    }
    if (d->pc_distance[next[dim_y] + 1][next[dim_x] + 1] <
        d->pc_distance[next[dim_y]][next[dim_x]]) {
      next[dim_y]++;
      next[dim_x]++;
      return;
    }
    if (d->pc_distance[next[dim_y] - 1][next[dim_x] - 1] <
        d->pc_distance[next[dim_y]][next[dim_x]]) {
      next[dim_y]--;
      next[dim_x]--;
      return;
    }
    if (d->pc_distance[next[dim_y] + 1][next[dim_x] - 1] <
        d->pc_distance[next[dim_y]][next[dim_x]]) {
      next[dim_y]++;
      next[dim_x]--;
      return;
    }
  }
}

void npc_next_pos(dungeon_t *d, character_t *c, pair_t next)
{
  next[dim_y] = c->position[dim_y];
  next[dim_x] = c->position[dim_x];

  /* There are now (1.09) 16 possibilities.  Since there are cases *
   * that overlap, a switch may not be the best option anymore,    *
   * but I'm not sure, so I'll stick with it, at least for now.    */
  switch (c->npc->characteristics &
          (NPC_SMART | NPC_TELEPATH | NPC_TUNNEL | NPC_PASS_WALL)) {
  case 0:
  case NPC_TUNNEL:
  case NPC_PASS_WALL:
  case NPC_PASS_WALL | NPC_TUNNEL:
    if (can_see(d, c, &d->pc)) {
      c->npc->pc_last_known_position[dim_y] = d->pc.position[dim_y];
      c->npc->pc_last_known_position[dim_x] = d->pc.position[dim_x];
      npc_next_pos_line_of_sight(d, c, next);
    } else {
      npc_next_pos_rand(d, c, next);
    }
    break;
  case NPC_SMART:
  case NPC_SMART | NPC_TUNNEL:
  case NPC_PASS_WALL | NPC_SMART:
  case NPC_PASS_WALL | NPC_SMART | NPC_TUNNEL:
    if (can_see(d, c, &d->pc)) {
      c->npc->pc_last_known_position[dim_y] = d->pc.position[dim_y];
      c->npc->pc_last_known_position[dim_x] = d->pc.position[dim_x];
      c->npc->have_seen_pc = 1;
    } else if ((c->npc->pc_last_known_position[dim_y] == c->position[dim_y]) &&
               (c->npc->pc_last_known_position[dim_x] == c->position[dim_x])) {
      c->npc->have_seen_pc = 0;
    }
    if (c->npc->have_seen_pc) {
      npc_next_pos_line_of_sight(d, c, next);
    }
    break;
  case NPC_TELEPATH:
  case NPC_TELEPATH | NPC_TUNNEL:
  case NPC_PASS_WALL | NPC_TELEPATH:
  case NPC_PASS_WALL | NPC_SMART | NPC_TELEPATH:
  case NPC_PASS_WALL | NPC_TELEPATH | NPC_TUNNEL:
  case NPC_PASS_WALL | NPC_SMART | NPC_TELEPATH | NPC_TUNNEL:
    /* We could implement a "toward pc" movement function, but this works *
     * just as well, since the line of sight calculations are down        *
     * outside of the next position function.                             */
    c->npc->pc_last_known_position[dim_y] = d->pc.position[dim_y];
    c->npc->pc_last_known_position[dim_x] = d->pc.position[dim_x];
    npc_next_pos_line_of_sight(d, c, next);      
    break;
  case NPC_SMART | NPC_TELEPATH:
  case NPC_SMART | NPC_TELEPATH | NPC_TUNNEL:
      npc_next_pos_gradient(d, c, next);
    break;
  }
}

uint32_t dungeon_has_npcs(dungeon_t *d)
{
  return d->num_monsters;
}
