// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "game_state.h"
namespace vcop {
// Original accepted on-screen bullet paths544b0/548e4 set these flags after
// decrementing valid ammo. Empty pulls, trigger stats and offscreen reloads
// do not set them. Count actual full stores, including consecutive game frames.
inline bool accepted_shot_write(uint32_t address,uint32_t data,uint32_t mask,const GameState& state,uint32_t instruction_pointer) {
 return (instruction_pointer==0x544b4||instruction_pointer==0x548e8)&&state.valid&&state.playing&&state.counting_shots&&mask==0xffffffffu&&data==1&&
    (address==0x5b1ea8||address==0x5b1f08);
}
}

