# -*- tcl -*-
# Tcl package index file, version 1.1
#
if {[package vsatisfies [package provide Tcl] 9.0-]} {
    package ifneeded tclmixer 2.0.1 \
	    [list load [file join $dir libtcl9tclmixer2.0.1.so] [string totitle tclmixer]]
} else {
    package ifneeded tclmixer 2.0.1 \
	    [list load [file join $dir libtclmixer2.0.1.so] [string totitle tclmixer]]
}
