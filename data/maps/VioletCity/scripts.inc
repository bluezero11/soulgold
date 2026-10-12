

VioletCity_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, VioletCity_OnFrame
	map_script MAP_SCRIPT_ON_TRANSITION, VioletCity_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, SetTimeEncounters
	map_script MAP_SCRIPT_ON_LOAD, VioletCity_OnLoad
	.byte 0

VioletCity_OnFrame::
	map_script_2 VAR_VIOLET_CITY_KIMONO_GIRL, 1, VioletCity_EventScript_Kimono_Girl
	map_script_2 VAR_VIOLET_CITY_STATE, 3, VioletCity_EventScript_ElmCall_Trigger
	.2byte 0

VioletCity_OnTransition::
	callnative SetTimeBasedEncounters
	end

VioletCity_OnLoad::
	setrespawn HEAL_LOCATION_VIOLET_CITY
	setflag FLAG_VISITED_VIOLET_CITY
	end

VioletCity_EventScript_Trigger::
	@LOCAL FLAGHEAP
	clearflag FLAG_RECEIVED_TOGEPI_EGG
	clearflag FLAG_VIOLET_NPC_TRADE_COMPLETED
	clearflag FLAG_HIDE_SPROUT_TOWER_SILVER
	clearflag FLAG_HIDE_JOHTO_GYM_LEADERS
	clearflag FLAG_HOOH_BROUGHT
	clearflag FLAG_OMANYTE_BROUGHT
	clearflag FLAG_AERODACTYL_BROUGHT
	clearflag FLAG_KABUTO_BROUGHT
	clearflag FLAG_HIDE_SUDOWOODO
	clearflag FLAG_GOT_HARD_STONE
	clearflag FLAG_GOT_POISON_BARB
	clearflag FLAG_HIDE_UNION_CAVE_LAPRAS
	clearflag FLAG_GOT_BLACK_GLASSES
	
	setvar VAR_SLOWPOKE_TAIL, 0
	setvar VAR_VIOLET_CITY_KIMONO_GIRL, 0
	setvar VAR_RUINSOFALPH_STATE, 0
	
	setflag FLAG_HIDE_TRAINER_SCHOOL_EARL
	setflag FLAG_HIDE_VIOLET_CITY_AIDE
	setflag FLAG_HIDE_VIOLET_CITY_KIMONO_GIRL
	
	setvar VAR_SPROUT_TOWER, 0
	setvar VAR_VIOLET_CITY_STATE, 1
	end

VioletCity_EventScript_Earl::
	lock
	faceplayer
	applymovement LOCALID_VIOLETCITY_EARL, VioletCity_Movement_Earl_Spin
	waitmovement 0
	faceplayer
	msgbox VioletCity_Text_EarlAsksIfYouBeatFalkner MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, VioletCity_EventScript_EarlTrainerSchool
	msgbox VioletCity_Text_VeryNiceIndeed, MSGBOX_DEFAULT
	closemessage
	release
	end

VioletCity_EventScript_EarlTrainerSchool::
	msgbox VioletCity_Text_FollowEarl, MSGBOX_DEFAULT
	closemessage
	playbgm MUS_HG_FOLLOW_ME_1, FALSE
	switch VAR_FACING
	case DIR_NORTH, VioletCity_EventScript_EarlTrainerSchoolSouth
	case DIR_WEST, VioletCity_EventScript_EarlTrainerSchoolEast
	release
	end

VioletCity_EventScript_EarlTrainerSchoolSouth::
	applymovement LOCALID_VIOLETCITY_EARL, VioletCity_Movement_Earl_South_Adjustment
	waitmovement 0
	applymovement LOCALID_VIOLETCITY_EARL, VioletCity_Movement_Earl_Spin
	applymovement OBJ_EVENT_ID_PLAYER, VioletCity_Movement_Player_South_Adjustment
	waitmovement 0
	waitmovement 0
	goto VioletCity_EventScript_EarlTrainerSchoolTransition
	release
	end

VioletCity_EventScript_EarlTrainerSchoolEast::
	applymovement LOCALID_VIOLETCITY_EARL, VioletCity_Movement_Earl_East_Adjustment
	applymovement OBJ_EVENT_ID_PLAYER, VioletCity_Movement_Player_East_Adjustment
	waitmovement 0
	waitmovement 0
	goto VioletCity_EventScript_EarlTrainerSchoolTransition
	release
	end

