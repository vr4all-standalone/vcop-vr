"""Generate independently verified VC1 native camera/accepted-shot write taps."""
import argparse,difflib,subprocess
from pathlib import Path
from prepare_capture_patch import COMMIT
from prepare_vr_patch import driver,replace

def modify(source):
    return replace(source,'void model2_state::machine_start()\n{','''void model2_state::machine_start()
{
    if(vcop::camera_hooks_requested() && std::string_view(machine().system().name)=="vcop") {
        auto& space=m_maincpu->space(AS_PROGRAM);
        space.install_write_tap(0x50f404,0x50f407,"vcop_vr_camera_focus",
            [this](offs_t,u32& data,u32 mask) {
                data=vcop::camera_write(vcop::CameraFocusAddress,data,mask,
                    [&](uint32_t a){return m_maincpu->space(AS_PROGRAM).read_dword(a);},
                    vcop::requested_camera_fov());
            });
        const bool cull_revision=vcop::camera_cull_revision_matches([&](uint32_t a){return space.read_dword(a);});
        if(cull_revision&&vcop::camera_revision_matches([&](uint32_t a){return space.read_dword(a);}))for(uint32_t address:{0xb8b0u,0xb8b8u,0x320fcu,0x32104u,0x506a0u,0x506a8u}) {
            space.install_read_tap(address,address+3,"vcop_vr_native_submission_cone",
                [this,address](offs_t,u32& data,u32 mask) {
                    const uint32_t original=data;
                    const bool operand=m_maincpu->pc()==address&&m_maincpu->state_int(I960_PIP)==address-4;
                    if(!operand)return;
                    data=vcop::widened_cull_read(address,data,mask,vcop::requested_camera_fov(),
                        vcop::camera_world_phase([&](uint32_t a){return m_maincpu->space(AS_PROGRAM).read_dword(a);}));
                    if(data!=original&&std::getenv("VCOP_CAPTURE_FOV")) {
                        static std::array<unsigned,6> samples{};
                        const unsigned index=address==0xb8b0?0:address==0xb8b8?1:address==0x320fc?2:address==0x32104?3:address==0x506a0?4:5;
                        if(samples[index]++<2)osd_printf_info("VCOP native cone operand %08x PIP %08x: %08x -> %08x\\n",address,unsigned(m_maincpu->state_int(I960_PIP)),original,data);
                    }
                });
        }
        for(unsigned player=0;player<2;player++) {
            const uint32_t address=player?0x5b1f08:0x5b1ea8;
            space.install_write_tap(address,address+3,player?"vcop_p2_accepted_shot":"vcop_p1_accepted_shot",
                [this,player,address](offs_t,u32& data,u32 mask) {
                    const auto state=vcop::read_game_state([&](uint32_t a){return m_maincpu->space(AS_PROGRAM).read_dword(a);});
                    if(vcop::accepted_shot_write(address,data,mask,state,m_maincpu->pc())) {
                        vcop::accepted_shot(player);
                        if(std::getenv("VCOP_CAPTURE_FOV"))osd_printf_info("VCOP accepted shot P%u PC %08x ammo %u\\n",player+1,m_maincpu->pc(),state.players[player].ammo);
                    }
                });
        }
    }''')
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--source',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
 rel='src/mame/sega/model2.cpp';old=driver(subprocess.check_output(['git','-C',str(a.source),'show',f'{COMMIT}:{rel}'],text=True));new=modify(old)
 a.output.write_text(''.join(difflib.unified_diff(old.splitlines(True),new.splitlines(True),fromfile='a/'+rel,tofile='b/'+rel)),encoding='utf8',newline='\n')
