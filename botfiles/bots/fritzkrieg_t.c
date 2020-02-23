chat "Fritzkrieg"
{
	//the teamplay.h file is included for all kinds of teamplay chats
	#include "teamplay.h"

	//======================================================
	//======================================================

	type "game_enter" //initiated when the bot enters the game
	{
		"Jaaaa. ~It's play-time for Fritzie!";
		"Nice place you got here. Mind if I make myself more comfortable?";
		"Hey ",0,", you gonna let me win again? Just like last time?";
		"Heh. ",4," again? I had me some fine victories here.";
		"That churning, squishy sound means ~Fritzkrieg has arrived.";
		"Heh! I'm just falling apart with anticipation!";
		"Guten Tag.";
		// 0 = bot name
	} //end type

	type "game_exit" //initiated when the bot exits the game
	{
		"Aah! Paper cut! ~Got to see the doctor before it gets infected.";
		"Hey! I'm checking out! Come and get your free frag before I leave.";
		"A pity that I'm wasting all this good ammo.";
		"Auf Wiedersehen.";
		// 0 = bot name
	} //end type

	type "level_start" //initiated when a new level starts
	{
		"LOS! LOS! LOS!";
		"I am feeling a serious case of BAD coming on.";
		// 0 = bot name
	} //end type

	type "level_end" //initiated when a level ends and the bot is not first and not last in the rankings
	{
		"'Clowns to the left of me, jokers to the right ...'.";
		2,", next time, I'm gonna do something real BAD to you.";
		"I managed to shove a few maggots in you, ",2,". ~They ought to be ripe real soon.";
		"HA!! ~You didn't bury me face down. I'm ready to fight again.";
		// 0 = bot name
	} //end type

	type "level_end_victory" //initiated when a level ends and the bot is first in the rankings
	{
		"Jaaaa!";
		"Fritzkrieg rules ",4,".";
		"Bow down to the King! Oh Ja!";
		"I rock!";
		"Feel that rot settling in. ~It kind of tickles, but you get used to it.";
		"To the victor goes the spoiled ... and the ripe and the rotting. Yummm!";
		// 0 = bot name
	} //end type

	type "level_end_lose" //initiated when a level ends and the bot is last in the rankings
	{
		"I hate losing to a lousy camper.";
		"The maggots beat everyone eventually.";
		"Drat. I popped a maggot out of my eye. Help me look for it.";
		"Course I lost. Dang maggots ate up all my mojo.";
		// 0 = bot name
		// 2 = opponent in first place
	} //end type

	//======================================================
	//======================================================

	type "hit_nodeath" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		"Oooooo. That's a big 'NO-pointer' for you, ",0,".";
		"I've had worse paper cuts, loser.";
		"~You better have another one of those, ",0," 'cause you're gonna need it.";
		"I ~ain't got any vital organs you can take out in one shot, ",0,".";
		"Wimp.";
		"Who said you couldn't hurt a fly? ~You just killed ~four of mine.";
		"Hey, ",0,"! ~You just whacked ~three of my favorite maggots!";
		"~You got me right in the butt. Now I got me a semi-colon.";
		"I let you hit me because I like to watch the pus run.";
		"Wuss. Gotta shoot straighter than that.";
		// 0 = shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		"Fraggitt! I could have sworn you were keeping your brain down there, ",0,".";
		"I'm such a tease.";
		"Let's speed this up, ",0,". I got larvae waiting to hatch.";
		"Fraggitt! I rolled a spare.";
		"Blast. No vitals organs there.";
		//0 = opponent
	} //end type

	//======================================================
	//======================================================

	type "death_telefrag" //initiated when the bot is killed by a telefrag
	{
		"That just cracks me up.";
		"~You get a gold star for that telefrag, ",0,".";
		"Wanna try for two out of three there, ",0,"?";
		// 0 = enemy name
	} //end type

	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{
		"Just scrape me off the walls and send me home.";
		"Scheissdreck!";
		"Splat! ~Did I get any on you?";
		"Awww. Fritzkrieg goes BOOM!";
		"Air Raid!";
		"Luftangriff!";
		"I've been buried in deeper holes than this!";
		"~This better be deeper than 6 feet.";
		"I wish I'd been around to watch that splat! ~Had to be beautiful.";
		// 0 = enemy name
	} //end type

	type "death_lava" //initiated when the bot dies in lava
	{
		"Smells just like my momma's cookin'.";
		"When you're hot, you're hot ..."
		"Toss some long pig on the barbie, mate.";
		"Hey, maggots pop like corn kernels!";
		"Sear me on both sides to lock in the juices.";
		// 0 = enemy name
	} //end type

	type "death_slime" //initiated when the bot dies in slime
	{
		"Someone get the janitor in here, ",0,"'s upchucked again.";
		"Medic! Medic!";
		"Too much chlorine in the pool!";
		// 0 = enemy name
	} //end type

	type "death_drown" //initiated when the bot drowns
	{
		"Guess I got to give back that swimming merit badge.";
		"Pool smells like somebody died in it.";
		"Anyone for some skinny-dipping?";
		"I thought dead bodies floated.";
		"Dead in the water. Heh.";
		"All the little ~bugs inside are drowning! Peace at last!";
		// 0 = enemy name
	} //end type

	type "death_suicide" //initiated when bot blows self up with a weapon or craters
	{
		"Ooops.";
		"~Got to practice that move on someone else next time.";
		"OK. I ~ain't giving out any more free lessons.";
		"I am a master at killing the dead, me included.";
		// 0 = enemy name
	} //end type

	type "death_gauntlet" //initiated when the bot is killed by a gauntlet attack
	{
		"~You got a mean right hook there, ",0,".";
		"~You gonna pay for that one, ",0,". Big Time.";
		"Fraggitt! Not again.";
		// 0 = enemy name
	} //end type

	type "death_rail" //initiated when the bot is killed by a rail gun shot
	{
		"Scharfschuetze!";
		"Now that definitely hit the spot!";
		"Heh heh, I see you in your little camper hole.";
		"~You can just zoom that rail gun right up your ...";
		"Laugh while you can sniper-wuss.";
		// 0 = enemy name
	} //end type

	type "death_bfg" //initiated when the bot died by a BFG
	{
		"I got you running scared now, ",0,".";
		"Hey! Gimme that gun.";
		"Oh! Scheisse!";
		"Skills check. Oops you failed again, ",0,".";
		"BFG-Wimp!";
		"The worst part is that most of these pops are coming from inside me.";
		"Guess the worms will be eating at separate tables.";
		"I'm falling to pieces ... and I mean that literally!";
		// 0 = enemy name
	} //end type

	type "death_kamikaze" //initiated when the bot died by a kamikazee blast
	{
		"Kamikaze?";
		"Verdammt!";
		"Yup. That sure smelled like ",0,"'s 'Divine Wind'.";
		"No skill there, ",fighter,".";
		"Somebody call the bomb squad on this ",fighter,".";
		"Now I'm mad. ~You know how long it took to collect all them maggots the first time?";
		"Now you've gone and scared my pretty little maggots.";
		"My point egg-zackly. No skills.";
		"I've just been promoted from dumpster bait to air pollution.";
		// 0 = enemy name
	} //end type

	type "death_insult" //insult initiated when the bot died
	{
		"I need a medic!";
		"Medic!";
		"Heh. ~Couldn't find the Kamikaze?";
		"~You know, ",0,". I'm starting not to like you.";
		"Yup. ~It's always easier to shoot a guy in the back.";
		"~You can't kill something what's already dead! At least not easily.";
		"Been there, done that. I'll make sure you share the experience, loser.";
		// 0 = enemy name
	} //end type

	type "death_praise" //praise initiated when the bot died
	{
		"Jawoll.";
		"Look, ",0,"! ~Is that your boss behind you?";
		"Some day, I'll look back at this and laugh, right?";
		"Nice shell casings there, ",fighter,".";
		"Lucky shot, ",0,"? Or did some other ",fighter," pull the trigger for you?";
		// 0 = enemy name
	} //end type


	//======================================================
	//======================================================

	type "kill_rail" //initiated when the bot kills someone with rail gun
	{
		"Heh. Auto-aiming at its finest.";
		"~Isn't there some rail arena match you could be playing somewhere else?";
		"I hate stinkin', snipin', camper wusses!";
		// 0 = enemy name
	} //end type

	type "kill_gauntlet" //initiated when the bot kills someone with gauntlet
	{
		"~You need me to pound any more sense into to your pointy little head, ",0,"?";
		"That was TOO easy.";
		"Oh yeah. Let's do that old 'Humiliation Dance' again, ",0,"!";
		"I just wanted to look you in the eyes, ",0,", while you die.";
		"BIFF!! WHAM!! POW! I love it!";
		"The touch of death.";
		KILL_GAUNTLET0;
		// 0 = enemy name
	} //end type

	type "kill_telefrag" //initiated when the bot telefragged someone
	{
		"Gotta admire my timing there.";
		"I just love jigsaw puzzles with LOTS of pieces.";
		"Face it, ",0,". My particles got mo' mojo than your particles.";
		"HEIL!";
		// 0 = enemy name
	} //end type

	type "kill_suicide" //initiated when another player kills self
	{
		"Stop doing that man! I got money on the point spread.";
		"Fraggit! I'm late again. ~You started the party without me.";
		"Oooooo. I'm just overwhelmed by your elite haxor skills.";
		// 0 = enemy name
	} //end type

	type "kill_insult" //insult initiated when the bot killed someone
	{
		"Jawoll!";
		"~You taking 'Loser Lessons' again, ",0,"?";
		"Oh yeah! ~It's Rabid Bunny Fever Time!";
		"Making an S.O.S. with your intestines isn't going to help.";
		KILL_INSULT2;
		"Never Surrender! Never Give Up! Never point that gun at me again!";
	
		// 0 = enemy name
	} //end type

	type "kill_kamikaze" //insult initiated when the bot killed another player with kamikaze
	{
		"That feels so much better!";
		"Heh. That was loud enough to wake the dead.";
		"Bartender! Maggots for everyone!";
		"Lunchtime!";
		"~Did the earth move for you too, ",0,"?";
		"Man, I got to cut the ",food," out of my diet. I can't handle the gas.";
		"Look at me ... I'm a one-man Bee-eF-Gee!";
		"I told you not to make me mad, ",0,".";
		"Was that me?";
		// 0 = enemy name
	} //end type

	type "kill_praise" //praise initiated when the bot killed someone
	{
		"Nice splatter there, ",0,". Good color choice.";
		"Join me, ",0," ... and we shall rule the Empire together ... NOT!";
		"... all this and I get your gun too!";
		"Okay, ",0," since you took your head off for me, I'll take my hat off for you!";
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "random_insult" //insult initiated randomly (just when the bot feels like it)
	{
		male_one_liners;
		"~Don't come griping to me. I ~ain't your ",random_counselor,".";
		"I can smell you over my own rot, ",0,". Means you really stink!";
		"Hey ",0,". Maybe if you weren't so ",adj," and ",different,", you'd be more of a challenge.";
		"Hey ",0,"! ~You talk funny. ~You are not from Germany. ~You from ",place," or somthing?";
		ramble;
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type

	type "random_misc" //miscellanous chats initiated randomly
	{
		one_liners;
		"Doomsday!";
		"~You doing ",substance," again, ",0,"? I told you to stay off the hard stuff.";
		"What do you think? ~Should I change this out for a ",color," trenchcoat?";
		"Hey ",0,". I heard that ",namez," was looking for you.";
		0,"'s final suffering will be exquisite.";
		"I hate it when flies fly OUT of my mouth.";
		"Anyone got a truss? I keep tripping over my guts.";
		0,", pull my finger. Hey, give it back!";
		"Jawohl!";
		"Anybody got a ~Lightning gun? I need me a recharge.";
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type
} //end chat Fritzkrieg
