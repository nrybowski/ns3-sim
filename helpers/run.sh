#! /bin/bash

FRR_HOME=/data/frr
DCE_HOME=/home/ns3dce/dce-linux-dev/source/ns-3-dce
CURDIR=$(pwd)

cd "${DCE_HOME}"
rm -rf files-*
#install -m 775 -o ns3dce -g ns3dce -d files-0/var/tmp
#install -m 775 -o ns3dce -g ns3dce -d files-0/var/run/frr
#install -m 775 -o ns3dce -g ns3dce -d files-0/var/log/frr
#install -m 775 -o ns3dce -g ns3dce -d files-0/etc/frr
mkdir -p files-0/var/tmp
mkdir -p files-0/var/run/frr
mkdir -p files-0/var/log/frr
mkdir -p files-0/etc/frr
mkdir -p files-0/usr/share/yang

mkdir -pv files-0/usr/local/etc
cat >> files-0/usr/local/etc/bird.conf <<EOF
# Configure logging
log "/var/log/bird.log" { debug, trace, info, remote, warning, error, auth, fatal, bug };

# Set router ID. It is a unique identification of your router, usually one of
# IPv4 addresses of the router. It is recommended to configure it explicitly.
router id 10.1.1.1;

# The Device protocol is not a real routing protocol. It does not generate any
# routes and it only serves as a module for getting information about network
# interfaces from the kernel. It is necessary in almost any configuration.
protocol device {
}

# The direct protocol is not a real routing protocol. It automatically generates
# direct routes to all network interfaces. Can exist in as many instances as you
# wish if you want to populate multiple routing tables with direct routes.
protocol direct {
	disabled;		# Disable by default
	ipv4;			# Connect to default IPv4 table
	ipv6;			# ... and to default IPv6 table
}

# The Kernel protocol is not a real routing protocol. Instead of communicating
# with other routers in the network, it performs synchronization of BIRD
# routing tables with the OS kernel. One instance per table.
protocol kernel {
	ipv4 {			# Connect protocol to IPv4 table by channel
#	      table master4;	# Default IPv4 table is master4
#	      import all;	# Import to table, default is import all
	      export all;	# Export to protocol. default is export none
	};
#	learn;			# Learn alien routes from the kernel
#	kernel table 10;	# Kernel table to synchronize with (default: main)
}

# Another instance for IPv6, skipping default options
protocol kernel {
	ipv6 { export all; };
}

# Static routes (Again, there can be multiple instances, for different address
# families and to disable/enable various groups of static routes on the fly).
protocol static {
	ipv4;			# Again, IPv4 channel with default options
}

# OSPF example, both OSPFv2 and OSPFv3 are supported
protocol ospf v2 {
	ipv4 {
	  import all;
	  export all;
	};
	area 0 {
		interface "sim*" {
			type broadcast;		# Detected by default
			cost 10;		# Interface metric
			hello 5;		# Default hello perid 10 is too long
		};
		interface "tun*" {
			type ptp;		# PtP mode, avoids DR selection
			cost 100;		# Interface metric
			hello 5;		# Default hello perid 10 is too long
		};
		interface "dummy0" {
			stub;			# Stub interface, just propagate it
		};
	};
}
EOF


#cp /usr/local/etc/bird.conf files-0/usr/local/etc/
mkdir -pv files-0/dev/
ln -s /dev/null files-0/dev/null
#cp /dev/null files-0/dev/null

install -m 640 -o ns3dce -g frrvty "${FRR_HOME}"/tools/etc/frr/vtysh.conf files-0/etc/frr/vtysh.conf
install -m 640 -o ns3dce -g ns3dce "${FRR_HOME}"/tools/etc/frr/frr.conf files-0/etc/frr/frr.conf
install -m 640 -o ns3dce -g ns3dce "${FRR_HOME}"/tools/etc/frr/daemons.conf files-0/etc/frr/daemons.conf
install -m 640 -o ns3dce -g ns3dce "${FRR_HOME}"/tools/etc/frr/daemons files-0/etc/frr/daemons

#cat > files-0/etc/group <<EOF
#root:x:0:
#ns3dce:x:1001:
#frrvty:x:85:frr,ns3dce
#frr:x:92:ns3dce
#EOF

install -m 775 -o ns3dce -g ns3dce -d files-1/var/tmp
install -m 775 -o ns3dce -g ns3dce -d files-1/var/run/frr
install -m 775 -o ns3dce -g ns3dce -d files-1/var/log/frr
install -m 775 -o ns3dce -g ns3dce -d files-1/etc/frr
mkdir -pv files-1/usr/local/etc
#cp /usr/local/etc/bird.conf files-1/usr/local/etc/
cat >> files-1/usr/local/etc/bird.conf <<EOF
# Configure logging
log "/var/log/bird.log" { debug, trace, info, remote, warning, error, auth, fatal, bug };

