NewBarkTown_PlayersHouse_2F_MapScripts::
	.byte 0

NewBarkTown_PlayersHouse_2F_Script_ClockSetupTrigger1::
    lock
	playse SE_PIN
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_ExclamationMark
	waitmovement 0
	delay 60
    applymovement OBJ_EVENT_ID_PLAYER, NewBarkTown_PlayersHouse_2F_Movement_PlayerWalkToClock1
	waitmovement 0
    msgbox PlayersHouse_2F_Text_ClockIsStopped, MSGBOX_DEFAULT
	checkplayergender
	copyvar VAR_0x8004, VAR_RESULT
	call PlayersHouse_2F_EventScript_SetWallClock
	delay 30
	setvar VAR_NEWBARK_TOWN_STATE, 1
	setflag FLAG_SET_WALL_CLOCK
	releaseall
	end

NewBarkTown_PlayersHouse_2F_Script_ClockSetupTrigger2::
    lock
	playse SE_PIN
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_ExclamationMark
	waitmovement 0
	delay 60
    applymovement OBJ_EVENT_ID_PLAYER, NewBarkTown_PlayersHouse_2F_Movement_PlayerWalkToClock2
	waitmovement 0
    msgbox PlayersHouse_2F_Text_ClockIsStopped, MSGBOX_DEFAULT
	checkplayergender
	copyvar VAR_0x8004, VAR_RESULT
	call PlayersHouse_2F_EventScript_SetWallClock
	delay 30
	setvar VAR_NEWBARK_TOWN_STATE, 1
	setflag FLAG_SET_WALL_CLOCK
	releaseall
	end

NewBarkTown_PlayersHouse_2F_Movement_PlayerWalkToClock1:
	walk_right
	walk_up
	delay_8
	step_end

NewBarkTown_PlayersHouse_2F_Movement_PlayerWalkToClock2:	
	walk_up
	delay_8
	step_end

NewBarkTown_PlayersHouse_2F_EventScript_NES::
    lockall
    msgbox NewBarkTown_PlayersHouse_2F_Text_NESPlayed, MSGBOX_DEFAULT
    goto_if_unset FLAG_SYS_POKEMON_GET, NewBarkTown_PlayersHouse_2F_EventScript_NESBeforeStarter
    msgbox NewBarkTown_PlayersHouse_2F_Text_NESCheatPrompt, MSGBOX_YESNO
    goto_if_ne VAR_RESULT, YES, NewBarkTown_PlayersHouse_2F_EventScript_NESExit
    special EnterCode
    waitstate
    special GetCodeFeedback
    switch VAR_RESULT
    case 4, NewBarkTown_PlayersHouse_2F_EventScript_NESMonotypeGift
    case 1, NewBarkTown_PlayersHouse_2F_EventScript_NESSampleCode
    case 2, NewBarkTown_PlayersHouse_2F_EventScript_NESCodeEnabled
    case 3, NewBarkTown_PlayersHouse_2F_EventScript_NESCodeDisabled
    msgbox NewBarkTown_PlayersHouse_2F_Text_NESInvalidCode, MSGBOX_DEFAULT
    goto NewBarkTown_PlayersHouse_2F_EventScript_NESExit

NewBarkTown_PlayersHouse_2F_EventScript_NESMonotypeGift::
    copyvar VAR_TEMP_1, VAR_0x8004
    bufferspeciesname STR_VAR_1, VAR_TEMP_1
    givemon VAR_TEMP_1, 5
    goto_if_eq VAR_RESULT, MON_CANT_GIVE, NewBarkTown_PlayersHouse_2F_EventScript_NESNoRoom
    showgivemonpic 10, 3
    call Common_EventScript_GiftMon
    hidemonpic
    goto NewBarkTown_PlayersHouse_2F_EventScript_NESExit

NewBarkTown_PlayersHouse_2F_EventScript_NESNoRoom::
    msgbox NewBarkTown_PlayersHouse_2F_Text_NESNoRoom, MSGBOX_DEFAULT
    goto NewBarkTown_PlayersHouse_2F_EventScript_NESExit

NewBarkTown_PlayersHouse_2F_EventScript_NESSampleCode::
    msgbox NewBarkTown_PlayersHouse_2F_Text_NESSampleCode, MSGBOX_DEFAULT
    goto NewBarkTown_PlayersHouse_2F_EventScript_NESExit

NewBarkTown_PlayersHouse_2F_EventScript_NESCodeEnabled::
    msgbox NewBarkTown_PlayersHouse_2F_Text_NESCodeEnabled, MSGBOX_DEFAULT
    goto NewBarkTown_PlayersHouse_2F_EventScript_NESExit

NewBarkTown_PlayersHouse_2F_EventScript_NESCodeDisabled::
    msgbox NewBarkTown_PlayersHouse_2F_Text_NESCodeDisabled, MSGBOX_DEFAULT
    goto NewBarkTown_PlayersHouse_2F_EventScript_NESExit

NewBarkTown_PlayersHouse_2F_EventScript_NESBeforeStarter::
    msgbox NewBarkTown_PlayersHouse_2F_Text_NESBeforeStarter, MSGBOX_DEFAULT

NewBarkTown_PlayersHouse_2F_EventScript_NESExit::
    releaseall
    end

NewBarkTown_PlayersHouse_2F_Text_NESPlayed:
    .string "{PLAYER} played with the SNES.$"

NewBarkTown_PlayersHouse_2F_Text_NESCheatPrompt:
    .string "Would you like to enter a cheat code?$"

NewBarkTown_PlayersHouse_2F_Text_NESBeforeStarter:
    .string "…Okay!\\n"
    .string "It's time to go!$"

NewBarkTown_PlayersHouse_2F_Text_NESInvalidCode:
    .string "That code was not recognized.$"

NewBarkTown_PlayersHouse_2F_Text_NESSampleCode:
    .string "The sample code worked!$"

NewBarkTown_PlayersHouse_2F_Text_NESCodeEnabled:
    .string "Cheat code activated!$"

NewBarkTown_PlayersHouse_2F_Text_NESCodeDisabled:
    .string "Cheat code deactivated!$"

NewBarkTown_PlayersHouse_2F_Text_NESNoRoom:
    .string "Your party and PC boxes are full!$"
