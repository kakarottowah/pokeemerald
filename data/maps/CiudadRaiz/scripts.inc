CiudadRaiz_MapScripts::
	.byte 0

CiudadRaiz_EventScript_MoverPersonaje::
lockall
applymovement 0xFF, CiudadRaiz_Movement
waitmovement 0xFF
warp MAP_MUELLE_AIL, 0xFF, 60, 25
releaseall
end

CiudadRaiz_Movement:
walk_slow_left
walk_slow_left
step_end
