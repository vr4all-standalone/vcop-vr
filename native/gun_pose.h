// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <optional>
#include <array>
namespace vcop {
// Displayed zero is the former -10 degree pose. The caller supplies the
// existing palm/aim pivot; every gun, muzzle, menu ray and native aim shares it.
inline glm::mat4 pitched_gun_pose(const glm::mat4& pose,float displayed_angle) {
    const float angle=std::isfinite(displayed_angle)?std::clamp(displayed_angle,-30.f,30.f):0.f;
    return pose*glm::rotate(glm::mat4(1),glm::radians(angle-10.f),glm::vec3(1,0,0));
}

// Original VC1 authored hand/handle centroids in source metres. Standard
// pistol uses the measured native hand volume (wrist analytically capped for
// measurement only); pickup grips use actual authored handle side profiles.
inline glm::vec3 native_gun_grip(unsigned weapon) {
 static const std::array<glm::vec3,7> grips{{{},{0.084325479f,-0.000847808404f,0.00994625859f},{-0.141510613f,-0.0984669193f,0},{-0.0715872317f,-0.0409723122f,-0.00100000016f},{-0.209664733f,-0.0291499601f,0},{-0.346704058f,-0.099967091f,0},{-0.0750999453f,-0.0653567605f,0}}};
 return weapon<grips.size()?grips[weapon]:glm::vec3(0);
}
inline glm::vec3 native_gun_muzzle(unsigned weapon) {
 static const std::array<glm::vec3,7> muzzles{{{},{0.386000007f,0.0960000008f,-0.0175000009f},{0.460999995f,0.02f,0},{0.148000002f,0.0465000011f,-0.00100000016f},{0.335999995f,0.0329999998f,0.000499999616f},{0.449000001f,0.0214999993f,0},{0.171000004f,0.0382844898f,0}}};
 return weapon<muzzles.size()?muzzles[weapon]:glm::vec3(0);
}
// The aim origin is above/in front of the palm. Keep its barrel orientation,
// but anchor the model to the runtime's physical grip position.
inline glm::mat4 gun_pose(glm::mat4 aim,const std::optional<glm::vec3>& grip) {
    if(grip)aim[3]=glm::vec4(*grip,1);
    return aim;
}
inline glm::mat4 gun_model_transform(const glm::mat4& pose,float scale,int hand,const glm::mat4& placement) {
    // Reflect in controller space AFTER aligning the authored grip and barrel.
    // This swaps the embedded right hand without reversing the barrel.
    return pose*glm::scale(glm::mat4(1),glm::vec3(hand==0?-scale:scale,scale,scale))*placement;
}
}