VioletCity_EventScript_EarlTrainerSchoolTransition:
	applymovement LOCALID_VIOLETCITY_EARL, VioletCity_Movement_Walk_To_School
	applymovement OBJ_EVENT_ID_PLAYER, VioletCity_Movement_Walk_To_School
	waitmovement 0
	waitmovement 0
	applymovement LOCALID_VIOLETCITY_EARL, VioletCity_Movement_Earl_Spin
	waitmovement 0
	turnobject LOCALID_VIOLETCITY_EARL, DIR_WEST
	msgbox VioletCity_Text_HereTeacherIAm, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_VIOLETCITY_EARL, VioletCity_Movement_Earl_Walk_Inside
	delay 20
	opendoor 39, 36
	waitdooranim
	waitmovement 0
	setflag FLAG_HIDE_VIOLET_CITY_EARL
	removeobject LOCALID_VIOLETCITY_EARL
	closedoor 39, 36
	waitdooranim
	clearflag FLAG_HIDE_TRAINER_SCHOOL_EARL
	fadedefaultbgm
	release
	end

VioletCity_EventScript_Youngster::
	lock
	faceplayer
	msgbox VioletCity_Text_Youngster, MSGBOX_DEFAULT
	closemessage
	release
	end

VioletCity_EventScript_Boy::
	lock
	faceplayer
	goto_if_set FLAG_RECEIVED_VIOLET_ACADEMY_GIFT, VioletCity_EventScript_Boy_AfterGift
	msgbox VioletCity_Text_BoyGiftIntro, MSGBOX_DEFAULT
	dynmultipush VioletCity_Text_BoyGiftCaterpie, SPECIES_CATERPIE
	dynmultipush VioletCity_Text_BoyGiftCutiefly, SPECIES_CUTIEFLY
	dynmultipush VioletCity_Text_BoyGiftGrubbin, SPECIES_GRUBBIN
	dynmultipush VioletCity_Text_BoyGiftJoltik, SPECIES_JOLTIK
	dynmultipush VioletCity_Text_BoyGiftNincada, SPECIES_NINCADA
	dynmultipush VioletCity_Text_BoyGiftSewaddle, SPECIES_SEWADDLE
	dynmultipush VioletCity_Text_BoyGiftSizzlipede, SPECIES_SIZZLIPEDE
	dynmultipush VioletCity_Text_BoyGiftSkorupi, SPECIES_SKORUPI
	dynmultipush VioletCity_Text_BoyGiftSpinarak, SPECIES_SPINARAK
	dynmultipush VioletCity_Text_BoyGiftVenipede, SPECIES_VENIPEDE
	dynmultipush VioletCity_Text_BoyGiftWeedle, SPECIES_WEEDLE
	dynmultipush VioletCity_Text_BoyGiftWimpod, SPECIES_WIMPOD
	dynmultipush VioletCity_Text_BoyGiftDecline, SPECIES_NONE
	@ Five visible entries; the longer menu scrolls and retains sprite previews.
	dynmultistack 0, 0, FALSE, 5, FALSE, SPECIES_CATERPIE, DYN_MULTICHOICE_CB_SHOW_POKEMON
	switch VAR_RESULT
	case SPECIES_CATERPIE, VioletCity_EventScript_Boy_GiveCaterpie
	case SPECIES_CUTIEFLY, VioletCity_EventScript_Boy_GiveCutiefly
	case SPECIES_GRUBBIN, VioletCity_EventScript_Boy_GiveGrubbin
	case SPECIES_JOLTIK, VioletCity_EventScript_Boy_GiveJoltik
	case SPECIES_NINCADA, VioletCity_EventScript_Boy_GiveNincada
	case SPECIES_SEWADDLE, VioletCity_EventScript_Boy_GiveSewaddle
	case SPECIES_SIZZLIPEDE, VioletCity_EventScript_Boy_GiveSizzlipede
	case SPECIES_SKORUPI, VioletCity_EventScript_Boy_GiveSkorupi
	case SPECIES_SPINARAK, VioletCity_EventScript_Boy_GiveSpinarak
	case SPECIES_VENIPEDE, VioletCity_EventScript_Boy_GiveVenipede
	case SPECIES_WEEDLE, VioletCity_EventScript_Boy_GiveWeedle
	case SPECIES_WIMPOD, VioletCity_EventScript_Boy_GiveWimpod
	msgbox VioletCity_Text_BoyGiftNoProblem, MSGBOX_DEFAULT
	release
	end

