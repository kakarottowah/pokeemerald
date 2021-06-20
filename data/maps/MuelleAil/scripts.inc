MuelleAil_MapScripts::
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, MuelleAil_OnWarp
	.byte 0

MuelleAil_OnWarp: 
	map_script_2 VAR_TEMP_1, 0, MuelleAil_EventScript_TurnPlayerLeft
	.2byte 0

MuelleAil_EventScript_TurnPlayerLeft:: @ 82295A3
	turnobject OBJ_EVENT_ID_PLAYER, DIR_WEST
	end
