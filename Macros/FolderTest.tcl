#!CVSGUI1.0 --folder --name "Clean-up merging files"

global numDeleted
set numDeleted 0

proc iterate {dirName} {
	if {[string compare cvs [string tolower $dirName]] == 0} {
		return
	}

	set oldDir [pwd]
	cd $dirName
	cvsout "Entering $dirName\n"

	set dirList [glob -nocomplain *]
	set dirSize [llength $dirList]
	for {set j 0} {$j < $dirSize} {incr j} {
		set fileName [lindex $dirList $j]
		if {[file isdirectory $fileName]} {
			iterate $fileName
		} elseif {[file isfile $fileName]} {
			if {[string compare [string range $fileName 0 1] ".#"] == 0} {
				cvsout "*** deleting -> " "'" $dirName / $fileName "'" ...\n
				file delete $fileName
				global numDeleted
				incr numDeleted
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
cvsout "$numDeleted file(s) deleted !\n"
