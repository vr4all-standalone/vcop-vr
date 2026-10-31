// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <cstdint>
namespace vcop {
template<class Read32> bool native_gun_strobe(Read32 read) {
    // Original Revision B routine58400 fills the primary window-selection
    // mask for a cabinet gun read, then clears it over its four-tick cooldown.
    // Empty fire and reload request this read before any ammo acknowledgement.
    if(uint32_t(read(0x5b1e50))!=4 || uint32_t(read(0x5016cc))!=11 ||
       (uint32_t(read(0x559330))&255)!=0)return false;
    const uint32_t submode=read(0x5016d4);
    if(submode!=2&&submode!=7)return false;
    if(!(uint32_t(read(0x50ee30))==1&&uint32_t(read(0x501528)))&&
       !(uint32_t(read(0x50ee34))==1&&uint32_t(read(0x50152c))))return false;
    constexpr uint32_t words[][2]={
        {0x5841c,0x80203000},{0x58420,0x00559330},
        {0x58444,0x90203000},{0x58448,0x005016d4},
        {0x58454,0x90203000},{0x58458,0x0050ee30},
        {0x58460,0x90203000},{0x58464,0x00501528},
        {0x5846c,0x90203000},{0x58470,0x0050ee34},
        {0x58478,0x90203000},{0x5847c,0x0050152c},
        {0x58488,0x8c303000},{0x5848c,0x0100c000},
        {0x58494,0x8c58017f},{0x58498,0x9847e040},
        {0x5849c,0x59285005},{0x584a0,0x5a02e085},
        {0x584a4,0x9a419000},{0x584a8,0x59321006},{0x584ac,0x16ffffec},
        {0x584b4,0x90203000},{0x584b8,0x005b1e50},
        {0x584d0,0x92203000},{0x584d4,0x005b1e50},{0x584d8,0x9847e050},
        {0x58598,0x5c401e04},{0x5859c,0x92403000},{0x585a0,0x005b1e50}
    };
    for(const auto& word:words)if(uint32_t(read(word[0]))!=word[1])return false;
    for(uint32_t a=0x0100c000;a<0x0100cc00;a+=4)
        if(uint32_t(read(a))!=0xffffffffu)return false;
    return true;
}
}
