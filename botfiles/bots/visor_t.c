chat "Visor"
{
	#include "teamplay.h"
	//
	type "game_enter"
	{
		"Hey, we're in ",4,"! I thought we trashed this place last time.";
		"Heh. Fresh Meat.";
		4,"? Super! Visor owns this place!";
		"Time to Rock 'n Roll.";
		"Oh yeah! ",4," is where ~Visor lays down the law!";
	} //end type

	type "game_exit"
	{
		"~You know I'll be back.";
		"I'm history. Later kids.";
		"I'm too good to waste my time with you ",1,".";
		4," makes for a level playing field ... but only for me.";
	} //end type

	type "level_start"
	{
		"~This place rocks!";
		4," rocks!";
		"Keep on bowing to me, ",fighter," ... I'm aiming for your bald spot.";
		"I'm ready to win.";
		"Who needs a lesson today?";
		"Alright. Make me look good.";
	} //end type

	type "level_end"
	{
		"I will accept this as yet another learning experience.";
		"Perhaps luck won't favor you next time, eh?";
		"I won't make the same mistakes next time.";
		"Only winning matters here.";
		"Awwww. I was having too much fun chopping you guys up.";
		"Hey, ",2,", you got in my way. ~You're gonna be lunchmeat.";
		"Okay, ",3,", why ~don't you just let ",2," kill you again? ~You guys working together?";
	} //end type

	type "level_end_victory"
	{
		"I deserve this!";
		"~This is what it's all about, kid!";
		"~This is the reason you ",fighter,"s are here. To make me look good.";
		"Took me longer to learn how to tie my shoes than how to frag you pus balls.";
		"Call housekeeping. Stains everywhere.";
		"~Don't cry, ",3,". Keep trying. I like stomping you.";
	} //end type

	type "level_end_lose"
	{
		"Grrrr.";
		"Luck. Pure luck.";
		"Got a hangnail on my trigger finger ... yeah, that's it.";
		"Okay, so it didn't work with my eyes closed. ~They're open now.";
		"Hey, ",2," is using my tactics! ~He's gotta be!";
		"Next time, your butts are mine.";
		"Next time, your butt is mine, ",2,"!";
	} //end type

	//======================================================
	//======================================================

	type "hit_nodeath" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		"~You actually hit me, ",0,". I'm impressed, kid.";
		"So kid, you going to make a habit of dinking ol' ~Visor?";
		"Another scar to remember you by, ",0,".";
		"I've stopped counting the scars.";
		"Hey ... they told me this armor was bulletproof.";
		"That hurt, kid. But not enough to matter.";
		"~You had your chance, ",0,". ~You've lost and just don't know it yet.";
		"~You've marred perfection, ",0,". ~You've made perfection mad. ~This is gonna hurt.";
		"~You can't take down this body with that little gun, pus ball.";
		"Was I supposed to yell ouch? ~Ain't gonna happen, ",fighter,".";
		"~Don't you go blaming your lack of skill on cheap guns and poor ammo.";
		"I'm immortal. Hitting me won't kill me.";
		//0 = shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		"~You survived? ~It's time to show you my latest move, ",0,".";
		"I win the simple way, by out-fragging the competition.";
		"I hope you heal fast kid, because there's more where that came from.";
		"That'll sting in the morning.";
		"That's it. Run for the health. I know where to find you.";
		"Cheap guns and bargain basement ammo loads are throwing me off my game.";
		"Off the wall, around the corner, nothing but net.";
		"When they're shaking this hard, you gotta shoot center mass.";
		"Clone ",0," a ~hundred times and you got a carnival duck shoot.";
		//0 = opponent
	} //end type

	//================================================================
	//================================================================

	type "death_telefrag"
	{
		"There's only room for ~one of us behind the mask.";
		"Too much power in ~one place! 99% of it was mine, but ...";
	} //end type

	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{
		0,", put your squishy little head under this ...";
		"~Didn't think I could jump that far, now did ya?";
		"All this armor ... and I go and land on my head.";
		"Alright, who pushed me?";
		// 0 = enemy name
	} //end type

	type "death_lava"
	{
		"Lousy stuff.";
		"~This stuff makes me mad.";
		"Whose idea was this?";
		"Just a shortcut, ",fighter,". I'll be rising right behind you!";
		"Where's the 'Under Construction' sign? Who built this freakin' place?";
	} //end type

	type "death_slime"
	{
		"I never get used to that stuff.";
		"Get that slime in your cybronics and they're never the same again.";
		botnames," put that here! I know it! ~They're all afraid I'll climb the ranks.";
		"Gurp!!";
		"I did that on purpose.";
		"There's never a personal teleporter when you need ~one.";
	} //end type

	type "death_drown"
	{
		"I knew I should have packed a snorkel for this trip.";
		"Wish this ~visor was fitted with a scuba hook up.";
		"I got time to do a little fishing and still frag you, ",0,".";
		"No way I'm going out like this! No bl.bl.bl.way ... glub.";
		"~Must ... FEEESH!";
	} //end type

	type "death_suicide"
	{
		"~Don't laugh kid, you've probably done this a lot.";
		"I've gone and made a mess of myself again.";
		"Unfriggin'-believable.";
		"~Ain't what it seems ... you bored me to death.";
		"~You see how it's done, ",0,"? Okay, now you do it.";
		"~Don't laugh, ",0,", you're next!";
	} //end type 

	type "death_gauntlet"
	{
		"~Will you stop touching me?";
		"~Do that again and you are toast.";
		"I thought you said we were gonna arm wrestle. Cheatin' pus ball!";
		"~You'll never wash that hand again, will you, ",0,"?";
	} //end type

	type "death_rail"
	{
		"Heh. Nice shot kid.";
		"I hate campers. I really do.";
		"~Did ~Xian show you how to do that?";
		"I'm a force of nature ... and nature always wins, ",fighter,".";
		"Hide and shoot. Figures for a ",fighter," like you, ",0,".";
		"Afraid to face me square, eh, ",0,"? ~You're smarter than you look.";
	} //end type 

	type "death_bfg"
	{
		"Meteor Shower from Hell!";
		"~Had to call in the big guns for me, didn't you, ",fighter,"?";
		"Loser!";
		"No-skill puke!";
		"Are you roasting marshmellows over there, Mr. Camper?";
	} //end type

	type "death_insult"
	{
		"Got any ammo left, punk? ~You're gonna need it.";
		"~Did you enjoy that?";
		"I'll bet you enjoyed that more than I did, ",0,".";
		"I knew you were a pus ball, ",0,", so I let up on you.";
		"I've never lost when I was trying to win.";
		"My shot must have bounced off your head, ",0,", and fragged me.";
		"That cheap frag is gonna cost you big time, ",0,"!";
		"I'm keeping track of these cheap shots for later.";
		"Blast! Shot in the back by ",0," again!";
		0,", your Judgement Day is on its way!";
	} //end type

	type "death_praise"
	{
		"Oh, yeah. ~You had me good there kid.";
		"Not bad kid.";
		"Work on your follow through, kid.";
		"~You copied that move from me, didn't you, ",0,"?";
		"I didn't like where they put my head anyways.";
		"Brag about this to just ~one guy and I'll use your tongue to clean your ears.";
		"If I didn't let 'em win once in a while, they'd lock me out.";
	} //end type

	type "death_kamikaze" //initiated when the bot is killed by kamikaze blast
	{
		"Kid, you don't know how much that just cost you.";
		0,", you don't know how much that just cost you.";
		"Get ready to fast-forward to the end kid.";
		"All weapons are good weapons. I just rather they were used elsewhere.";
		"Generally speaking, kid, it takes skill to progress onward from here.";
		"Were you getting jiggy with that kamikaze there, ",0,"?";
		"~You're not licensed for that weapon, are you ",0,".";
		"Big deal. I once wiped out fourty ",fighter,"s in a single blast with it.";
		"^1**NUKED**";
		"I like you, ",0,". Nobody else masters the cheap shot like you do.";
		"~You almost missed with that.";
		"Bah. I should have expected a trick like that from you, ",0,".";
		DEATH_KAMIKAZE3;
		// 0 = enemy name
	} //end type 


	//======================================================
	//======================================================

	type "kill_kamikaze" //initiated when the bot kills someone with kamikaze
	{
		"Sorry. ~Didn't mean to blow up at you folks.";
		"Great thrill ride, but over too quick for my money.";
		"Guess it's just not your day today, kid.";
		"Guess it's just not your day today, ",0,".";
		"Who wants another one just like that?";
		"None shall pass!";
		"I OWN you all!";
		"Bow before my godlike power!";
		"I RAWK BABY!!";
		"~You can keep the change!";
		KILL_KAMIKAZE1;
		// 0 = enemy name
	} //end type

	type "kill_rail" 
	{
		"Yessssss!!";
		"Gotcha!";
		"~You're it!";
		"~Railguns rule my universe!";
		"~Don't whine about things you can't control, ",0,".";
		"I didn't even shoot that ... I threw it.";
	} //end type

	type "kill_gauntlet"
	{
		"~Thanks, ",0,". I love doing that!";
		"Humiliation is the name of the game, kid.";
		"Hey, ",0,". When will you learn to just kneel before me?";
		"~It's like mole wacking at the carnival!";
		"Heh. And I consider stealth the least of my talents.";
		"Slap bag, zipperface.";
	} //end type

	type "kill_telefrag"
	{
		"Stand aside, kid. Whoops. Too late.";
		"Nothing but memories left there.";
		"Behold the power of a sneeze.";
		"Pop!";
		"When I stomp, I ^1STOMP!";
	} //end type

	type "kill_suicide" //initiated when the player kills self
	{
		"Facing me, your actions are understandable.";
		"I'm gonna paint a big 'L' on your toe tag, LOSER.";
	
		// 0 = enemy name
	} //end type

	type "kill_insult"
	{
		"Somebody wanna clean this mess up?";
		"~Can't you at least make this interesting, ",0,"?";
		"Why ~don't you just give up ",0,"?";
		"And I'm still just warming up.";
		"Pick up your toys and go home kid.";
		"Here lies, ",0,", the scab. Fought a god and bought a slab.";
		"Here lies, ",0,", the whining puke. ~He'd have had more chance if he swallowed a nuke.";
		"~You never had a chance. ~Do you know how good I am?";
		"I was told you were competition, ",0,". Somebody got that wrong.";
		"What was that? ~You can't even die right!";
	} //end type

	type "kill_praise"
	{
		"Not bad, kid. Most don't last that long.";
		"Almost didn't get you there, kid.";
		"No shame in this, ",0," ... you were killed by the best.";
		"~Three more seconds, ",0,", and you'd have set a new survival record against me.";
		"If they cloned you ~ten times and you fought me together, you might have a chance, ",0,".";
	} //end type

	//================================================================
	//================================================================

	type "random_insult"
	{
		"~You will learn to fear me.";
		"I will be your personal nightmare, kid.";
		"Get over here, ",0,". I want to press your face in dough and make monkey cookies.";
		"I could kill you with a look, ",0,", but then I'd have to look at you.";
		"Bring me a bag of ",substance,", and I might let you live next time, ",1,".";
		"Deity in the house!";
	} //end type

	type "random_misc"
	{
		"Victory will be mine!";
		"~You can't win, ",0,". ~It's not in you to be a winner.";
		"~This is all I know. No ~one can take it from me.";
		"~They come by the millions, but I turn them all back.";
		"Nothing fancy here, just solid skill.";
		"I would be proud to be to your father, ",0,".";
		"I am the god of frags, and ",4," is my temple!";
		"Someone tell me what it's like to get fragged. I just can't imagine it.";
		"~Don't you EVER call me ~Al!";
		"I just use the teddy bear to keep my knees in the proper position at night.";
		"I practice with a mirror. I'm the only ~one good enough for me to learn from.";
		"Mmmmmmm, ",food,".";
	} //end type
} //end chat Visor