VioletCity_EventScript_Boy_GiveCaterpie::
	setvar VAR_TEMP_TRANSFERRED_SPECIES, SPECIES_CATERPIE
	bufferspeciesname STR_VAR_1, SPECIES_CATERPIE
	givemon SPECIES_CATERPIE, 5, move1=MOVE_TACKLE, move2=MOVE_STRING_SHOT, move3=MOVE_BUG_BITE, move4=MOVE_ELECTROWEB
	showgivemonpic 10, 3
	call Common_EventScript_GiftMon
	hidemonpic
	@ PC transfer uses STR_VAR_1 for the box name; restore the received species.
	bufferspeciesname STR_VAR_1, VAR_TEMP_TRANSFERRED_SPECIES
	setflag FLAG_RECEIVED_VIOLET_ACADEMY_GIFT
	msgbox VioletCity_Text_BoyGiftAfter, MSGBOX_DEFAULT
	release
	end

VioletCity_EventScript_Boy_GiveCutiefly::
	setvar VAR_TEMP_TRANSFERRED_SPECIES, SPECIES_CUTIEFLY
	bufferspeciesname STR_VAR_1, SPECIES_CUTIEFLY
	givemon SPECIES_CUTIEFLY, 5, move1=MOVE_ABSORB, move2=MOVE_FAIRY_WIND, move3=MOVE_STICKY_WEB, move4=MOVE_POWDER
	showgivemonpic 10, 3
	call Common_EventScript_GiftMon
	hidemonpic
	@ PC transfer uses STR_VAR_1 for the box name; restore the received species.
	bufferspeciesname STR_VAR_1, VAR_TEMP_TRANSFERRED_SPECIES
	setflag FLAG_RECEIVED_VIOLET_ACADEMY_GIFT
	msgbox VioletCity_Text_BoyGiftAfter, MSGBOX_DEFAULT
	release
	end

VioletCity_EventScript_Boy_GiveGrubbin::
	setvar VAR_TEMP_TRANSFERRED_SPECIES, SPECIES_GRUBBIN
	bufferspeciesname STR_VAR_1, SPECIES_GRUBBIN
	givemon SPECIES_GRUBBIN, 5, move1=MOVE_VISE_GRIP, move2=MOVE_MUD_SLAP, move3=MOVE_STRING_SHOT, move4=MOVE_ELECTROWEB
	showgivemonpic 10, 3
	call Common_EventScript_GiftMon
	hidemonpic
	@ PC transfer uses STR_VAR_1 for the box name; restore the received species.
	bufferspeciesname STR_VAR_1, VAR_TEMP_TRANSFERRED_SPECIES
	setflag FLAG_RECEIVED_VIOLET_ACADEMY_GIFT
	msgbox VioletCity_Text_BoyGiftAfter, MSGBOX_DEFAULT
	release
	end

VioletCity_EventScript_Boy_GiveJoltik::
	setvar VAR_TEMP_TRANSFERRED_SPECIES, SPECIES_JOLTIK
	bufferspeciesname STR_VAR_1, SPECIES_JOLTIK
	givemon SPECIES_JOLTIK, 5
	showgivemonpic 10, 3
	call Common_EventScript_GiftMon
	hidemonpic
	@ PC transfer uses STR_VAR_1 for the box name; restore the received species.
	bufferspeciesname STR_VAR_1, VAR_TEMP_TRANSFERRED_SPECIES
	setflag FLAG_RECEIVED_VIOLET_ACADEMY_GIFT
	msgbox VioletCity_Text_BoyGiftAfter, MSGBOX_DEFAULT
	release
	end

VioletCity_EventScript_Boy_GiveNincada::
	setvar VAR_TEMP_TRANSFERRED_SPECIES, SPECIES_NINCADA
	bufferspeciesname STR_VAR_1, SPECIES_NINCADA
	givemon SPECIES_NINCADA, 5, move1=MOVE_SAND_ATTACK, move2=MOVE_SCRATCH, move3=MOVE_HARDEN, move4=MOVE_BUG_BITE
	showgivemonpic 10, 3
	call Common_EventScript_GiftMon
	hidemonpic
	@ PC transfer uses STR_VAR_1 for the box name; restore the received species.
	bufferspeciesname STR_VAR_1, VAR_TEMP_TRANSFERRED_SPECIES
	setflag FLAG_RECEIVED_VIOLET_ACADEMY_GIFT
	msgbox VioletCity_Text_BoyGiftAfter, MSGBOX_DEFAULT
	release
	end

