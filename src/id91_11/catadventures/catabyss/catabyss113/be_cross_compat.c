/*** Code generated by state_offset_conv_code_generator ***/

#include "def.h"

// List of all statetypes
extern statetype s_pshot1;
extern statetype s_pshot2;
extern statetype s_pshot_exp1;
extern statetype s_pshot_exp2;
extern statetype s_pshot_exp3;
extern statetype s_player;
extern statetype s_explode;
extern statetype s_boltbonus;
extern statetype s_boltbonus2;
extern statetype s_boltbonus3;
extern statetype s_nukebonus;
extern statetype s_nukebonus2;
extern statetype s_nukebonus3;
extern statetype s_potionbonus;
extern statetype s_rkey2bonus;
extern statetype s_rkeybonus;
extern statetype s_ykeybonus;
extern statetype s_gkeybonus;
extern statetype s_bkeybonus;
extern statetype s_scrollbonus;
extern statetype s_chestbonus;
extern statetype s_waterchestbonus1;
extern statetype s_waterchestbonus2;
extern statetype s_rgem1bonus;
extern statetype s_ygem1bonus;
extern statetype s_ggem1bonus;
extern statetype s_bgem1bonus;
extern statetype s_pgem1bonus;
extern statetype s_rgem2bonus;
extern statetype s_ygem2bonus;
extern statetype s_ggem2bonus;
extern statetype s_bgem2bonus;
extern statetype s_pgem2bonus;
extern statetype s_bonus_die;
extern statetype s_tombs[3];
extern statetype s_ftimebonus;
extern statetype s_ftimebonus2;
extern statetype s_walldie1;
extern statetype s_walldie2;
extern statetype s_walldie3;
extern statetype s_walldie4;
extern statetype s_walldie5;
extern statetype s_walldie6;
extern statetype s_obj_gate1;
extern statetype s_obj_gate2;
extern statetype s_obj_gate3;
extern statetype s_obj_gate4;
extern statetype s_pit;
extern statetype s_trollpause;
extern statetype s_troll1;
extern statetype s_troll2;
extern statetype s_troll3;
extern statetype s_troll4;
extern statetype s_trollattack1;
extern statetype s_trollattack2;
extern statetype s_trollattack3;
extern statetype s_trollouch;
extern statetype s_trolldie1;
extern statetype s_trolldie2;
extern statetype s_trolldie3;
extern statetype s_wet_bubbles1;
extern statetype s_wet_bubbles2;
extern statetype s_wet_bubbles3;
extern statetype s_wet_peek;
extern statetype s_wet_rise1;
extern statetype s_wet_rise3;
extern statetype s_wet_rise4;
extern statetype s_wet_rise5;
extern statetype s_wet_sink1;
extern statetype s_wet_sink2;
extern statetype s_wet_sink3;
extern statetype s_wet_walk1;
extern statetype s_wet_walk2;
extern statetype s_wet_walk3;
extern statetype s_wet_walk4;
extern statetype s_wet_attack1;
extern statetype s_wet_attack2;
extern statetype s_wet_attack3;
extern statetype s_wet_attack4;
extern statetype s_wet_ouch;
extern statetype s_wet_die1;
extern statetype s_wet_die2;
extern statetype s_wet_die3;
extern statetype s_wet_die4;
extern statetype s_wet_die5;
extern statetype s_zombie_risen;
extern statetype s_zombie_pause;
extern statetype s_zombie_inground;
extern statetype s_zombie_rise1;
extern statetype s_zombie_rise2;
extern statetype s_zombie_rise3;
extern statetype s_zombie_rise4;
extern statetype s_zombie_alive1;
extern statetype s_zombie_alive2;
extern statetype s_zombie_alive3;
extern statetype s_zombie_death1;
extern statetype s_zombie_death2;
extern statetype s_zombie_death3;
extern statetype s_zombie_attack;
extern statetype s_zombie_ouch;
extern statetype s_spook_wait;
extern statetype s_spook0;
extern statetype s_spook0_1;
extern statetype s_spook0_2;
extern statetype s_spook1;
extern statetype s_spook2;
extern statetype s_spook3;
extern statetype s_spook4;
extern statetype s_spook5;
extern statetype s_spook6;
extern statetype s_spook_attack1;
extern statetype s_spook_attack3;
extern statetype s_spook_pause;
extern statetype s_spookouch;
extern statetype s_spookdie;
extern statetype s_spookdie1;
extern statetype s_spookdie2;
extern statetype s_spookdie3;
extern statetype s_spookdie4;
extern statetype s_spookdie5;
extern statetype s_wallskel;
extern statetype s_wallskel2;
extern statetype s_skel_pause;
extern statetype s_skel_1;
extern statetype s_skel_2;
extern statetype s_skel_3;
extern statetype s_skel_4;
extern statetype s_skel_attack1;
extern statetype s_skel_attack2;
extern statetype s_skel_attack3;
extern statetype s_skel_ouch;
extern statetype s_skel_die1;
extern statetype s_skel_die2;
extern statetype s_skel_die3;
extern statetype s_eye_pause;
extern statetype s_eye_1;
extern statetype s_eye_2;
extern statetype s_eye_3;
extern statetype s_eye_4;
extern statetype s_eye_ouch;
extern statetype s_eye_ouch2;
extern statetype s_eye_die1;
extern statetype s_eye_die2;
extern statetype s_eye_die3;
extern statetype s_eshot1;
extern statetype s_eshot2;
extern statetype s_orcpause;
extern statetype s_orc1;
extern statetype s_orc2;
extern statetype s_orc3;
extern statetype s_orc4;
extern statetype s_orcattack1;
extern statetype s_orcattack2;
extern statetype s_orcattack3;
extern statetype s_orcouch;
extern statetype s_orcdie1;
extern statetype s_orcdie2;
extern statetype s_orcdie3;
extern statetype s_demonpause;
extern statetype s_demon1;
extern statetype s_demon2;
extern statetype s_demon3;
extern statetype s_demon4;
extern statetype s_demonattack1;
extern statetype s_demonattack2;
extern statetype s_demonattack3;
extern statetype s_demonouch;
extern statetype s_demondie1;
extern statetype s_demondie2;
extern statetype s_demondie3;
extern statetype s_magepause;
extern statetype s_mage1;
extern statetype s_mage2;
extern statetype s_mageattack3;
extern statetype s_mageouch;
extern statetype s_magedie1;
extern statetype s_magedie2;
extern statetype s_mshot1;
extern statetype s_mshot2;
extern statetype s_red_demonpause;
extern statetype s_red_demon1;
extern statetype s_red_demon2;
extern statetype s_red_demon3;
extern statetype s_red_demon4;
extern statetype s_red_demonattack1;
extern statetype s_red_demonattack2;
extern statetype s_red_demonattack3;
extern statetype s_red_demonouch;
extern statetype s_red_demondie1;
extern statetype s_red_demondie2;
extern statetype s_red_demondie3;
extern statetype s_grelpause;
extern statetype s_grel1;
extern statetype s_grel2;
extern statetype s_grelattack3;
extern statetype s_grelouch;
extern statetype s_greldie1;
extern statetype s_greldie2;
extern statetype s_greldie3;
extern statetype s_greldie4;
extern statetype s_greldie5;
extern statetype s_greldie5a;
extern statetype s_greldie6;
extern statetype s_gshot1;
extern statetype s_bat1;
extern statetype s_bat2;
extern statetype s_bat3;
extern statetype s_bat4;
extern statetype s_batpast;
extern statetype s_batdie1;
extern statetype s_batdie2;

