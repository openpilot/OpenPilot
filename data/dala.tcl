# 
# Copyright (C) 1996-2001 LAAS/CNRS 
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
package provide Gdhe::models 1.0

proc atrvSonar {} {
    # le capteur du bas
    pushMatrix
    color 20 20 20
    box -0.005 0 -0.025 0.005 0.05 0.05
    color 200 200 80
    cylinder 0 0 0 x 0.04 0.04 0.002
    popMatrix
}

proc atrv_roue {} {
    
    color 80 80 80 
    cylinder 0.0 -0.08 0.2 y 0.38 -0.40 0.08 24
    cylinder 0.0 0.0 0.2 y -0.40 0.38 0.08 24
    color 200 200 200 
    cylinder 0.0 0.085 0.2 y 0.23 0.0 0.0 24
    cylinder 0.0 -0.085 0.2 y 0.23 0.0 0.0 24
    color 60 60 60
    cylinder 0.0 -0.11 0.2 y -0.05 0.025

}


proc atrv {} {
    object Atrv {

	cullFace True
	pushMatrix
	translate -0.25 0.33 0
	atrv_roue
	popMatrix
	pushMatrix
	translate -0.25 -0.33 0
	rotate 180 0 0 1
	atrv_roue
	popMatrix
	pushMatrix
	translate 0.25 0.33 0
	atrv_roue
	popMatrix
	pushMatrix
	translate 0.25 -0.33 0
	rotate 180 0 0 1
	atrv_roue
	popMatrix
	
	# Partie inferieure
	color 180 0 0 
	prism 6 0 .45 0 0.34 -0.225 0.12 \
	                -0.34 -0.225 0.12 \
                        -0.42 -0.225 0.20 \
                        -0.42 -0.225 0.38 \
	                 0.42 -0.225 0.38 \
                         0.42 -0.225 0.20
	# bumpers 
	color 60 60 60
	box -0.48 0 0.245 0.04 0.5 0.08
	prism 4 0 0.005 0 -0.46 -0.225 0.245 \
	                 -0.42 -0.225 0.20 \
	                 -0.42 -0.225 0.36 \
                         -0.46 -0.225 0.325
	prism 4 0 0.005 0 -0.46 0.22 0.245 \
	                 -0.42 0.22 0.20 \
	                 -0.42 0.22 0.36 \
                         -0.46 0.22 0.325

	box 0.48 0 0.245 0.04 0.5 0.08
	prism 4 0 0.005 0 0.46 -0.225 0.245 \
	                 0.42 -0.225 0.20 \
	                 0.42 -0.225 0.36 \
                         0.46 -0.225 0.325
	prism 4 0 0.005 0 0.46 0.22 0.245 \
	                 0.42 0.22 0.20 \
	                 0.42 0.22 0.36 \
                         0.46 0.22 0.325

	# Support intermediaire
	color 60 60 60
	box 0 0 0.38 0.84 0.45 0.01
	color 0 0 0
	box 0 0 0.39 0.838 0.448 0.001
	color 180 0 0
	#partie avant avec evidement pour les sonars
	box 0.4195 -0.215 0.40 0.001 0.02 0.045
	box 0.4195 0.215 0.40 0.001 0.02 0.045
	box 0.4195 0 0.40 0.001 0.09 0.045
	box 0.4194 0 0.39 0.001 0.45 0.01
	box 0.4194 0 0.445 0.001 0.45 0.015
	# parties laterales et arriere avec evidement pour les sonars
	prism 8 0 0 0.01  0.419  0.225 0.39 \
	                 -0.420  0.225 0.39 \
	                 -0.420 -0.225 0.39 \
                          0.419 -0.225 0.39 \
                          0.419 -0.224 0.39 \
                         -0.419 -0.224 0.39 \
	                 -0.419  0.224 0.39 \
                          0.419  0.224 0.39
	prism 8 0 0 0.015  0.419  0.225 0.445 \
	                  -0.420  0.225 0.445 \
	                  -0.420 -0.225 0.445 \
                           0.419 -0.225 0.445 \
                           0.419 -0.224 0.445 \
                          -0.419 -0.224 0.445 \
	                  -0.419  0.224 0.445 \
                           0.419  0.224 0.445
	box 0.3875 0.2245 0.39  0.065 0.001 0.07
	box -0.3875 0.2245 0.39  0.065 0.001 0.07
	box 0.3875 -0.2245 0.39  0.065 0.001 0.07
	box -0.3875 -0.2245 0.39  0.065 0.001 0.07
	box -0.4195 -0.2125 0.40 0.001 0.025 0.045
	box -0.4195 0.2125 0.40 0.001 0.025 0.045
	box 0 -0.2245 0.40 0.61 0.001 0.045
	box 0 0.2245 0.40 0.61 0.001 0.045
	box -0.4195 0 0.40 0.001 0.30 0.045
	# partie superieure
	prism 4 0.71 0 0 -0.42 -0.225 0.46 \
	                 -0.42 -0.19 0.60 \
	                 -0.42 0.19 0.60 \
	                 -0.42 0.225 0.46 
	prism 3 0 0.002 0 0.29 -0.225 0.46 \
	                  0.42 -0.225 0.46 \
	                  0.29 -0.19 0.60
	prism 3 0 0.002 0 0.29 0.2248 0.46 \
	                  0.42 0.2248 0.46 \
	                  0.29 0.188 0.60
	color 60 60 60
	prism 4 0.01 0 0 -0.43 -0.225 0.46 \
	                 -0.43 -0.19 0.60 \
	                 -0.43 0.19 0.60 \
	                 -0.43 0.225 0.46 
	prism 4 0 0.005 0 -0.42 -0.11 0.60 \
	                  -0.07 -0.11 0.60 \
	                  -0.13 -0.11 0.68 \
	                  -0.42 -0.11 0.68
	prism 4 0 0.005 0 -0.42 0.105 0.60 \
	                  -0.07 0.105 0.60 \
	                  -0.13 0.105 0.68 \
	                  -0.42 0.105 0.68
	prism 4 0.04 0 -0.01 -0.40 -0.105 0.64 \
	                    -0.40 0.105 0.64 \
	                    -0.35 0.105 0.78 \
	                    -0.35 -0.105 0.78 
	# sick
	color 20 20 200
	box .33 .0 .41 .15 .15 .07
	box .28 .0 .48 .05 .15 .08
	box .33 .0 .56 .15 .15 .02
	color 20 20 20
	box .345 .0 .48 .08 .10 .08
	
	# Support platine
	color 60 60 60
	box .11 0 0.60 .32 .24 0.005
	cylinder 0.24 -0.06 0.605 z -0.03 0.25
	cylinder 0.24 0.06 0.605 z -0.03 0.25
	cylinder -0.02 0 0.605 z -0.03 0.25
	prism 4 0 0 0.005 -0.04 -0.02 0.855 \
	                 -0.04  0.02 0.855 \
                          0.26  0.08 0.855 \
                          0.26 -0.08 0.855

	# tableau des positions des capteurs ultrasons
	set posSonars {
	    { { -0.41 0.175 0.425 } { 180 0 0 1 } }
	    { { -0.33 0.215 0.425 } {  90 0 0 1} }
	    { { -0.33 -0.215 0.425 } { -90 0 0 1 } }
	    { { -0.41 -0.175 0.425 } { 180 0 0 1 } }
	    { { 0.41 0.07 0.425 } { 0 1 0 0 } }
	    { { 0.41 0.115 0.425 } { 15 0 0 1  } }
	    { { 0.41 0.17 0.425 } { 30 0 0 1 } }
	    { { 0.33 0.215 0.425 } { 90 0 0 1 } }
	    { { 0.41 -0.07 0.425} { 0 1 0 0 } }
	    { { 0.41 -0.115 0.425 } { -15 0 0 1 } }
	    { { 0.41 -0.17 0.425 } { -30 0 0 1 } }
	    { { 0.33 -0.215 0.425 } { -90 0 0 1 } }
	}
	foreach position $posSonars {
	    pushMatrix
	    eval translate [ lindex $position 0 ]
	    eval rotate [ lindex $position 1 ]
	    atrvSonar
	    popMatrix
	}


    }
    dala_platform 0 -30
}