VioletCity_EventScript_Boy_GiveSewaddle::
	setvar VAR_TEMP_TRANSFERRED_SPECIES, SPECIES_SEWADDLE
	bufferspeciesname STR_VAR_1, SPECIES_SEWADDLE
	givemon SPECIES_SEWADDLE, 5, move1=MOVE_TACKLE, move2=MOVE_STRING_SHOT, move3=MOVE_CAMOUFLAGE, move4=MOVE_GRASSY_TERRAIN
	showgivemonpic 10, 3
	call Common_EventScript_GiftMon
	hidemonpic
	@ PC transfer uses STR_VAR_1 for the box name; restore the received species.
	bufferspeciesname STR_VAR_1, VAR_TEMP_TRANSFERRED_SPECIES
	setflag FLAG_RECEIVED_VIOLET_ACADEMY_GIFT
	msgbox VioletCity_Text_BoyGiftAfter, MSGBOX_DEFAULT
	release
	end

VioletCity_EventScript_Boy_GiveSizzlipede::
	setvar VAR_TEMP_TRANSFERRED_SPECIES, SPECIES_SIZZLIPEDE
	bufferspeciesname STR_VAR_1, SPECIES_SIZZLIPEDE
	givemon SPECIES_SIZZLIPEDE, 5, move1=MOVE_EMBER, move2=MOVE_SMOKESCREEN, move3=MOVE_WRAP, move4=MOVE_BUG_BITE
	showgivemonpic 10, 3
	call Common_EventScript_GiftMon
	hidemonpic
	@ PC transfer uses STR_VAR_1 for the box name; restore the received species.
	bufferspeciesname STR_VAR_1, VAR_TEMP_TRANSFERRED_SPECIES
	setflag FLAG_RECEIVED_VIOLET_ACADEMY_GIFT
	msgbox VioletCity_Text_BoyGiftAfter, MSGBOX_DEFAULT
	release
	end

VioletCity_EventScript_Boy_GiveSkorupi::
	setvar VAR_TEMP_TRANSFERRED_SPECIES, SPECIES_SKORUPI
	bufferspeciesname STR_VAR_1, SPECIES_SKORUPI
	givemon SPECIES_SKORUPI, 5
	showgivemonpic 10, 3
	call Common_EventScript_GiftMon
	hidemonpic
	@ PC transfer uses STR_VAR_1 for the box name; restore the received species.
	bufferspeciesname STR_VAR_1, VAR_TEMP_TRANSFERRED_SPECIES
	setflag FLAG_RECEIVED_VIOLET_ACADEMY_GIFT
	msgbox VioletCity_Text_BoyGiftAfter, MSGBOX_DEFAULT
	release
	end

VioletCity_EventScript_Boy_GiveSpinarak::
	setvar VAR_TEMP_TRANSFERRED_SPECIES, SPECIES_SPINARAK
	bufferspeciesname STR_VAR_1, SPECIES_SPINARAK
	givemon SPECIES_SPINARAK, 5, move1=MOVE_POISON_STING, move2=MOVE_TWINEEDLE, move3=MOVE_TOXIC_SPIKES, move4=MOVE_RAGE_POWDER
	showgivemonpic 10, 3
	call Common_EventScript_GiftMon
	hidemonpic
	@ PC transfer uses STR_VAR_1 for the box name; restore the received species.
	bufferspeciesname STR_VAR_1, VAR_TEMP_TRANSFERRED_SPECIES
	setflag FLAG_RECEIVED_VIOLET_ACADEMY_GIFT
	msgbox VioletCity_Text_BoyGiftAfter, MSGBOX_DEFAULT
	release
	end

