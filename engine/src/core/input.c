#include "input.h"

#include <core/memory.h>

void input_state_create(struct input_state *input_state)
{
    engine_zero_memory(input_state->keyboard, LARRAY_LENGTH(input_state->keyboard));
}
