#!CVSGUI1.0 --files --name "Files sample"

set selList [cvssel]
set selSize [llength $selList]

cvsout "Hello, this is a sample macro !\n"
cvsout "Total selected : " $selSize "\n"

for {set i 0} {$i < $selSize} {incr i} {
	cvsout "    -> " [lindex $selList $i] \n
}