VioletCity_EventScript_Boy_GiveVenipede::
	setvar VAR_TEMP_TRANSFERRED_SPECIES, SPECIES_VENIPEDE
	bufferspeciesname STR_VAR_1, SPECIES_VENIPEDE
	givemon SPECIES_VENIPEDE, 5, move1=MOVE_POISON_STING, move2=MOVE_ROLLOUT, move3=MOVE_TOXIC_SPIKES, move4=MOVE_SPIKES
	showgivemonpic 10, 3
	call Common_EventScript_GiftMon
	hidemonpic
	@ PC transfer uses STR_VAR_1 for the box name; restore the received species.
	bufferspeciesname STR_VAR_1, VAR_TEMP_TRANSFERRED_SPECIES
	setflag FLAG_RECEIVED_VIOLET_ACADEMY_GIFT
	msgbox VioletCity_Text_BoyGiftAfter, MSGBOX_DEFAULT
	release
	end

VioletCity_EventScript_Boy_GiveWeedle::
	setvar VAR_TEMP_TRANSFERRED_SPECIES, SPECIES_WEEDLE
	bufferspeciesname STR_VAR_1, SPECIES_WEEDLE
	givemon SPECIES_WEEDLE, 5, move1=MOVE_POISON_STING, move2=MOVE_STRING_SHOT, move3=MOVE_BUG_BITE, move4=MOVE_ELECTROWEB
	showgivemonpic 10, 3
	call Common_EventScript_GiftMon
	hidemonpic
	@ PC transfer uses STR_VAR_1 for the box name; restore the received species.
	bufferspeciesname STR_VAR_1, VAR_TEMP_TRANSFERRED_SPECIES
	setflag FLAG_RECEIVED_VIOLET_ACADEMY_GIFT
	msgbox VioletCity_Text_BoyGiftAfter, MSGBOX_DEFAULT
	release
	end

VioletCity_EventScript_Boy_GiveWimpod::
	setvar VAR_TEMP_TRANSFERRED_SPECIES, SPECIES_WIMPOD
	bufferspeciesname STR_VAR_1, SPECIES_WIMPOD
	givemon SPECIES_WIMPOD, 5, move1=MOVE_STRUGGLE_BUG, move2=MOVE_AQUA_JET, move3=MOVE_WIDE_GUARD, move4=MOVE_SPIKES
	showgivemonpic 10, 3
	call Common_EventScript_GiftMon
	hidemonpic
	@ PC transfer uses STR_VAR_1 for the box name; restore the received species.
	bufferspeciesname STR_VAR_1, VAR_TEMP_TRANSFERRED_SPECIES
	setflag FLAG_RECEIVED_VIOLET_ACADEMY_GIFT
	msgbox VioletCity_Text_BoyGiftAfter, MSGBOX_DEFAULT
	release
	end

VioletCity_EventScript_Boy_AfterGift::
	msgbox VioletCity_Text_BoyGiftAlreadyReceived, MSGBOX_DEFAULT
	release
	end

VioletCity_EventScript_Gramps::
	lock
	faceplayer
	msgbox VioletCity_Text_Gramps, MSGBOX_DEFAULT
	closemessage
	release
	end

VioletCity_EventScript_Lass::
	lock
	faceplayer
	msgbox VioletCity_Text_Lass, MSGBOX_DEFAULT
	closemessage
	release
	end

VioletCity_EventScript_Pichu::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_PICHU, CRY_MODE_NORMAL
	msgbox VioletCity_Text_Pichu, MSGBOX_DEFAULT
	waitmoncry
	release
	end

VioletCity_EventScript_Bellsprout::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_BELLSPROUT, CRY_MODE_NORMAL
	msgbox VioletCity_Text_Bellsprout, MSGBOX_DEFAULT
	waitmoncry
	release
	end

VioletCity_EventScript_Rattata::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_RATTATA, CRY_MODE_NORMAL
	msgbox VioletCity_Text_Rattata, MSGBOX_DEFAULT
	waitmoncry
	release
	end

VioletCity_EventScript_ElmCall_Trigger::
	lock
	pokenavcall VioletCity_Text_Elm_Call
	waitmessage
	delay 30
	clearflag FLAG_HIDE_VIOLET_CITY_AIDE
	setflag FLAG_HIDE_NEWBARKTOWN_LAB_AIDE
	setvar VAR_VIOLET_CITY_STATE, 4
	release
	end

