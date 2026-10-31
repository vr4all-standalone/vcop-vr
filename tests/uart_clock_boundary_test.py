"""Run actual retained-clock coordination across writes and timer callbacks."""
import argparse,os,subprocess,tempfile
from pathlib import Path
from native_clip_fastpath_test import function
from uart_idle_edges_test import harness
def run(uart,serial,model,cxx):
    cpp=harness(uart,serial).split('int main(){')[0]
    cpp=cpp.replace('#include <algorithm>','#include <algorithm>\n#include <functional>')
    cpp=cpp.replace('struct i8251_device {','struct i8251_device {\n std::function<void(bool)> m_idle_clock_notify;void write_rxd(int);')
    cpp+='#define LOGBITS(...) ((void)0)\n'+function(Path(uart).read_text(),'void i8251_device::write_rxd(')
    model_text=Path(model).read_text()
    callbacks=''.join(function(model_text,s) for s in ['void model2o_state::schedule_vcop_uart_clock(', 'void model2o_state::vcop_uart_clock_notify(', 'void model2o_state::vcop_uart_clock_w('])
    cpp+=r'''
using u64=uint64_t;
int64_t now_ns{};
struct attotime {int64_t ns;static attotime from_ticks(u64 n,unsigned rate){return {int64_t(n)*(1000000000/rate)};}u64 as_ticks(unsigned rate)const{return u64(ns/(1000000000/rate));}attotime operator-(attotime b)const{return {ns-b.ns};}};
struct Timer{void adjust(attotime){assert(false && "Retained clock must not reschedule a synthetic timer");}};
struct Machine{attotime time()const{return {now_ns};}};
struct segam1audio_device{i8251_device uart;unsigned uart_clock_deadline(int l)const{return uart.idle_clock_deadline(l);}void skip_uart_clock_edges(unsigned e,int l){uart.skip_idle_clock_edges(e,l);}void uart_clock_w(int l){uart.write_txc(l);uart.write_rxc(l);}};
struct model2o_state {
 i8251_device root;i8251_device* m_uart=&root;segam1audio_device audio;segam1audio_device* m_m1audio=&audio;
 bool m_retained_uart_clock=true,m_uart_clock_busy=false,m_idle_uart_clock=false,m_uart_clock_idle=false;u64 m_uart_clock_edge=0,m_uart_clock_next=0,m_uart_raw_edge=0;Timer* m_uart_clock_timer=nullptr;Machine m;
 Machine& machine(){return m;}void schedule_vcop_uart_clock();void vcop_uart_clock_notify(bool);void vcop_uart_clock_w(int);
};
CALLBACKS
int main(){unsigned cases=0;
 for(int edge:{15,16})for(int offset:{-40,0,40})for(bool after:{false,true}){
  now_ns=0;model2o_state folded;auto notify=[&](bool changed){folded.vcop_uart_clock_notify(changed);};folded.root.m_idle_clock_notify=notify;folded.audio.uart.m_idle_clock_notify=notify;folded.schedule_vcop_uart_clock();
  i8251_device raw;folded.vcop_uart_clock_w(0);assert(folded.m_uart_raw_edge==0&&folded.m_uart_clock_edge==0);
  int last=after?edge:edge-1;
  for(int e=1;e<=last;++e){now_ns=e*1000;raw.write_txc(e&1);raw.write_rxc(e&1);folded.vcop_uart_clock_w(e&1);}
  now_ns=edge*1000+offset;raw.write_rxd(0);folded.root.write_rxd(0);
  for(int e=last+1;e<=17;++e){now_ns=e*1000;raw.write_txc(e&1);raw.write_rxc(e&1);folded.vcop_uart_clock_w(e&1);}
  folded.vcop_uart_clock_notify(false);assert(raw.state()==folded.root.state());assert(raw.is_receive_register_synchronized());assert(folded.m_uart_raw_edge==17);++cases;
 }
 // A save flush after a lazy idle edge must record exactly the same counters.
 now_ns=0;model2o_state folded;folded.schedule_vcop_uart_clock();i8251_device raw;
 for(int e=1;e<=15;++e){now_ns=e*1000;raw.write_txc(e&1);raw.write_rxc(e&1);folded.vcop_uart_clock_w(e&1);}
 folded.vcop_uart_clock_notify(false);assert(raw.state()==folded.root.state());assert(folded.m_uart_clock_edge==15);
 std::cout<<cases<<" actual coordinator before/after-edge and local-time overshoot cases plus initial-output/lazy-save checks passed\n";
}
'''.replace('CALLBACKS',callbacks)
    env=os.environ.copy();env['PATH']=str(Path(cxx).parent)+os.pathsep+env.get('PATH','')
    with tempfile.TemporaryDirectory(prefix='.uart-boundary-',dir=Path(__file__).parent) as directory:
        root=Path(directory).resolve();env['TMP']=env['TEMP']=str(root)
        src=root/'boundary.cpp';src.write_text(cpp);exe=root/'boundary.exe'
        subprocess.run([str(cxx),'-std=c++20','-O2','-UNDEBUG',str(src),'-o',str(exe)],env=env,check=True);subprocess.run([str(exe)],env=env,check=True)
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--uart',type=Path,required=True);p.add_argument('--serial',type=Path,required=True);p.add_argument('--model',type=Path,required=True);p.add_argument('--cxx',type=Path,required=True);run(**vars(p.parse_args()))
