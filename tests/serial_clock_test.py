"""Compare the actual shared-clock forwarding code with native edge order."""
import argparse,os,subprocess,tempfile
from pathlib import Path
from native_clip_fastpath_test import function
def run(model,audio,cxx):
    callbacks=function(Path(model).read_text(),'void model2o_state::vcop_uart_clock_w(')
    callbacks+=function(Path(audio).read_text(),'void segam1audio_device::uart_clock_w(')
    harness=r'''
#include <cassert>
#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>
std::vector<std::pair<int,int>> events;
using u64=uint64_t;
struct Uart{int id;void write_txc(int s){events.emplace_back(id,s);}void write_rxc(int s){events.emplace_back(id+1,s);}void skip_idle_clock_edges(unsigned,int){assert(false);}};
struct segam1audio_device{Uart uart{0};Uart* m_uart=&uart;void uart_clock_w(int);void skip_uart_clock_edges(unsigned,int){assert(false);}};
struct model2o_state{
 Uart uart{2};Uart* m_uart=&uart;segam1audio_device audio;segam1audio_device* m_m1audio=&audio;
 // This check exercises the normal forwarding path. The opt-in coordinator
 // is separately exercised by uart_clock_boundary_test.py.
 bool m_retained_uart_clock=false,m_uart_clock_busy=false;u64 m_uart_raw_edge=0,m_uart_clock_edge=0,m_uart_clock_next=0;
 struct Time{u64 as_ticks(int){assert(false);return 0;}};struct Machine{Time time(){return {};}};
 Machine machine(){return {};}void schedule_vcop_uart_clock(){assert(false);}
 void vcop_uart_clock_w(int);
};
CALLBACKS
int main(){model2o_state model;
 for(int edge=0;edge<1000;++edge){auto start=events.size();int level=edge&1;model.vcop_uart_clock_w(level);
  assert(events.size()==start+4);for(int n=0;n<4;++n)assert(events[start+n]==std::pair(n,level));}
 std::cout<<"1000 shared serial clock edges preserve audio TX/RX then main TX/RX order and levels\n";
}
'''.replace('CALLBACKS',callbacks)
    env=os.environ.copy();env['PATH']=str(Path(cxx).parent)+os.pathsep+env.get('PATH','')
    with tempfile.TemporaryDirectory(prefix='.serial-proof-',dir=Path(__file__).parent) as directory:
        root=Path(directory).resolve();env['TMP']=env['TEMP']=str(root)
        cpp=root/'serial.cpp';cpp.write_text(harness);exe=root/'serial.exe'
        subprocess.run([str(cxx),'-std=c++20','-O2','-UNDEBUG',str(cpp),'-o',str(exe)],env=env,check=True);subprocess.run([str(exe)],env=env,check=True)
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--model',type=Path,required=True);p.add_argument('--audio',type=Path,required=True);p.add_argument('--cxx',type=Path,required=True);run(**vars(p.parse_args()))
