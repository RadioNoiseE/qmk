#pragma once

// safe bet for atmega32u2
#define DYNAMIC_MACRO_NO_NESTING
#define DYNAMIC_MACRO_SIZE 42

// force nkey rollover
#define FORCE_NKRO

// disable tapping features and one-shot modifiers
#define NO_ACTION_TAPPING
#define NO_ACTION_ONESHOT
