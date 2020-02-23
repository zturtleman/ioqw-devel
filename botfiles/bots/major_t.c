chat "Major"
{
	#include "teamplay.h"
	//
	type "game_enter"
	{
		"I hope you're ready to lose today ",1,".";
		"~You do know that I'm a VERY poor loser.";
		"~Did you miss me, ",1,"?";
		"If you quit now, ",1,", it will save a lot of bother.";
		"I think I'm gonna like this place.";
		1,", I got a warrant for your destruction.";
		1,"'s here again? What am I ... loser bait?";
		"I love ",4,"!";
	} //end type

	type "game_exit"
	{
		"Just look at the time. I'm late already.";
		"Next time, ~don't bring the geeks ... they make the place look all trashy.";
		"Better worlds to conquer than this ",fighter," heaven.";
		"I got contracts to catch up on.";
		"Hasta la vista! Babies!";
	} //end type

	type "level_start"
	{
		"~Do I make you ",fighter,"s nervous? If you are smart, you'd be nervous.";
		"~It was a dark and rainy night in ",4,". Suddenly, A shot rang out!";
		"I know this place. ~You are so seriously dead now, ",fighter,".";
		"I'm good to go. Let's do it.";
	} //end type

	type "level_end"
	{
		"Hmphh, ",3,", looks like you've got a lot of work to do.";
		"~You're so good, ",2,", we're promoting you to Xaero's punching bag.";
		"As far as I'm concerned, you're all a bunch of numbers and I'm about to win the lottery.";
		"So, I went easy on you this time. No biggie.";
		"~It won't go the same way next time. Count on that.";
	} //end type

	type "level_end_victory"
	{
		"Uh-huh. A girl could really get used to this.";
		"Superior training, superior tactics, superior everything.";
		"I'm surprised ... that it took me so long.";
		"And just how many times did you fall for the 'your-shoes-are-untied' bit, eh ",3,"?";
		"~Do I have to lead you around by the nose, ",3,"? If you had ~one left, I mean.";
		"~Do it again. But this time, with feeling.";
		"Someone had to win. Today, I'm most definitely someone.";
		"Awwwww. ~Did mean ol' me pop a hole in someone's pretty little ego?";
	} //end type

	type "level_end_lose"
	{
		"I can't believe I let you do that.";
		"Look at you ",fighter,"s! Now I know what's below the bottom of the barrel!";
		"~Do you guys play harder when you think a girl might beat you.";
		"If this were just a game, I wouldn't be so pissed right now.";
	} //end type

	//================================================================
	//================================================================

	type "hit_nodeath" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		"~You should have taken me out when you had the chance, ",0,".";
		"~You know the rules, ",0,". Anything less than a frag doesn't count.";
		"Only a flesh wound. I'll live. But YOU won't, ",0,".";
		"~Don't dirty the leathers, ",0,"!";
		"~Don't blame the gun, ",0,". That was YOUR fault.";
		"I'm tougher than I look. Really.";
		"Ow! ~Don't make me come over there, ",0,".";
		"~You are SO gonna wish you could take that back.";
		"I was gonna go easy on you ... but no more, ~jerk.";
		"Score ~one for ",0,". ~We'll put that on ",0,"'s tombstone.";
		//0 = shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		"Shoot! Something jiggled my arm!";
		"I hate wasting ammo like that.";
		"~You could have done me the courtesy of dying, ",0,".";
		"Cha-ching!";
		"Keep running away, ",0,". I can hit a moving target, no problem.";
		"I'm a pinball wizard, ",fighter,".";
		//0 = opponent
	} //end type

	type "enemy_suicide"
	{
		"Ugh. That's no way for a warrior to die, ",0,".";
		"Oh, I get it now, ",0,". ~You're trying to make it more challenging for yourself.";
		0,", my mother always said, 'Stupid is as stupid does'. Whatever that means.";
		"Victors don't let anyone kill them ... including themselves.";
		//0 = enemy
	} //end type
	//================================================================
	//================================================================

	type "death_telefrag"
	{
		"~Thanks for ruining my day, ",0,".";
		"~You could've knocked first, you know.";
	} //end type

	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{
		"I've just enrolled in the WILE E. School of Stupidity.";
		"~It's not how you fall, ",0,". ~It's how you land that coun...";
		"No, ~jerk, I can't see my house from up there.";
		// 0 = enemy name
	} //end type

	type "death_lava"
	{
		"~What is it with all this lava?";
		"Figures. I missed the lava dancing class.";
		"Always a back flip. Never a swan dive. So predictable.";
		"To me, Hell is like tropic resort. A vacation.";
		"No big deal. I had that stuff in my back yard.";
		"Never a battle suit when you need ~one.";
	} //end type

	type "death_slime"
	{
		"So, which ~one of you slugs left this slime trail?";
		"Well, ",0,". I can see you left your slime trail here.";
		"Gick. And I just polished those shoes.";
		"At least the goo that killed me was brighter than ",0,".";
		"Revenge of the lime gelatin.";
		"The slime, he reminds me of my ex-husband, only cuter.";
	} //end type

	type "death_drown"
	{
		"Hey, at least I didn't get sand in my swimsuit. I hate that.";
		"Forgot to pack my floaties.";
		"I miss my rebreather.";
		"Leather's heavy when it's wet. Dang.";
		"Well, this pool's going to need a good brushing down.";
		"~You could've marked the deep end, ",0,".";
		"C'mon in! The water's fine!";
		"Next time, women and children first.";
		"Everybody, out of the pool.";
		"I'm sleeping with the fishes now.";
	} //end type

	type "death_suicide"
	{
		"Well that was friggin' spectacular. Guess I win the prize.";
		"Oh ha ha. What am I ... comic relief?";
		"~You know you're getting too old for this when ...";
		"Someone had to do it.";
		"Stop laughing ",0,"!";
	} //end type

	type "death_gauntlet"
	{
		0,", I suppose your mommy never said it was wrong to hit girls?";
		"I just love it when you do that ",0,". I dare you to try it again.";
		"~You don't EVER touch a lady like that. ~You hear me?";
		"Now aren't you the tough ~one.";
		"Scratch a little lower, ",0,".";
		"I ~don't do 'patty-cake' on a first date, ~jerk.";
	} //end type

	type "death_rail"
	{
		"Well, so much for having a quiet day at home.";
		"Camping AGAIN ",0,"? ~Didn't your therapist tell you to stop?";
		"That ~one hits you before you hear it. Ouch!";
		"Fastball, low and across the plate! ~She's outta there!";
		"Way to go, ",fighter,". Let's get us a 'pic-a-nic' basket and go camping.";
		"How did you fire that thing? Chickens don't got fingers.";
	} //end type

	type "death_bfg"
	{
		"~You're developing a really sick relationship with that thing, ",0,".";
		"C'mon ",0,". ~Don't you feel cheap and trashy when you do that?";
		"Ow. Really.";
		"~It's not how big your gun is ~jerk ... well, maybe for you it is.";
		"No skill needed there ",0,".";
		"I think this is ",0,"'s first kill.";
	} //end type

	type "death_insult"
	{
		"Let's see, ",0,", was that skill or just luck? I'm guessing ... luck.";
		"~Did that make you feel like a big Warrior ",0,".";
		"Mmmmmmm. Was it good for you too, ",0,"?";
		"That's right ",0,", pick on the girl.";
		"~You got me because I aimed for your brain, ",0,". Impossible shot.";
		0,", you are gonna be so dead now.";
		"I ~don't do the dance with losers like you, ",0,".";
		"Drat! There goes ~three hours at the salon down the drain.";
		"I'm sorry. I forgot to mention that I hold grudges.";
		"That's it, ",0,". ~We are just so over now.";
		"Are you wearing your rabbit's foot AND your lucky underwear, ",0,"?";
		"Amazing what ",0," will do to impress a girl.";
		"Why must we always let the geek squad play?";
		"Let me guess, first time for you ",0,"?";
	} //end type

	type "death_praise"
	{
		"Nice. Really nice.";
		"I've seen better ",0,", but not many.";
		"Wooo! Looks like some new competition has moved into town.";
		"Oh my! Xaero look OUT!";
		androbot," better keep an eye on his ~rear view mirror now.";
		femalebot," can't coast along on her good looks anymore.";
		"I'd say that this train is bound for glory.";
		"Sorry. I'm not much of a challenge.";
		"~You're all right, ",0,". ~You just might be around for a while.";
		"Nice job, ",0,". Maybe next time, they let you carry a real gun.";
		"~You do that pretty good, ",0,".";
	} //end type

	type "death_kamikaze" //initiated when the bot is killed by kamikaze blast
	{
		"Soldier, the penalties for war crimes like that are severely permanent.";
		"Who gave this ",fighter," access to classified ordinance?";
		"BFG just doesn't do it for you anymore, eh, ",0,"?";
		"And as usual, ",0," lets loose way too soon.";
		"Drat! I just had that uniform cleaned.";
		"~Can't we all just get along?";
		"~You'll hear from the neighborhood association about this one.";
		"That's a grand slam for the visitors.";
		"Well, my headache's gone now.";
		"~You sure know how to impress a girl.";
		"I bet you don't get invited to many parties, ",0,".";
		"So those weren't flying toasters after all.";
		"So, ",0,". I see you are now buying the large economy cans of Whoop-Ass.";
		// 0 = enemy name
	} //end type 


	//======================================================
	//======================================================

	type "kill_kamikaze" //initiated when the bot kills someone with rail gun
	{
		"I'll bet that took the crease out of your trousers, ",0,".";
		"~Mother of Mercy!";
		"A soldier does what she must.";
		"Messy and loud, but otherwise effective.";
		"Love me, love my bomb.";
		"Awesome. I never get to play with the big toys.";
		// 0 = enemy name
	} //end type

	type "kill_rail" 
	{
		"Pow!! Right in the kisser.";
		"I have a deep and abiding love for power tools.";
		"Explosive decompression works for me.";
		"That ~one's out of the park.";
		"Quick. Clean. Efficient.";
		"Point and squirt. There's something very primal about that.";
		"Hurry back, ",0,". I can use the points.";
		"Train leaving on track ~four! All aboard!";
		"Splat. Nothing to take back for a bounty.";
		"Orale!";
		"Rule #1, 'Avoid big bullets'.";
		"Rule #2, '~It is better to give than receive'.";
		"Rule #3, '~Don't let your lame, ~pansy, camping butt get anywhere in range of my return fire'.";
	} //end type

	type "kill_gauntlet"
	{
		"That left me breathless.";
		"Oops. ~Don't know my own strength.";
		"Well, I am stronger than I look.";
		"If the glove don't fit ... Looks like it fit. Heh.";
		"Whattayaknow, the batteries in that were still good.";
		"Whoooooaaa. That felt great ",0,". Mind if I do it again?";
		"Drat, I broke a nail. Heh. Looks like a fair trade.";
		"Give me that! ~You'll hurt someone with it.";
		"Coochie-coochie-coo!";
	} //end type

	type "kill_telefrag"
	{
		"A frag's a frag, ",0,", but I feel so ... cheap.";
		"Heh. I think you look better this way ",0,".";
		"~You know when those little hairs on the back of your neck stand up ...";
		"~Did you eat something that didn't agree with you, ",0,"?";
	} //end type

	type "kill_suicide" //initiated when the players kills self
	{
		"~Don't give up so easily. I'm tough, but not THAT tough.";
		"And you had so much to live for.";
		"I'm not cleaning that mess up.";
		"Well, I can see you don't need me, ",0,".";
		"Typical low-life. Always looking for the easy way out.";
		"~You really shouldn't look down the barrel to see if it's loaded.";
		// 0 = enemy name
	} //end type

	type "kill_insult"
	{
		"Oh. ~Did I ruin your day, ",0,"?";
		"Wheee. I love fragging easy targets.";
		"I came here expecting a challenge.";
		"C'mon, ",0,". Try harder. I'm not breaking a sweat here yet.";
		"~Don't worry, that toe tag will let your mommy know who you are.";
		"Who's your Mommy?";
		"~It's never too late for a change of career, ",0,".";
		"Trust me when I say this: '",0,", you suck'.";
		"C'mon ",0,"! Try to give me some challenge here.";
		"Are you normally this bad ",0,"?";
		"~Isn't it your bedtime ",0,"?";
		"Hey, ",0," does your mommy know you're here?";
	} //end type

	type "kill_praise"
	{
		"~You made that really hard, ",0,". I like a challenge.";
		"~You blow up real pretty.";
		"That was so sweet of you, ",0,", giving me a clear shot like that.";
		"~Your gore color coordinates so nicely with this arena, ",0,".";
		"Well ",0,", you move better than a mannequin. There's that.";
		"~You did your best, ",0,", ... that's what should really bother you.";
		"~You die pretty good for a ",fighter,".";
	} //end type

	//================================================================
	//================================================================

	type "random_insult"
	{
		"There must be worse nicknames than 'Ol' Harmless'.";
		one_liners;
		"Big clue here. ~Don't quit your day job, ",0,".";
		"~You know, ",1,", I've decided I like you better when you're dead.";
		"By the way, ",1,", I've decided I like you better when you're dead.";
		"Oh, was that your ego I deflated back there, ",1,"?";
		"~You know, ",0,", bathing regularly isn't considered a crime.";
		1,", anyone ever mistake you for someone dangerous?";
		"So, ",0," is supposed to be the best. Yeah, sure.";
		"~Is that your face, ",0,", or are you mooning me?";
		"Hey, ",0,", I'm salt and you're a slug. Bad combination.";
		"Let me guess, ",0,". ~You ate a lot of paint chips when you were a kid?";
		"So you got brain damage, ",0,"? Looks like an improvement to me.";
		"I nominate ",0," for the 'Happy Camper Hall of Fame' award.";
		"I've got no use for snipers, assassins and campers.";
		female_one_liners;
		"~You're allowed ~one lucky shot a day, ",0,".";
		"Grrrrrr!";
	} //end type

	type "random_misc"
	{
		"What am I doing here? I promised myself no more hanging with losers.";
		"Huh. I was going to say something, but now my mind's a blank.";
		"When you see Lucy, ask her why she never calls.";
		one_liners;
		"What? Are you trying to come on to me, ",1,"?";
		"What was that you said about my mother, ",0,".";
		"~Can we speed this up? I'm falling asleep out here.";
		"The bounty on ",0," is worth the extra effort.";
		"See the universe. Frag interesting people.";
		"I'm hauling all you, ",fighter,"s in.";
		"Hey, ",0,". ~Your Mom did a great job dressing you this morning.";
		"Xaero reminds me of the Vampire who made me this way.";
		"Medic! Quick! I'm laughing so hard I'm choking.";
	} //end type
} //end chat Major
