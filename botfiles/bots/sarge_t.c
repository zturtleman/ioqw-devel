chat "Sarge"
{
	#include "teamplay.h"
	//
	type "game_enter"
	{
		"Mmmmm... Smells like fresh meat.";
		"~TEN-SHUN! ~Sarge is in the House.";
		"Alright, ~Hero. ~It's time for some Pain.";
		"*SNIFF* I smell victory in the air today.";
		"~You think you got what it takes to beat me, ",1,"?";
		"Make it challenging!";
		"Time to face your deepest fears, ",1,". ~Have you got it in you?";
		"Let's give the alien bastards a show they won't soon forget.";
	} //end type

	type "game_exit"
	{
		"Keep up the pace soldiers. I'm up for some ~R and ~R.";
		"Duty calls. Try to keep from fragging yourselves.";
		"Duty calls.";
		"Anyone for getting lunch? ",food," sounds good to me!";
	} //end type

	type "level_start"
	{
		"Let the operation commence!";
		"Battle stations!";
		"Prepare to repel the enemy!";
		"Once more into the breach!";
		"Alright, Heroes. ~It's time for some ^1Pain.";
		"~Everyone drop and give me 20!";
		"Let's get this over with.";
		"~It's time to separate the warriors from the kiddies.";
	} //end type

	type "level_end"
	{
		"Well fought troops. Well fought.";
		"I think we all earned a medal for that ~one.";
		"~You call that a good match? Sloppy. Very sloppy.";
		"Finally it starts to get interesting.";
		"Betrayed into the hands of fiends again.";
		"Unacceptable! I will not let my superiors down!";
		"Very challenging.";
		"Getting out of here while I still can.";
	} //end type

	type "level_end_victory"
	{
		"~You call that effort? ~You're not worthy of my ordinance, pissant.";
		"Yeah! Better luck next time, ",3,".";
		"~You gave it a good shot, ",3,", but next time I won't be so gentle.";
		"I'm feeling mighty fine right now. Mighty fine indeed.";
		"Yeah! The sweet smell of victory.";
		"I wish my momma could've been here to see this day.";
		"A true leader will always finish first!";
		"Careful planning and good execution are the keys to victory.";
		"I win ... and no ~one is surprised.";
		"Pfah! I think I still have ",3,"'s entrails on me!";
		"Bag them and tag them!";
	} //end type

	type "level_end_lose"
	{
		"Frag off. I'll see you again, Jumping Junior.";
		"Nice shootin' Fragbait. Just don't be getting' cocky.";
		"Little Pig, you just won by the hair of your chinny chin, chin.";
		"Fragbait.";
		"There will be some serious payback for this.";
		"Gloat while you can, fragbait. Gloat while you can.";
		"Looks like ol' ~Sarge's tactics could use some re-evaluation.";
		"~One promise: this is not going to happen again.";
		"I do what I do to preserve the human way of life. ~Do ~not question me.";
		"I no longer trust ",2," to fight fairly.";
		"~This was not an acceptable outcome to this mission.";
		"~You're going to die horribly, ",2,"! ~Your slimy tactics insult us all!";
	} //end type

	//======================================================
	//======================================================

	type "hit_nodeath" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		"~You comin' on to me, Private?";
		"Bring it on! Bring it on!";
		"That pin-prick is not going stop me from dominating ",4,".";
		"Well done, ",0,". ~Your skills are indeed worthy. ~Shall we duel?";
		"Typical attack of the under-educated opponent!";
		//0 = shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		"~It'd be quicker if you just hold still.";
		"That's just me bein' friendly.";
		"Bravo, ",0,"! ~You make a superior practice target.";
		"Bah, it is merely a flesh wound. Stand up and start running!";
		"Okay, ",0,", I will accept your unconditional surrender.";
		"The chase! Ah, the thrill of the chase!";
		"Use that medpack now, or the next shot's for the money!";
		"Drop the ",1," and step away from the wall!";
		"That was just a ranging shot, ",0,".";
		//0 = opponent
	} //end type
	type "enemy_suicide"
	{
		"~This your first day with that weapon, ",0,"?";
		"Get that ",fighter," to the infirmary!";
		"Barely enough to send home to Momma.";
		"~You have a very unusual technique, ",0,". ~Is this to confuse your enemies?";
		"~Three words, ",0,". 'Be more careful'.";
		//0 = enemy
	} //end type

	//================================================================
	//================================================================

	type "death_telefrag"
	{
		"Rrrrrr.";
		"Well I'll be a son-of-a ... gun.";
		"That's gonna cost you a month of K-P duty, ",0,"!";
		"Quite a concept ... being everywhere at once!";
		"I am indeed 'a real nowhere man'.";
		"Sharing the same space with ",femalebot," would be a treat.";
	} //end type 

	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{
		"I did better in jump school.";
		"That cursed ledge needs a safety ~rail.";
		// 0 = enemy name
	} //end type

	type "death_lava"
	{
		"I didn't come here to be barbequed.";
		"Blast! Was it my turn to bring the marshmallows?";
		"Sprinkle my ashes among those of my fellow officers.";
		"Never a ~cigar when you need one.";
	} //end type

	type "death_slime"
	{
		"Horrible stuff.";
		"I think I had that slime crap in an MRE once.";
		"~It's so hard to tell the difference between real slime and the aliens.";
		"Argh! Bioweapons!";
	} //end type

	type "death_drown"
	{
		"I'm Army, not some waterlogged ~Marine.";
		"I bet I can hold my breath longer than you.";
		"~It's a good thing my battle armor is waterproof.";
		"~This is not what you think ... it just makes it easier to do push-ups.";
	} //end type

	type "death_suicide" //includes weapon, cratering, & trigger_hurt
	{
		"Accidents happen.";
		"~This battlefield is booby-trapped!";
		"I did that to make the odds more fair.";
		"That's no way for a warrior to die.";
		"There are worse ways to die ...";
		"No ~one said ANYTHING about land mines!";
		"Pay attention soldier, a classic example of what not to do!";
	} //end type

	type "death_gauntlet"
	{
		"~You comin' on to me, ",fighter,"?";
		"~You're allowed to do that just once, soldier.";
		"That's an ugly way to die.";
		"My gloves will come off if you wish to make war like this!";
		"~Is that the same ",1," that you were picking your nose with, ",0,"?";
	} //end type

	type "death_rail"
	{
		"~Everyone gets lucky, once in awhile, ",0,".";
		"Dirty, rotten, lousy sniper.";
		"A lucky shot, ",0,".";
		1," is the perfect weapon for cowards like ",0,".";
	} //end type

	type "death_bfg"
	{
		"A real soldier can't depend on weapons of mass destruction.";
		"Next time, Private, face to face like real soldiers.";
		"Worthless fragbait.";
		"So this is the light at the end of the tunnel everyone talks about.";
		"Who allowed ",0," to play with daddy's gun?";
		"I think this passes the weapon effectiveness evaluation.";
	} //end type

	type "death_insult"
	{
		"Wipe that smile off your face, ",0,". ~You ~ain't out of the woods yet.";
		"Anybody can be lucky once, fragbait. ~Do you understand me?";
		"~You can dish it out, ",0,". But can you take it?";
		"My dog could have done that better and quicker.";
		"Sloppy, soldier. Very sloppy.";
		"That was my favorite cigar back there. Now I'm angry.";
		"Finally! ~Is someone else doing the aiming for you now, ",0,"?";
		"~You've had your lucky shot for the day, ",0,".";
		"I'll bet you expect to get a medal for doing that, eh ",0,"?";
		"Blast! Allowing you to kill me ",0," is an insult to this uniform.";
		"Bah. I'll wager you can't repeat that accident.";
		"Coward. Look me in the eye next time if you dare.";
	} //end type

	type "death_praise"
	{
		"Nice shootin', Fragbait. Just don't be getting' cocky.";
		"Well, ",0,". There's hope for you yet, soldier.";
		"Good shooting.";
		"~You see, ",0,", practice does pay off in the long run.";
		"I see you've been practicing again, ",0,".";
		"~We'll make a soldier out of you yet, ",0,".";
		"Congratulations, ",0,", I am victim to your victor.";
		"With you in my squad, we could take on the rest of these ",fighter,"s.";
		"I salute you for a shot well placed, ",0,".";
		"~You aren't making this easy for me ",0,".";
	} //end type
	
		type "death_kamikaze" //initiated when the bot is killed by kamikaze blast
	{
		"Fragbait, you are about to experience a personal tragedy.";
		"~You use enough nukes and you might have a chance around here, ",fighter,".";
		"That might help you win, ",0," ... and I just might be voted Miss Quake III Arena too.";
		"~You shoulda saved that for later, ",0,". 'Cause now your in deep doo-doo.";
		"I got nothing good to say to you, ",0,".";
		"~You waste life so casually for one so young, ",0,".";
		"Rrrrrrrrrr!";
		"Soldier, the penalties for war crimes like that are severely permanent.";
		"In other circumstances, that act would make you a hero, ",0,".";
		"Soldier, you're a disgrace to that uniform.";
		"War just hasn't been the same since.";
		"These are not acceptable casualty figures.";
		// 0 = enemy name
	} //end type 


	//======================================================
	//======================================================

	type "kill_kamikaze" //initiated when the bot kills someone with kamikaze
	{
		"Let's just say I decided to be generous when handing out whoop-ass today.";
		"A true soldier uses any weapon at hand.";
		"~You have all failed a crucial test here.";
		"That ought to dampen your spirits a bit.";
		"That one had your name AND address on it, ",0,".";
		"Alright! Fragbait by the bushel load!";
		"I'll bet that took the crease out of your trousers, ",0,".";
		"I must be crazier than I thought.";
		"That's a small price to pay for a clear victory.";
		"Assessing peripheral damage now.";
		"Simply awe-inspiring.";
		// 0 = enemy name
	} //end type

	type "kill_rail"
	{
		"Alright!"; //VOICE RANDOM_1
		"Remind me to show you my sharpshooter ribbon.";
		"~This is a truly fine weapon.";
	} //end type

	type "kill_gauntlet"
	{
		"Lesson Number ~One: Never let the enemy get that close.";
		"That's just me bein' friendly, ",0,".";
		"I guess ",0," is too fragile to handle a real punch.";
		"Where I come from, we call that a 'glass jaw', ",0,".";
	} //end type

	type "kill_telefrag"
	{
		"Alright!";
		"Let that be a lesson to you soldier ... move your butt next time.";
		"I love the smell of toasted carcass in the morning!";
		"~This is a new weapon, I think. I'm going to have to try it again.";
	} //end type 

	type "kill_insult"
	{
		"~It's just like a bad dream ain't it.";
		"Get used to it, fragbait.";
		"Alright. That's more like it.";
		"Whole lot more where that came from.";
		"Heheh. Now you come in wallet size, ",0,".";
		"~This your first day with that weapon?";
		"Let that be a lesso, ",0," ... move your butt next time.";
		"~It'd be quicker, ",0,", if you just hold still.";
		"Move around more, fragbait! ~You're too easy a target.";
		"~You'd last about ~two seconds in a real shootin' war, ",0,".";
		0,", you are making this far too easy.";
		"I do not hold your inferiority against you, ",0,".";
		"Evolution predicted your extinction long ago, ",0,".";
		"That's what a steady diet of ",food," will do to you.";
		"~What is this, some kind of game to you ",0,"?";
		"Clean up your mess before you go ",0,".";
		"~Don't point that gun at me ",0,", unless you know how to use it.";
	} //end type

	type "kill_praise"
	{
		"~You need eyes in the back of your head, ",0,".";
		"Finally, a challenge worthy of my skills.";
		"~You may lose, ",0,", but you don't have to like it.";
		"~We'll make a soldier out of you yet, ",0,".";
		"My heart aches when such a worthy adversary must perish.";
		"Be proud, ",0,", you died bravely.";
		"Nice gibs there, ",0,".";
		"Well done, you remind me of my younger self, ",0,".";
	} //end type
	
	//================================================================
	//================================================================

	type "random_insult"
	{
		"Rrrrrr.";
		"~It'd be quicker if you just hold still, ",1,".";
		"~Does your mommy know you're here, ",0,"?";
		"~You're not naturally bad, ",0,". What you got takes practice.";
		"~You want a piece of me? ~You gotta take it the hard way!";
		"Come and get me fragbait.";
		"My old granny would be opening a can of whoopass here.";
		"Smells like a latrine. Or is that ",0,"'s cologne?";
		"What? ~You waitin' for your nails to dry, ",0,"?";
		"Run freaks! Run while you still can!";
		"Let me throw out a new word to ",0,". 'Tactics'.";
		"~Is ",0," trying to give us the impression that by firing continuously ~one may hit something?";
		"~Your fear of death will be your downfall, ",0,".";
		"Stupidity is not to be confused with bravery.";
		"Wearing 'Eau de Cat Box' again, ",0,"? At least I can smell you coming.";
		1,", please tell me why you seem to enjoy dying so often?";
		"Apparently ",0," has camped the ",4," before.";
	} //end type

	type "random_misc"
	{
		"Alright!";
		"Rrrrrr.";
		"Yeah!";
		"Bring it on! Bring it on!";
		"~You bring any cigars with you ",0,"? I'm almost out.";
		"~This is all some kind of game to you, isn't it?";
		"~It's time to wrap this up and move on.";
		"Move your sorry butt around more, ",1,"! ~You're too easy a target.";
		1,", you'd last about ~two seconds in a real shootin' war.";
		"~This is all so terribly real.";
		"My spirit is indomitable. I will persevere and survive this.";
		"What in the universe did you do to earn this billet, ",0,"?";
		0,", you grab the ",5," and I'll grab the ",5,". ~We'll own this place!";
	} //end type
} //end chat Sarge
