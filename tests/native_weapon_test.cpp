#include "../native/native_weapon.h"
#include <cassert>
#include <fstream>
#include <vector>
#include <cstring>
using namespace vcop;
std::vector<char> load(const char* path){std::ifstream f(path,std::ios::binary);assert(f);return {std::istreambuf_iterator<char>(f),{}};}
uint32_t word(const std::vector<char>&v,uint32_t at){assert(uint64_t(at)+4<=v.size());uint32_t n;std::memcpy(&n,v.data()+at,4);return n;}
int main(int argc,char**argv){assert(argc==5);auto prog=load(argv[1]),data=load(argv[2]),poly=load(argv[3]),tex=load(argv[4]);
 auto program=[&](uint32_t a){if(a<prog.size())return word(prog,a);assert(a>=0x2000000);return word(data,a-0x2000000);};
 auto polygon=[&](uint32_t a){return word(poly,a*4);};
 auto texture=[&](uint32_t a){assert(uint64_t(a)*2+2<=tex.size());uint16_t n;std::memcpy(&n,tex.data()+a*2,2);return n;};
 const unsigned models[]={0,0x3b5,0x3cf,0x3ca,0x3cc,0x3d0,0x3cd},faces[]={0,189,85,87,80,68,104};
 for(unsigned i=1;i<=6;i++){auto gun=extract_native_weapon(program,polygon,texture,i,poly.size()/4,tex.size()/2);assert(gun.valid&&gun.model==models[i]&&gun.polygons.size()==faces[i]);}
 assert(!extract_native_weapon(program,polygon,texture,7,poly.size()/4,tex.size()/2).valid);
 auto wrong=[&](uint32_t a){return a==0xbae8?0x200000:program(a);};
 assert(!extract_native_weapon(wrong,polygon,texture,1,poly.size()/4,tex.size()/2).valid);
 auto label=[&](uint32_t a){return a==0x281db50?program(a+4):program(a);};
 assert(!extract_native_weapon(label,polygon,texture,2,poly.size()/4,tex.size()/2).valid);
}
