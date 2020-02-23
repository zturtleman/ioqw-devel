chat "Ranger"
{
	//the teamplay.h file is included for all kinds of teamplay chats
	#include "teamplay.h"

	//======================================================
	//======================================================

	type "game_enter" //initiated when the bot enters the game
	{
		"Special orders for ",4,": ... 'Remove all hostiles'!";
		"Where have the portals taken me now?";
		"~Ranger reporting in. Am engaging the enemy in close combat.";
		"I'm surrounded by Quake minions. Laying down suppressing fire.";
		"Negative. Negative. I'm going in.";
		"~Ranger to Base. ",negative," I can handle the opposition here.";
		"~Ranger to Base. ",affirmative," ~We're good to go here.";
		"How much blood must I bathe in before I find peace?";
		"~This is ",4,"? ~They've really cleaned this place up!";
		"Aren't there supposed to be 'slipgates' here?";
		// 0 = bot name
		// 4 = Level's title
	} //end type

	type "game_exit" //initiated when the bot exits the game
	{
		"Come with me, ",1,". Find peace.";
		"Gonna find me some fiends to frag.";
		"~Ranger to base ... evac of ",4," commencing!";
		"I'm going for reinforcements! Cover me!";
		"~We're pulling out ...";
		// 0 = bot name
		// 4 = Level's title
	} //end type

	type "level_start" //initiated when a new level starts
	{
		"Go! Go! Go!";
		"Deja vu all over again ... aargh!";
		"I'm having flashbacks.";
		"And the sky was full of rockets.";
		"Wish I had some real opponents here.";
		"~You set 'em up, I'll knock 'em down!";
		"Commencing operation ", operation1," ", operation2,".";
		// 0 = bot name
	} //end type

	type "level_end" //initiated when a level ends and the bot is not first and not last in the rankings
	{
		"Hey, ",2,", you train with the SEALS?";
		"A win's a win, but that was a waste of freaking time.";
		"~This operation's outcome is not acceptable.";
		"Why is everyone stopping? I'm still here ... I think ... yeah, I am!";
		2," is bucking for sergeant.";
		3," should've stayed back in the mess hall.";
		"Over already? I remember these things lasting for hours.";
		// 0 = bot name
		// 2 = opponent in first place
	} //end type

	type "level_end_victory" //initiated when a level ends and the bot is first in the rankings
	{
		"All in a day's work.";
		"Semper fi, do or die! Semper fi, I did, you died!";
		"Hey, ",3,", you sissied out of boot camp, didn't you?";
		"Ranger to base. ",4," is secured. Phase two can begin. Over.";
		"Operation ", operation1," ", operation2," is complete. Mission objectives obtained.";
		"~They ~don't train 'em like the used to.";
		"",4," always was sweet to me.";
		"~Don't take it personally,",1,", I was doing this when you were in diapers.";
		"The air smells like sweet victory right now.";
		// 0 = bot name
	} //end type

	type "level_end_lose" //initiated when a level ends and the bot is last in the rankings
	{
		"Failure is not a part of a ~Marine's vocabulary.";
		"Operation ", operation1," ", operation2," has failed to achieve its objective.";
		"Freaking Navy-issue equipment!";
		"Son-of-a-butt-smellin'-momma-dog!";
		"~Ranger to base. Mission Objectives not attainable. Permission to withdraw requested.";
		"The Enemy has bested us.";
		"Great match. Just ~don't expect to repeat it the next time, ",2,".";
		"Yeah. Technology over tactics. Big guns over good moves.";
		"~You proud, ",2,"? ~You shouldn't be proud! That was lame!";
		"~They've changed this place. I remember ",4," before they messed it up.";
		"I ~don't care what it freakin' says. ^1I DON'T LOSE.";
		// 0 = bot name
	} //end type

	//======================================================
	//======================================================

	type "hit_nodeath" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		"~You're just pissin' me off, ",0,".";
		"I've cut farts that hurt more than that.";
		"Nice shot, private-first-class-wiseguy.";
		0,", I'm gonna rip your arm off and beat you to death with it.";
		"Not using regulation ordinance again, eh ",0,".";
		"I'm gonna convene a ~one man court martial right now.";
		"~Can't jump right anymore, can't run fast. When I was younger ...";
		"~You're making it harder for me to go through airports ... the bells, the bells!";
		"~Pinged another ~one off the plate in my head. What're you shooting ... magnets?";
		//0 = shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		"I'm goin' slow, cuz I like the look on your face, ",fighter,".";
		"Stay still, ",0,", so I can connect the dots.";
		"~You got guts, ",0,". I can see most of 'em now.";
		"Gotta correct my aim for windage.";
		"Not dead? ~Must be defective ordinance.";
		"Let me guess, ",0,". That was a headshot.";
		"When I started fighting, you were in diapers, ",0,", and now, you need 'em again.";
		"Stop screaming, ",0,"! I can't hear the other voices in my head!";
		"Wait, you're not 'Old Yeller'!";
		//0 = opponent
	} //end type

	//======================================================
	//======================================================

	type "death_telefrag" //initiated when the bot is killed by a telefrag
	{
		"What brainy-butt occifer plotted this course?";
		"I hate portals with a freakin' passion.";
		"Leave me alone, ",0,"! There's already too many people in here!";
		"Stinkin' slipgates.";
		"Nothing but static at this end.";
		// 0 = enemy name
	} //end type

	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{
		"Wasn't I issued a parachute?";
		"I failed FLYING 101 in Basic.";
		"Cats can do it, so can I!";
		"Up, up, and away! I'm a bird! I'm a plane!";
		"Used to fall a lot farther than that with no pain.";
		"Let's see you do better, ",0,".";
		// 0 = enemy name
	} //end type

	type "death_lava" //initiated when the bot dies in lava
	{
		"~It's just form-fitting armor. Wait till it cools.";
		"~They taught me to keep my head down.";
		"Stop, drop, and roll!";
		"Nope. Still hate this stuff.";
		"How did Cthon manage this stuff?";
		"Hey, ",0,". Got any marshmallows?";
		"Battle suits are for wimps. Give me an Enviro Suit any day.";
		// 0 = enemy name
	} //end type

	type "death_slime" //initiated when the bot dies in slime
	{
		"MRE production plant. Freakin' great."
		"I hope I can get a shot to clear this up.";
		"Ranger to Base: Toxic Spill complicating operation ", operation1," ", operation2,".";
		"KP again ... too much creamed corn ... way too much creamed corn.";
		"~Doesn't smell like liniment.";
		"~This stuff is primordial ooze.";
		// 0 = enemy name
	} //end type

	type "death_drown" //initiated when the bot drowns
	{
		"Dropped the chute on the LV a bit early, didn't we?";
		"Join the Navy, they said. See the world, they said ...";
		"I can't hear! Why can't I hear?";
		"~We had gills when I was a boy!";
		"Hey, ",0,", throw me a lifesaver. And I ~don't mean candy!";
		"Course I can swim. Just didn't feel like it then.";
		"~This doesn't look like the well of wishes ...";
		// 0 = enemy name
	} //end type

	type "death_suicide" //initiated when bot blows self up with a weapon or craters
	{
		"In a blaze of glory! Oh, hell, at least it's a blaze.";
		"Now that I've demonstrated how NOT to do it ...";
		"Too complicated. Used to be simple. Mano ~y mano.";
		"Not the first time I've done this ... won't be the last.";
		// 0 = enemy name
	} //end type

	type "death_gauntlet" //initiated when the bot is killed by a gauntlet attack
	{
		"I knew you were there, ",0,". I was just testing you.";
		"Charlie got tougher in the jungle.";
		"~You just try that again when I'm drunk.";
		// 0 = enemy name
	} //end type

	type "death_rail" //initiated when the bot is killed by a rail gun shot
	{
		"Whatever happened to waiting till you see the whites of their eyes?";
		"I hate taking the point.";
		"Sissy gun! Sissy gun! ~We wouldn't use crap like that in the old days!";
		// 0 = enemy name
	} //end type

	type "death_bfg" //initiated when the bot died by a BFG
	{
		"Messy ... it's just not like the old days.";
		"Incoming! Lots of incoming!";
		"Oh look, a viking burial! Hey wait, it's mine!";
		"Whose bright idea was this weapon?";
		"No-skill llama!";
		"What're the fireworks for, ",0,"? Ow ... never mind.";
		// 0 = enemy name
	} //end type

	type "death_insult" //insult initiated when the bot died
	{
		"Best trophy you'll ever get, scumbag.";
		"So your momma's apron proved to be good armor. Big deal.";
		"~Ranger to Base, we're under heavy fire here.";
		"~Four years ago, I could've killed you with a projectile-shaped piece of earwax.";
		"Yeah, in the old days, I had to walk a mile through ~six feet of snow to frag someone.";
		"~They make things too easy now. Anybody can fight here.";
		"~You're a long way from the elite, ",fighter,".";
		// 0 = enemy name
	} //end type

	type "death_praise" //praise initiated when the bot died
	{
		"Okay, ",0,", I'd salute you ... if I could find my head!";
		"I'd give you my boots, but I ~don't think you could fill 'em!";
		"I can respect a shot like that, ",0,".";
		"Pin me up and call me a target. An outstanding shot, ",0,".";
		"I'd take a bow but I can't find my legs.";
		"And the old stallion is driven from the herd ...";
		"Some mighty fine shooting there, ",friendname,".";
		"~You fight like that for 30 years ... you just might be me someday.";
		"Nice. ~You trained with the 'old ones', didn't you, ",0,"?";
		"~That you, G. Narley?";
		// 0 = enemy name
	} //end type

	type "death_kamikaze" //initiated when the bot is killed by kamikaze blast
	{
		"The medic's got medications that can help you cope with that, ",0,".";
		"~Ranger to Base, enemy has deployed tactical nukes. Advise.";
		"~Ranger to Base. Operation ", operation1," ", operation2," at risk. Advise.";
		"All that training just to qualify as hazardous waste.";
		0,", I'm gonna make you eat the next one of those, ",0,".";
		"What happened, ",fighter,"? ~Your backpack explode?";
		"Never had to put up with that kind of crap that I can remember.";
		"Either quit eatin' beans, ",0,", or find yourself some other opponents.";
		"I'm gonna pretend I didn't see you do that, ",0,".";
		"~You work for Shub, right, ",0,"?";
		"~You're not licensed for that weapon, are you ",0,".";
		DEATH_KAMIKAZE0;
		// 0 = enemy name
	} //end type 


	//======================================================
	//======================================================

	type "kill_kamikaze" //initiated when the bot kills someone with kamikaze
	{
		"Quakin'!!!";
		"Blast. I think I missed one of ya.";
		"Airstrike!";
		"Not this time sucker!";
		"~This is too much like fun. I gotta get out of the house more.";
		"Hey ",0,"! Anybody tell you that you got a purty spleen?";
		"Yeah, there'll be guys complaining about this one til the end of time.";
		"Heh. Guess I'm not ready to retire just yet.";
		"**SIGH** Whatever happened to weapons you had to aim?";
		"No whining! ~There is no whining in the arena!";
		KILL_KAMIKAZE0;
		KILL_KAMIKAZE1;
		// 0 = enemy name
	} //end type

	type "kill_rail" //initiated when the bot kills someone with rail gun
	{
		"SBD, Silent But Deadly ... and I ain't talking spicy food.";
		"I just don't get to use these ~railgun's often enough.";
		"Woohoo! That was too easy. ~They're taking all the fun out of it.";
		"Choo choo! Train's coming through!";
		"I'd rather have a nail gun.";
		"I'd rather have a 'Thunderbolt'.";
		// 0 = enemy name
	} //end type

	type "kill_gauntlet" //initiated when the bot kills someone with gauntlet
	{
		"'Mr. Glove' just wanted to be friendly, ",0,".";
		"See, I used to have this axe ...";
		"See how close I can get, ",0,"? I can whisper in your ear.";
		"In the old days, we cooked hot dogs like this."
		"Gimme back my axe! I hate this thing.";
		"Way too easy.";
		"No challenge here.";
		// 0 = enemy name
	} //end type

	type "kill_telefrag" //initiated when the bot telefragged someone
	{
		"Call me ~Ranger, human hand-grenade.";
		"That's like putting an 8-track in a cassette deck!";
		"I'm a slip-gating fool! Wahoooo!";
		// 0 = enemy name
	} //end type

	type "kill_suicide" //initiated when the player kills self
	{
		"Way to go, kamikaze. Next time rent a plane.";
		"That's the coward's way out, ",0,".";
		"Trying to go negative on me, eh, ",0,"?";
		"Wait minute ... what's the point of this again?";
		"Go as negative as you want, ",0,".";
		"If you can't handle the heat, ",0,", stay outta the lava.";
		"Lamebrain, ",fighter,".";
		// 0 = enemy name
	} //end type

	type "kill_insult" //insult initiated when the bot killed someone
	{
		"There's a letter your Mommy don't want to see, ",0,"!";
		"I'd take your dog-tags, ",0,", but there's no market for baby bracelets.";
		"Like shootin' fish in a barrel.";
		"Bah! Too easy!";
		"~This is too easy.";
		"~You can't win a match with your thumb in your mouth, ",0,"!";
		"~You quit in the third minute of basic training, right ",0,"?";
		"~Ranger to Base, the enemy is taking heavy losses.";
		"I've had better fights trying to put my pants on in the middle of the night.";
		"I hate fraggin' them before they're old enough to drive.";
		"~You old enough to be here, ",0,"? ~Can I see some ID.";
		"Come back when you've improved enough to be competition, ",0,".";
		"How about giving me a little challenge here, ",0,"?";
		"I'm gonna keep doing this til you get it right, ",0,".";
		"Age and treachery always overcome youth and skill. ~You bet!";
		// 0 = enemy name
	} //end type

	type "kill_praise" //praise initiated when the bot killed someone
	{
		"~You practiced with paint-balls, didn't you, ",0,"?";
		"Well done, ",0,", we'll put a flag on your box.";
		"~You almost got it right, maggot.";
		"~You done good, ",0,". Too bad you won't get to polish your technique.";
		"~You remind me of me in the old days ... course, I got better.";
		"~You've been watching ol' ~Ranger's moves, right, ",0,"?";
		"A solid hit ",0,"!";
		"~Cool. Show me how you did that ",0,".";
		"~You made me work hard for that ~one ",0,"!";
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "random_insult" //insult initiated randomly (just when the bot feels like it)
	{
		"~You got 'retreat' down to a science, ~don't you, ",0,"?";
		"Hey, ",0,", get that butt down. Oh, that's your head.";
		"If I gave myself a sucking chest wound, you'd still lose, ",1,".";
		"Surrender and it will go easier for you.";
		"Wipe your nose, ",fighter,". ~Your whiney sniffling's giving you away.";
		"~You want a little cheese with that whine, ",0,"?";
		"And I thought I didn't know where I was. ~You lost there, ",0,"?";
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type

	type "random_misc" //miscellanous chats initiated randomly
	{
		"Hey, ",0,", is my gig-line straight?";
		"From the Halls of Montezuma ... hmm hmm hmm ... hey, anyone know the rest of it?";
		"Operation ", operation1," ", operation2," is runnin' smooth as silk.";
		"Surrender is not an option.";
		one_liners;
		"I got a cramp. Hey, ",1,", rub my leg.";
		"When you pull out the pin, Mr. Grenade is no longer your friend.";
		"Anyone seen my other leg?";
		"Improve or die, ",fighter,"s. Improve or die.";
		"I want to feel some more of your pain, ",1,".";
		"And the sky was full of planes ...";
		"Hey, ",0,"! I need a ",5,". Think I'll take yours.";
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type
} //end chat Ranger
