chat "Pi"
{
	//the teamplay.h file is included for all kinds of teamplay chats
	#include "teamplay.h"

	//======================================================
	//======================================================

	type "game_enter" //initiated when the bot enters the game
	{
		"Pi has been summoned to be your death today, ",1,".";
		"Beware ",1,". Pi's skill is endless does not repeat itself.";
		"Let it begin.";
		"Somebody farted. ~Got to be ~one of those organic ",fighter,"s.";
		"I'm gonna own ",4," before we're done.";
		"Jacked in, powered up, and ready to scatter.";
		// 0 = bot name
	} //end type

	type "game_exit" //initiated when the bot exits the game
	{
		"~This is not ending, only change of direction.";
		"The circle will come about and with it, Pi will return.";
		"~You ",fighter,"s wouldn't know real beauty if it bit your sorry butt. I'm leaving.";
		"Yeah, that was fun. Not.";
		"I do not terminate civilians.";
		"I'm gone.";
		// 0 = bot name
	} //end type

	type "level_start" //initiated when a new level starts
	{
		"~We begin ... NOW!";
		"~We come about full circle again.";
		"All variables are equal once again. Victory favors Pi.";
		"Hey, ",1,"! Old, slow and ugly is no way to go through life.";
		"Okay, fleshy slugs. Let's get it on.";
		LEVEL_START0;
		"Wake up, fleshbags. Pi crashed your little party.";
		"What, you expected cherubs and sugary stuff?";
		// 0 = bot name
	} //end type

	type "level_end" //initiated when a level ends and the bot is not first and not last in the rankings
	{
		"Today Pi was unequal to challenge. Tomorrow will care for itself.";
		"Reboot! Reboot!";
		"These cybronic enhancements are slowing me down.";
		"I need a better ping.";
		"Anyone up for a ",food," break?";
		"~This is totally bogus. ~You were totally bogarting all the weapons, ",2,"!";
		"~Your techniques warrant future study, ",2,".";
		"Pi lives for challenge. Another match?";
		"Again. My mistakes will not be repeated.";
		"I would have won, too, if I had taken nothin' but cheap and easy shots.";
		"Yeah, well, we're all having fun now.";
		"Stuck in the middle again.";
		"There just ain't no justice.";
		"I thought I was a bit rusty.";
		// 0 = bot name
	} //end type

	type "level_end_victory" //initiated when a level ends and the bot is first in the rankings
	{
		"The circle turns, crushing all before it.";
		"Give up, ",3,". I was designed for combat.";
		"Combat is my religion. Victory, my Paradise.";
		"~You accept defeat far too well, ",3,".";
		"Whoa! I just completely messed you up, ",1,".";
		"~You lamers running 96 baud, or what?";
		"Okay, kiddies. Pick up all your body parts and get out.";
		"Bet you ",fighter,"s didn't expect me to be prom queen.";
		"Hey, ",3,". ~You make an easy target with that big 'L' painted on your forehead.";
		"I told you ",fighter,"s I would own this place.";
		// 0 = bot name
	} //end type

	type "level_end_lose" //initiated when a level ends and the bot is last in the rankings
	{
		"I will think on my mistakes. ~This will not happen again.";
		"Defeat tastes of ashes and bile. ~It is not good taste.";
		"System overload. I'm multitasking way too much here.";
		"My hatred, I will store away until next we meet.";
		"Hail, ",2,". ~You have earned my respect.";
		"I'm ready for that oil change now.";
		"Fragged by ",fighter,"s!";
		"Ooh, well isn't ",2," a big ~hero?";
		"Always a bridesmaid, never a bride.";
		"Wait. All of you ",fighter,"s here, and I'm the loser? Who else finds this funny?";
		// 0 = bot name
		// 2 = opponent in first place
	} //end type

	//======================================================
	//======================================================

	type "hit_nodeath" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		"~Your mistake, ",0,". ~You let me live.";
		"For shame, ",0,". ~You led me to believe you had bigger gun.";
		"~You waste shot, ",0,".";
		"Ooh, I like the rough stuff.";
		"~You little puke, you're gonna need a bigger gun.";
		// 0 = shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		"Hehe. ",0,", I can't believe I hit you with my eyes closed.";
		"That was a tracer round.";
		"Bah. Pi thinks you have no vital organs.";
		"Obviously my human feelings are spoiling my aim.";
		"No matter. I am just practicing that time.";
		"Oh wait ... I'm sorry ... did that hurt, ",0,"?";
		"Guess I'll have to set this gun to SPLATTER.";
		//0 = opponent
	} //end type

	//======================================================
	//======================================================

	type "death_telefrag" //initiated when the bot is killed by a telefrag
	{
		"~Could I get a magnet here?";
		"I am breathless. Was it good for you also, ",0,"?";
		"Once this happens and I respawn with a big eyeball for head.";
		"~This is very hard on cybronic parts.";
		"Wow. I feel like I'm in the middle of a download.";
		"Whoaaaa, like, overclockin' brain drain!";
		"That's murder on the cybronics.";
		"Yow! That some kind of EMP you hit me with, ",0,"?";
		DEATH_TELEFRAGGED1;
		// 0 = enemy name
	} //end type

	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{
		"Pi makes crater like circle, yes?";
		"Sometimes, ground is harder than Pi.";
		"Hey, so what? ~It's the only rush I'll get in here.";
		"So I'll get a job as an ashtray.";
		"So this is why they call it TERMINAL velocity.";
		"Gravity can be very unkind to a girl.";
		"Ctrl-Alt-Del! Ctrl-Alt-Del!";
		"That's what they forgot to build in ... booster rockets.";
		"~Could've made it if I landed on the cyborg part. Figures.";
		// 0 = enemy name
	} //end type

	type "death_lava" //initiated when the bot dies in lava
	{
		"~Must remember to retune my pain sensors.";
		"Cooling system shutdown."
		"Now I know the melting point of ... me. ~Ten degrees less than this.";
		"Now you think I'm hot, right?";
		"Terminal hotfoot!";
		"Bah. Flesh is weak. If I am all metal this is not happening.";
		"System meltdown imminent ... I wonder if the part that eats will survive.";
		"I'm gonna need another cooling fan implant.";
		// 0 = enemy name
	} //end type

	type "death_slime" //initiated when the bot dies in slime
	{
		"I do not think that green is my color.";
		"~It feels like my batteries are leaking again.";
		"Ugh. ~It is vat of molten ",0," goo.";
		"~It's like being hugged by a human.";
		"Gaaahhhh! ~It's not supposed to hurt like this!";
		// 0 = enemy name
	} //end type

	type "death_drown" //initiated when the bot drowns
	{
		"I curse this weak flesh. Air should not be necessary!";
		"Cybronics do not float well.";
		"Air is for weaklings.";
		"Who filled the pool?";
		"Wait a minute ... robots can't drown! Blub, blub, blub.";
		"If the lack of air don't get you, the rust will.";
		"Anyone got some rust remover?";
		"Great, all this technology and they forget the waterproofing.";
		"Break out the rust remover. I'm gonna need it.";
		// 0 = enemy name
	} //end type

	type "death_suicide" //initiated when bot blows self up with a weapon or craters
	{
		"A slight miscalculation.";
		"I think I experience error in my targeting computer.";
		"Pi killed self to deprive you of the pleasure, ",0,".";
		"Permanently kicked.";
		"Aw, the heck with it.";
		"I just wanted to see how much it hurts before I use it on ",0,".";
		"Aarrrgh. Friggin' computer glitches.";
		"~They must have programmed me in DOS.";
		"Triple redundancy in everything except common sense.";
		"Stink!! My onboard computer just lagged me out.";
		// 0 = enemy name
	} //end type

	type "death_gauntlet" //initiated when the bot is killed by a gauntlet attack
	{
		"Strange. I feel no humiliation at this. I feel I should kick ",0,"'s butt.";
		"Luck is strong with you, ",0,". But skill is not.";
		"Get your sweaty, fleshy hands off me, ",0,".";
		// 0 = enemy name
	} //end type

	type "death_rail" //initiated when the bot is killed by a rail gun shot
	{
		"So. ~You fear to face me in level combat, ",0,".";
		"Hah. ~Xaero, you are not.";
		"How do you win when there is no rail gun?";
		"Pisser shot, ",0,".";
		"Now there's ~one happy camper.";
		"Ever get the feeling you're being watched?";
		"Okay, ",0,". ~You like what you see?";
		// 0 = enemy name
	} //end type

	type "death_bfg" //initiated when the bot died by a BFG
	{
		"~It is good weapon, this BFG, yes?";
		"All fair. No complaining when I do same to you.";
		"No skill there.";
		"Metal, dude, lots of metal.";
		"~Don't stand in a puddle when re-charging. Gotta write that down ...";
		"Hey, ",0,". Quit laughing. Half those sparks are coming from me!";
		"Green. I hate green.";
		// 0 = enemy name
	} //end type

	type "death_kamikaze" //initiated when the bot died by a kamikazee blast
	{
		"~Your radius is quite impressive, ",0,".";
		"How clumsy of you.";
		"Whooaaaa, trippin'!";
		"That one reformatted my hard drive!";
		"And you are happier now, yes?";
		"So you can light your farts, ",0,", big deal.";
		"Oh yeah, that took REAL skill.";
		"~You ought to take something for that gas problem, ",0,".";
		// 0 = enemy name
	} //end type


	type "death_insult" //insult initiated when the bot died
	{
		"I'll get re-wired, ",0,", ... you'll just decompose.";
		"At least I don't stink when I rot.";
		"I'd tell you to kiss my butt, but I'm afraid you'd take me up on it.";
		"Hmmpf. I am not impressed, ",0,".";
		"Chaos theory in action, ",0,".";
		"Nice shot, PC Junior. What's that, ",0,", like a Y1K thing?";
		"Shot in the back again. I should not expect more of you, ",0,".";
		"Was cheap shot.";
		"~You were best fighter at what, a school for little girls?";
		"Bah. I have seen better moves in chess game.";
		// 0 = enemy name
	} //end type

	type "death_praise" //praise initiated when the bot died
	{
		"~It's good that you did this before you got old and weak, ",0,".";
		"I'm impressed, ",0,". ~You're the best inferior I've ever met.";
		"That was shot was worthy of my own skill, ",0,".";
		"Bravo. Pi salutes you, ",0,"!";
		"I am impressed. ~You are a warrior of many surprises, ",0,".";
		"That was a move worthy of a champion, ",0,". Are you sure it was yours?";
		"Who's the ~Man? Who's the ~Man?";
		"I didn't know you could still move like that, ",0,"!";
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "kill_rail" //initiated when the bot kills someone with rail gun
	{
		"I am sniper, not camper.";
		"See, ",0,", that's what V-Chips are made for.";
		"That puddle was your brain? Awesome!";
		"Totally awesome.";
		"~This way I ~don't have to risk getting your guts all over me, ",0,".";
		// 0 = enemy name
	} //end type

	type "kill_gauntlet" //initiated when the bot kills someone with gauntlet
	{
		"'Humiliation'. I think I am liking this word.";
		"~You should not come within the radius of my touch, ",0,".";
		"Some things require a woman's touch. A very, very hard touch.";
		"How very touching.";
		"I'm too quick and you're too lame, ",0,".";
		"~It'd be your pleasure to wake up next to me, but tragically, you won't be waking up.";
		KILL_GAUNTLET0;
		// 0 = enemy name
	} //end type

	type "kill_telefrag" //initiated when the bot telefragged someone
	{
		"Pi feels so very close to you now, ",0,".";
		"Mmmm. That was linked into my pleasure circuits.";
		"Pi thinks you need faster processor, ",0,".";
		"Hah! ~You are too slow again, ",fighter,".";
		"Go defrag your drive, ",0,". ~You're scattered.";
		"~You think you own the sidewalk, ",0,"?";
		// 0 = enemy name
	} //end type

	type "kill_suicide" //initiated when the player kills self
	{
		"I am confused, ",0,". ~Is not purpose of match to kill other person?";
		"~You are doing comedy routine, ",0,"? That was big joke, yes?";
		"If you are trying to help Pi win, ",0,", I am most grateful.";
		"Face it, ",0,", you suck.";
		// 0 = enemy name
	} //end type

	type "kill_insult" //insult initiated when the bot killed someone
	{
		"Pi is like kamikazee. Inside my kill radius, no one lives.";
		"~We must do this again sometime ... after you have improved skill.";
		"I think you like this dying. ~You do so much of it.";
		"~You must practice more, ",0,". That was a very unprofessional death.";
		"~Don't bleed so much, ",0,". Makes the road slippery.";
		"Bah! I am needing more challenge to be arena lord.";
		"Someone call the big orange truck to clean up this road kill.";
		"~You're a pus bag, ",0,", and you can take that literally.";
		// 0 = enemy name
	} //end type

	type "kill_kamikaze" //insult initiated when the bot killed another player with kamikaze
	{
		"Fear my diameter!";
		"Pi does not recognize nuclear arms treaty. Treaties are for weak.";
		"If you cannot stand heat, stay out of kitchen.";
		"No skill, yes. But good for easy kill.";
		"Bah. ~That is not as satisfying as I had hoped.";
		"I thought those servos was overheatin'.";
		"Splat, splat, splat!";
		// 0 = enemy name
	} //end type

	type "kill_praise" //praise initiated when the bot killed someone
	{
		"I'll high-five you, ",0," ... if you re-attach your hand.";
		"~We are good match, ",0,". I am programmed to kill ... and you die so very well.";
		"Pi wishes for more opponents like you, ",0,". ~You are easy to kill.";
		"~You've got to have some non-organic parts in there somewhere, ",0,".";
		"I wanted you to feel like you could do something well, so I killed you again.";
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "random_insult" //insult initiated randomly (just when the bot feels like it)
	{
		"~Do you know when my REAL opponent is supposed to arrive?";
		"Who wrote your program, ",0,"? Benjamin Franklin?";
		"Try a game-pad, fat fingers. ~You're, like, hitting all the wrong keys.";
		"Bend over and hold your arm out straight so I can catch a slide.";
		"Charging my batteries is tougher than fragging you, ",1,".";
		"I do not think you have the radius to be here, ",0,".";
		"~You cut the cheese, ",0,"? Or is this your normal perfume?";
		"Hey, ",0,", repeat after me ... I WILL brush my teeth.";
		"All the leaves have shaken off that bush. I see you now.";
		"Surrender now. Cyborg steel will dominate.";
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type

	type "random_misc" //miscellanous chats initiated randomly
	{
		"I have argument to settle with a 'hardcore' warrior. ~Do you know 'Anonymous'?";
		"~We are all points on the circumference of universe.";
		"~This match is lacking challenge.";
		"Aww! Freakin' gum. Who dropped it on the stairs?";
		"The hot wings made my fingers all sticky.";
		"Robot? I'm a synthetic humanoid, thank you very much.";
		"Anybody got an oil can?";
		"Sometimes, I imagine I got ",liquid," runnin' in my hydraulics.";
		"I ~don't dance, unless it's on ",0,"'s head.";
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type
} //end chat Pi
