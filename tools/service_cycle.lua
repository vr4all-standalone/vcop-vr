-- Original cabinet inputs only: no game RAM writes, patches or skipped boot checks.
local root=assert(os.getenv('VCOP_RESEARCH_DIR'))
local mode=os.getenv('VCOP_DIAGNOSTIC_MODE') or 'attract'
local log=assert(io.open(root..'/events.txt','w'))
local machine=manager.machine
local ports=machine.ioport.ports
for tag,port in pairs(ports) do
    for name,field in pairs(port.fields) do log:write(string.format('PORT %s | %s | %x\n',tag,name,field.mask)) end
end
log:flush()
local function set(tag,mask,value)
    local port=assert(ports[tag],'Missing port '..tag)
    for _,field in pairs(port.fields) do if field.mask==mask then field:set_value(value);return end end
    error('Missing field '..tag..' '..mask)
end
local function press(tag,mask,enabled) set(tag,mask,enabled and 1 or 0) end
local previous=-1
vcop_diagnostic_subscription=emu.add_machine_frame_notifier(function()
    local t=machine.time:as_double();local second=math.floor(t)
    if second~=previous then
        previous=second
        log:write(string.format('TIME %.6f IP %08x IN0 %02x IN1 %02x\n',t,machine.devices[':maincpu'].state['ip'].value,ports[':IN0']:read(),ports[':IN1']:read()));log:flush()
        if second==1 or second==5 or second%5==0 then
            machine.screens[":screen"]:snapshot(root..string.format('/frame-%03d.png',second))
        end
        local space=machine.devices[':maincpu'].spaces.program
        log:write(string.format('STATE mode%u sub%u service%u players%u/%u ammo%u/%u lives%u/%u weapon%u/%u\n',space:read_u32(0x5016cc),space:read_u32(0x5016d4),space:read_u8(0x559330),space:read_u32(0x50ee30),space:read_u32(0x50ee34),space:read_u32(0x50ee38),space:read_u32(0x50ee3c),space:read_u32(0x50ee70),space:read_u32(0x50ee74),space:read_u32(0x50ee88),space:read_u32(0x50ee8c)));log:flush()
        log:write(string.format('SERVICE menu%u cursor%u busy%u\n',space:read_u8(0x559365),space:read_u8(0x55936e),space:read_u32(0x559820)));log:flush()
        if second==5 or second==15 or second==25 or second==30 or second==35 or second==45 then
            local out=assert(io.open(root..string.format('/workram-%03d.bin',second),'wb'))
            out:write(machine.devices[':maincpu'].spaces.program:read_range(0x500000,0x5fffff,8));out:close()
            local backup=assert(io.open(root..string.format('/backup-%03d.bin',second),'wb'));backup:write(space:read_range(0x1d00000,0x1d007ff,8));backup:close()
        end
    end
    if mode=='gameplay' then
        press(':IN0',0x01,t>=16 and t<16.2)
        press(':IN0',0x10,t>=16.5 and t<16.8)
        press(':IN0',0x02,t>=30 and t<30.2)
        press(':IN0',0x20,t>=30.5 and t<30.8)
        set(':P1_X',0x3ff,380);set(':P1_Y',0x3ff,230)
        set(':P2_X',0x3ff,377);set(':P2_Y',0x3ff,230)
        press(':IN1',0x01,t>=24 and math.floor(t*4)%2==0)
        press(':IN1',0x02,t>=32 and math.floor(t*4)%2==0)
    elseif mode=='service' then
        -- Native prompt: Service advances the row; Test selects it.
        -- Nine Service presses from default EXIT select GUN SETTING.
        local advance=((t>=12 and t<18) or (t>=34 and t<41) or (t>=45 and t<50)) and (t-math.floor(t))<.2
        advance=advance or (t>=25 and t<25.2)
        press(':IN0',0x04,(t>=6 and t<6.2) or (t>=20 and t<20.2) or (t>=27 and t<27.2) or (t>=31 and t<31.2) or (t>=43 and t<43.2) or (t>=52 and t<52.2))
        press(':IN0',0x08,advance)
        if t>=28 and t<31 then set(':P1_X',0x3ff,200);set(':P1_Y',0x3ff,100);set(':P2_X',0x3ff,550);set(':P2_Y',0x3ff,350) end
    end
end)
