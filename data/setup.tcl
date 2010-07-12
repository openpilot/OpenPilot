# 
# Copyright (C) 1996-2006 LAAS/CNRS 
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 
#    - Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#    - Redistributions in binary form must reproduce the above
#      copyright notice, this list of conditions and the following
#      disclaimer in the documentation and/or other materials provided
#      with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
# 
# $LAAS$
#----------------------------------------------------------------------
package provide Gdhe::setup 1.0

# Load default lights definition
package require Gdhe::lights

global gdheNoControlPanel

###
### setup - general interface setup
###
proc setup {} {
    global obsDist obsElev obsAzi obsX obsY obsZ
	global realObsDist realObsX realObsY
    global xmin ymin zmin xmax ymax zmax
    global polygon_mode perspective_mode
    global gdheBase gdheNoControlPanel
    global gdheToglWidth gdheToglHeight

    # gdheBase is the widget in which the main window is
    # displayed. Defaults to ".".
    if {![info exists gdheBase]} { 
	set gdheBase ""
	wm title . "GDHE"
	wm geometry . +0+0
	wm deiconify .
    } elseif { "$gdheBase" != "" } {
	if {![winfo exists $gdheBase]} {
	    error "No such widget: $gdheBase"
	}
    }

    # default value for togl widget size
    if {![info exists gdheToglWidth]} {
        set gdheToglWidth 640
    }
    if {![info exists gdheToglHeight]} {
        set gdheToglHeight 480
    }

    # main window containing the OpenGL widget
    frame $gdheBase.gdhe
    
    set w $gdheBase.gdhe.ogl
    addWindow $w
    togl $w -width $gdheToglWidth -height $gdheToglHeight \
        -privatecmap true -rgba true  \
	-double true -depth true -ident main
    setLights

    # geometry manager for main window
    pack $w  -side left -padx 3 -pady 3 -fill both -expand t
    pack $gdheBase.gdhe  -fill both -expand t

    # biggest possible distance in current world
    set maxdist [expr hypot($ymax - $ymin, $xmax - $xmin)*2.0]

    # Initialise observer's position from Togl's internal values
    set pos [$w setObs]
    set obsDist [lindex $pos 0]
    set obsElev [lindex $pos 1]
    set obsAzi [lindex $pos 2]
    set obsX [lindex $pos 3]
    set obsY [lindex $pos 4]
    set obsZ [lindex $pos 5]

    # trace writes to the Tcl variables that represent the position
    trace add variable obsDist write "traceObs $w"
    trace add variable obsElev write "traceObs $w"
    trace add variable obsAzi write "traceObs $w"
    trace add variable obsX write "traceObs $w"
    trace add variable obsY write "traceObs $w"
    trace add variable obsZ write "traceObs $w"

    # Control window
    if {![info exists gdheNoControlPanel]} {
	toplevel $w.pos 
	wm title $w.pos "Position"
	# wm group $w.pos "$gdheBase."
	wm geometry $w.pos -0+0

	# distance between the observer and the scene
	scale  $w.pos.dist  -length 200  -label {Distance} \
	    -from 0 -to $maxdist \
	    -resolution 0.1 \
	    -variable "::obsDist" \
	    -orient horizontal
	# Elevation
	scale  $w.pos.elev   -label {Elevation} -from -180 -to 180 \
	    -variable "::obsElev" \
	    -orient horizontal
	# Azimut
	scale $w.pos.azi -label {Azimut} -from -180 -to 180 \
	    -variable "::obsAzi" \
	    -orient horizontal
	# X Position  
	scale $w.pos.x -label {Target X} -from $xmin -to $xmax \
	    -resolution 0.01\
	    -variable "::obsX" \
	    -orient horizontal
	# Y Position
	scale $w.pos.y -label {Target Y} -from $ymin -to $ymax \
	    -resolution 0.01\
	    -variable "::obsY" \
	    -orient horizontal
	# Z Position
	scale $w.pos.z -label {Target Z} -from $zmin -to $zmax \
	    -resolution 0.01\
	    -variable "::obsZ" \
	    -orient horizontal

	label $w.pos.rate -text rate:

	frame $w.pos.props
	checkbutton $w.pos.props.light -text "Lights" \
	    -variable enable_lighting -command " $w redraw"

	set polygonModeMenu \
	    [tk_optionMenu $w.pos.props.fillStyle polygon_mode fill line]
	$w.pos.props.fillStyle configure -width 10
	$polygonModeMenu entryconfigure 0 -command "setPolygonMode $w"
	$polygonModeMenu entryconfigure 1 -command "setPolygonMode $w"
	set perpectiveModeMenu \
	    [ tk_optionMenu $w.pos.props.perspMode perspective_mode \
		  perspective orthogonal]
	$w.pos.props.perspMode configure -width 10
	$perpectiveModeMenu entryconfigure 0 \
	    -command "$w setObs -perspective ; $w redraw true"
	$perpectiveModeMenu entryconfigure 1 \
	    -command "$w setObs -ortho ; $w redraw true"
    

	grid $w.pos.props.light -column 0 -row 0 -columnspan 2
	grid $w.pos.props.fillStyle -column 0 -row 1
	grid $w.pos.props.perspMode -column 1 -row 1

	# Print button
	button $w.pos.print -text "Dump EPS" -command "print $w"

	# Quit button
	button $w.pos.btn  -text Quit -command exit

	# Geometry management
	pack $w.pos.dist  -fill x
	pack $w.pos.elev  -fill x
	pack $w.pos.azi -fill x
	pack $w.pos.x -fill x
	pack $w.pos.y -fill x
	pack $w.pos.z -fill x
	pack $w.pos.rate -fill x
	pack $w.pos.props -fill x
	pack $w.pos.print -fill x
	pack $w.pos.btn -fill x

    }

    # Handler of mouse events in the main window
    bind $w <ButtonPress-1> {
	set oldX %x
	set oldY %y
    }
    bind $w <ButtonPress-2> {
	set oldX %x
	set oldY %y
    }
    bind $w <ButtonPress-3> {
	set oldX %x
	set oldY %y
    }
    bind $w <B1-Motion> [concat "motion_event_b1 $w" { 
	[lindex [%W config -width] 4] \
	    [lindex [%W config -height] 4] \
	    [expr %x - $oldX] [expr %y - $oldY]
	set oldX %x
	set oldY %y
    }]
    bind $w <B2-Motion> [concat "motion_event_b2 $w $maxdist" {
	[lindex [%W config -width] 4] \
		[lindex [%W config -height] 4] \
		[expr %x - $oldX] [expr %y - $oldY]
	set oldX %x
	set oldY %y
    }]
    bind $w <B3-Motion> [concat "motion_event_b3 $w" {
	[lindex [%W config -width] 4] \
		[lindex [%W config -height] 4] \
		[expr %x - $oldX] [expr %y - $oldY]
	set oldX %x
	set oldY %y
    }]

    # Some keyboard mappings too
    focus $w
    bind $w <Key-Up> "move $w z +"
    bind $w <Key-Down> "move $w z -"
    bind $w <Key-Left> "move $w x -"
    bind $w <Key-Right> "move $w x +"
    bind $w <Shift-Key-Left> "move $w y -"
    bind $w <Shift-Key-Right> "move $w y +"

    # load pre-defined models
    load_env $w;
    
}


