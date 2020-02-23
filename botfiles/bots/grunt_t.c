chat "Grunt"
{
	#include "teamplay.h"
	//
	type "game_enter"
	{
		"Anyone standing in my way is gonna be dogmeat.";
		"A ~Marine laughs at danger. And I see nothing here to laugh about.";
		"I can see that the words 'no contest' mean something here.";
		"I am jacked in and good to go.";
		"~You ",fighter,"s are stinkin' the place up.";
		"I see ",1," is stinkin' up the place again.";
		"Outstanding!";
		4," is my kind of place!";
		"How's it hangin'?";
		"Nice place you got here. ~Could use a few gibs though.";
		"I lived in worse places than, ",4,".";
		"All the comforts of home and good company too.";
	} //end type

	type "game_exit"
	{
		"I am so outta here.";
		"Gotta run. Happy fraggin'.";
		"~It's time for me to ~bug on out of here.";
		"No ~bugs to squash in ",4,".";
	} //end type

	type "level_start"
	{
		"OK you rubber monkeys, make me look good.";
		"OK, time for you ",fighter,"s to make me look good.";
		"Let's rumble.";
		"I'm psyched up. Let's get it on.";
		"Show me what y'll got, ~bug-lover.";
	} //end type

	type "level_end"
	{
		"At least I'm not in the basement on this ~one.";
		"Next time, let's all hammer on ",2,".";
		"Always the fun ends too soon.";
		"Now I know to focus my attention on ",2," and not ",3,".";
		"I feel like I just kissed my sista'.";
		"I might as well start raising pigeons.";
		"I'm definitely out of my league here.";
		"~You're excused, ",3,". But ",2,", I'm gonna pay you back good.";
	} //end type

	type "level_end_victory"
	{
		"Winning ain't everything ... but losing sure stinks, right, ",3,"?";
		"Wasn't my turn to win today ... so I took someone else's.";
		"My momma didn't raise me to be a loser.";
		"What else would ya'll expect from an elite killing machine?";
		"Move like a dragonfly, hit like a tank, I looked pretty, but you don't even rank.";
		"I expect you to salute your superiors, ",3,".";
		"~It's survival of the fittest.";
		"~You ",fighter,"s can all go home now!";
	} //end type

	type "level_end_lose"
	{
		"Losing make me mad. Real mad. Sort of homicidal.";
		"Losing sucks big time.";
		"Next time, let's all hammer on ",2,".";
		"I'm a disgrace to the Legion.";
		"Go hunt some ~bugs, ",2,".";
	} //end type

	//================================================================
	//================================================================

	type "death_telefrag"
	{
		"Get out of my head!";
		"~What is this? A friggin' turf war?";
	} //end type

	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{
		"I wish I was wearing my game shoes.";
		"Double-pump, 360 slam!";
		// 0 = enemy name
	} //end type

	type "death_lava"
	{
		"I normally like the hot stuff.";
		"Flip me over when I'm about medium rare.";
		"And I thought Louisiana Hot Sauce was hot.";
		"Man! Almost as hot as a Texas summer.";
		"How do they keep this stuff from setting up?";
	} //end type

	type "death_slime"
	{
		"Bah! I drink coffee stronger than that.";
		"AAAAAAAAH! ~Bug dip.";
		"Reminds me of the ",liquid," we used to get at the mess hall.";
	} //end type

	type "death_drown"
	{
		"Guess I was having too good a time down there.";
		"There's Pirate Treasure down here!";
		"~They didn't cover this in basic.";
		"Well that's just great. Who broke the hydrant?";
		"Instant death, just add water.";
		"Now that's a water slide!";
	} //end type

	type "death_suicide"
	{
		"I ought to get hazard pay for this.";
		"I'm better than that. Honest.";
		"I swear somebody rearranged the arena.";
		"Heh. ~Had me a terminal case of hangnail there.";
		"That toe fungus just sneaked up and grabbed me.";
		"That's not right.";
		"I can't believe I did that.";
		"I ~don't plan on repeating this.";
		"I never was a good rebounder.";
	} //end type

	type "death_gauntlet"
	{
		"~You gotta have a lotta guts to come here and do that, ",0,".";
		"I'm impressed. Now it's my turn.";
		0," ~You are in for a beating now.";
		"~Thanks for sharing that with me, ",0,".";
		"I might as well put on a dress and go back to the pen.";
		"Lean on the car and spread 'em, right?";
		"~Do that again and I'll cut you, man!";
	} //end type

	type "death_rail"
	{
		"OK, that was a good shot.";
		"That just ruins my day. Now I gotta ruin yours, ",0,".";
		"I didn't know I was within ~five feet of the fence.";
		"That gun don't mess around.";
		"Wow. Where did that shot come from?";
		"Nice shot ",0,"!";
		"Great! ",0," gets lucky once and I'm the target!";
	} //end type

	type "death_bfg"
	{
		0,", I'm tempted to grab that thing and shove it ...";
		"Use that thing once more and you're gonna eat it ",0,".";
		ponder," I remember that weapon being a lot scarier.";
		"Now I know what a piece of toast feels like.";
		"Use a skill weapon next time.";
		"~You want fireworks? I'll give you friggin' fireworks!";
	} //end type

	type "death_insult"
	{
		"I gave you an easy shot there ",0,".";
		"OK, ",0,", now try that on a moving target.";
		"I'm gonna wrap that stinkin' ",1," around your neck ",0,"!";
		"Nobody does that to Grunt and just walks away from it.";
		"I don't take that kind of crap from ",fighter,"s like you, ",0,"!";
		"Lousy stinking ... Camper!";
		"Oh, so you want to play rough?";
		"~You piece of crap! Are you wearing a rabbit's foot?";
		"Stinkin', ",fighter,"! What kind of a shot was that?";
		"I didn't enjoy that.";
		"Blast!";
		"I'm taking off the kid gloves now, ",0,"!";
		"That was your last free shot, ",0,"!";
		"~You better have some tricks you haven't shown me yet, ",0,".";
	} //end type

	type "death_praise"
	{
		"Not half bad there ",0,".";
		"~This could be more difficult than I thought.";
		"~You been taking lessons, ",0,"?";
		"Looks like you can give some pointers about the ",1,", ",0,".";
		"Consider yourself lucky, ",0,". ~You just offed a bonafide war ~hero.";
		"Just get my name right on the tombstone, ",fighter,".";
	} //end type

	//======================================================
	//======================================================

	type "hit_nodeath" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		0,", buddy, you are going to regret not fragging me in ~one shot.";
		"I've had worse cuts shaving, ",0,".";
		"Nice poke there, ",0,". Too bad you didn't finish the job.";
		"Looks like you just failed target practice today, ",0,".";
		"Either I'm getting slow or you are using a sloppy weapon.";
		"I must have a tracer on me.";
		"~Don't play games with me, ",0,".";
		//0 = shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		"Yeah!! I could feel that ~one hit all the way over here.";
		"~You survived that, ",0,"? I must be gettin' old.";
		"I think I found my range with that hit.";
		"I wish I had my .44.";
		"Look at ",0," go! Bet that ",fighter," could outrun police dogs!";
		"Last time I shot something that ran as fast as you, I cooked it for dinner.";
		"Be nice if they give us some live ammo.";
		//0 = opponent
	} //end type

	type "enemy_suicide"
	{
		"Dang it, ",0,". I ~don't need your help to win here.";
		"That can't be good for your complexion ",0,".";
		"Dr. Death would be so proud of you ",0,".";
		//0 = enemy
	} //end type

	//================================================================
	//================================================================

	type "kill_rail" 
	{
		"'Old Betsy' still works like a charm.";
		"I just love modern technology.";
		0,", consider yourself terminated with extreme prejudice.";
		"~Thanks for being such a great target ",0,".";
		"Am I good or what?";
	} //end type

	type "kill_gauntlet"
	{
		"Feel the ^1POWER!";
		"Ow. My hand hurts now.";
		"Beats a hand blaster any day.";
		"Gonna have me a weiner roast; no need to build a fire ...";
	} //end type

	type "kill_telefrag"
	{
		"Blood and gore all over the floor and me without a spoon.";
		"Get your sorry, ",fighter," ~ass off my turf!";
		"There's ~one monkey off my back.";
	} //end type

	type "kill_insult"
	{
		"That's another score for the good guys.";
		"Of course I'm picking on you, ",0,". ~You're an easy target.";
		"I'm trying to cure you of the uglies, but it ain't working.";
		"Bag 'em, tag 'em and drag 'em!";
		"~You'e slowing down, ",0,". That was far too easy.";
		"I'm so good I even impress me!";
		0,"'s Arena career looks to be a short ~one.";
		"The ~bugs in the kitchen gave me more of a fight than you did, ",0,".";
		"Let's hope you never go to prison.";
		"Maybe next time I just use my bare hands.";
		"Next time, I'll do it blindfolded and backwards.";
		"Cooperate and we'll all be done much sooner.";
	} //end type

	type "kill_praise"
	{
		"~You're a hard foe to pin down, ",0,".";
		"Dang, you're good, ",0,"! That frag should count double.";
		"For a moment there, I thought you were a ~Marine, ",0,".";
		"~You consider yourself honored. I'm a trained professional.";
	} //end type

	type "death_kamikaze" //initiated when the bot is killed by kamikaze blast
	{
		"Up until now, I thought you were one of the good guys, ",0,".";
		"~You got any more internal organs you'd like to share with us, ",0,"?";
		"I.";
		"~You almost missed with that.";
		"Bah. I should have expected a trick like that from you, ",0,".";
		"What the ... am I wearin' a red shirt today?";
		DEATH_KAMIKAZE0;
		// 0 = enemy name
	} //end type 


	//======================================================
	//======================================================

	type "kill_kamikaze" //initiated when the bot kills someone with rail gun
	{
		"I get the points whether you whine about it or not!";
		"I am out of control here!";
		"Anyone get a photo of that?";
		"Yeah! Feel the burn, you ",fighter,"s!";
		"Come an' get it!";
		"~Ain't nobody here gonna be shooting back too soon.";
		"~You can take THAT to the bank.";
		"~Don't you say nothin' 'bout my momma.";
		"Yo! Superfly! Dodge THAT!";
		"I see dead people.";
		KILL_KAMIKAZE1;
		// 0 = enemy name
	} //end type
	
	type "random_insult"
	{
		"If only the good die young, I think you just might live forever ",0,".";
		0,", I don't think you can handle me.";
		"And here I thought this match was going to be challenging.";
		"~Does your momma know where you are, ",1,"?";
		"Firing them video guns in the arcade don't count as field training.";
	} //end type

	type "random_misc"
	{
		"I'm too hot for my own good.";
		"I've got a grenade with the protofiend's name on it.";
		"I've got a grenade with ",0,"'s name on it.";
		"I'm ready to dance when you are ",0,".";
		"I can't believe I still have my safety on.";
		"I could go for a big plate of ",food,", the way my momma made it.";
		"I ain't yo' brother.";
	} //end type
} //end chat Grunt
