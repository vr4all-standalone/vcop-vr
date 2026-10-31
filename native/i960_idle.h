// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <cstdint>
namespace vcop {
template<class Read> bool idle_poll_revision_matches(Read read) {
    constexpr uint32_t words[][2]={{0x9114,0x90a03000},{0x9118,0x0098000c},{0x911c,0x5c801e00},
        {0x9120,0x58a91094},{0x9124,0x90a03000},{0x9128,0x0098000c},{0x912c,0x59805010},
        {0x9130,0x58a11094},{0x9134,0x3aad1ff0},{0x27c4,0x92803000},{0x27c8,0x005016b4}};
    for(const auto& word:words)if(uint32_t(read(word[0]))!=word[1])return false;
    return true;
}
// This exact VC1 loop polls the pure videoctl bit, increments g0 and compares.
// videoctl changes only between scheduler slices. Fold complete 10-cycle
// iterations within this slice; the interpreter handles any partial tail and
// the first changed-bit iteration. Never advance past a timer/IRQ boundary.
template<class Read> bool fold_idle_poll(bool enabled,bool debug,bool burst,uint32_t& ip,uint32_t& pip,
        uint32_t* registers,uint32_t& ac,int& cycles,Read read) {
    if(!enabled||debug||burst||ip!=0x9124||cycles<10)return false;
    const uint32_t bit=uint32_t(read(0x98000c))&4u;
    if(registers[21]!=bit)return false;
    const uint32_t iterations=uint32_t(cycles/10);
    registers[16]+=iterations;registers[20]=bit;
    ac=(ac&~7u)|2u;pip=0x9134;
    cycles-=int(iterations*10u);
    return true;
}
}