VioletCity_EventScript_Kimono_Girl::
	lock
	playbgm MUS_HG_KIMONO_GIRL, TRUE
	playse SE_PIN
	applymovement LOCALID_VIOLET_KIMONO, Common_Movement_ExclamationMark
	delay 60
	applymovement LOCALID_VIOLET_KIMONO, VioletCity_Movement_Kimono_Girl_1
	waitmovement 0
	msgbox VioletCity_Text_Kimono_Girl1, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_VIOLET_KIMONO, VioletCity_Movement_Kimono_Girl_2
	waitmovement 0
	msgbox VioletCity_Text_Kimono_Girl2, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_VIOLET_KIMONO, VioletCity_Movement_Kimono_Girl_3
	waitmovement 0
	msgbox VioletCity_Text_Kimono_Girl3, MSGBOX_DEFAULT
	closemessage
	playse SE_M_PETAL_DANCE
	applymovement LOCALID_VIOLET_KIMONO, VioletCity_Movement_Kimono_Girl_4
	waitmovement 0
	msgbox VioletCity_Text_Kimono_Girl4, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_VIOLET_KIMONO, VioletCity_Movement_Kimono_Girl_5
	waitmovement 0
	fadedefaultbgm
	setvar VAR_VIOLET_CITY_KIMONO_GIRL, 2
	setflag FLAG_HIDE_VIOLET_CITY_KIMONO_GIRL
	removeobject LOCALID_VIOLET_KIMONO
	release
	end


VioletCity_EventScript_ItemRareCandy::
	finditem ITEM_RARE_CANDY
	end

VioletCity_EventScript_ItemPechaBerry::
	finditem ITEM_PECHA_BERRY
	end

VioletCity_EventScript_ItemHyperPotion::
	finditem ITEM_HYPER_POTION
	end

VioletCity_Movement_Earl_Spin:
	face_down
	face_right
	face_up
	face_left
	face_down
	face_right
	face_up
	face_left
	face_down
	face_right
	face_up
	face_left
	face_down
	step_end

VioletCity_Movement_Earl_East_Adjustment:
	walk_down
	walk_down
	walk_down
	walk_right
	step_end

VioletCity_Movement_Player_East_Adjustment:
	walk_down
	walk_down
	walk_left
	walk_down
	face_right
	step_end

VioletCity_Movement_Earl_South_Adjustment:
	walk_right
	walk_down
	walk_down
	walk_down
	face_right
	step_end

VioletCity_Movement_Player_South_Adjustment:
	walk_down
	walk_down
	step_end

VioletCity_Movement_Walk_To_School:
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	step_end

VioletCity_Movement_Walk_To_School_Player:
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	step_end

VioletCity_Movement_Earl_Walk_Inside:
	walk_up
	walk_up
	step_end

VioletCity_Movement_Kimono_Girl_1:
	walk_left
	walk_left
	walk_left
	walk_left
	face_up
	step_end

VioletCity_Movement_Kimono_Girl_2:
	walk_up
	step_end

VioletCity_Movement_Kimono_Girl_3:
	lock_facing_direction
	walk_down
	unlock_facing_direction
	step_end

VioletCity_Movement_Kimono_Girl_4:
	face_right
	delay_8
	face_down
	delay_8
	face_left
	delay_8
	face_up
	delay_8
	face_right
	delay_8
	face_down
	delay_8
	face_left
	delay_8
	face_up
	delay_8
	step_end

VioletCity_Movement_Kimono_Girl_5:
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end	

VioletCity_EventScript_LiquidJustice::
	msgbox VioletCity_Text_LiquidJustice, MSGBOX_NPC
	end

VioletCity_EventScript_GymSign::
	msgbox VioletCity_Text_GymSign, MSGBOX_SIGN
	end

VioletCity_Text_LiquidJustice:
	.string "Look around!\p"
	.string "It's easy to see what Pokémon\n"
	.string "you can catch in an area without\l"
	.string "having to run around in tall\l"
	.string "grass all day!$"

VioletCity_Text_EarlAsksIfYouBeatFalkner:
	.string "Hello!\n"
	.string "You are trainer?\p"
	.string "Battle Gym Leader,\n"
	.string "win you did?$"

VioletCity_Text_VeryNiceIndeed:
	.string "Ooh, la la!\n"
	.string "Very indeed nice!$"

VioletCity_Text_FollowEarl:
	.string "Is that so? Then study shall\n"
	.string "you! Follow me!$"

VioletCity_Text_HereTeacherIAm:
	.string "Here, teacher I am.\n"
	.string "Good it is you study here!$"

