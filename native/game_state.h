// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "game_memory.h"
#include <array>
#include <cstdint>
namespace vcop {
struct PlayerState {
    uint32_t state{},ammo{},capacity{},shots{},weapon{};
    float health{};
    uint32_t native_state{};
    bool joined() const { return state==1 || state==2; }
    bool alive() const { return state==1 && health>0; }
};
struct GameState {
    bool valid{},playing{},counting_shots{};
    uint32_t mode{};
    std::array<PlayerState,2> players{};
    bool ready{},service{},gun_calibration{};
    uint32_t submode{},service_menu{},service_submode{},service_cursor{},service_busy{};
    std::array<GunCalibration,2> calibration{};
    // Observations only; never used to gate gameplay or cabinet controls.
    uint32_t native_game_loops{},native_idle_polls{},native_vblank_total{},native_video_control{};
};
// Original vcop Revision B physical i960 words: mode dispatcher, gameplay
// submode, P1/P2 join stores, ammo pointers, accepted bullet decrement and
// accepted-shot flags, native service loop. Never accept a VC2 ROM/state.
template<class Read32> bool game_revision_matches(Read32 read) {
    constexpr uint32_t words[][2]={
        {0x24c8,0x90a83000},{0x24cc,0x005016cc},{0xa740,0x90a03000},{0xa744,0x005016d4},
        {0xad40,0x92e83000},{0xad44,0x0050ee30},{0xae70,0x92e83000},{0xae74,0x0050ee34},
        {0x54288,0x8c883000},{0x5428c,0x0050ee38},{0x542c0,0x8c883000},{0x542c4,0x0050ee3c},
        {0x544a4,0x59a50901},{0x544a8,0x92a19000},{0x54138,0x92f03000},{0x5413c,0x005b1ea8},
        {0x28af4,0x82a83000},{0x28af8,0x00559330}
    };
    for(const auto& w:words)if(uint32_t(read(w[0]))!=w[1])return false;
    return true;
}
template<class Read32> GameState read_game_state(Read32 read) {
    if(!game_revision_matches(read))return {};
    const auto byte=[&](uint32_t a){return (uint32_t(read(a&~3u))>>((a&3)*8))&255u;};
    GameState result;
    result.mode=read(0x5016cc);result.submode=read(0x5016d4);
    // Original VC1 main-loop store2530, IRQ store754 and sync-wait store27c4.
    // These are totals/slack, never a borrowed VC2 backlog or pending count.
    result.native_game_loops=read(0x5016c0);result.native_vblank_total=read(0x5016c8);result.native_idle_polls=read(0x5016b4);
    const uint32_t service=byte(0x559330);
    if(result.mode>13||result.submode>15||service>1)return {};
    result.service=service==1;
    result.service_menu=byte(0x559365);result.service_cursor=byte(0x55936e);result.service_busy=read(0x559820);
    result.gun_calibration=result.service&&result.service_menu==6&&result.service_cursor>=1&&result.service_cursor<=7&&result.service_busy>0;
    result.ready=!result.service&&result.mode>=2&&result.mode<=11;
    result.playing=!result.service&&result.mode==11;
    result.counting_shots=result.playing&&result.submode==7;
    for(unsigned i=0;i<2;i++) {
        auto& p=result.players[i];p.native_state=read(0x50ee30+4*i);
        p.ammo=read(0x50ee38+4*i);p.capacity=read(0x50ee40+4*i);
        p.health=float(read(0x50ee70+4*i));p.weapon=read(0x50ee88+4*i);
        if(p.native_state>3||p.capacity>256||p.ammo>p.capacity||p.health>255||p.weapon>6)return {};
        p.state=result.playing?p.native_state:0;
        result.calibration[i]=read_gun_calibration(read,i);
        // Native trigger statistics count rejected/empty shots too. The engine
        // separately supplies cumulative verified accepted-shot write events.
    }
    result.valid=true;return result;
}
}
