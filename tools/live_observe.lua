-- Read-only original state observer for live VR input integration.
local root=assert(os.getenv('VCOP_RESEARCH_DIR'))
local machine=manager.machine
local space=machine.devices[':maincpu'].spaces.program
local out=assert(io.open(root..'/native-state.csv','w'))
out:write('time,mode,submode,p1,p2,ammo1,ammo2,accepted_flag1,accepted_flag2,service,gunmenu,busy\n')
local previous=-1
vcop_live_observer=emu.add_machine_frame_notifier(function()
 local t=machine.time:as_double();local tick=math.floor(t*2)
 if tick~=previous then
  previous=tick
  out:write(string.format('%.6f,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n',t,space:read_u32(0x5016cc),space:read_u32(0x5016d4),space:read_u32(0x50ee30),space:read_u32(0x50ee34),space:read_u32(0x50ee38),space:read_u32(0x50ee3c),space:read_u32(0x5b1ea8),space:read_u32(0x5b1f08),space:read_u8(0x559330),space:read_u8(0x559365),space:read_u32(0x559820)));out:flush()
 end
end)