#----------------------------------------------------------------------

##
## Move the point of attention
##
proc move {w axis sign} {
    upvar \#0 obs[string toupper $axis] pos

    set pos [expr $pos $sign 0.1]
}
	      

#----------------------------------------------------------------------

##
## draw_all - redraw all the scene in a window
##
## called by the Togl widget every time it needs to repaint the window
##
proc draw_all {w} {
    global robots pos platform load trajectory rs_trajectory walls \
	window displayTime transp
    set window $w

    # draw the "robots" (in fact any kind of element)
    foreach {r} [array names robots] {
	if {[info exists pos($r)]} {
	    pushName $r
	    pushMatrix
	    switch [llength $pos($r)] {
		3 {
		    # Three parameters: x y theta
		    set x [lindex $pos($r) 0]
		    set y [lindex $pos($r) 1]
		    set theta [lindex $pos($r) 2]
		    translate $x $y 0
		    rotate $theta 0 0 1
		}
		6 {
		    # Six parameters:  Bryant angles
		    set cap [lindex $pos($r) 0]
		    set roulis [lindex $pos($r) 1]
		    set tangage [lindex $pos($r) 2]
		    set x [lindex $pos($r) 3]
		    set y [lindex $pos($r) 4]
		    set z [lindex $pos($r) 5]
		    translate $x $y $z
		    rotate $cap 0 0 1
		    rotate $roulis 0 1 0
		    rotate $tangage 1 0 0 
		}
		default {
		    puts "r: bizarre llength pos($r) != 3 or 6 [llength $pos($r)]"
		}
	    }
	    eval $robots($r)
	    if {[info exists platform($r)]} {
		eval $platform($r)
	    }
	    if {[info exists load($r)]} {
		eval $load($r)
	    }
	}
	popMatrix
	popName
	# A Reeds-Shepp trajectory (XXX)
	if {[info exists rs_trajectory($r)]} {
	    draw_rs_trajectory $r
	}
    }
    # draw translucent parts last.  (XXX not fully implemented yet)
    foreach {r} [array names transp] {
	if {[info exists pos($r)]} {
	    pushMatrix
	    switch [llength $pos($r)] {
		3 {
		    # Three parameters: X Y theta
		    set x [lindex $pos($r) 0]
		    set y [lindex $pos($r) 1]
		    set theta [lindex $pos($r) 2]
		    translate $x $y 0
		    rotate $theta 0 0 1
		}
		6 {
		    # Six parameters: Bryant angles
		    set cap [lindex $pos($r) 0]
		    set roulis [lindex $pos($r) 1]
		    set tangage [lindex $pos($r) 2]
		    set x [lindex $pos($r) 3]
		    set y [lindex $pos($r) 4]
		    set z [lindex $pos($r) 5]
		    translate $x $y $z
		    rotate $cap 0 0 1
		    rotate $roulis 0 1 0
		    rotate $tangage 1 0 0 
		}
		default {
		    puts "bizarre llength pos($r) != 3 or 6 [llength $pos($r)]"
		}
	    }
	    eval $transp($r)
	}
	popMatrix
    }
    # Display the refresh rate if requested
    if {[winfo exists $w.pos.rate]} {
	if {[info exists displayTime]} {
	    if { $displayTime > 10 } {
		set rate [format "%.1f" [expr 1000.0/$displayTime]]
		$w.pos.rate configure -text "rate: $rate frames/s"
	    } else {
		$w.pos.rate configure -text "display time $displayTime ms"
	    }
	} else {
	    $w.pos.rate configure -text ""
	}
    }
}

