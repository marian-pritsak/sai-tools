PREFIX = /usr
AM_CPPFLAGS = -I$(PREFIX)/include/sai

bin_PROGRAMS = \
	       sai-sample-packet \
	       sai-ip-vxlan

sai_sample_packet_SOURCES = sample-packet.cpp
sai_sample_packet_CPPFLAGS = $(AM_CPPFLAGS)
sai_sample_packet_LDADD = -lsai

sai_ip_vxlan_SOURCES = ip-vxlan.cpp
sai_ip_vxlan_CPPFLAGS = $(AM_CPPFLAGS)
sai_ip_vxlan_LDADD = -lsai
