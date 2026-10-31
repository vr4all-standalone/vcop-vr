#include "../native/native_shot.h"
#include <cassert>
using namespace vcop;
int main(){GameState s;s.valid=s.playing=s.counting_shots=true;
 assert(accepted_shot_write(0x5b1ea8,1,0xffffffff,s,0x544b4));
 assert(accepted_shot_write(0x5b1f08,1,0xffffffff,s,0x544b4));
 assert(!accepted_shot_write(0x5b1ea8,0,0xffffffff,s,0x544b4));
 assert(!accepted_shot_write(0x5b1ea8,1,0xff,s,0x544b4));
 assert(!accepted_shot_write(0x50f310,1,0xffffffff,s,0x544b4));
 assert(!accepted_shot_write(0x5b1ea8,1,0xffffffff,s,0x544b0));
 s.playing=false;assert(!accepted_shot_write(0x5b1ea8,1,0xffffffff,s,0x544b4));
 s.playing=true;s.counting_shots=false;assert(!accepted_shot_write(0x5b1ea8,1,0xffffffff,s,0x544b4));
 s.counting_shots=true;s.valid=false;assert(!accepted_shot_write(0x5b1ea8,1,0xffffffff,s,0x544b4));
}