#----------------------------------------------------------------------

##
## Update the polygon drawing mode of a window and redraw it
##
proc setPolygonMode {w} {
    global polygon_mode

    $w polygonMode $polygon_mode
    $w redraw t
}

#----------------------------------------------------------------------

##
## motion_event - Handler of events in the OpenGL window
##    left button changes azimut and elevation of the observer
##     middle button changes the distance of the observer 
##    right button changes the x and y coordinates of the target point

# left button
proc motion_event_b1 { w width height dx dy } { 
    global obsElev obsAzi

    # compute new  azi and elev
    set elev [expr $obsElev + 180.0 * $dy / $height ]
    if { $elev > 180 } { set elev [expr $elev - 360] }
    if { $elev < -180 } { set elev [expr $elev + 360] }
    set obsElev $elev

    set azi [expr $obsAzi + 360.0 * $dx/$width ]
    if { $azi > 180 } { set azi [expr $azi - 360] }
    if { $azi < -180 } { set azi [expr $azi + 360] }

    # update sliders & view
    set obsAzi $azi

}

# middle button
proc motion_event_b2 { w maxdist width height dx dy } { 
    global obsDist
	global realObsDist

    # compute new distance
	#<<<
#    set realObsDist [expr $obsDist + $maxdist * $dy / $height ]
	#===
	# FIXME use variables for resolution 0.1 (with setup())
	if {![info exists realObsDist]} { set realObsDist $obsDist } else {
	if { abs($realObsDist - $obsDist) > 0.1 } { set realObsDist $obsDist } }
    set realObsDist [expr $realObsDist * pow(1.01,$dy) ]
	#>>>
    if { $realObsDist > $maxdist } { set realObsDist $maxdist }
    if { $realObsDist < 1e-3 } { set realObsDist 1e-3 }

    # update sliders & view
    set obsDist $realObsDist
# FIXME BUG sometimes obsDist get rounded to the scale's ticks, sometimes not (and we would like never)
# it seems to depend on the range, not the time, eg < 0.25 no problem, after it jumps at 0.3, etc...
#puts "motion_event_b2: set obsDist $obsDist realObsDist $realObsDist"
}

