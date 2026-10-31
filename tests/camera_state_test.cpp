#include "../native/camera_state.h"
#include <cassert>
#include <bit>
#include <unordered_map>
#include <cmath>
using namespace vcop;
int main(){
 std::unordered_map<uint32_t,uint32_t> m;
 const uint32_t words[][2]={{0x1ce30,0x90983000},{0x1ce34,0x0050f3f4},{0x1ce94,0x69a01714},{0x1ce98,0x79a01594},{0x1cea4,0x92a03000},{0x1cea8,0x0050f404},{0x1decc,0x90a03000},{0x1ded0,0x0050f404},{0x1def4,0x9ab03000},{0x25114,0x90a03000},{0x25118,0x0050f404},{0x2512c,0x78450604},{0x25130,0x90a03000},{0x25134,0x0050f404}};
 for(auto&w:words)m[w[0]]=w[1];auto read=[&](uint32_t a){return m[a];};
 m[0x5016cc]=11;m[0x5016d4]=7;
 auto bits=[](float f){return std::bit_cast<uint32_t>(f);};
 assert(camera_revision_matches(read));
 float focus=std::bit_cast<float>(camera_write(0x50f404,bits(500),0xffffffff,read,110));
 assert(std::abs(focus-248.f/std::tan(110.f*3.14159265358979323846f/360))<.001);
 assert(camera_write(0x50f404,bits(100),0xffffffff,read,110)==bits(100));
 assert(camera_write(0x50f404,bits(500),0xffff,read,110)==bits(500));
 assert(camera_write(0x50f3f4,bits(500),0xffffffff,read,110)==bits(500));
 m[0x5016d4]=2;assert(camera_write(0x50f404,bits(500),0xffffffff,read,110)==bits(500));
 m[0x5016d4]=7;m[0x559330]=1;assert(camera_write(0x50f404,bits(500),0xffffffff,read,110)==bits(500));
 m[0x559330]=0;m[0x1cea8]=0x506044;assert(!camera_revision_matches(read));
 assert(camera_write(0x50f404,bits(500),0xffffffff,read,110)==bits(500));
 const uint32_t low=widened_cull_read(0xb8b0,0xfc8b007a,0xffffffff,110,true);
 const uint32_t high=widened_cull_read(0xb8b8,0x3fd921fa,0xffffffff,110,true);
 const double angle=std::bit_cast<double>((uint64_t(high)<<32)|low);
 assert(std::abs(angle-55*3.14159265358979323846/180)<1e-12);
 const uint32_t actorlo=widened_cull_read(0x320fc,0x306f3395,0xffffffff,110,true),actorhi=widened_cull_read(0x32104,0x3fe41b2f,0xffffffff,110,true);
 assert(std::abs(std::bit_cast<double>((uint64_t(actorhi)<<32)|actorlo)-68.5*3.14159265358979323846/180)<1e-12);
 assert(widened_cull_read(0xb8b0,0xfc8b007a,0xffffffff,110,false)==0xfc8b007a);
 assert(widened_cull_read(0xb8b0,0xfc8b007a,0xffff,110,true)==0xfc8b007a);
 assert(widened_cull_read(0xb8b4,0x8c483000,0xffffffff,110,true)==0x8c483000);
 assert(widened_cull_read(0xb8b0,0x12345678,0xffffffff,110,true)==0x12345678);
}