void BE_Cross_Compat_FillObjStatesWithDOSOffsets(void)
{
	s_pshot1.compatdosoffset = 0x1F0A;
	s_pshot2.compatdosoffset = 0x1F14;
	s_pshot_exp1.compatdosoffset = 0x1F1E;
	s_pshot_exp2.compatdosoffset = 0x1F28;
	s_pshot_exp3.compatdosoffset = 0x1F32;
	s_player.compatdosoffset = 0x1F48;
	s_explode.compatdosoffset = 0x1F52;
	s_boltbonus.compatdosoffset = 0x2060;
	s_boltbonus2.compatdosoffset = 0x206A;
	s_boltbonus3.compatdosoffset = 0x2074;
	s_nukebonus.compatdosoffset = 0x207E;
	s_nukebonus2.compatdosoffset = 0x2088;
	s_nukebonus3.compatdosoffset = 0x2092;
	s_potionbonus.compatdosoffset = 0x209C;
	s_rkey2bonus.compatdosoffset = 0x20A6;
	s_rkeybonus.compatdosoffset = 0x20B0;
	s_ykeybonus.compatdosoffset = 0x20BA;
	s_gkeybonus.compatdosoffset = 0x20C4;
	s_bkeybonus.compatdosoffset = 0x20CE;
	s_scrollbonus.compatdosoffset = 0x20D8;
	s_chestbonus.compatdosoffset = 0x20E2;
	s_waterchestbonus1.compatdosoffset = 0x20EC;
	s_waterchestbonus2.compatdosoffset = 0x20F6;
	s_rgem1bonus.compatdosoffset = 0x2100;
	s_ygem1bonus.compatdosoffset = 0x210A;
	s_ggem1bonus.compatdosoffset = 0x2114;
	s_bgem1bonus.compatdosoffset = 0x211E;
	s_pgem1bonus.compatdosoffset = 0x2128;
	s_rgem2bonus.compatdosoffset = 0x2132;
	s_ygem2bonus.compatdosoffset = 0x213C;
	s_ggem2bonus.compatdosoffset = 0x2146;
	s_bgem2bonus.compatdosoffset = 0x2150;
	s_pgem2bonus.compatdosoffset = 0x215A;
	s_bonus_die.compatdosoffset = 0x2164;
	s_tombs[0].compatdosoffset = 0x216E;
	s_tombs[1].compatdosoffset = 0x2178;
	s_tombs[2].compatdosoffset = 0x2182;
	s_ftimebonus.compatdosoffset = 0x218C;
	s_ftimebonus2.compatdosoffset = 0x2196;
	s_walldie1.compatdosoffset = 0x21A0;
	s_walldie2.compatdosoffset = 0x21AA;
	s_walldie3.compatdosoffset = 0x21B4;
	s_walldie4.compatdosoffset = 0x21BE;
	s_walldie5.compatdosoffset = 0x21C8;
	s_walldie6.compatdosoffset = 0x21D2;
	s_obj_gate1.compatdosoffset = 0x21DC;
	s_obj_gate2.compatdosoffset = 0x21E6;
	s_obj_gate3.compatdosoffset = 0x21F0;
	s_obj_gate4.compatdosoffset = 0x21FA;
	s_pit.compatdosoffset = 0x2204;
	s_trollpause.compatdosoffset = 0x220E;
	s_troll1.compatdosoffset = 0x2218;
	s_troll2.compatdosoffset = 0x2222;
	s_troll3.compatdosoffset = 0x222C;
	s_troll4.compatdosoffset = 0x2236;
	s_trollattack1.compatdosoffset = 0x2240;
	s_trollattack2.compatdosoffset = 0x224A;
	s_trollattack3.compatdosoffset = 0x2254;
	s_trollouch.compatdosoffset = 0x225E;
	s_trolldie1.compatdosoffset = 0x2268;
	s_trolldie2.compatdosoffset = 0x2272;
	s_trolldie3.compatdosoffset = 0x227C;
	s_wet_bubbles1.compatdosoffset = 0x2286;
	s_wet_bubbles2.compatdosoffset = 0x2290;
	s_wet_bubbles3.compatdosoffset = 0x229A;
	s_wet_peek.compatdosoffset = 0x22A4;
	s_wet_rise1.compatdosoffset = 0x22AE;
	s_wet_rise3.compatdosoffset = 0x22B8;
	s_wet_rise4.compatdosoffset = 0x22C2;
	s_wet_rise5.compatdosoffset = 0x22CC;
	s_wet_sink1.compatdosoffset = 0x22D6;
	s_wet_sink2.compatdosoffset = 0x22E0;
	s_wet_sink3.compatdosoffset = 0x22EA;
	s_wet_walk1.compatdosoffset = 0x22F4;
	s_wet_walk2.compatdosoffset = 0x22FE;
	s_wet_walk3.compatdosoffset = 0x2308;
	s_wet_walk4.compatdosoffset = 0x2312;
	s_wet_attack1.compatdosoffset = 0x231C;
	s_wet_attack2.compatdosoffset = 0x2326;
	s_wet_attack3.compatdosoffset = 0x2330;
	s_wet_attack4.compatdosoffset = 0x233A;
	s_wet_ouch.compatdosoffset = 0x2344;
	s_wet_die1.compatdosoffset = 0x234E;
	s_wet_die2.compatdosoffset = 0x2358;
	s_wet_die3.compatdosoffset = 0x2362;
	s_wet_die4.compatdosoffset = 0x236C;
	s_wet_die5.compatdosoffset = 0x2376;
	s_zombie_risen.compatdosoffset = 0x2380;
	s_zombie_pause.compatdosoffset = 0x238A;
	s_zombie_inground.compatdosoffset = 0x2394;
	s_zombie_rise1.compatdosoffset = 0x239E;
	s_zombie_rise2.compatdosoffset = 0x23A8;
	s_zombie_rise3.compatdosoffset = 0x23B2;
	s_zombie_rise4.compatdosoffset = 0x23BC;
	s_zombie_alive1.compatdosoffset = 0x23C6;
	s_zombie_alive2.compatdosoffset = 0x23D0;
	s_zombie_alive3.compatdosoffset = 0x23DA;
	s_zombie_death1.compatdosoffset = 0x23E4;
	s_zombie_death2.compatdosoffset = 0x23EE;
	s_zombie_death3.compatdosoffset = 0x23F8;
	s_zombie_attack.compatdosoffset = 0x2402;
	s_zombie_ouch.compatdosoffset = 0x240C;
	s_spook_wait.compatdosoffset = 0x2416;
	s_spook0.compatdosoffset = 0x2420;
	s_spook0_1.compatdosoffset = 0x242A;
	s_spook0_2.compatdosoffset = 0x2434;
	s_spook1.compatdosoffset = 0x243E;
	s_spook2.compatdosoffset = 0x2448;
	s_spook3.compatdosoffset = 0x2452;
	s_spook4.compatdosoffset = 0x245C;
	s_spook5.compatdosoffset = 0x2466;
	s_spook6.compatdosoffset = 0x2470;
	s_spook_attack1.compatdosoffset = 0x247A;
	s_spook_attack3.compatdosoffset = 0x2484;
	s_spook_pause.compatdosoffset = 0x248E;
	s_spookouch.compatdosoffset = 0x2498;
	s_spookdie.compatdosoffset = 0x24A2;
	s_spookdie1.compatdosoffset = 0x24AC;
	s_spookdie2.compatdosoffset = 0x24B6;
	s_spookdie3.compatdosoffset = 0x24C0;
	s_spookdie4.compatdosoffset = 0x24CA;
	s_spookdie5.compatdosoffset = 0x24D4;
	s_wallskel.compatdosoffset = 0x24DE;
	s_wallskel2.compatdosoffset = 0x24E8;
	s_skel_pause.compatdosoffset = 0x24FA;
	s_skel_1.compatdosoffset = 0x2504;
	s_skel_2.compatdosoffset = 0x250E;
	s_skel_3.compatdosoffset = 0x2518;
	s_skel_4.compatdosoffset = 0x2522;
	s_skel_attack1.compatdosoffset = 0x252C;
	s_skel_attack2.compatdosoffset = 0x2536;
	s_skel_attack3.compatdosoffset = 0x2540;
	s_skel_ouch.compatdosoffset = 0x254A;
	s_skel_die1.compatdosoffset = 0x2554;
	s_skel_die2.compatdosoffset = 0x255E;
	s_skel_die3.compatdosoffset = 0x2568;
	s_eye_pause.compatdosoffset = 0x2572;
	s_eye_1.compatdosoffset = 0x257C;
	s_eye_2.compatdosoffset = 0x2586;
	s_eye_3.compatdosoffset = 0x2590;
	s_eye_4.compatdosoffset = 0x259A;
	s_eye_ouch.compatdosoffset = 0x25A4;
	s_eye_ouch2.compatdosoffset = 0x25AE;
	s_eye_die1.compatdosoffset = 0x25B8;
	s_eye_die2.compatdosoffset = 0x25C2;
	s_eye_die3.compatdosoffset = 0x25CC;
	s_eshot1.compatdosoffset = 0x25D6;
	s_eshot2.compatdosoffset = 0x25E0;
	s_orcpause.compatdosoffset = 0x25EA;
	s_orc1.compatdosoffset = 0x25F4;
	s_orc2.compatdosoffset = 0x25FE;
	s_orc3.compatdosoffset = 0x2608;
	s_orc4.compatdosoffset = 0x2612;
	s_orcattack1.compatdosoffset = 0x261C;
	s_orcattack2.compatdosoffset = 0x2626;
	s_orcattack3.compatdosoffset = 0x2630;
	s_orcouch.compatdosoffset = 0x263A;
	s_orcdie1.compatdosoffset = 0x2644;
	s_orcdie2.compatdosoffset = 0x264E;
	s_orcdie3.compatdosoffset = 0x2658;
	s_demonpause.compatdosoffset = 0x2662;
	s_demon1.compatdosoffset = 0x266C;
	s_demon2.compatdosoffset = 0x2676;
	s_demon3.compatdosoffset = 0x2680;
	s_demon4.compatdosoffset = 0x268A;
	s_demonattack1.compatdosoffset = 0x2694;
	s_demonattack2.compatdosoffset = 0x269E;
	s_demonattack3.compatdosoffset = 0x26A8;
	s_demonouch.compatdosoffset = 0x26B2;
	s_demondie1.compatdosoffset = 0x26BC;
	s_demondie2.compatdosoffset = 0x26C6;
	s_demondie3.compatdosoffset = 0x26D0;
	s_magepause.compatdosoffset = 0x26DA;
	s_mage1.compatdosoffset = 0x26E4;
	s_mage2.compatdosoffset = 0x26EE;
	s_mageattack3.compatdosoffset = 0x26F8;
	s_mageouch.compatdosoffset = 0x2702;
	s_magedie1.compatdosoffset = 0x270C;
	s_magedie2.compatdosoffset = 0x2716;
	s_mshot1.compatdosoffset = 0x2720;
	s_mshot2.compatdosoffset = 0x272A;
	s_red_demonpause.compatdosoffset = 0x2734;
	s_red_demon1.compatdosoffset = 0x273E;
	s_red_demon2.compatdosoffset = 0x2748;
	s_red_demon3.compatdosoffset = 0x2752;
	s_red_demon4.compatdosoffset = 0x275C;
	s_red_demonattack1.compatdosoffset = 0x2766;
	s_red_demonattack2.compatdosoffset = 0x2770;
	s_red_demonattack3.compatdosoffset = 0x277A;
	s_red_demonouch.compatdosoffset = 0x2784;
	s_red_demondie1.compatdosoffset = 0x278E;
	s_red_demondie2.compatdosoffset = 0x2798;
	s_red_demondie3.compatdosoffset = 0x27A2;
	s_grelpause.compatdosoffset = 0x27AC;
	s_grel1.compatdosoffset = 0x27B6;
	s_grel2.compatdosoffset = 0x27C0;
	s_grelattack3.compatdosoffset = 0x27CA;
	s_grelouch.compatdosoffset = 0x27D4;
	s_greldie1.compatdosoffset = 0x27DE;
	s_greldie2.compatdosoffset = 0x27E8;
	s_greldie3.compatdosoffset = 0x27F2;
	s_greldie4.compatdosoffset = 0x27FC;
	s_greldie5.compatdosoffset = 0x2806;
	s_greldie5a.compatdosoffset = 0x2810;
	s_greldie6.compatdosoffset = 0x281A;
	s_gshot1.compatdosoffset = 0x2824;
	s_bat1.compatdosoffset = 0x282E;
	s_bat2.compatdosoffset = 0x2838;
	s_bat3.compatdosoffset = 0x2842;
	s_bat4.compatdosoffset = 0x284C;
	s_batpast.compatdosoffset = 0x2856;
	s_batdie1.compatdosoffset = 0x2860;
	s_batdie2.compatdosoffset = 0x286A;
}

