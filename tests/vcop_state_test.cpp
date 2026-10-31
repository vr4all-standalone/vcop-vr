#include "../native/game_state.h"
#include <cassert>
#include <unordered_map>
using namespace vcop;
int main(){
 std::unordered_map<uint32_t,uint32_t> mem;
 const uint32_t words[][2]={{0x24c8,0x90a83000},{0x24cc,0x005016cc},{0xa740,0x90a03000},{0xa744,0x005016d4},{0xad40,0x92e83000},{0xad44,0x0050ee30},{0xae70,0x92e83000},{0xae74,0x0050ee34},{0x54288,0x8c883000},{0x5428c,0x0050ee38},{0x542c0,0x8c883000},{0x542c4,0x0050ee3c},{0x544a4,0x59a50901},{0x544a8,0x92a19000},{0x54138,0x92f03000},{0x5413c,0x005b1ea8},{0x28af4,0x82a83000},{0x28af8,0x00559330}};
 for(auto& w:words)mem[w[0]]=w[1];
 auto read=[&](uint32_t a){return mem[a];};
 mem[0x5016cc]=5;mem[0x5016d4]=0;mem[0x50ee30]=1;mem[0x50ee70]=3;
 mem[0x50ee38]=mem[0x50ee40]=6;mem[0x50ee88]=1;
 auto attract=read_game_state(read);assert(attract.valid&&attract.ready&&!attract.playing);assert(!attract.players[0].joined());
 mem[0x5016cc]=11;mem[0x5016d4]=7;
 auto play=read_game_state(read);assert(play.valid&&play.playing&&play.counting_shots);assert(play.players[0].alive()&&play.players[0].ammo==6);
 mem[0x5016c0]=878;mem[0x5016c8]=1872;mem[0x5016b4]=66615;
 auto cadence=read_game_state(read);assert(cadence.native_game_loops==878&&cadence.native_vblank_total==1872&&cadence.native_idle_polls==66615);
 mem[0x50ee34]=1;mem[0x50ee74]=3;mem[0x50ee3c]=mem[0x50ee44]=6;mem[0x50ee8c]=1;
 assert(read_game_state(read).players[1].joined());
 mem[0x50ee38]=7;assert(!read_game_state(read).valid);mem[0x50ee38]=6;
 mem[0x5016cc]=0;assert(!read_game_state(read).ready);
 mem[0x5016cc]=2;assert(read_game_state(read).ready);
 mem[0x559330]=1;assert(read_game_state(read).service&&!read_game_state(read).ready);
 mem[0x24cc]=0x00500032;assert(!read_game_state(read).valid); // VC2 fingerprint rejected.
}
