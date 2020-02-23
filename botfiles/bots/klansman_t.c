chat "Klansman"
{
	//the teamplay.h file is included for all kinds of teamplay chats
	#include "teamplay.h"

	//======================================================
	//======================================================

	type "game_enter" //initiated when the bot enters the game
	{
		"Bow down before the Messenger of God!";
		"All will suffer!";
		"I'm here for you ",1,"!";
		"~We meet at last ",0,"!";
		"God give me your force!";
		"Death to you all!";
		"I'm collectin' donations for the hungry dead.";
		"Guuuuuu-Waaaaaaah.........!";
		"I come to massacre!";
		"I'm here to cut out the dead wood.";
		"~This place is dead.";
		"I'm NOT gonna need to work my fingers to the ~bone to beat this crew.";
		"How many must I kill before my scale is balanced again?";
		4,". ~This place looks too familiar.";
		"The call of the ages beckons me once again.";
		"Tremble, mortals, Klansman returns to ",4,".";
		// 0 = bot name
	} //end type

	type "game_exit" //initiated when the bot exits the game
	{
		"I shall return ...";
		"I don't frag with wimps, sissies, or ",fighter,"s. That rules out all of you.";
		"I'm rattling on out of here.";
		"Dead man walking ... right on outta here.";
		"Farewell ",2,"!";
		"God be with thee.";
		"My mission here is over.";
		"Another time, perhaps, ",fighter,". I've been around for years.";
		"When ~one is immortal, ~one can afford to bide one's time.";
		// 0 = bot name
	} //end type

	type "level_start" //initiated when a new level starts
	{
		"~You want 'em marked or unmarked? ~Your graves, I mean.";
		"Look how they line up like lambs to the slaughter.";
		"What better time to fight than the dead of the night?";
		"'Into the valley of death rode the ~six ~hundred'!";
		"Tremble, mortals, and more so if you survive.";
		// 0 = bot name
	} //end type

	type "level_end" //initiated when a level ends and the bot is not first and not last in the rankings
	{
		"And I will strike upon thee with vengeance and furious anger...";
		"Eternal is the Arena ... I fear that more than I fear you, ",2,".";
		"BURN!!!!!!!!!!";
		"PRAISE THE LORD";
		"And here I was hoping to finish in a dead heat.";
		2,", you are now wanted 'dead or alive' -- and you know my preference.";
		2,", I hand you my heart. ",3,", you get the butt. Har Har Har!";
		// 0 = bot name
		// 2 = opponent in first place
		// 3 = opponent in last place
	} //end type

	type "level_end_victory" //initiated when a level ends and the bot is first in the rankings
	{
		"Blood flows!";
		"I take no pleasure in killing you ",3,". ~It is expected.";
		"Yeah! ~This is LIVING ... uh, so to speak.";
		"Score ~one for the dead guy.";
		"I should have told you this before, ",3," ... you can't kill me.";
		// 0 = bot name
	} //end type

	type "level_end_lose" //initiated when a level ends and the bot is last in the rankings
	{
		"How dare you?";
		"Hmpf!";
		"If you defeat a thousand, this will remain your greatest trophy, ",2,".";
		"Move on, ",2,". Ascend, and then descend, and know despair.";
		"Keep smiling, ",2,". Life is brief and so is glory, but death is forever.";
		"~It can't be!";
		"I'm dead to the world here.";
		"Dead last again! Arrrrrrrr.";
		"No sense in beating a dead horse.";
		"Looks like I've been given up for dead again.";
		// 0 = bot name
		// 2 = opponent in first place
	} //end type

	//======================================================
	//======================================================

	type "hit_nodeath" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		"Argh!";
		"Tsk Tsk Tsk. Try again!!!";
		"Grrrrr!!!";
		"~You'd better apologize before I find you, ",0,"!";
		"That ~one cut close to the ~bone!";
		"Well, you'll be the death of me ...";
		//0 = shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		"Run! Run little one!";
		0,", you would not last a span of ~five heartbeats in Vadrigar.";
		"~You can not run from death!";
		"~Your armor is gone and soon your life!";
		"Insignificant worm. ~You spill your life upon halls stained by the blood of thousands.";
		"Seek my face and death shall follow!";
		"A marrow Escape. Sorry. Make that A NARROW escape.";
		"~Your life is fleeting!";
		"Honor me with thy substance!";
		"Come to me, you cannot escape!";
		"What shall you do now?";
		//0 = opponent
	} //end type

	//======================================================
	//======================================================

	type "death_telefrag" //initiated when the bot is killed by a telefrag
	{
		"Grrrraaa!";
		"Not yet! ",0,".";
		"Like being cremated and having your ashes scattered.";
		"Anyone for pick up sticks?";
		"~They crowd the Arena, eager to ascend ... but there is only death.";
		"Ashes to Ashes. Dust to dust. Drippy, smelly gunk to drippy, smelly gunk.";
		// 0 = enemy name
	} //end type

	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{
		"My deeds have caused many to fall!";
		"I am on the path of the fallen.";
		"With wisdom comes understanding!";
		"Naaaaaaaaaah!";
		"Brittle as an old ~bone.";
		"That's going ~six feet under the hard way ... no shovel.";
		"I'm drop dead ~sexy!";
		// 0 = enemy name
	} //end type

	type "death_lava" //initiated when the bot dies in lava
	{
		"My Lord! Save me!";
		"The flames are the teacher of understanding!";
		"Arrgh! I burn!";
		"Arrgh! Not cremation!!!!";
		// 0 = enemy name
	} //end type

	type "death_slime" //initiated when the bot dies in slime
	{
		"Slime. Better non-living through chemistry.";
		// 0 = enemy name
	} //end type

	type "death_drown" //initiated when the bot drowns
	{
		"Not a watery grave!";
		"Water, a slow death!";
		"Fallen to a water grave!";
		"Water?";
		ponder," Drowning? Who would have figured?";
		"Worse things happen at sea.";
		// 0 = enemy name
	} //end type

	type "death_suicide" //initiated when bot blows self up with a weapon or craters
	{
		"Consumed by my own undoing!";
		"I have cast myself down!";
		"My power has been taken!";
		"I shall not do this again!";
		"No death is truly final.";
		"~This tragedy is only temporary!";
		"Nothing like digging your own grave, eh?";
		"Hey, beats cremation.";
		"'I still can say ... I did it my way!'";
		"~You see what death looks like, ",0,"? ~Ain't pretty, is it?";
		// 0 = enemy name
	} //end type

	type "death_gauntlet" //initiated when the bot is killed by a gauntlet attack
	{
		"The hunt begins!";
		"Not at the hands of a mortal!";
		"That was like being savaged by a dead sheep.";
		"I'm all broken up about this.";
		"~You touching me ... now THAT's a fate wose than death.";
		"How dare you touch me puny mortal!";
		"~Your mama slaps harder than that, ",0,".";
		"Push any harder, ",0,", and your hand will go right through me.";
		// 0 = enemy name
	} //end type

	type "death_rail" //initiated when the bot is killed by a rail gun shot
	{
		"~You shall not escape me!";
		"Blink once your shot, blink twice your buried!";
		"Forget not, i shall come for thee!";
		"Soon... ~You shall stumble on my rail!";
		"~You choose the rail, you shall also die by the rail!";
		"I respect a fighter like you ",0," but I shall return to haunt you!";
		// 0 = enemy name
	} //end type

	type "death_bfg" //initiated when the bot died by a BFG
	{
		"Aaaaah Hellfire!!!!!";
		"I pray that this mighty weapon is enough power to end my reign.";
		"Just call me ~bone meal.";
		"Ow. When the pony dies, the ride is over.";
		// 0 = enemy name
	} //end type

	type "death_insult" //insult initiated when the bot died
	{
		"That just tickled me to death.";
		"~You'll be dead and gone soon enough, ",0,".";
		"God, forgive me!";
		"~You will regret that ",0,"! I swear it!";
		"~You seek my face and death shall follow!";
		"~You have entered the path of the wicked!";
		"Prepare thy self for hell is near!";
		"~Can you smell death! ",0,".";
		"I have been cast down and so shall you!";
		"~Your tragity shall come suddenly!";
		"I will free your lost soul!";
		// 0 = enemy name
	} //end type

	type "death_praise" //praise initiated when the bot died
	{
		"The hunt has just begun, ",0,"!";
		"Luckily for us, ",0,", the colonies did not face an opponent of your skill.";
		"Even the strong can be slain!";
		"Luck has found you, ",0,"!";
		"~Your next ugly devil!";
		"Fallen to a mortal!";
		"I honor you in deaths prayer, ",0," ... may your fall will be as swift as your rise.";
		"And here I'd already given you up for dead, ",0,".";
		"~You had me dead to rights there, ",0,".";
		"'Because I could not stop for death, he kindly stopped for me'.";
		"~You were fun. I'll stick around in your nightmares, I promise.";
		"Bring out your dead! Bring out your Dead!";
		// 0 = enemy name
	} //end type

	type "death_kamikaze" //initiated when the bot is killed by kamikaze blast
	{
		"~It is a good day to die.";
		"I am spread to the winds, a fine mist, floating upon fate's dark vapours.";
		"Anyone find a left tibia? I'm missing a left tibia here.";
		"Ashes to ashes, dust to dust.";
		0,"! Thou'rt a pestilence upon the land!";
		"~You see that babe on the explosion? What a hottie!";
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================
	type "kill_kamikaze" //initiated when the bot kills someone with kamikaze
	{
		"I'm just bad to the bone.";
		"I've been dying to try this out. Dying I tell you.";
		"Call me MR. Death today.";
		"All the destruction, half the romantic poetry.";
		"And it even has my picture on it ...";
		"Tsk. Tsk. Add gluttony to my list of sins.";
		"Personalized death, discount prices.";
		// 0 = enemy name
	} //end type

	type "kill_rail" //initiated when the bot kills someone with rail gun
	{
		"Fell the Holy light ",0,"?";
		"That's one less rebel!";
		"How's that for a 'kiss of death'?";
		"Quite an improvement over black powder and lead shot.";
		"That had to be good for a broken ~bone or ~two.";
		"Sometimes it is preferable not to hear the serpent's rattle.";
		// 0 = enemy name
	} //end type

	type "kill_gauntlet" //initiated when the bot kills someone with gauntlet
	{
		"God forgives you, ",0,"!";
		"I prefer to look into the eyes of my prey when I feed.";
		"More knuckles than you're used too, eh, ",0,"?";
		"~This gauntlet thingy gnaws right to the ~bone.";
		// 0 = enemy name
	} //end type

	type "kill_telefrag" //initiated when the bot telefragged someone
	{
		"Here I come, heh.";
		"Step aside, ",0,". Demigod coming through.";
		"The fates have deemed you unworthy ",0," ... obviously.";
		"That pretty much scatters your ashes, ",0,".";
		"Grind you up for bonemeal!";
		// 0 = enemy name
	} //end type

	type "kill_suicide" //initiated when the player kills self with a weapon of craters
	{
		"Tsk Tsk Tsk.";
		"Wh-What??";
		"If I was you, ",0,", I'd be turning in my grave!";
		"~You normally got ~one foot in the grave like that, ",0,"?";
		// 0 = enemy name
	} //end type

	type "kill_insult" //insult initiated when the bot killed someone
	{
		"Feel my WRATH, ",0,"!";
		"~Didn't your ",random_counselor," tell you never to hunt rabbit with a dead dog?";
		0,", that last move of yours was a dead giveaway.";
		"~You've slammed into yet another dead end, ",0,".";
		"Gotcha! ~You are dead as a doornail.";
		"That ought to put the fear of death into you, ",0,"!";
		"In a thousand years, I have never seen ~one as pitiful as thee, ",0,".";
		"~You're better off dead, ",0,".";
		"~You have just reached a dead end, ",0,".";
		"Sic Semper Tyrannis!";
		"Meet your creator, ",0,"!";
		"HAHAHAHAHAHA.";
		"Farewell, ",0,"!";
		// 0 = enemy name
	} //end type

	type "kill_praise" //praise initiated when the bot killed someone
	{
		"Listen, ",0,", being dead and gone ain't as bad as it seems.";
		"I had you dead to rights there, ",0,".";
		"Wrong. Dead Wrong.";
		"~Do not be ashamed, I have seen empires rise and fall.";
		"~It will comfort you to know that I have not had this much fun in ages.";
		"I can see that you fight for what you believe in.";
		"I will write your name in my great book ... that, I have not done for centuries untold.";
		"The Lords shall be grateful that I killed you. ~You would have brought them pain, ",0,".";
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "random_insult" //insult initiated randomly (just when the bot feels like it)
	{
		"~Your streets shall flow with your blood!";
		"I will find you even in the dark corners!";
		"I am unstoppable, ",0,"!";
		0,", you are already dead from the neck up. So a little more won't hurt.";
		0," you are a dead man walking.";
		"Oh drop dead already, ",1,".";
		"Patriotrism is indeed the last resort of a scoundrel.";
		"Good thing I don't lose frags for being bored to death.";
		"~Don't roll the ~bones with death and expect to win. His dice are loaded.";
		"~You shall not escape hell!";
		"I will not stop until the end.";
		"Death is calling!";
		"~This will be the end of you!";
		"Burn baby, burn!";
		"Spell yer name for me, ",0,"? I'm doing up yer gravestone now.";
		"",curse," you, ",0,"!";
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type

	type "random_misc" //miscellanous chats initiated randomly
	{
		"Make America great again!";
		"America first! America first!";
		"I shall deliver you to the grave!";
		"Make America white again!";
		"I want to see ",liquidmeasures," of blood!";
		"'Better dead than red'? ... sounds like blue-centric bigotry to me.";
		"Throw me a frickin' ~bone here.";
		femalebot, " is drop dead gorgeous!";
		"'The wages of sin are death' ... and I am handing out paychecks here.";
		"Death is just a part of life ... the last part.";
		"Hey life goes on. Death just goes on longer.";
		"General Washington gave me this scarf.";
		"To fight and die for what ~one believes, that is a worthy life.";
		"I fought and died to make men free ... many times.";
		"~You remind me of that Benedict Arnold character, ",0,".";
		"Why yes, Ben Franklin did program me.";
		"Yeah, I still miss my old girlfriend, ",femalebot,". But my aim is getting better.";
		one_liners;
		"Anyone got a bottle of ",response,"? I'm dry as a ~bone here.";
		"Hey! ~You seen those cuties in Deva Station ~quad room?! WoooHooo!";
		"I'm outta the closet now. Say it now and say it loud. 'I'm dead and I'm proud'.";
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type
} //end chat Klansman
