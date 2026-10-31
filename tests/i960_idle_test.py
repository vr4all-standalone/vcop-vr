"""Differential check against the pinned i960 interpreter's actual operations."""
import argparse
import os
from pathlib import Path
import subprocess
import tempfile
from native_clip_fastpath_test import function

def run(source,cxx):
    text=Path(source).read_text(encoding='utf8')
    signatures=['uint32_t i960_cpu_device::get_ea(', 'uint32_t i960_cpu_device::get_1_ri(', 'uint32_t i960_cpu_device::get_2_ri(', 'void i960_cpu_device::set_ri(', 'uint32_t i960_cpu_device::get_1_ci(', 'uint32_t i960_cpu_device::get_2_ci(', 'uint32_t i960_cpu_device::get_disp_s(', 'void i960_cpu_device::cmp_s(', 'void i960_cpu_device::bxx_s(']
    methods=''.join(function(text,s) for s in signatures)
    def between(start,end,begin=0):
        a=text.index(start,begin);return text[a:text.index(end,a)]
    cases=between('case 0x90: { // ld','case 0x92: // st')
    cases+=between('case 0x3a: // cmpibe','case 0x3b: // cmpibge')
    andop=between('case 0x1: // and','case 0x2: // andnot',text.index('case 0x58:'))
    addop=between('case 0x0: // addo','case 0x1: // addi',text.index('case 0x59:'))
    cases+='case 0x58: switch((opcode>>7)&15){'+andop+'default:assert(false);}break;'
    cases+='case 0x59: switch((opcode>>7)&15){'+addop+'default:assert(false);}break;'
    harness=r'''
#include "i960_idle.h"
#include <array>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
using u32=uint32_t;
namespace util {int32_t sext(uint32_t n,unsigned bits){return int32_t(n<<(32-bits))>>(32-bits);}}
template<class...T> [[noreturn]] void fatalerror(const char*,T...){throw std::runtime_error("bad instruction");}
struct i960_cpu_device {
 uint32_t m_IP=0x9124,m_PIP=0x911c,m_AC{},m_r[32]{};int m_icount{};bool m_stalled{};
 uint32_t video{};unsigned reads{},folded{};
 struct Cache {uint32_t read_dword(uint32_t a){switch(a){case 0x9124:return 0x90a03000;case 0x9128:return 0x98000c;case 0x912c:return 0x59805010;case 0x9130:return 0x58a11094;case 0x9134:return 0x3aad1ff0;default:return 0;}}}m_cache;
 uint32_t i960_read_dword_unaligned(uint32_t a){assert(a==0x98000c);++reads;return video;}
 uint32_t get_ea(uint32_t);uint32_t get_1_ri(uint32_t);uint32_t get_2_ri(uint32_t);void set_ri(uint32_t,uint32_t);
 uint32_t get_1_ci(uint32_t);uint32_t get_2_ci(uint32_t);uint32_t get_disp_s(uint32_t);void cmp_s(int32_t,int32_t);void bxx_s(uint32_t,int);
 void execute_op(uint32_t opcode){uint32_t t1,t2;switch(opcode>>24){CASES default:assert(false);}}
 void run(bool fast,bool debug=false,bool burst=false){while(m_icount>0&&m_IP<=0x9134){
   if(fast&&vcop::fold_idle_poll(true,debug,burst,m_IP,m_PIP,m_r,m_AC,m_icount,[&](uint32_t a){return i960_read_dword_unaligned(a);})){++folded;continue;}
   m_PIP=m_IP;auto opcode=m_cache.read_dword(m_IP);m_IP+=4;m_stalled=false;execute_op(opcode);
 }}
};
METHODS
bool same(const i960_cpu_device&a,const i960_cpu_device&b){return a.m_IP==b.m_IP&&a.m_PIP==b.m_PIP&&a.m_AC==b.m_AC&&a.m_icount==b.m_icount&&std::equal(std::begin(a.m_r),std::end(a.m_r),std::begin(b.m_r));}
int main(int argc,char**argv){
 if(argc>1){std::ifstream file(argv[1],std::ios::binary);std::vector<uint8_t> rom(0x200000);file.read(reinterpret_cast<char*>(rom.data()),rom.size());assert(file.gcount()==rom.size());
  auto read=[&](uint32_t a){return uint32_t(rom[a])|(uint32_t(rom[a+1])<<8)|(uint32_t(rom[a+2])<<16)|(uint32_t(rom[a+3])<<24);};
  assert(vcop::idle_poll_revision_matches(read));rom[0x912c]^=1;assert(!vcop::idle_poll_revision_matches(read));
 }
 for(auto start:{0x9124u,0x912cu,0x9130u,0x9134u})for(int budget=0;budget<=512;budget++)for(uint32_t video=0;video<8;video++)for(auto old:{0u,4u})for(auto count:{0u,1u,0xfffffffeu})for(auto ac:{0u,0xffffffffu,0x80000205u}){
  i960_cpu_device a;a.m_IP=start;a.m_icount=budget;a.video=video;a.m_r[16]=count;a.m_r[21]=old;a.m_AC=ac;auto b=a;
  a.run(false);b.run(true);if(!same(a,b)){std::cerr<<"Differential failed budget="<<budget<<" video="<<video<<" old="<<old<<'\n';return 1;}
 }
 for(bool debug:{false,true})for(bool burst:{false,true}){i960_cpu_device a;a.m_icount=100;a.run(true,debug,burst);if(debug||burst)assert(!a.folded);}
 i960_cpu_device original;original.m_icount=100;auto folded=original;original.run(false);folded.run(true);
 assert(folded.folded>0&&folded.reads<original.reads&&same(original,folded));
 std::cout<<"295488 actual-interpreter entry/budget/bit/count/flag comparisons plus debug/burst and read-reduction checks passed\n";
}
'''.replace('CASES',cases).replace('METHODS',methods)
    env=os.environ.copy();env['PATH']=str(Path(cxx).parent)+os.pathsep+env.get('PATH','')
    with tempfile.TemporaryDirectory(prefix='.idle-proof-',dir=Path(__file__).parent) as directory:
        root=Path(directory).resolve();env['TMP']=env['TEMP']=str(root)
        cpp=root/'idle.cpp';cpp.write_text(harness);exe=root/'idle.exe'
        subprocess.run([str(cxx),'-std=c++20','-O2','-UNDEBUG','-I',str(Path(__file__).parents[1]/'native'),str(cpp),'-o',str(exe)],env=env,check=True)
        program=Path(__file__).parents[1]/'research/native-data/program.bin'
        subprocess.run([str(exe),*([str(program)] if program.is_file() else [])],env=env,check=True)
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--source',type=Path,required=True);p.add_argument('--cxx',type=Path,required=True)
    run(**vars(p.parse_args()))