# Set router ID. It is a unique identification of your router, usually one of
# IPv4 addresses of the router. It is recommended to configure it explicitly.
router id 10.1.1.2;

# The Device protocol is not a real routing protocol. It does not generate any
# routes and it only serves as a module for getting information about network
# interfaces from the kernel. It is necessary in almost any configuration.
protocol device {
}

# The direct protocol is not a real routing protocol. It automatically generates
# direct routes to all network interfaces. Can exist in as many instances as you
# wish if you want to populate multiple routing tables with direct routes.
protocol direct {
	disabled;		# Disable by default
	ipv4;			# Connect to default IPv4 table
	ipv6;			# ... and to default IPv6 table
}

# The Kernel protocol is not a real routing protocol. Instead of communicating
# with other routers in the network, it performs synchronization of BIRD
# routing tables with the OS kernel. One instance per table.
protocol kernel {
	ipv4 {			# Connect protocol to IPv4 table by channel
#	      table master4;	# Default IPv4 table is master4
#	      import all;	# Import to table, default is import all
	      export all;	# Export to protocol. default is export none
	};
#	learn;			# Learn alien routes from the kernel
#	kernel table 10;	# Kernel table to synchronize with (default: main)
}

# Another instance for IPv6, skipping default options
protocol kernel {
	ipv6 { export all; };
}

# Static routes (Again, there can be multiple instances, for different address
# families and to disable/enable various groups of static routes on the fly).
protocol static {
	ipv4;			# Again, IPv4 channel with default options
}

# OSPF example, both OSPFv2 and OSPFv3 are supported
protocol ospf v2 {
        ipv4 {
	  import all;
	  export all;
        };
	area 0 {
		interface "sim*" {
			type broadcast;		# Detected by default
			cost 10;		# Interface metric
			hello 5;		# Default hello perid 10 is too long
		};
		interface "tun*" {
			type ptp;		# PtP mode, avoids DR selection
			cost 100;		# Interface metric
			hello 5;		# Default hello perid 10 is too long
		};
		interface "dummy0" {
			stub;			# Stub interface, just propagate it
		};
	};
}
EOF

mkdir -pv files-1/dev/
ln -s /dev/null files-1/dev/null
#cp /dev/null files-1/dev/null

install -m 775 -o ns3dce -g ns3dce -d files-2/var/tmp
install -m 775 -o ns3dce -g ns3dce -d files-2/var/run/frr
install -m 775 -o ns3dce -g ns3dce -d files-2/var/log/frr
install -m 775 -o ns3dce -g ns3dce -d files-2/etc/frr
mkdir -pv files-2/usr/local/etc
cat >> files-2/usr/local/etc/bird.conf <<EOF
# Configure logging
log "/var/log/bird.log" { debug, trace, info, remote, warning, error, auth, fatal, bug };

# Set router ID. It is a unique identification of your router, usually one of
# IPv4 addresses of the router. It is recommended to configure it explicitly.
router id 192.168.1.1;

# The Device protocol is not a real routing protocol. It does not generate any
# routes and it only serves as a module for getting information about network
# interfaces from the kernel. It is necessary in almost any configuration.
protocol device {
}

# The direct protocol is not a real routing protocol. It automatically generates
# direct routes to all network interfaces. Can exist in as many instances as you
# wish if you want to populate multiple routing tables with direct routes.
protocol direct {
	disabled;		# Disable by default
	ipv4;			# Connect to default IPv4 table
	ipv6;			# ... and to default IPv6 table
}

# The Kernel protocol is not a real routing protocol. Instead of communicating
# with other routers in the network, it performs synchronization of BIRD
# routing tables with the OS kernel. One instance per table.
protocol kernel {
	ipv4 {			# Connect protocol to IPv4 table by channel
#	      table master4;	# Default IPv4 table is master4
#	      import all;	# Import to table, default is import all
	      export all;	# Export to protocol. default is export none
	};
#	learn;			# Learn alien routes from the kernel
#	kernel table 10;	# Kernel table to synchronize with (default: main)
}

# Another instance for IPv6, skipping default options
protocol kernel {
	ipv6 { export all; };
}

# Static routes (Again, there can be multiple instances, for different address
# families and to disable/enable various groups of static routes on the fly).
protocol static {
	ipv4;			# Again, IPv4 channel with default options
}

# OSPF example, both OSPFv2 and OSPFv3 are supported
protocol ospf v2 {
        ipv4 {
	  import all;
	  export all;
        };
	area 0 {
		interface "sim*" {
			type broadcast;		# Detected by default
			cost 10;		# Interface metric
			hello 5;		# Default hello perid 10 is too long
		};
		interface "tun*" {
			type ptp;		# PtP mode, avoids DR selection
			cost 100;		# Interface metric
			hello 5;		# Default hello perid 10 is too long
		};
		interface "dummy0" {
			stub;			# Stub interface, just propagate it
		};
	};
}
EOF
mkdir -pv files-2/dev/
ln -s /dev/null files-2/dev/null