#----------------------------------------------------------------------
### 
### platine
###
proc dala_platform {azi site} {
    pushMatrix
    translate 0.22 0 0.86
    object h2PlatineInf {
        color 30 30 30
        box -0.02 0.0 0 0.08 0.05 0.05
        color 180 180 180
        cylinder -0.075 0 0.025 x 0.012 0.02
    }
    rotate $azi 0 0 1
    object h2PlatineMid {
        color 30 30 30
        box -0.02 0 0.051 0.08 0.05 0.05
        color 180 180 180
        cylinder -0.075 0 0.075 x 0.012 0.02
        translate 0 0 0.075
    }
    rotate $site 0 -1 0
    object h2PlatineSup {
        color 30 30 30
        box 0 -0.028 -0.01 0.02 0.005 0.05
        box 0  0.028 -0.01 0.02 0.005 0.05
        color 100 100 100
        box 0 0 0.04 0.02 0.2 0.005
        color 20 20 20
        box -0.013 0.07 0.045 0.053 0.045 0.035
        color 70 70 70
        cylinder 0.01 0.07 0.065 x 0.028 0.01
        cylinder 0.02 0.07 0.065 x 0.028 0.05 0.02
        cylinder 0.04 0.07 0.065 x 0.05 0.008
        color 20 20 20
        box -0.013 -0.07 0.045 0.053 0.045 0.035
        color 70 70 70
        cylinder 0.01 -0.07 0.065 x 0.028 0.01
        cylinder 0.02 -0.07 0.065 x 0.028 0.05 0.02
        cylinder 0.04 -0.07 0.065 x 0.05 0.008
    }
    popMatrix
}

