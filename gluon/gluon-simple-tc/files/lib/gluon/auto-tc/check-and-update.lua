#!/usr/bin/lua

site = require 'gluon.site_config'
c=site.simple_tc.mesh_vpn.autotest

local meshvpn_name = "mesh_vpn"
local uci = require('luci.model.uci').cursor()
interface=uci:get("gluon-simple-tc", meshvpn_name, "ifname")
limit_in=uci:get("gluon-simple-tc", meshvpn_name, "limit_ingress")
limit_up=uci:get("gluon-simple-tc", meshvpn_name, "limit_egress")
last_test=uci:get("gluon-simple-tc", meshvpn_name, "last_autotest")

for i=1,18 do	-- give ntp some time to work
	now=os.time()
	if (now >1400000000) then -- ntp ok, so save timestamp to avoid measuring too often
		uci:section('gluon-simple-tc', 'interface', "mesh_vpn", { last_autotest = now } )
		uci:save('gluon-simple-tc')
		uci:commit('gluon-simple-tc')
		if (not last_test) then last_test=now end
		rebootstoofast= ( (now - last_test) < 7*86400 )
		break
	else
		os.execute("sleep 10")
	end
end

math.randomseed (os.time())

maxtries=4
maxdown=400
maxup=100
delay=9600

success_down, success_up = 0, 0

-- if no own server -> use fallbacks
if (not c.iperf_own) then c.iperf_own=#c.iperf_server end
if (not c.download_own) then c.iperf_own=#c.download_links end

for round=1,9 do
	iserver, downloadserver = math.random(c.iperf_own), math.random(c.download_own)	-- beginn with own servers
	io.write(round.." ("..os.date("%H:%M %m-%d").."): ")
	
	if limit_in-1 < 99 and #c.download_links then
		for try=1,maxtries do
			time=io.open("/proc/uptime"):read('*n')
			erg=os.execute ("wget -q ".. c.download_links[downloadserver] .. " -O /dev/null ")
			time=io.open("/proc/uptime"):read('*n')-time
			if erg==0 then break else downloadserver=math.random(#c.download_links) end
		end
		if erg==0 then
			speed=math.floor(c.download_filesize/time*8/1000)
			if speed > maxdown then maxdown=speed end
			success_down = success_down + 1
			io.write ("DownServer "..downloadserver..": "..speed.."\t"..maxdown.."\t")
		else
			io.write ("Probleme beim Downloadtest\t\t")
		end
	end

	if limit_up-1 < 99 and #c.iperf_server then
		for try=1,maxtries do
			speed=io.popen("iperf -fk -x SMCV -c ".. c.iperf_server[iserver]..'|tail -1|sed "s/ */ /g"|cut -d" " -f8'):read("*n")
			if speed then break else iserver=math.random(#c.iperf_server) end
		end
		if speed then
			if speed > maxup then maxup=speed end
			success_up = success_up + 1
			io.write ("UpServer "..iserver..": "..speed.."\t"..maxup)
		else
			io.write ("Probleme beim Uploadtest")
		end
	end

	os.execute("gluon-simple-tc "..interface.." "..math.floor(maxdown*limit_in/100).." "..math.floor(maxup*limit_up/100))

	-- if we have had some successfull tries, but the router rebooted too fast
	if rebootstoofast and (success_up>2) and (success_down>2) then
		io.write("\nNote: Router rebooted after less than a week uptime, switching to slower adaption")
		delay=delay+86400
		rebootstoofast=false
	end
	io.write "\n-----\n"
	io.flush()
	os.execute("sleep "..delay)
end