VioletCity_Text_Lass:
	.string "Ghosts are rumored to appear\n"
	.string "in Sprout Tower.\p"
	.string "They say that Ghost-type\n"
	.string "moves can bypass the defenses\p"
	.string "of tough Pokémon like Onix\n"
	.string "and Geodude.$"

VioletCity_Text_BoyGiftIntro:
	.string "Hey, you're a Pokémon trainer?\p"
	.string "The Academy is finding homes for\n"
	.string "some Bug Pokémon we've been\l"
	.string "looking after.\p"
	.string "Would you like to take one?$"

VioletCity_Text_BoyGiftCaterpie:
	.string "Caterpie$"

VioletCity_Text_BoyGiftCutiefly:
	.string "Cutiefly$"

VioletCity_Text_BoyGiftGrubbin:
	.string "Grubbin$"

VioletCity_Text_BoyGiftJoltik:
	.string "Joltik$"

VioletCity_Text_BoyGiftNincada:
	.string "Nincada$"

VioletCity_Text_BoyGiftSewaddle:
	.string "Sewaddle$"

VioletCity_Text_BoyGiftSizzlipede:
	.string "Sizzlipede$"

VioletCity_Text_BoyGiftSkorupi:
	.string "Skorupi$"

VioletCity_Text_BoyGiftSpinarak:
	.string "Spinarak$"

VioletCity_Text_BoyGiftVenipede:
	.string "Venipede$"

VioletCity_Text_BoyGiftWeedle:
	.string "Weedle$"

VioletCity_Text_BoyGiftWimpod:
	.string "Wimpod$"

VioletCity_Text_BoyGiftDecline:
	.string "No thanks!$"

VioletCity_Text_BoyGiftAfter:
	.string "Take good care of {STR_VAR_1}!\p"
	.string "You'll learn a lot together.$"

VioletCity_Text_BoyGiftNoProblem:
	.string "No problem! They'll be here if you\n"
	.string "change your mind.$"

VioletCity_Text_BoyGiftAlreadyReceived:
	.string "How's your new partner doing?\p"
	.string "The Academy teaches trainers as much\n"
	.string "as it teaches Pokémon!$"


VioletCity_Text_Gramps:
	.string "Falkner, from the Violet\n"
	.string "Pokémon Gym, is a fine trainer!\p"
	.string "He inherited his father's gym\n"
	.string "and has done a great job with\l"
	.string "it.$"

VioletCity_Text_Youngster:
	.string "I saw a wiggly tree up ahead!\p"
	.string "If you touch it, it squirms\n"
	.string "and dances! Cool!$"

VioletCity_Text_Sign:
	.string "Violet City\p"
	.string "The City of Nostalgic Scents$"

VioletCity_Text_GymSign:
	.string "Violet City Pokémon Gym\p"
	.string "Leader: Falkner, The Elegant\n"
	.string "Master of Flying Pokémon$"

VioletCity_Text_SproutTowerSign:
	.string "Sprout Tower\p"
	.string "Experience the Way of Pokémon$"

VioletCity_Text_EarlsAcademySign:
	.string "Earl's Pokémon Academy$"

VioletCity_Text_Pichu:
	.string "Pichu: Chuuuu!$"

VioletCity_Text_Bellsprout:
	.string "Bellsprout: Sprouuuut.$"

VioletCity_Text_Rattata:
	.string "Rattata: Rattataratta!$"

VioletCity_Text_Elm_Call:
	.string "Elm: Hello, {PLAYER}? We discovered\n"
	.string "something about the Egg!\p"
	.string "My assistant is at the Pokémon\n"
	.string "Center in Violet City.\p"
	.string "Could you talk to him?$"

VioletCity_Text_Kimono_Girl1:
.string "Me oh my...\n"
.string "That Egg must be...!$"

VioletCity_Text_Kimono_Girl2:
.string "I see...\p"
.string "So it was handed from Mr. Pokémon\n"
.string "to Prof. Elm, then from \l"
.string "Prof. Elm to you.\p"
.string "Now I get it...$"

VioletCity_Text_Kimono_Girl3:
.string "That Egg is truly an important Egg.\p"
.string "Please do take good care of it\n"
.string "for me. Is that understood?$"

VioletCity_Text_Kimono_Girl4:
.string "I am counting on you.$"

