#include "../native/settings_io.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
using namespace vcop;
static void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
static void captured(const Settings& s) {
    require(s.gun==0,"Captured Arcade gun must be the default");
    require(s.reticle_mode==ReticleMode::Custom&&s.reticle_min_size==.20f&&s.reticle_max_size==.35f&&s.reticle_shrink_distance==7.f,"Captured Custom20%/35%/7m must be defaults");
    require(s.p1_hand==0&&s.reload==ReloadMode::ShootOutside&&s.laser,"Captured hand/reload/laser defaults");
    require(s.world_scale==1.f&&s.panel_distance==1.8f&&s.hud_size==1.f&&s.gun_angle==0&&s.gun_scale==1.f&&s.haptics==.65f,"Captured view/gun/haptics defaults");
    require(s.wider_view&&s.view_fov==110.f&&!s.game_zoom,"Captured Wider/110 and session zoom Off");
}
int main() {
    captured(Settings{});
    std::istringstream empty("");captured(read_settings(empty));
    for(const char* bad:{"nan","0","20.1","1junk","inf"}) {
        std::istringstream in(std::string("version=4\nreticle_shrink_distance=")+bad+"\nreticle_min_size=-1\nreticle_max_size=99\ngun=99\n");captured(read_settings(in));
    }
    std::istringstream explicit_file("version=4\ngun_angle_zero_version=1\ngun=8\nreticle_mode=2\nreticle_min_size=0.4\nreticle_max_size=2.15\nreticle_shrink_distance=9.6\nlaser=0\np1_hand=1\nworld_scale=0.75\n");
    const auto chosen=read_settings(explicit_file);std::ostringstream saved;write_settings(saved,chosen);std::istringstream again(saved.str());const auto reloaded=read_settings(again);
    require(reloaded.gun==8&&reloaded.reticle_mode==ReticleMode::None&&reloaded.reticle_min_size==.4f&&reloaded.reticle_max_size==2.15f&&reloaded.reticle_shrink_distance==9.6f&&!reloaded.laser&&reloaded.p1_hand==1&&reloaded.world_scale==.75f,"Explicit saved choices must survive new defaults and restart");
    std::cout<<"Captured Quest defaults, empty/invalid preference fallback and explicit-choice roundtrip pass\n";
}
