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
    closemessage
    special EnterCode
    waitstate
    special GetCodeFeedback
    switch VAR_RESULT
    case 4, NewBarkTown_PlayersHouse_2F_EventScript_NESMonotypeGift
    case 5, NewBarkTown_PlayersHouse_2F_EventScript_NESQOLUnlock
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
    special GetNextRandomizerGift
    goto_if_eq VAR_RESULT, 1, NewBarkTown_PlayersHouse_2F_EventScript_NESMonotypeGift
    goto NewBarkTown_PlayersHouse_2F_EventScript_NESExit

NewBarkTown_PlayersHouse_2F_EventScript_NESNoRoom::
    msgbox NewBarkTown_PlayersHouse_2F_Text_NESNoRoom, MSGBOX_DEFAULT
    goto NewBarkTown_PlayersHouse_2F_EventScript_NESExit

NewBarkTown_PlayersHouse_2F_EventScript_NESQOLUnlock::
    @ Re-entry does not duplicate rewards, but an absent charm can be recovered.
    goto_if_unset FLAG_QOLUNLOCK_REDEEMED, NewBarkTown_PlayersHouse_2F_EventScript_NESGiveQOLRewards
    checkitem ITEM_REPEL_CHARM, 1
    goto_if_eq VAR_RESULT, TRUE, NewBarkTown_PlayersHouse_2F_EventScript_NESCodeAlreadyEntered

NewBarkTown_PlayersHouse_2F_EventScript_NESGiveQOLRewards::
    @ A legacy save can already own one or both items.
    checkitem ITEM_REPEL_CHARM, 1
    goto_if_eq VAR_RESULT, TRUE, NewBarkTown_PlayersHouse_2F_EventScript_NESGiveBoxLink
    giveitem ITEM_REPEL_CHARM, 1
    @ Do not permanently redeem when the Bag cannot accept the charm.
    goto_if_eq VAR_RESULT, FALSE, NewBarkTown_PlayersHouse_2F_EventScript_NESExit

NewBarkTown_PlayersHouse_2F_EventScript_NESGiveBoxLink::
    @ Use the existing Box Link item/field script; never duplicate an owned copy.
    checkitem ITEM_POKEMON_BOX_LINK, 1
    goto_if_eq VAR_RESULT, TRUE, NewBarkTown_PlayersHouse_2F_EventScript_NESFinishQOLUnlock
    giveitem ITEM_POKEMON_BOX_LINK, 1
    @ If the Key Items pocket is full, the code remains redeemable.
    goto_if_eq VAR_RESULT, FALSE, NewBarkTown_PlayersHouse_2F_EventScript_NESExit

NewBarkTown_PlayersHouse_2F_EventScript_NESFinishQOLUnlock::
    @ The Poké Vial is deliberately claimed on the next nurse visit.
    setflag FLAG_QOLUNLOCK_REDEEMED
    msgbox NewBarkTown_PlayersHouse_2F_Text_NESStatEditorUnlocked, MSGBOX_DEFAULT
    msgbox NewBarkTown_PlayersHouse_2F_Text_NESPokeVialUnlocked, MSGBOX_DEFAULT
    msgbox NewBarkTown_PlayersHouse_2F_Text_NESRepelCharmUnlocked, MSGBOX_DEFAULT
    msgbox NewBarkTown_PlayersHouse_2F_Text_NESBoxLinkUnlocked, MSGBOX_DEFAULT
    goto NewBarkTown_PlayersHouse_2F_EventScript_NESExit

NewBarkTown_PlayersHouse_2F_EventScript_NESCodeAlreadyEntered::
    msgbox NewBarkTown_PlayersHouse_2F_Text_NESCodeAlreadyEntered, MSGBOX_DEFAULT
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
    .string "Would you like to enter a\n"
    .string "cheat code?$"

NewBarkTown_PlayersHouse_2F_Text_NESBeforeStarter:
    .string "…Okay!\n"
    .string "It's time to go!$"

NewBarkTown_PlayersHouse_2F_Text_NESInvalidCode:
    .string "That code was not recognized.$"

NewBarkTown_PlayersHouse_2F_Text_NESSampleCode:
    .string "The sample code worked!$"

NewBarkTown_PlayersHouse_2F_Text_NESCodeEnabled:
    .string "Cheat code activated!$"

NewBarkTown_PlayersHouse_2F_Text_NESCodeDisabled:
    .string "Cheat code deactivated!$"

NewBarkTown_PlayersHouse_2F_Text_NESStatEditorUnlocked:
    .string "Stat Editor unlocked!$"

NewBarkTown_PlayersHouse_2F_Text_NESPokeVialUnlocked:
    .string "Poké Vial unlocked!\p"
    .string "Visit a Pokémon Center\n"
    .string "to claim it!$"

NewBarkTown_PlayersHouse_2F_Text_NESRepelCharmUnlocked:
    .string "Repel Charm unlocked!$"

NewBarkTown_PlayersHouse_2F_Text_NESBoxLinkUnlocked:
    .string "{PKMN} Box Link unlocked!$"

NewBarkTown_PlayersHouse_2F_Text_NESCodeAlreadyEntered:
    .string "You've already entered\n"
    .string "that cheat code.$"

NewBarkTown_PlayersHouse_2F_Text_NESNoRoom:
    .string "Your party and PC boxes\n"
    .string "are full!$"
