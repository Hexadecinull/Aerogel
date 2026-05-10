#pragma once
#include <types.h>

typedef struct {
    i32  x, y;
    bool left, right, middle;
} mouse_state_t;

void               mouse_init(void);
const mouse_state_t *mouse_get(void);
