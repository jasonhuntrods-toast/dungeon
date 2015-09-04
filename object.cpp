#include <vector>
#include <ncurses.h> /* for COLOR_BLACK and COLOR_WHITE */

#include "object.h"
#include "dungeon.h"
#include "utils.h"

object::object(const object_description &o, pair_t p, object *next) :
  name(o.get_name()),
  description(o.get_description()),
  type(o.get_type()),
  color(o.get_color()),
  damage(o.get_damage()),
  hit(o.get_hit().roll()),
  dodge(o.get_dodge().roll()),
  defence(o.get_defence().roll()),
  weight(o.get_weight().roll()),
  speed(o.get_speed().roll()),
  attribute(o.get_attribute().roll()),
  value(o.get_value().roll()),
  next(next)
{
  position[dim_x] = p[dim_x];
  position[dim_y] = p[dim_y];
}

object::~object()
{
  if (next) {
    delete next;
  }
}

void gen_object(dungeon_t *d)
{
  object *o;
  const std::vector<object_description> &v =
    *((std::vector<object_description> *) d->object_descriptions);
  const object_description &od = v[rand_range(0, v.size() - 1)];
  uint32_t room;
  pair_t p;

  room = rand_range(0, d->num_rooms - 1);
  p[dim_y] = rand_range(d->rooms[room].position[dim_y],
                        (d->rooms[room].position[dim_y] +
                         d->rooms[room].size[dim_y] - 1));
  p[dim_x] = rand_range(d->rooms[room].position[dim_x],
                        (d->rooms[room].position[dim_x] +
                         d->rooms[room].size[dim_x] - 1));

  o = new object(od, p, (object *) d->object[p[dim_y]][p[dim_x]]);

  d->object[p[dim_y]][p[dim_x]] = (object_t *) o;  
}

void gen_objects(dungeon_t *d, uint32_t numobj)
{
  uint32_t i;

  d->num_objects = numobj;
  for (i = 0; i < numobj; i++) {
    gen_object(d);
  }
}

char get_symbol(object_t *o)
{
  return ((object *) o)->next ? '&' : object_symbol[((object *) o)->type];
}

uint32_t get_color(object_t *o)
{
  return (((object *) o)->color == COLOR_BLACK ?
          COLOR_WHITE                          :
          ((object *) o)->color);
}

uint32_t get_type(object_t *o)
{
  return ((object *) o)->type;
}

const char* get_name(object_t *o)
{
  return ((object *) o)->name.c_str();
}

int32_t get_damage(object_t *o)
{
  return ((object *) o)->damage.roll();
}

int32_t get_speed(object_t *o)
{
  return ((object *) o)->speed;
}

void destroy_objects(dungeon_t *d)
{
  uint32_t y, x;

  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      if (d->object[y][x]) {
        delete (object *) d->object[y][x];
        d->object[y][x] = 0;
      }
    }
  }
}