# Right button
proc motion_event_b3 { w width height dx dy } { 
    global obsDist obsElev obsAzi obsX obsY obsZ
	global distCoeff realObsX realObsY realObsZ
    global xmin ymin zmin xmax ymax zmax

    # compute new position
    set azimuth [expr $obsAzi * 3.1415926 / 180]
    set elev [expr $obsElev * 3.1415926 / 180]

	#<<<
#	set gwidth [expr $xmax - $xmin]
#	set gheight [expr $ymax - $ymin]
	#===
	if {![info exists distCoeff]} { 
		#FIXME get it from [lindex $pos 6] in setup()
		set obsFovy 60
		set distCoeff [expr 2 * tan($obsFovy/2 * 3.1415926 / 180)] }
	# FIXME use variables for resolution 0.01 (with setup())
	if {![info exists realObsX]} { set realObsX $obsX } else {
	if { abs($realObsX - $obsX) > 0.01 } { set realObsX $obsX } }
	if {![info exists realObsY]} { set realObsY $obsY } else {
	if { abs($realObsY - $obsY) > 0.01 } { set realObsY $obsY } }
	if {![info exists realObsZ]} { set realObsZ $obsZ } else {
	if { abs($realObsZ - $obsZ) > 0.01 } { set realObsZ $obsZ } }
	set gwidth [expr $distCoeff * $obsDist]
	set gheight [expr $gwidth * $height/$width]
	#>>>

    set realObsX [expr $realObsX - $gwidth * $dx * cos( $azimuth) / $width ]
    set realObsY [expr $realObsY + $gwidth * $dx * sin( $azimuth) / $width ]

	#<<<
#    set realObsX [expr $realObsX  + $gheight * $dy * sin( $azimuth) / $height ]
#    set realObsY [expr $realObsY  + $gheight * $dy * cos( $azimuth) / $height ]
	#===
    set realObsX [expr $realObsX  + $gheight * $dy * sin($azimuth) * sin($elev) / $height ]
    set realObsY [expr $realObsY  + $gheight * $dy * cos($azimuth) * sin($elev) / $height ]
    set realObsZ [expr $realObsZ  + $gheight * $dy * cos($elev) / $height ]
	#>>>


    if { $realObsX > $xmax } { set realObsX $xmax }
    if { $realObsX < $xmin } { set realObsX $xmin }
    if { $realObsY > $ymax } { set realObsY $ymax }
    if { $realObsY < $ymin } { set realObsY $ymin }
    if { $realObsZ > $zmax } { set realObsZ $zmax }
    if { $realObsZ < $zmin } { set realObsZ $zmin }

    # update sliders & view
    set obsX $realObsX
    set obsY $realObsY
    set obsZ $realObsZ

}

#----------------------------------------------------------------------

##
## traceObs - handler of modification to observer variables
##
proc traceObs {w name index op} {
    # get the value of the modified variable in $var
    upvar \#0 $name var 

    # Get the argument to 'setObs' corresponding to the modified variable
    # ie 'dist' for 'obsDist' or '::obsDist'
    set arg [string tolower [string range [string trimleft $name :] 3 end]]

    # update observer
    $w setObs -$arg $var

    # redraw
    $w redraw
}
	