install -m 775 -o ns3dce -g ns3dce -d files-3/var/tmp
install -m 775 -o ns3dce -g ns3dce -d files-3/var/run/frr
install -m 775 -o ns3dce -g ns3dce -d files-3/var/log/frr
install -m 775 -o ns3dce -g ns3dce -d files-3/etc/frr
mkdir -pv files-3/usr/local/etc
cat >> files-3/usr/local/etc/bird.conf <<EOF
# Configure logging
log "/var/log/bird.log" { debug, trace, info, remote, warning, error, auth, fatal, bug };

# Set router ID. It is a unique identification of your router, usually one of
# IPv4 addresses of the router. It is recommended to configure it explicitly.
router id 10.1.2.2;

# The Device protocol is not a real routing protocol. It does not generate any
# routes and it only serves as a module for getting information about network
# interfaces from the kernel. It is necessary in almost any configuration.
protocol device {
}

# The direct protocol is not a real routing protocol. It automatically generates
# direct routes to all network interfaces. Can exist in as many instances as you
# wish if you want to populate multiple routing tables with direct routes.
protocol direct {
	disabled;		# Disable by default
	ipv4;			# Connect to default IPv4 table
	ipv6;			# ... and to default IPv6 table
}

# The Kernel protocol is not a real routing protocol. Instead of communicating
# with other routers in the network, it performs synchronization of BIRD
# routing tables with the OS kernel. One instance per table.
protocol kernel {
	ipv4 {			# Connect protocol to IPv4 table by channel
#	      table master4;	# Default IPv4 table is master4
#	      import all;	# Import to table, default is import all
	      export all;	# Export to protocol. default is export none
	};
#	learn;			# Learn alien routes from the kernel
#	kernel table 10;	# Kernel table to synchronize with (default: main)
}

# Another instance for IPv6, skipping default options
protocol kernel {
	ipv6 { export all; };
}

# Static routes (Again, there can be multiple instances, for different address
# families and to disable/enable various groups of static routes on the fly).
protocol static {
	ipv4;			# Again, IPv4 channel with default options
}

# OSPF example, both OSPFv2 and OSPFv3 are supported
protocol ospf v2 {
        ipv4 {
	  import all;
	  export all;
        };
	area 0 {
		interface "sim*" {
			type broadcast;		# Detected by default
			cost 10;		# Interface metric
			hello 5;		# Default hello perid 10 is too long
		};
		interface "tun*" {
			type ptp;		# PtP mode, avoids DR selection
			cost 100;		# Interface metric
			hello 5;		# Default hello perid 10 is too long
		};
		interface "dummy0" {
			stub;			# Stub interface, just propagate it
		};
	};
}
EOF
mkdir -pv files-3/dev/
ln -s /dev/null files-3/dev/null


install -m 640 -o ns3dce -g frrvty "${FRR_HOME}"/tools/etc/frr/vtysh.conf files-1/etc/frr/vtysh.conf
install -m 640 -o ns3dce -g ns3dce "${FRR_HOME}"/tools/etc/frr/frr.conf files-1/etc/frr/frr.conf
install -m 640 -o ns3dce -g ns3dce "${FRR_HOME}"/tools/etc/frr/daemons.conf files-1/etc/frr/daemons.conf
install -m 640 -o ns3dce -g ns3dce "${FRR_HOME}"/tools/etc/frr/daemons files-1/etc/frr/daemons

#cat > files-1/etc/group <<EOF
#root:x:0:
#ns3dce:x:1001:
#frrvty:x:85:frr,ns3dce
#frr:x:92:ns3dce
#EOF

#export DCE_PATH=/data/frr/zebra/.libs:$DCE_PATH 
export DCE_PATH=/data/bird:/data/tcpdump:$DCE_PATH 
export NS_LOG="Bird=level_all"
#export NS_LOG="*=level_all"
#export DCE_PATH=/home/ns3dce/dce-linux-dev/source/quagga/zebra:$DCE_PATH 
#sudo -E capsh --caps="cap_net_admin,cap_net_raw,cap_sys_admin+p" -- -c "./waf \
#./waf --run dce-zebra-simple \
#./waf --run zebra_test \
./waf --run bird_test \
	--disable-examples \
	--disable-tests \
	#--command-template="gdb --args %s" \
	#--command-template="valgrind --track-origins=yes --leak-check=full %s" \
	#--command-template="strace %s" \
	#--command-template="gdb --args %s"
cat files-*/var/log/*/*
cd "${CURDIR}"


