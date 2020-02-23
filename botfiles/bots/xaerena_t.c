chat "Xaerena"
{
	#include "teamplay.h"
	//
	type "game_enter"
	{
		"Konnichiwa, campers.";
		"Konnichiwa.";
		"Ooooo. Gimme some room, I feel a victory comin' on.";
		"My wetware is hot tonight!";
		"Been here, done this and am ready to do it again.";
		"Are you prepared for pain?";
		"Ahh, there is so much Chi here... I will feed well!";
	} //end type

	type "game_exit"
	{
		"Sayonara, babies.";
		"Sayonara.";
		"My hunger grows, I am off to find better Chi.";
		"I'm off to find Tenchi ... I bet he's with that Ayeka woman!";
		"Go! Ryo-ohki!";
		"Where shall we go now, Ryo-ohki?";
	} //end type

	type "level_start"
	{
		0," it is time for some payback!";
		1," you are going to have some pain in ",4,".";
		"Ooh ",4,"! I like this place!";
		"~This contest will finally show who the true winner is, ",1,".";
	} //end type

	type "level_end"
	{
		"So soon? I'm not breathing hard yet.";
		"I see I underestimated you fellows.";
		"HA! ",3," is at the bottom, as usual.";
		2,", I let you win.";
		4," has changed a lot, it is much faster now.";
		"~Is it over yet?";
		"I can remember when ",4," didn't suck.";
		3," sucks!";
		2,", I let you win.";
		"Not bad ... I like ",4," fast, clean, deadly, and filled with Chi.";
		"I'll just go on ahead Tenchi.";
		"There's only winning and losing. Nothing in between.";
	} //end type

	type "level_end_victory"
	{
		"Winning comes naturally to me.";
		"I think I'll have my gun bronzed.";
		"~This is so sweet.";
		"Thank you for your Chi.";
		3," you need to practice more ... and forget about winning against me.";
		"I really like ",4," since they fixed it, it is so easy to win!";
		"Next time, don't bother.";
	} //end type

	type "level_end_lose"
	{
		"I have brought, y'know, dishonor to those who trusted me.";
		"Where I come from, they don't teach you how to lose.";
		"I just hate losing.";
		2," cheated.";
		"I felt bad for you so I let you win.";
		2," must be an Arena Lord.";
		1," kept getting in my way.";
		4," has never been good to me.";
	} //end type

	//======================================================
	//======================================================

	type "hit_nodeath" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		"Nice shot, ",0,". But definitely not nice enough.";
		"Oooooo! ~You like me. ~You really like me!";
		1," is not a good weapon, a ",weapon," should be more your speed.";
		"~You need to aim better.";
		"Try harder next time.";
		//0 = shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		"~One shot, ~one ... Ok, I guess I'll have to make it ~two shots.";
		"~You've been whoring the armor again, ",0,", haven't you?";
		"~Don't run off, ",0,", I've got more just like that for you.";
		0," must be cheating.";
		"Dammit! Why won't you die?";
		"Well that sucked ...";
		"Note to self ... turn off safety next ~time!";
		//0 = opponent
	} //end type

	type "enemy_suicide"
	{
		"Laughing? Me? Of course I'm laughing at you, ",0,".";
		"Wow! ~Do you do that often, ",0,"?";
		"Death by papercut. What a terrible way to go.";
		"Wooohooo. ~Can we catch that on instant replay, ",0,"?";
		"Stop smoking ",substance," and maybe you won't kill yourself so often.";

		//0 = enemy
	} //end type

	//================================================================
	//================================================================

	type "death_telefrag"
	{
		"No way ...";
		"I hate praetorian impressions.";
		"My ",counselor," warned about days like this.";
		"That's ~one way to become one with the universe.";
		"Billions and billions of little tiny atoms everywhere.";
	} //end type
	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{
		"~It didn't look that high from up there ...";
		"I need some saki ...";
		"*sigh*";
		// 0 = random opponent
	} //end type
	type "death_lava"
	{
		"That lava stuff really messes with my wetware.";
		"At least it is warm here.";
		"~It burns!";
		"My feet were cold.";
	} //end type

	type "death_slime"
	{
		"That slime reminds me of a guy I once dated.";
		"~It's 'Slime Time' again.";
		"Green has never been my color ...";
	} //end type

	type "death_drown"
	{
		"All that water and no hunky lifeguards. No beach either.";
		"~It isn't that difficult to just breath water ...";
		"Now all my ware is wet.";
		"Like, terminal wet t-shirt contest!";
		"I can't swim!";
	} //end type

	type "death_gauntlet"
	{
		"~Did I give you permission to touch me?";
		"Y'know, that really pisses me off.";
		"What part of '~Don't touch me there' do you not understand?";
	} //end type

	type "death_rail"
	{
		"That won't happen again.";
		"Camping Scum!";
	} //end type

	type "death_suicide"
	{
		"So. I suppose you never off yourself? Right?";
		"'Ow'.";
		"There is no fun in wasting ~one's self like that.";
		"So much for life and all of its cruel irony.";
		"~It is just easier for ",0," this way.";
	} //end type

	type "death_bfg"
	{
		"I take special delight in fragging BFG campers.";
		"So that was the great Jurai power ...";
		"That was a mistake. ~You've made me very angry now.";
		"Gross! Fragged by my own fave tool.";
		"~Can't find a skill weapon? Or is it just that you suck?";
		"~Your mommy say you could use that big gun, ",0,"?";
		"Awful big gun for a puny wimp.";
	} //end type

	type "death_insult"
	{
		"Now the match gets serious.";
		"Enjoy that ~one. ~Your luck is about to run out, ",0,".";
		"~This is far more painful than it looks.";
		"Am I supposed to be impressed by that?";
	} //end type

	type "death_praise"
	{
		"~This match might be interesting after all.";
		"Not bad, ",0,", have you been practicing?";
		"~You win the prize for that ~one.";
		"Impressive!";
		"Excellent!!";
		"Looks like all the legends about ",0," are true.";
	} //end type

	type "death_kamikaze" //initiated when the bot is killed by kamikaze blast
	{
		"Kid, you don't know how much that just cost you.";
		"All weapons are good weapons. I just rather they were used elsewhere.";
		"No! No! No!";
		"Real funny, ",0,". ~You're like a one-man BFG.";
		"Was that the best you could do?";
		"Up until now, I thought you were one of the good guys, ",0,".";
		"~You got any more internal organs you'd like to share with us, ",0,"?";
		"That's a grand slam for the visitors.";
		"Well, my headache's gone now.";
		"~You sure know how to impress a girl.";
		"I bet you don't get invited to many parties, ",0,".";
		"All that training just to qualify as hazardous waste.";
		"So those weren't flying toasters after all.";
		"~You work for Shub, right, ",0,"?";
		"Either quit eating beans, ",0,", or find yourself some other opponents.";
		"What happened, ",fighter,"? ~Your backpack explode?";
		// 0 = enemy name
	} //end type 


	//======================================================
	//======================================================

	type "kill_kamikaze" //initiated when the bot kills someone with kamikaze
	{
		"~That is just too kewl!";
		"Nice moves babe! The gib bounce at the end is the best.";
		"One Neural Overload with sashimi to go!";
		"I'll bet that purged your hard drive!";

		// 0 = enemy name
	} //end type

	type "kill_rail" 
	{
		"I love modern technology.";
		"So I'm a camper. Sue me.";
		"Ka-POW!! **[giggle]**";
		"Not a BFG, but I do what I can.";
	} //end type

	type "kill_gauntlet"
	{
		"Nothing like a friendly touch to make a girl's day.";
		"I've got more where that came from.";
	} //end type

	type "kill_telefrag"
	{
		"That was so wrong in all the right ways.";
		"Aww man ... now I have to take a bath to get the brains out.";
		"~Does anyone have a quick way of getting brains out of your hair?";
	} //end type 

	type "kill_insult"
	{
		"Wrongful!";
		"Why do you even bother?";
		"Hurry up and respawn. I need another cheap frag.";
		"I'm like a hot knife and you're butter.";
		"That was quick.";
		"And the Lady Xaerena scores again.";
		"~You might want to put that gun down before you hurt yourself.";
	} //end type

	type "kill_praise"
	{
		"~You dance really fine, babe. Just not fast enough.";
		"Nice moves, but not quick enough, ",0,".";
		"~I'm impressed.";
		"~You are getting better, ",0,", keep it up and you might actually win something.";
	} //end type

	//================================================================
	//================================================================

	type "random_insult"
	{
		"~You! ",0,"! ~Out of the gene pool!";
		"~Did you buy those implants in a pawn shop, ",0,"?";
		"~Can't you ",fighter,"s die with a little more style than that?";
		"~You should give up your ",substance," addiction by next ",month,".";
		"I saw those pictures of ",0," and ",femalebot," posted on the web last night... Like, ewww!";
		"~Is that the best you've got to offer?";
	} //end type

	type "random_misc"
	{
		one_liners;
		neuterbot," is cute.";
		0," die already so I can feast on your Chi!";
		"I so love a challenge. But I guess I have to take what I'm given.";
		"Zaphod? ~Isn't he just 'some guy'?";
		"~Has anyone tried out the new ",5,"? ~Is it worth the upgrade?";
	} //end type
} //end chat Xaerena
