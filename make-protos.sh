#!/bin/sh -e

cproto -I/usr/local/include *.c > protos.h
