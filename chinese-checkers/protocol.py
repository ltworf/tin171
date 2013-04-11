# -*- coding: utf-8 -*-

# Copyright (C) 2012  Salvo "LtWorf" Tomaselli
# 
# This is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
# 
# author Salvo "LtWorf" Tomaselli <tiposchi@tiscali.it>

def login_message():
    import os
    import socket

    username = "%s.%d@%s" % (os.getenv("USER"),os.getpid(),socket.gethostname())
    
    #TODO username could be something else
    
    return "{login,\"%s\"}.\n" % username

def host_game_message(name):
    return "{host_game,\"%s\"}.\n" % name
    
def spectate_game(name):
    return "{spectate,\"%s\"}.\n" % name
    
def join_game(name):
    return "{join_game,\"%s\"}.\n" % name

def start_game():
    return "start_game.\n"

def get_gui_board(b):
    b=b.replace('#','9').replace(' ','0')
    r=[]
    for i in b:
        r.append(int(i))
    return r

def move(l):
    return "{move,%s}.\n" % repr(l).replace(' ','')

def list_games():
    return "list_games.\n"