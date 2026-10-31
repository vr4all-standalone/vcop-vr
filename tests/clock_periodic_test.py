"""Execute the real pinned clock callbacks against a deterministic timer boundary."""
import argparse
import os
from pathlib import Path
import subprocess
import tempfile
from native_clip_fastpath_test import function


def run(source, cxx):
    text = Path(source).read_text(encoding='utf8')
    callbacks = function(text, 'TIMER_CALLBACK_MEMBER(clock_device::clock_init)')
    callbacks += function(text, 'TIMER_CALLBACK_MEMBER(clock_device::clock_tick)')
    harness = r'''
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>
#include <utility>
#include <vector>
struct attotime {
    int64_t ns;
    static const attotime zero,never;
    static attotime from_hz(int hz){return {1000000000/hz};}
    static attotime from_double(double s){return {int64_t(s*1e9)};}
    bool is_zero() const{return ns==0;}
    bool is_never() const{return ns==never.ns;}
    double as_double() const{return ns/1e9;}
    auto operator<=>(const attotime&) const=default;
    attotime operator/(int n) const{return {ns/n};}
    attotime operator-(attotime b) const{return {ns-b.ns};}
};
const attotime attotime::zero{0},attotime::never{INT64_MAX/4};
int64_t now;
struct Timer {
    attotime next=attotime::never,repeat=attotime::never;
    unsigned adjusts{};
    attotime remaining() const{return {next.ns-now};}
    attotime period() const{return repeat;}
    void adjust(attotime delay,int=0,attotime period=attotime::never){next={now+delay.ns};repeat=period;++adjusts;}
};
#define TIMER_CALLBACK_MEMBER(name) void name()
struct clock_device {
    int m_clock=500000,m_signal=0;
    double m_duty=.5;
    attotime m_period=attotime::never,m_pw=attotime::never,m_thigh{},m_tlow{};
    Timer timer;Timer* m_timer_tick=&timer;
    std::vector<std::pair<int64_t,int>> edges;
    void output(){edges.emplace_back(now,m_signal);}
    void clock_init();void clock_tick();
    void tick(){now=timer.next.ns;auto before=timer.adjusts;clock_tick();if(before==timer.adjusts)timer.next.ns+=timer.repeat.ns;}
};
CALLBACKS
int main(){
    now=0;clock_device square;square.clock_init();
    for(int edge=1;edge<=1000;++edge){square.tick();assert(square.edges.back()==std::pair(int64_t(edge*1000),edge%2));}
    now=0;clock_device asymmetric;asymmetric.m_duty=.25;asymmetric.clock_init();
    for(auto expected:{std::pair<int64_t,int>{1500,1},{2000,0},{3500,1},{4000,0}}){asymmetric.tick();assert(asymmetric.edges.back()==expected);}
    now=0;clock_device constant;constant.m_duty=0;constant.clock_init();constant.tick();assert(constant.edges.back()==std::pair(int64_t(2000),0));
    now=0;clock_device changed;changed.clock_init();changed.tick();
    now=1250;changed.m_clock=250000;changed.clock_init();changed.tick();assert(changed.edges.back()==std::pair(int64_t(2000),0));
    changed.tick();assert(changed.edges.back()==std::pair(int64_t(4000),1));
    changed.m_duty=.25;changed.clock_init();changed.tick();assert(changed.edges.back()==std::pair(int64_t(5000),0));
    changed.tick();assert(changed.edges.back()==std::pair(int64_t(8000),1));
    // An older saved state has a one-shot timer. Its first edge must repair
    // the timer period so the clock keeps running after an app update.
    now=0;clock_device restored;restored.m_thigh=restored.m_tlow={1000};restored.timer.adjust({1000});
    restored.tick();restored.tick();assert(restored.edges.back()==std::pair(int64_t(2000),0));
    // This catches the measured hot-path defect, after the edge semantics.
    if(square.timer.adjusts!=1){std::cerr<<"Symmetric clock redundantly rearms "<<square.timer.adjusts<<" times\n";return 1;}
    std::cout<<"1000 exact square-wave edges, asymmetric/constant clocks and live clock changes passed; no per-edge rearm\n";
}
'''.replace('CALLBACKS', callbacks)
    env = os.environ.copy()
    env['PATH'] = str(Path(cxx).parent) + os.pathsep + env.get('PATH', '')
    with tempfile.TemporaryDirectory(prefix='.clock-proof-', dir=Path(__file__).parent) as directory:
        root = Path(directory).resolve()
        env['TMP'] = env['TEMP'] = str(root)
        cpp = root/'clock.cpp'
        cpp.write_text(harness)
        exe = root/'clock.exe'
        subprocess.run([str(cxx), '-std=c++20', '-O2', '-UNDEBUG', str(cpp), '-o', str(exe)], env=env, check=True)
        subprocess.run([str(exe)], env=env, check=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, required=True)
    parser.add_argument('--cxx', type=Path, required=True)
    run(**vars(parser.parse_args()))