void* BE_Cross_Compat_GetObjStatePtrFromDOSOffset(uint16_t offset)
{
	switch (offset)
	{
	case 0x1F0A: return &s_pshot1;
	case 0x1F14: return &s_pshot2;
	case 0x1F1E: return &s_pshot_exp1;
	case 0x1F28: return &s_pshot_exp2;
	case 0x1F32: return &s_pshot_exp3;
	case 0x1F48: return &s_player;
	case 0x1F52: return &s_explode;
	case 0x2060: return &s_boltbonus;
	case 0x206A: return &s_boltbonus2;
	case 0x2074: return &s_boltbonus3;
	case 0x207E: return &s_nukebonus;
	case 0x2088: return &s_nukebonus2;
	case 0x2092: return &s_nukebonus3;
	case 0x209C: return &s_potionbonus;
	case 0x20A6: return &s_rkey2bonus;
	case 0x20B0: return &s_rkeybonus;
	case 0x20BA: return &s_ykeybonus;
	case 0x20C4: return &s_gkeybonus;
	case 0x20CE: return &s_bkeybonus;
	case 0x20D8: return &s_scrollbonus;
	case 0x20E2: return &s_chestbonus;
	case 0x20EC: return &s_waterchestbonus1;
	case 0x20F6: return &s_waterchestbonus2;
	case 0x2100: return &s_rgem1bonus;
	case 0x210A: return &s_ygem1bonus;
	case 0x2114: return &s_ggem1bonus;
	case 0x211E: return &s_bgem1bonus;
	case 0x2128: return &s_pgem1bonus;
	case 0x2132: return &s_rgem2bonus;
	case 0x213C: return &s_ygem2bonus;
	case 0x2146: return &s_ggem2bonus;
	case 0x2150: return &s_bgem2bonus;
	case 0x215A: return &s_pgem2bonus;
	case 0x2164: return &s_bonus_die;
	case 0x216E: return &s_tombs[0];
	case 0x2178: return &s_tombs[1];
	case 0x2182: return &s_tombs[2];
	case 0x218C: return &s_ftimebonus;
	case 0x2196: return &s_ftimebonus2;
	case 0x21A0: return &s_walldie1;
	case 0x21AA: return &s_walldie2;
	case 0x21B4: return &s_walldie3;
	case 0x21BE: return &s_walldie4;
	case 0x21C8: return &s_walldie5;
	case 0x21D2: return &s_walldie6;
	case 0x21DC: return &s_obj_gate1;
	case 0x21E6: return &s_obj_gate2;
	case 0x21F0: return &s_obj_gate3;
	case 0x21FA: return &s_obj_gate4;
	case 0x2204: return &s_pit;
	case 0x220E: return &s_trollpause;
	case 0x2218: return &s_troll1;
	case 0x2222: return &s_troll2;
	case 0x222C: return &s_troll3;
	case 0x2236: return &s_troll4;
	case 0x2240: return &s_trollattack1;
	case 0x224A: return &s_trollattack2;
	case 0x2254: return &s_trollattack3;
	case 0x225E: return &s_trollouch;
	case 0x2268: return &s_trolldie1;
	case 0x2272: return &s_trolldie2;
	case 0x227C: return &s_trolldie3;
	case 0x2286: return &s_wet_bubbles1;
	case 0x2290: return &s_wet_bubbles2;
	case 0x229A: return &s_wet_bubbles3;
	case 0x22A4: return &s_wet_peek;
	case 0x22AE: return &s_wet_rise1;
	case 0x22B8: return &s_wet_rise3;
	case 0x22C2: return &s_wet_rise4;
	case 0x22CC: return &s_wet_rise5;
	case 0x22D6: return &s_wet_sink1;
	case 0x22E0: return &s_wet_sink2;
	case 0x22EA: return &s_wet_sink3;
	case 0x22F4: return &s_wet_walk1;
	case 0x22FE: return &s_wet_walk2;
	case 0x2308: return &s_wet_walk3;
	case 0x2312: return &s_wet_walk4;
	case 0x231C: return &s_wet_attack1;
	case 0x2326: return &s_wet_attack2;
	case 0x2330: return &s_wet_attack3;
	case 0x233A: return &s_wet_attack4;
	case 0x2344: return &s_wet_ouch;
	case 0x234E: return &s_wet_die1;
	case 0x2358: return &s_wet_die2;
	case 0x2362: return &s_wet_die3;
	case 0x236C: return &s_wet_die4;
	case 0x2376: return &s_wet_die5;
	case 0x2380: return &s_zombie_risen;
	case 0x238A: return &s_zombie_pause;
	case 0x2394: return &s_zombie_inground;
	case 0x239E: return &s_zombie_rise1;
	case 0x23A8: return &s_zombie_rise2;
	case 0x23B2: return &s_zombie_rise3;
	case 0x23BC: return &s_zombie_rise4;
	case 0x23C6: return &s_zombie_alive1;
	case 0x23D0: return &s_zombie_alive2;
	case 0x23DA: return &s_zombie_alive3;
	case 0x23E4: return &s_zombie_death1;
	case 0x23EE: return &s_zombie_death2;
	case 0x23F8: return &s_zombie_death3;
	case 0x2402: return &s_zombie_attack;
	case 0x240C: return &s_zombie_ouch;
	case 0x2416: return &s_spook_wait;
	case 0x2420: return &s_spook0;
	case 0x242A: return &s_spook0_1;
	case 0x2434: return &s_spook0_2;
	case 0x243E: return &s_spook1;
	case 0x2448: return &s_spook2;
	case 0x2452: return &s_spook3;
	case 0x245C: return &s_spook4;
	case 0x2466: return &s_spook5;
	case 0x2470: return &s_spook6;
	case 0x247A: return &s_spook_attack1;
	case 0x2484: return &s_spook_attack3;
	case 0x248E: return &s_spook_pause;
	case 0x2498: return &s_spookouch;
	case 0x24A2: return &s_spookdie;
	case 0x24AC: return &s_spookdie1;
	case 0x24B6: return &s_spookdie2;
	case 0x24C0: return &s_spookdie3;
	case 0x24CA: return &s_spookdie4;
	case 0x24D4: return &s_spookdie5;
	case 0x24DE: return &s_wallskel;
	case 0x24E8: return &s_wallskel2;
	case 0x24FA: return &s_skel_pause;
	case 0x2504: return &s_skel_1;
	case 0x250E: return &s_skel_2;
	case 0x2518: return &s_skel_3;
	case 0x2522: return &s_skel_4;
	case 0x252C: return &s_skel_attack1;
	case 0x2536: return &s_skel_attack2;
	case 0x2540: return &s_skel_attack3;
	case 0x254A: return &s_skel_ouch;
	case 0x2554: return &s_skel_die1;
	case 0x255E: return &s_skel_die2;
	case 0x2568: return &s_skel_die3;
	case 0x2572: return &s_eye_pause;
	case 0x257C: return &s_eye_1;
	case 0x2586: return &s_eye_2;
	case 0x2590: return &s_eye_3;
	case 0x259A: return &s_eye_4;
	case 0x25A4: return &s_eye_ouch;
	case 0x25AE: return &s_eye_ouch2;
	case 0x25B8: return &s_eye_die1;
	case 0x25C2: return &s_eye_die2;
	case 0x25CC: return &s_eye_die3;
	case 0x25D6: return &s_eshot1;
	case 0x25E0: return &s_eshot2;
	case 0x25EA: return &s_orcpause;
	case 0x25F4: return &s_orc1;
	case 0x25FE: return &s_orc2;
	case 0x2608: return &s_orc3;
	case 0x2612: return &s_orc4;
	case 0x261C: return &s_orcattack1;
	case 0x2626: return &s_orcattack2;
	case 0x2630: return &s_orcattack3;
	case 0x263A: return &s_orcouch;
	case 0x2644: return &s_orcdie1;
	case 0x264E: return &s_orcdie2;
	case 0x2658: return &s_orcdie3;
	case 0x2662: return &s_demonpause;
	case 0x266C: return &s_demon1;
	case 0x2676: return &s_demon2;
	case 0x2680: return &s_demon3;
	case 0x268A: return &s_demon4;
	case 0x2694: return &s_demonattack1;
	case 0x269E: return &s_demonattack2;
	case 0x26A8: return &s_demonattack3;
	case 0x26B2: return &s_demonouch;
	case 0x26BC: return &s_demondie1;
	case 0x26C6: return &s_demondie2;
	case 0x26D0: return &s_demondie3;
	case 0x26DA: return &s_magepause;
	case 0x26E4: return &s_mage1;
	case 0x26EE: return &s_mage2;
	case 0x26F8: return &s_mageattack3;
	case 0x2702: return &s_mageouch;
	case 0x270C: return &s_magedie1;
	case 0x2716: return &s_magedie2;
	case 0x2720: return &s_mshot1;
	case 0x272A: return &s_mshot2;
	case 0x2734: return &s_red_demonpause;
	case 0x273E: return &s_red_demon1;
	case 0x2748: return &s_red_demon2;
	case 0x2752: return &s_red_demon3;
	case 0x275C: return &s_red_demon4;
	case 0x2766: return &s_red_demonattack1;
	case 0x2770: return &s_red_demonattack2;
	case 0x277A: return &s_red_demonattack3;
	case 0x2784: return &s_red_demonouch;
	case 0x278E: return &s_red_demondie1;
	case 0x2798: return &s_red_demondie2;
	case 0x27A2: return &s_red_demondie3;
	case 0x27AC: return &s_grelpause;
	case 0x27B6: return &s_grel1;
	case 0x27C0: return &s_grel2;
	case 0x27CA: return &s_grelattack3;
	case 0x27D4: return &s_grelouch;
	case 0x27DE: return &s_greldie1;
	case 0x27E8: return &s_greldie2;
	case 0x27F2: return &s_greldie3;
	case 0x27FC: return &s_greldie4;
	case 0x2806: return &s_greldie5;
	case 0x2810: return &s_greldie5a;
	case 0x281A: return &s_greldie6;
	case 0x2824: return &s_gshot1;
	case 0x282E: return &s_bat1;
	case 0x2838: return &s_bat2;
	case 0x2842: return &s_bat3;
	case 0x284C: return &s_bat4;
	case 0x2856: return &s_batpast;
	case 0x2860: return &s_batdie1;
	case 0x286A: return &s_batdie2;
	}
	return NULL;
}