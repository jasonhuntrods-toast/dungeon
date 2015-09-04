#ifndef DESCRIPTIONS_H
# define DESCRIPTIONS_H

# include <stdint.h>

typedef struct dungeon dungeon_t;
typedef struct character character_t;
typedef struct object_t object_t;

# ifdef __cplusplus

# include <string>
# include "dice.h"

extern "C" {
# endif


uint32_t parse_descriptions(dungeon_t *d);
uint32_t print_descriptions(dungeon_t *d);
uint32_t destroy_descriptions(dungeon_t *d);
character_t *generate_monster(dungeon_t *d);
object_t *generate_object(dungeon_t *d);

typedef enum object_type {
  objtype_no_type,
  objtype_WEAPON,
  objtype_OFFHAND,
  objtype_RANGED,
  objtype_ARMOR,
  objtype_HELMET,
  objtype_CLOAK,
  objtype_GLOVES,
  objtype_BOOTS,
  objtype_RING,
  objtype_AMULET,
  objtype_LIGHT,
  objtype_SCROLL,
  objtype_BOOK,
  objtype_FLASK,
  objtype_GOLD,
  objtype_AMMUNITION,
  objtype_FOOD,
  objtype_WAND,
  objtype_CONTAINER
} object_type_t;

extern const char object_symbol[];

# ifdef __cplusplus
} /* extern "C" */

class monster_description {
 private:
  std::string name, description;
  char symbol;
  uint32_t color, abilities;
  dice speed, hitpoints, damage;
 public:
  monster_description() : name(),       description(), symbol(0),   color(0),
                          abilities(0), speed(),       hitpoints(), damage()
  {
  }
  void set(const std::string &name,
           const std::string &description,
           const char symbol,
           const uint32_t color,
           const dice &speed,
           const uint32_t abilities,
           const dice &hitpoints,
           const dice &damage);
  std::ostream &print(std::ostream &o);

  friend character_t *generate_monster(dungeon_t *);
};

class object_description {
 private:
  std::string name, description;
  object_type_t type;
  uint32_t color;
  dice hit, damage, dodge, defence, weight, speed, attribute, value;
 public:
  object_description() : name(),    description(), type(objtype_no_type),
                         color(0),  hit(),         damage(),
                         dodge(),   defence(),     weight(),
                         speed(),   attribute(),   value()
  {
  }
  void set(const std::string &name,
           const std::string &description,
           const object_type_t type,
           const uint32_t color,
           const dice &hit,
           const dice &damage,
           const dice &dodge,
           const dice &defence,
           const dice &weight,
           const dice &speed,
           const dice &attrubute,
           const dice &value);
  std::ostream &print(std::ostream &o);
  /* Need all these accessors because otherwise there is a *
   * circular dependancy that is difficult to get around.  */
  inline const std::string &get_name() const { return name; }
  inline const std::string &get_description() const { return description; }
  inline const object_type_t get_type() const { return type; }
  inline const uint32_t get_color() const { return color; }
  inline const dice& get_hit() const { return hit; }
  inline const dice& get_damage() const { return damage; }
  inline const dice& get_dodge() const { return dodge; }
  inline const dice& get_defence() const { return defence; }
  inline const dice& get_weight() const { return weight; }
  inline const dice& get_speed() const { return speed; }
  inline const dice& get_attribute() const { return attribute; }
  inline const dice& get_value() const { return value; }

  friend void gen_object(dungeon_t *d);
};

# endif

#endif
