#!CVSGUI1.0 --folder --name "Change Root"

global numChanged
set numChanged 0
global changeTo
set changeTo ":pserver:alexgui@stratadev.strata3d.com:/cvspub/cvsgui"

proc changeRoot {dirName} {
	set oldDir [pwd]
	cd $dirName

	set fileid [open Root w]
	global changeTo
	puts $fileid $changeTo
	close $fileid
	
	global numChanged
	incr numChanged
	
	cd $oldDir
}

proc iterate {dirName} {
	set oldDir [pwd]
	cd $dirName
	cvsout "Entering $dirName\n"

	set dirList [glob -nocomplain *]
	set dirSize [llength $dirList]
	for {set j 0} {$j < $dirSize} {incr j} {
		set fileName [lindex $dirList $j]
		if {[file isdirectory $fileName]} {
			if {[string compare cvs [string tolower $fileName]] == 0} {
				changeRoot $fileName
			} else {
				iterate $fileName
			}
		}
	}
	cd $oldDir
}

set selList [cvssel]
set selSize [llength $selList]

for {set i 0} {$i < $selSize} {incr i} {
	iterate [lindex $selList $i]
}
cvsout "Done !\n"
cvsout "$numChanged file(s) changed !\n"
