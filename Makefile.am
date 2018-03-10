PREFIX = /usr
AM_CPPFLAGS = -I$(PREFIX)/include/sai

bin_PROGRAMS = sai-sample-packet

sai_sample_packet_SOURCES = sample-packet.cpp
sai_sample_packet_CPPFLAGS = $(AM_CPPFLAGS)
sai_sample_packet_LDADD = -lsai
