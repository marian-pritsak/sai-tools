#!/bin/bash

autoreconf --force --install -I m4
automake --add-missing --foreign