#----------------------------------------------------------------------
###
### load other files
###
proc load_env {w} {
    
    # objects models
    catch {
	package require Gdhe::models
    } result

    # lighting data
    package require Gdhe::lights
    Gdhe::lights::setLights
}

#----------------------------------------------------------------------
###
### Resize world
###
proc env_size {win xm ym zm xM yM zM} {
    global xmin xmax ymin ymax zmin zmax

    set xmin $xm
    set ymin $ym
    set zmin $zm
    set xmax $xM
    set ymax $yM
    set zmax $zM

    $win.pos.x configure -from $xmin -to $xmax
    $win.pos.y configure -from $ymin -to $ymax
    $win.pos.z configure -from $zmin -to $zmax

    $win.pos.dist configure -from 0 \
	-to [expr hypot($ymax - $ymin, $xmax - $xmin)*2.0]
}

#----------------------------------------------------------------------
###
### Definition of GDHE objects
###
### This function uses a global variable called "objects"
### to store the list of defined objects with the associated 
### display list.
###
proc object {name def} {
    global objects

    # Is this object already defined ? 
    if {![info exists objects($name)]} {
	# Allocate a new display list
	set objects($name) [genLists 1]
	# Create the display list
	newList $objects($name)
	# Evaluate the definition of the object in the caller's context
	uplevel 1 $def
	# Close the display list
	endList 
    } else {
	# Already defined object just call the display list.
	callList $objects($name)
    }
}

#----------------------------------------------------------------------
##
## Destruction of an objet
##
proc objectDelete {name} {
    global objects

    if { [info exists objects($name)]} {
	# Free the display list
	deleteLists $objects($name)
	# Free the reference in the object array
	unset objects($name)
    }
}
#----------------------------------------------------------------------
###
### Management of the list of windows
###

#----------------------------------------------------------------------
##
## Add a window to the list
##
proc addWindow {w} {
    global allWindows

    if {[info exists allWindows]} {
	set allWindows [concat $allWindows $w]
    } else {
	set allWindows $w
    }
}

#----------------------------------------------------------------------
##
## Redraw all existing windows
##
## f: true forces an immediate redisplay
##    otherwise wait until main loop is idle
##
proc redrawAllWindows {{f false}} {
    global allWindows

    foreach w $allWindows {
	$w redraw $f
    }    
}

#----------------------------------------------------------------------
##
## Printing dialog
##
proc print {w} {
    set types {
	{{EPS Files} {.eps}}
	{{All Files} *}
    }
    set filename [tk_getSaveFile -title "Save EPS file" -filetypes $types]
    if {$filename != ""} {
	$w dumpEps -color $filename
    }
}

#----------------------------------------------------------------------
###
### This procedure is called by the GDHE server to check if
### a new client can connect. the server passes the IP address
### and/or the hostname of the client, if available.
### 
### This function uses a simple allow/deny scheme. It can be redefined
### to create more sophisticated policies if needed. 
###
proc clientAuthorize {ip name} {
    global allowedClients deniedClients

    foreach c $allowedClients {
	if {[regexp $c $ip] || [regexp $c $name]} {
	    return 
	}
    }
    foreach c $deniedClients {
	if {[regexp $c $ip] || [regexp $c $name]} {
	    error "Connection refused from $ip"
	}
    }
    error "Connection refused from $ip"
}

## Allow access from specicied hosts (regexp)
proc allow {exp} {
    global allowedClients
    
    lappend allowedClients $exp
}

## Deny access from specified hosts (regexp)
proc deny {exp} {
    global deniedClients

    lappend deniedClients $exp
}

#----------------------------------------------------------------------
###
### Startup code
###
  
set polygonMode fill  

#----------------------------------------------------------------------
#  Access control
# 
# Allow rules are evaluated first
#
allow "^127\.0\.0\.1"
allow "^::1$"
allow "^::ffff:127\.0\.0\.1$"

# allow "\.laas\.fr$"
# allow "^140\.93\."
# allow "^195\.83\.132\."
# allow "^192\.168\."
deny ".*"
