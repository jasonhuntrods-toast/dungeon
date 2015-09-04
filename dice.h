#ifndef DICE_H
# define DICE_H

# include <stdint.h>

# ifdef __cplusplus

# include <iostream>

extern "C" {
# endif

typedef struct dice_t {
} dice_t;

dice_t *new_dice(int32_t base, uint32_t number, uint32_t sides);
void destroy_dice(dice_t *d);
int32_t roll_dice(dice_t *d);

# ifdef __cplusplus
} /* extern "C" */

class dice {
 private:
  int32_t base;
  uint32_t number, sides;
 public:
  dice() : base(0), number(0), sides(0)
  {
  }
  dice(int32_t base, uint32_t number, uint32_t sides) :
  base(base), number(number), sides(sides)
  {
  }
  inline void set(int32_t base, uint32_t number, uint32_t sides)
  {
    this->base = base;
    this->number = number;
    this->sides = sides;
  }
  inline void set_base(int32_t base)
  {
    this->base = base;
  }
  inline void set_number(uint32_t number)
  {
    this->number = number;
  }
  inline void set_sides(uint32_t sides)
  {
    this->sides = sides;
  }
  int32_t roll(void) const;
  std::ostream &print(std::ostream &o);
};

# endif

#endif