berrytrader_juggler::
	lock
	faceplayer
	msgbox berrytrader_juggler_Text_0
	dynmultichoice 0, 0, FALSE, 5, 0, DYN_MULTICHOICE_CB_NONE, berrytrader_juggler_Text_1, berrytrader_juggler_Text_2, berrytrader_juggler_Text_3, berrytrader_juggler_Text_4, berrytrader_juggler_Text_5
	switch VAR_RESULT
	case 0, berrytrader_juggler_3
	case 1, berrytrader_juggler_4
	case 2, berrytrader_juggler_5
	case 3, berrytrader_juggler_6
	msgbox berrytrader_juggler_Text_11, MSGBOX_DEFAULT
	waitmessage
	release
	end

	release
	end

berrytrader_juggler_3:
	checkitem ITEM_RED_SHARD
	compare VAR_RESULT, TRUE
	goto_if_eq berrytrader_juggler_9
	msgbox berrytrader_juggler_Text_7
	release
	end

berrytrader_juggler_4:
	checkitem ITEM_BLUE_SHARD
	compare VAR_RESULT, TRUE
	goto_if_eq berrytrader_juggler_12
	msgbox berrytrader_juggler_Text_7
	release
	end

berrytrader_juggler_5:
	checkitem ITEM_YELLOW_SHARD
	compare VAR_RESULT, TRUE
	goto_if_eq berrytrader_juggler_15
	msgbox berrytrader_juggler_Text_7
	release
	end

berrytrader_juggler_6:
	checkitem ITEM_GREEN_SHARD
	compare VAR_RESULT, TRUE
	goto_if_eq berrytrader_juggler_18
	msgbox berrytrader_juggler_Text_7
	release
	end

berrytrader_juggler_9:
	msgbox berrytrader_juggler_Text_6
	removeitem ITEM_RED_SHARD, 1
	giveitem ITEM_CHERI_BERRY
	giveitem ITEM_PECHA_BERRY
	giveitem ITEM_LEPPA_BERRY
	waitmessage
	release
	end

berrytrader_juggler_12:
	msgbox berrytrader_juggler_Text_8
	removeitem ITEM_BLUE_SHARD, 1
	giveitem ITEM_ORAN_BERRY
	giveitem ITEM_CHESTO_BERRY
	giveitem ITEM_WIKI_BERRY
	waitmessage
	release
	end

berrytrader_juggler_15:
	msgbox berrytrader_juggler_Text_9
	removeitem ITEM_YELLOW_SHARD, 1
	giveitem ITEM_ASPEAR_BERRY
	giveitem ITEM_SITRUS_BERRY
	giveitem ITEM_IAPAPA_BERRY
	waitmessage
	release
	end

berrytrader_juggler_18:
	msgbox berrytrader_juggler_Text_10
	removeitem ITEM_GREEN_SHARD, 1
	giveitem ITEM_RAWST_BERRY
	giveitem ITEM_LUM_BERRY
	giveitem ITEM_AGUAV_BERRY
	waitmessage
	release
	end


DexnavDude::
	msgbox DexnavDude_Text_0, MSGBOX_NPC
	return


berrytrader_juggler_Text_0:
	.string "Yah yah! I'm a huge fan of colored\n"
	.string "shards. Do you have any to trade for\l"
	.string "me? I will give you three berries for\l"
	.string "each shard.$"

berrytrader_juggler_Text_1:
	.string "Red Shard$"

berrytrader_juggler_Text_2:
	.string "Blue Shard$"

berrytrader_juggler_Text_3:
	.string "Yellow Shard$"

berrytrader_juggler_Text_4:
	.string "Green Shard$"

berrytrader_juggler_Text_5:
	.string "Cancel$"

berrytrader_juggler_Text_6:
	.string "I'll give you my berry set A for that\n"
	.string "one, yah!$"

berrytrader_juggler_Text_7:
	.string "You don't seem to have that shard with\n"
	.string "you.$"

berrytrader_juggler_Text_8:
	.string "I'll give you my berry set B for that\n"
	.string "one, yah!$"

berrytrader_juggler_Text_9:
	.string "I'll give you my berry set C for that\n"
	.string "one, yah!$"

berrytrader_juggler_Text_10:
	.string "I'll give you my berry set D for that\n"
	.string "one, yah!$"

berrytrader_juggler_Text_11:
	.string "Not interested huh? Shame...$"

DexnavDude_Text_0:
	.string "If you have a Pokegéar, you should\n"
	.string "make use of the Dexnav feature in it.$"
