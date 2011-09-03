/*----------------------------------------------------------------------------
    TAPESTREA: Techniques And Paradigms for Expressive Synthesis,
               Transformation, and Rendering of Environmental Audio
      Engine and User Interface

    Copyright (c) 2006 Ananya Misra, Perry R. Cook, and Ge Wang.
      http://taps.cs.princeton.edu/
      http://soundlab.cs.princeton.edu/

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
    U.S.A.
-----------------------------------------------------------------------------*/

//-----------------------------------------------------------------------------
// name: ui_filesave.h
// desc: utility methods for saving templates
//
// authors: Ananya Misra (amisra@cs.princeton.edu)
//          Ge Wang (gewang@cs.princeton.edu)
//          Perry R. Cook (prc@cs.princeton.edu)
//          Tom Lieber (lieber@princeton.edu)
// date: Fall 2008
//-----------------------------------------------------------------------------
#ifndef __UI_FILESAVE_H__
#define __UI_FILESAVE_H__

#include <string>
#include "ui_library.h"
#include "audicle_utils.h"

// checks and gets name for a template to be saved as if one is provided, otherwise returns false
bool save_as_name( std::string & myname, bool tap_file = true, bool wav_file = true );

// saves template to given filename
bool save_to_file( Template * temp, std::string fname, void * anal_info = NULL );

#endif
