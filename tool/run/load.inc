##
# Get the spawn_id of the load process
#
proc load_spawn_id { } {
	global load_spawn_id
	if {[info exists load_spawn_id]} {
		return $load_spawn_id }
	return -1
}
